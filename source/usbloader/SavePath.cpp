/****************************************************************************
 * Copyright (C) 2011
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <gctypes.h>
#include <stdio.h>
#include "settings/CSettings.h"
#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "FileOperations/fileops.h"
#include "gecko.h"

void CreateTitleTMD(const char *path, const struct discHdr *hdr)
{
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) hdr->id);
	if (!disc)
		return;

	wiidisc_t *wdisc = wd_open_disc((int(*)(void *, u32, u32, void *)) wbfs_disc_read, disc);
	if (!wdisc)
	{
		WBFS_CloseDisc(disc);
		return;
	}

	u8 *titleTMD = wd_extract_file(wdisc, ONLY_GAME_PARTITION, (char *) "TMD");
	int tmd_size = wdisc->extracted_size;

	wd_close_disc(wdisc);
	WBFS_CloseDisc(disc);

	if(!titleTMD)
		return;

	FILE *f = fopen(path, "wb");
	if(f)
	{
		fwrite(titleTMD, 1, tmd_size, f);
		fclose(f);
		gprintf("Written Game Title TDM to: %s\n", path);
	}

	free(titleTMD);
}

static void CreateNandPath(const char *path)
{
	if(CheckFile(path))
		return;

	gprintf("Creating Nand Path: %s\n", path);
	CreateSubfolder(path);
}

void CreateSavePath(const struct discHdr *hdr)
{
	char nandPath[512];

	snprintf(nandPath, sizeof(nandPath), "%s/import", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/meta", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/shared1", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/shared2", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/sys", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/ticket", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/tmp", Settings.NandEmuPath);
	CreateNandPath(nandPath);

	const char *titlePath = "title/00010000";

	if(	memcmp(hdr->id, "RGWX41", 6) == 0 || memcmp(hdr->id, "RGWP41", 6) == 0 ||
		memcmp(hdr->id, "RGWJ41", 6) == 0 || memcmp(hdr->id, "RGWE41", 6) == 0)
	{
		titlePath = "title/00010004";
	}

	if(hdr->type == TYPE_GAME_NANDCHAN || hdr->type == TYPE_GAME_EMUNANDCHAN)
		titlePath = "title/00010001";

	snprintf(nandPath, sizeof(nandPath), "%s/%s/%02x%02x%02x%02x/data", Settings.NandEmuPath, titlePath, hdr->id[0], hdr->id[1], hdr->id[2], hdr->id[3]);
	CreateNandPath(nandPath);

	snprintf(nandPath, sizeof(nandPath), "%s/%s/%02x%02x%02x%02x/content", Settings.NandEmuPath, titlePath, hdr->id[0], hdr->id[1], hdr->id[2], hdr->id[3]);
	CreateNandPath(nandPath);

	strcat(nandPath, "/title.tmd");
	if(!CheckFile(nandPath))
		CreateTitleTMD(nandPath, hdr);
}
