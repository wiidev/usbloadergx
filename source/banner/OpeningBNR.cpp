/*
Copyright (c) 2012 - Dimok

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include <malloc.h>
#include <stdio.h>
#include "Channels/channels.h"
#include "GameCube/GCGames.h"
#include "libs/libwbfs/gcdisc.h"
#include "FileOperations/fileops.h"
#include "language/gettext.h"
#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "usbloader/wdvd.h"
#include "usbloader/wbfs/wbfs_rw.h"
#include "utils/uncompress.h"
#include "themes/CTheme.h"
#include "settings/GameTitles.h"
#include "wstring.hpp"
#include "OpeningBNR.hpp"

BNRInstance * BNRInstance::instance = NULL;

OpeningBNR::OpeningBNR()
	: imetHdr(0), filesize(0)
{
	memset(gameID, 0, sizeof(gameID));
}

OpeningBNR::~OpeningBNR()
{
	if(imetHdr)
		free(imetHdr);
}

bool OpeningBNR::LoadCachedBNR(const char *id)
{
	char path[255];
	snprintf(path, sizeof(path), "%s%.6s.bnr", Settings.BNRCachePath, id);
	if((filesize = FileSize(path)) == 0)
	{
		snprintf(path, sizeof(path), "%s%.3s.bnr", Settings.BNRCachePath, id);
		if((filesize = FileSize(path)) == 0)
			return false;
	}

	FILE *f = fopen(path, "rb");
	if(!f)
		return false;

	imetHdr = (IMETHeader *) malloc(filesize);
	if(!imetHdr)
	{
		fclose(f);
		return false;
	}

	fread(imetHdr, 1, filesize, f);
	fclose(f);

	if (imetHdr->fcc != 'IMET')
	{
		//! check if it's a channel .app file
		IMETHeader *channelImet = (IMETHeader *)(((u8 *)imetHdr) + 0x40);
		if(channelImet->fcc != 'IMET')
		{
			free(imetHdr);
			imetHdr = NULL;
			return false;
		}
		//! just move it 0x40 bytes back, the rest 0x40 bytes will just be unused
		//! as it's a temporary file usually it's not worth to reallocate
		filesize -= 0x40;
		memcpy(imetHdr, channelImet, filesize);
	}

	return true;
}

void OpeningBNR::WriteCachedBNR(const char *id, const u8 *buffer, u32 size)
{
	char path[255];
	snprintf(path, sizeof(path), "%s%.6s.bnr", Settings.BNRCachePath, id);

	CreateSubfolder(Settings.BNRCachePath);

	FILE *f = fopen(path, "wb");
	if(!f)
		return;

	fwrite(buffer, 1, size, f);
	fclose(f);
}

bool OpeningBNR::Load(const discHdr * header)
{
	if(!header)
		return false;

	if(memcmp(gameID, header->id, 6) == 0)
		return true;

	memcpy(gameID, header->id, 6);

	if(imetHdr)
		free(imetHdr);
	imetHdr = NULL;

	switch(header->type)
	{
	case TYPE_GAME_WII_IMG:
	case TYPE_GAME_WII_DISC:
		return LoadWiiBanner(header);
	case TYPE_GAME_NANDCHAN:
	case TYPE_GAME_EMUNANDCHAN:
		return LoadChannelBanner(header);
	case TYPE_GAME_GC_IMG:
	case TYPE_GAME_GC_DISC:
	case TYPE_GAME_GC_EXTRACTED:
		if(!Settings.CacheBNRFiles)
			return false;
		return LoadCachedBNR((char *)header->id);
	default:
		break;
	}

	return false;
}

bool OpeningBNR::LoadWiiBanner(const discHdr * header)
{
	if(!header || (   (header->type != TYPE_GAME_WII_IMG)
				   && (header->type != TYPE_GAME_WII_DISC)))
		return false;


	if(Settings.CacheBNRFiles && LoadCachedBNR((const char *)header->id))
		return true;

	if(header->type == TYPE_GAME_WII_DISC)
	{
		wiidisc_t *wdisc = wd_open_disc(__ReadDVD, 0);
		if (!wdisc)
			return false;

		imetHdr = (IMETHeader*) wd_extract_file(wdisc, ALL_PARTITIONS, (char *) "opening.bnr");

		filesize = wdisc->extracted_size;

		wd_close_disc(wdisc);
	}
	else
	{
		wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) gameID);
		if (!disc)
			return false;

		wiidisc_t *wdisc = wd_open_disc((s32(*)(void *, u32, u32, void *)) wbfs_disc_read, disc);
		if (!wdisc)
		{
			WBFS_CloseDisc(disc);
			return false;
		}

		imetHdr = (IMETHeader*) wd_extract_file(wdisc, ALL_PARTITIONS, (char *) "opening.bnr");

		filesize = wdisc->extracted_size;

		wd_close_disc(wdisc);
		WBFS_CloseDisc(disc);
	}

	if(!imetHdr)
		return false;

	if (imetHdr->fcc != 'IMET')
	{
		free(imetHdr);
		imetHdr = NULL;
		return false;
	}

	if(Settings.CacheBNRFiles)
		WriteCachedBNR((const char *) header->id, (u8 *) imetHdr, filesize);

	return true;
}

bool OpeningBNR::LoadChannelBanner(const discHdr *header)
{
	if(!header || (header->tid == 0) || (   (header->type != TYPE_GAME_NANDCHAN)
										 && (header->type != TYPE_GAME_EMUNANDCHAN)))
		return false;

	if(Settings.CacheBNRFiles && LoadCachedBNR((char *) header->id))
		return true;

	const u64 &tid = header->tid;
	const char *pathPrefix = (header->type == TYPE_GAME_EMUNANDCHAN) ? Settings.NandEmuChanPath : "";

	imetHdr = (IMETHeader*) Channels::GetOpeningBnr(tid, &filesize, pathPrefix);
	if(!imetHdr)
		return false;

	if (imetHdr->fcc != 'IMET')
	{
		free(imetHdr);
		imetHdr = NULL;
		return false;
	}

	if(Settings.CacheBNRFiles)
		WriteCachedBNR((char *) header->id, (u8 *) imetHdr, filesize);

	return true;
}

const u16 * OpeningBNR::GetIMETTitle(int lang)
{
	if(!imetHdr || lang < 0 || lang >= 10)
		return NULL;

	if (imetHdr->fcc != 'IMET')
		return NULL;

	if(imetHdr->names[lang][0] == 0)
		lang = CONF_LANG_ENGLISH;

	return imetHdr->names[lang];
}

static s32 GC_Disc_Read(void *fp, u32 offset, u32 count, void*iobuf)
{
	if(fp)
	{
		fseek((FILE *)fp, offset, SEEK_SET);
		return fread(iobuf, 1, count, (FILE *)fp);
	}

	return __ReadDVDPlain(iobuf, count, offset);
}

u8 *OpeningBNR::LoadGCBNR(const discHdr * header, u32 *len)
{
	if(!header || (   (header->type != TYPE_GAME_GC_IMG)
				   && (header->type != TYPE_GAME_GC_DISC)
				   && (header->type != TYPE_GAME_GC_EXTRACTED)))
		return NULL;

	const char *path = GCGames::Instance()->GetPath((char *) header->id);
	if(!path)
		return NULL;

	FILE *file = NULL;
	GC_OpeningBnr *openingBnr = NULL;

	// read from file
	if((header->type == TYPE_GAME_GC_IMG) || (header->type == TYPE_GAME_GC_DISC))
	{
		//! open iso file if it's iso
		if(header->type == TYPE_GAME_GC_IMG)
		{
			file = fopen(path, "rb");
			if(!file)
				return NULL;
		}

		gcdisc_t *disc = gc_open_disc(GC_Disc_Read, file);
		if(!disc) {
			fclose(file);
			return NULL;
		}

		if(!strcmp(Settings.db_language, "JA")) {
			bool loaded = gc_extract_file(disc, "openingJA.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "opening.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "openingUS.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "openingEU.bnr");
		}
		else if(!strcmp(Settings.db_language, "EN")) {
			bool loaded = gc_extract_file(disc, "openingUS.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "opening.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "openingEU.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "openingJA.bnr");
		}
		else {
			bool loaded = gc_extract_file(disc, "openingEU.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "opening.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "openingUS.bnr");
			if(!loaded)
				loaded = gc_extract_file(disc, "openingJA.bnr");
		}

		openingBnr = (GC_OpeningBnr *) disc->extracted_buffer;
		if(len)
			*len = disc->extracted_size;

		gc_close_disc(disc);
	}
	else if(header->type == TYPE_GAME_GC_EXTRACTED)
	{
		string gamePath = path;
		gamePath += "root/";
		//! open default file first
		file = fopen((gamePath + "opening.bnr").c_str(), "rb");

		// if not found try the region specific ones
		if(!strcmp(Settings.db_language, "JA")) {
			if(!file)
				file = fopen((gamePath + "openingJA.bnr").c_str(), "rb");
			if(!file)
				file = fopen((gamePath + "openingUS.bnr").c_str(), "rb");
			if(!file)
				file = fopen((gamePath + "openingEU.bnr").c_str(), "rb");
		}
		else if(!strcmp(Settings.db_language, "EN")) {
			if(!file)
				file = fopen((gamePath + "openingUS.bnr").c_str(), "rb");
			if(!file)
				file = fopen((gamePath + "openingEU.bnr").c_str(), "rb");
			if(!file)
				file = fopen((gamePath + "openingJA.bnr").c_str(), "rb");
		}
		else {
			if(!file)
				file = fopen((gamePath + "openingEU.bnr").c_str(), "rb");
			if(!file)
				file = fopen((gamePath + "openingUS.bnr").c_str(), "rb");
			if(!file)
				file = fopen((gamePath + "openingJA.bnr").c_str(), "rb");
		}

		// file not found
		if(!file)
			return NULL;

		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		rewind(file);

		openingBnr = (GC_OpeningBnr *) malloc(size);
		if(openingBnr)
		{
			if(len)
				*len = size;
			fread(openingBnr, 1, size, file);
		}

	}

	if(file)
		fclose(file);

	if(!openingBnr)
		return NULL;

	// check magic of the opening bnr
	if(openingBnr->magic != 'BNR1' && openingBnr->magic != 'BNR2') {
		free(openingBnr);
		return NULL;
	}

	return (u8 *) openingBnr;
}

CustomBanner *OpeningBNR::CreateGCBanner(const discHdr * header)
{
	int language = 0;
	u32 openingBnrSize;
	GC_OpeningBnr *openingBnr = (GC_OpeningBnr *) LoadGCBNR(header, &openingBnrSize);

	CustomBanner *banner = new CustomBanner;
	banner->LoadBanner(Resources::GetFile("custom_banner.bnr"), Resources::GetFileSize("custom_banner.bnr"));
	banner->SetBannerPngImage("bg.tpl", Resources::GetFile("gc_banner_bg.png"), Resources::GetFileSize("gc_banner_bg.png"));

	banner->SetBannerText("T_PF", tr("GameCube"));

	if(openingBnr)
	{
		banner->SetBannerTexture("HBPic.tpl", openingBnr->tpl_data, 96, 32, GX_TF_RGB5A3);

		// European opening bnr file
		if(openingBnr->magic == 'BNR2')
		{
			if(!strcmp(Settings.db_language, "DE")) {
				language = 1;
			}
			else if(!strcmp(Settings.db_language, "FR")) {
				language = 2;
			}
			else if(!strcmp(Settings.db_language, "ES")) {
				language = 3;
			}
			else if(!strcmp(Settings.db_language, "IT")) {
				language = 4;
			}
			else if(!strcmp(Settings.db_language, "NL")) {
				language = 5;
			}

			if((0x1820 + sizeof(openingBnr->description[0]) * language) > openingBnrSize) {
				language = 0;
			}
		}

		wString str;
		str.resize(strlen((char *) openingBnr->description[language].developer));
		for(u32 i = 0; i < str.size(); i++)
			str[i] = *(openingBnr->description[language].developer + i);

		banner->SetBannerText("T_Coded_by", tr("Developer:"));
		banner->SetBannerText("T_coder", str.toUTF8().c_str());

		str.resize(strlen((char *) openingBnr->description[language].long_description));
		for(u32 i = 0; i < str.size(); i++)
			str[i] = *(openingBnr->description[language].long_description + i);

		banner->SetBannerText("T_short_descript", str.toUTF8().c_str());

		// free buffer
		free(openingBnr);
	}
	else
	{
		banner->SetBannerPngImage("HBPic.tpl", Resources::GetFile("gc_icon_bg.png"), Resources::GetFileSize("gc_icon_bg.png"));
		banner->SetBannerText("T_Coded_by", tr("Developer:"));
		banner->SetBannerText("T_coder", tr("Unknown"));
		banner->SetBannerText("T_short_descript", " ");
	}

	banner->SetBannerText("T_name", GameTitles.GetTitle(header));
	banner->SetBannerPaneVisible("Line1", false);
	banner->SetBannerPaneVisible("Line2", false);
	banner->SetBannerPaneVisible("T_Released", false);
	banner->SetBannerPaneVisible("T_release_date", false);
	banner->SetBannerPaneVisible("T_versiontext", false);
	banner->SetBannerPaneVisible("T_version", false);
	banner->SetBannerTextureScale(Settings.GCBannerScale);

	return banner;
}

CustomBanner *OpeningBNR::CreateGCIcon(const discHdr * header)
{
	GC_OpeningBnr *openingBnr = (GC_OpeningBnr *) LoadGCBNR(header);

	CustomBanner *newBanner = new CustomBanner;
	newBanner->LoadIcon(Resources::GetFile("custom_banner.bnr"), Resources::GetFileSize("custom_banner.bnr"));

	if(openingBnr)
		newBanner->SetIconTexture("Iconpng.tpl", openingBnr->tpl_data, 96, 32, GX_TF_RGB5A3);
	else
		newBanner->SetIconPngImage("Iconpng.tpl", Resources::GetFile("gc_icon_bg.png"), Resources::GetFileSize("gc_icon_bg.png"));

	newBanner->SetIconPngImage("HBLogo.tpl", Resources::GetFile("gc_icon_bg.png"), Resources::GetFileSize("gc_icon_bg.png"));

	// free buffer
	if(openingBnr)
		free(openingBnr);

	return newBanner;
}
