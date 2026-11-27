/* On-Wii (or Dolphin) test of source/channel: for every sd:/fwdtest/<GAMEID>.bnr build the
 * channel with fwdchannel.c, install it via ES with fwdinstall.c, also save the WAD next to
 * the banner, and log the results to sd:/fwdtest/result.txt. Then power off.
 * If sd:/fwdtest/delete exists, it deletes those channels with fwd_uninstall instead. */
#include <gccore.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "fwdchannel.h"
#include "fwdinstall.h"
#include "fwd_template_bin.h"
#include "fwd_forwarder_bin.h"

static const u8 common_key[16] = {0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4,
                                  0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7};

static FILE *logf;
#define LOG(...) do { printf(__VA_ARGS__); if (logf) { fprintf(logf, __VA_ARGS__); fflush(logf); } } while (0)

static u8 *slurp(const char *path, u32 *size)
{
	FILE *f = fopen(path, "rb");
	u8 *buf;
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	rewind(f);
	buf = malloc(*size);
	if (buf && fread(buf, 1, *size, f) != *size) { free(buf); buf = NULL; }
	fclose(f);
	return buf;
}

int main(int argc, char **argv)
{
	VIDEO_Init();
	GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
	void *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	ISFS_Initialize();
	if (!fatMountSimple("sd", &__io_wiisd))
	{
		printf("no SD\n");
		sleep(5);
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	}
	logf = fopen("sd:/fwdtest/result.txt", "w");
	LOG("fwdinstall_test, IOS %d\n", IOS_GetVersion());

	FILE *mode = fopen("sd:/fwdtest/delete", "r");
	int delete_mode = mode != NULL;
	if (mode) fclose(mode);
	{
		u32 n = 0;
		ES_GetNumTitles(&n);
		LOG("mode: %s, titles in NAND: %u\n", delete_mode ? "delete" : "install", n);
	}

	/* which 00010001 channels count as our shortcuts (the game list hides those) */
	{
		u32 n = 0, k;
		if (ES_GetNumTitles(&n) >= 0 && n)
		{
			u64 *t = memalign(32, (n * 8 + 31) & ~31);
			if (t && ES_GetTitles(t, n) >= 0)
				for (k = 0; k < n; k++)
					if ((t[k] >> 32) == 0x00010001)
					{
						u8 id[8];
						int b;
						for (b = 0; b < 8; b++) id[b] = (u8)(t[k] >> (56 - 8 * b));
						LOG("channel %.4s: shortcut %d\n", (char *)id + 4, fwd_is_forwarder_channel(id));
					}
			free(t);
		}
	}

	DIR *dir = opendir("sd:/fwdtest");
	struct dirent *de;
	while (dir && (de = readdir(dir)) != NULL)
	{
		char game[7], title[5], path[256];
		u32 size = 0;
		u8 *bnr;
		fwd_channel ch;
		int r, step = 0;
		if (strlen(de->d_name) != 10 || strcmp(de->d_name + 6, ".bnr") != 0)
			continue;
		memcpy(game, de->d_name, 6);
		game[6] = 0;
		snprintf(path, sizeof(path), "sd:/fwdtest/%s", de->d_name);
		bnr = slurp(path, &size);
		if (!bnr) { LOG("%s: cannot read banner\n", game); continue; }
		if (fwd_default_title_id(game, title) < 0) { LOG("%s: no title ID rule\n", game); free(bnr); continue; }
		if (delete_mode)
		{
			u8 tid[8] = {0, 1, 0, 1, (u8)title[0], (u8)title[1], (u8)title[2], (u8)title[3]};
			u32 n = 0;
			free(bnr);
			LOG("%s: 00010001-%s installed: %d, ours: %d\n", game, title, fwd_title_installed(tid), fwd_is_forwarder_channel(tid));
			r = fwd_uninstall(tid);
			ES_GetNumTitles(&n);
			LOG("%s: uninstall -> %d, installed now: %d, titles in NAND: %u\n", game, r, fwd_title_installed(tid), n);
			continue;
		}
		{
			u8 *nl = NULL; u32 nls = 0;
			int nr = fwd_read_nandloader(&nl, &nls);
			LOG("%s: nandloader from UNEO -> %d, %u bytes\n", game, nr, nls);
			r = fwd_build(&ch, fwd_template_bin, nl, nls, fwd_forwarder_bin, fwd_forwarder_bin_size, bnr, size,
			              game, title, "/apps/usbloader_gx/boot.dol", common_key);
			free(nl);
		}
		free(bnr);
		if (r < 0) { LOG("%s: build failed: %s\n", game, fwd_error_text(r)); continue; }
		LOG("%s: built 00010001-%s, banner %s, existed before: %d\n", game, title,
		    ch.banner_fixed ? "fixed" : "as is", fwd_title_installed(ch.title_id));
		{
			u8 *wad; u32 wn;
			if (fwd_make_wad(&ch, &wad, &wn) == 0)
			{
				snprintf(path, sizeof(path), "sd:/fwdtest/%s.wad", game);
				FILE *f = fopen(path, "wb");
				if (f) { fwrite(wad, 1, wn, f); fclose(f); }
				free(wad);
				LOG("%s: WAD saved, %u bytes\n", game, wn);
			}
		}
		r = fwd_install(&ch, NULL, &step);
		LOG("%s: install -> %d at %s, installed now: %d\n", game, r, fwd_step_text(step), fwd_title_installed(ch.title_id));
		fwd_free(&ch);
	}
	if (dir) closedir(dir);
	LOG("end\n");
	if (logf) fclose(logf);
	fatUnmount("sd:/");
	sleep(2);
	SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	return 0;
}
