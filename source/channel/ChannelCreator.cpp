/****************************************************************************
 * "Create Wii Menu Channel" for a Wii game: builds a fakesigned channel with the
 * game's opening.bnr and a forwarder DOL that starts this loader with the game ID
 * (StartUpProcess::ParseArguments -> QuickGameBoot), then installs it via ES.
 * If installing fails, the channel can be saved as a WAD for a WAD manager.
 ***************************************************************************/
#include <gccore.h>
#include <ogc/es.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "ChannelCreator.h"
#include "fwdchannel.h"
#include "fwdinstall.h"
#include "fwd_template_bin.h"
#include "fwd_forwarder_bin.h"
#include "fwd_gc_logo_bin.h"
#include "fwd_gc_bg_bin.h"
#include "custom_banner_bnr.h"
#include "utils/sjis.h"
#include "wstring.hpp"
#include "banner/OpeningBNR.hpp"
#include "FileOperations/fileops.h"
#include "language/gettext.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "gecko.h"
#include "libs/libruntimeiospatch/runtimeiospatch.h"
#include "utils/tools.h"

static const u8 wii_common_key[16] = {0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4,
                                      0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7};

static bool IsWiiGame(const struct discHdr *h)
{
	return h->type == TYPE_GAME_WII_IMG || h->type == TYPE_GAME_WII_DISC;
}

static bool IsGCGame(const struct discHdr *h)
{
	return h->type == TYPE_GAME_GC_IMG || h->type == TYPE_GAME_GC_EXTRACTED;
}

//! UTF-16 for the banner, from UTF-8
static std::vector<u16> ToUtf16(const std::string &utf8)
{
	wString w;
	w.fromUTF8(utf8.c_str());
	std::vector<u16> out;
	for (size_t i = 0; i < w.size(); i++)
		if (w[i] < 0x10000)
			out.push_back((u16)w[i]);
	out.push_back(0);
	return out;
}

//! A text field of a GameCube opening.bnr (first language), whitespace trimmed
static std::string GCText(const u8 *bnr, u32 off, u32 n, bool sjis)
{
	std::string s((const char *)bnr + 0x1820 + off, strnlen((const char *)bnr + 0x1820 + off, n));
	const char *ws = " \t\n\r\v\f\x1c\x1d\x1e\x1f\x85\xa0";
	size_t a = s.find_first_not_of(ws), b = s.find_last_not_of(ws);
	s = (a == std::string::npos) ? std::string() : s.substr(a, b - a + 1);
	if (sjis)
		return sj2utf8(s);
	std::string utf8; //! latin-1 -> UTF-8
	for (size_t i = 0; i < s.size(); i++)
	{
		unsigned char c = s[i];
		if (c < 0x80)
			utf8 += (char)c;
		else
		{
			utf8 += (char)(0xC0 | c >> 6);
			utf8 += (char)(0x80 | (c & 0x3F));
		}
	}
	return utf8;
}

//! Channel banner for a GameCube game from the loader's template
static u8 *LoadGCChannelBanner(const struct discHdr *header, u32 *size)
{
	OpeningBNR loader;
	u32 bnr_size = 0;
	u8 *gc = loader.LoadGCBNR(header, &bnr_size);
	if (!gc || bnr_size < 0x1820 + 0x140)
	{
		free(gc);
		return NULL;
	}
	bool sjis = header->id[3] == 'J' || header->id[3] == 'W';
	std::string dev = GCText(gc, 0x80, 0x40, sjis);
	if (dev.empty())
		dev = GCText(gc, 0x20, 0x20, sjis);
	std::vector<u16> title = ToUtf16(GameTitles.GetTitle(header)), developer = ToUtf16(dev),
	                 description = ToUtf16(GCText(gc, 0xC0, 0x80, sjis));
	u8 *out = NULL;
	int r = fwd_gc_banner(custom_banner_bnr, custom_banner_bnr_size, gc, bnr_size, &title[0], &developer[0], &description[0],
	                      fwd_gc_logo_bin, fwd_gc_logo_bin_size, fwd_gc_bg_bin, fwd_gc_bg_bin_size, &out, size);
	gprintf("ChannelCreator: GameCube banner -> %d, %u bytes\n", r, r ? 0 : *size);
	free(gc);
	return r ? NULL : out;
}

