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
#include "usbloader/nand.h"
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

void CreateSavePath(const struct discHdr *hdr)
{
	char contentPath[512];
	char dataPath[512];
	snprintf(contentPath, sizeof(contentPath), "%s/title/00010000/%02x%02x%02x%02x/content", Settings.NandEmuPath, hdr->id[0], hdr->id[1], hdr->id[2], hdr->id[3]);
	snprintf(dataPath, sizeof(dataPath), "%s/title/00010000/%02x%02x%02x%02x/data", Settings.NandEmuPath, hdr->id[0], hdr->id[1], hdr->id[2], hdr->id[3]);

	if(CheckFile(contentPath) && CheckFile(dataPath))
		return;

	gprintf("Creating Save Path: %s\n", contentPath);
	gprintf("Creating Save Path: %s\n", dataPath);

	CreateSubfolder(contentPath);
	CreateSubfolder(dataPath);

	strcat(contentPath, "/title.tmd");
	CreateTitleTMD(contentPath, hdr);
}