//! Banner: the cache file if there is one, else straight from the game image
static u8 *LoadBanner(const struct discHdr *header, u32 *size)
{
	char path[255];
	snprintf(path, sizeof(path), "%s%.6s.bnr", Settings.BNRCachePath, (const char *)header->id);
	u8 *buf = NULL;
	if (LoadFileToMem(path, &buf, size) >= 0 && buf && *size > 0x600 && memcmp(buf + 0x40, "IMET", 4) == 0)
		return buf;
	free(buf);

	OpeningBNR bnr;
	if (!bnr.LoadWiiBanner(header) || !bnr.Get() || bnr.GetSize() <= 0x600)
		return NULL;
	*size = bnr.GetSize();
	buf = (u8 *)malloc(*size);
	if (buf)
		memcpy(buf, bnr.Get(), *size);
	if (buf && memcmp(buf + 0x40, "IMET", 4) != 0)
	{
		free(buf);
		return NULL;
	}
	return buf;
}

//! Path of this loader relative to the device root, e.g. /apps/usbloader_gx/boot.dol
static void LoaderPath(char *out, int size)
{
	const char *p = strchr(Settings.ConfigPath, ':');
	if (p && strstr(p, "/apps/"))
		snprintf(out, size, "%sboot.dol", p + 1);
	else
		snprintf(out, size, "/apps/usbloader_gx/boot.dol");
}

static void Progress(u32 done, u32 total)
{
	ShowProgress(done, total);
}

static bool SaveWad(const fwd_channel *ch, const char *game_id, char *path, int size)
{
	u8 *wad = NULL;
	u32 wad_size = 0;
	if (fwd_make_wad(ch, &wad, &wad_size) < 0)
		return false;
	char dir[150];
	snprintf(dir, sizeof(dir), "%s/wad/", Settings.BootDevice);
	CreateSubfolder(dir);
	snprintf(path, size, "%s%s_channel.wad", dir, game_id);
	FILE *f = fopen(path, "wb");
	bool ok = f && fwrite(wad, 1, wad_size, f) == wad_size;
	if (f)
		fclose(f);
	free(wad);
	return ok;
}

void CreateGameChannel(const struct discHdr *header)
{
	if (!header || (!IsWiiGame(header) && !IsGCGame(header)))
		return;

	char game_id[7];
	snprintf(game_id, sizeof(game_id), "%.6s", (const char *)header->id);
	char title4[5];
	if (fwd_default_title_id(game_id, title4) < 0)
	{
		WindowPrompt(tr("Create Wii Menu Channel"), tr("This game ID is not supported."), tr("OK"));
		return;
	}
	u8 title_id[8] = {0x00, 0x01, 0x00, 0x01, (u8)title4[0], (u8)title4[1], (u8)title4[2], (u8)title4[3]};
	bool exists = fwd_title_installed(title_id);
	if (exists && !fwd_is_forwarder_channel(title_id))
	{
		WindowPrompt(tr("Create Wii Menu Channel"), tr("Another title already uses this channel ID."), tr("OK"));
		return;
	}

	char loader[128];
	LoaderPath(loader, sizeof(loader));
	char msg[512];
	//! WindowPrompt ignores line breaks, so the parts are separated by ". "
	snprintf(msg, sizeof(msg), "%s. %s: 00010001-%s. %s: %s.%s%s", GameTitles.GetTitle(header), tr("Channel"), title4,
	         tr("Starts"), loader, exists ? " " : "", exists ? tr("This channel exists and will be replaced.") : "");
	if (WindowPrompt(tr("Create Wii Menu Channel?"), msg, tr("Create"), tr("Cancel")) != 1)
		return;

	u32 bnr_size = 0;
	u8 *bnr = IsGCGame(header) ? LoadGCChannelBanner(header, &bnr_size) : LoadBanner(header, &bnr_size);
	if (!bnr)
	{
		WindowPrompt(tr("Create Wii Menu Channel"), tr("Could not load the game's banner."), tr("OK"));
		return;
	}

	u8 *nandloader = NULL;
	u32 nandloader_size = 0;
	int nl = fwd_read_nandloader(&nandloader, &nandloader_size);
	gprintf("ChannelCreator: nandloader from UNEO -> %d, %u bytes\n", nl, nandloader_size);
	if (nl < 0)
	{
		free(bnr);
		WindowPrompt(tr("Create Wii Menu Channel"), tr("Install the UNEO forwarder channel first: its nandloader is needed to start the channel from the Wii Menu."), tr("OK"));
		return;
	}

	fwd_channel ch;
	int ret = fwd_build(&ch, fwd_template_bin, nandloader, nandloader_size, fwd_forwarder_bin, fwd_forwarder_bin_size,
	                    bnr, bnr_size, game_id, title4, loader, wii_common_key);
	free(bnr);
	free(nandloader);
	if (ret < 0)
	{
		WindowPrompt(tr("Create Wii Menu Channel"), fwd_error_text(ret), tr("OK"));
		return;
	}

	//! d2x cIOS accepts fakesigned titles; with AHB access also patch the running IOS
	//! (trucha + ES_Identify) as WAD managers do. No-op if the patterns are not there.
	if (AHBPROT_DISABLED)
		gprintf("ChannelCreator: IosPatch_RUNTIME -> %d\n", IosPatch_RUNTIME(true, false, false, false, false));

	StartProgress(tr("Installing channel..."), GameTitles.GetTitle(header), 0, false, false);
	int step = 0;
	s32 es = fwd_install(&ch, Progress, &step);
	ProgressStop();
	gprintf("ChannelCreator: %s 00010001-%s -> %d at %s\n", game_id, title4, es, fwd_step_text(step));

	if (es >= 0)
	{
		WindowPrompt(tr("Channel created"), tr("It is on the Wii Menu now."), tr("OK"));
	}
	else
	{
		snprintf(msg, sizeof(msg), "%s (%s, %d). %s", tr("Installing failed"), fwd_step_text(step), es, tr("Save the channel as a WAD file to install it with a WAD manager?"));
		if (WindowPrompt(tr("Create Wii Menu Channel"), msg, tr("Save WAD"), tr("Cancel")) == 1)
		{
			char path[200];
			if (SaveWad(&ch, game_id, path, sizeof(path)))
				WindowPrompt(tr("WAD saved"), path, tr("OK"));
			else
				WindowPrompt(tr("Create Wii Menu Channel"), tr("Could not save the WAD."), tr("OK"));
		}
	}
	fwd_free(&ch);
}

static bool ChannelTitleId(const struct discHdr *header, char title4[5], u8 title_id[8])
{
	if (!header || (!IsWiiGame(header) && !IsGCGame(header)))
		return false;
	char game_id[7];
	snprintf(game_id, sizeof(game_id), "%.6s", (const char *)header->id);
	if (fwd_default_title_id(game_id, title4) < 0)
		return false;
	const u8 tid[8] = {0x00, 0x01, 0x00, 0x01, (u8)title4[0], (u8)title4[1], (u8)title4[2], (u8)title4[3]};
	memcpy(title_id, tid, 8);
	return true;
}

bool GameChannelInstalled(const struct discHdr *header)
{
	char title4[5];
	u8 title_id[8];
	return ChannelTitleId(header, title4, title_id) && fwd_is_forwarder_channel(title_id);
}

void RemoveGameChannel(const struct discHdr *header)
{
	char title4[5];
	u8 title_id[8];
	if (!ChannelTitleId(header, title4, title_id) || !fwd_is_forwarder_channel(title_id))
		return;
	char msg[256];
	snprintf(msg, sizeof(msg), "%s. %s: 00010001-%s. %s", GameTitles.GetTitle(header), tr("Channel"), title4,
	         tr("The game and its saves stay."));
	if (WindowPrompt(tr("Remove Wii Menu Channel?"), msg, tr("Remove"), tr("Cancel")) != 1)
		return;
	s32 ret = fwd_uninstall(title_id);
	gprintf("ChannelCreator: remove 00010001-%s -> %d\n", title4, ret);
	if (ret < 0)
	{
		snprintf(msg, sizeof(msg), "%s (%d)", tr("Removing failed"), ret);
		WindowPrompt(tr("Remove Wii Menu Channel"), msg, tr("OK"));
	}
	else
		WindowPrompt(tr("Channel removed"), 0, tr("OK"));
}

bool IsGameShortcutChannel(u64 tid)
{
	u8 title_id[8];
	for (int i = 0; i < 8; i++)
		title_id[i] = (u8)(tid >> (56 - 8 * i));
	return fwd_is_forwarder_channel(title_id);
}
