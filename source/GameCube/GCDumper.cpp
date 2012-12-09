/***************************************************************************
 * Copyright (C) 2012
 * by OverjoY and FIX94 for Wiiflow
 *
 * Adjustments for USB Loader GX by Dimok
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
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/statvfs.h>

#include "GCDumper.hpp"
#include "FileOperations/fileops.h"
#include "language/gettext.h"
#include "prompts/ProgressWindow.h"
#include "usbloader/disc.h"
#include "usbloader/wdvd.h"
#include "usbloader/wbfs/wbfs_fat.h"
#include "usbloader/wbfs/wbfs_rw.h"
#include "utils/ShowError.h"
#include "utils/tools.h"
#include "gecko.h"

static const u32 BUF_SIZE = (64*1024);

GCDumper::GCDumper()
	: force_align32(false)
	, compressed(false)
	, ReadBuffer((u8 *) memalign(32, ALIGN32(BUF_SIZE)))
{

}
GCDumper::~GCDumper()
{
	if(ReadBuffer)
		free(ReadBuffer);
}

s32 GCDumper::CopyDiscData(FILE *f, u64 offset, u32 length, u8 *buffer)
{
	u32 toread = 0;
	u32 wrote = 0;

	while(length)
	{
		if(ProgressCanceled())
			return PROGRESS_CANCELED;

		ShowProgress(discWrote, discTotal);

		toread = std::min(length, BUF_SIZE);
		s32 ret = __ReadDVDPlain(buffer, toread, offset);
		if (ret < 0)
			return ret;

		fwrite(buffer, 1, toread, f);
		wrote += toread;
		offset += toread;
		length -= toread;
		discWrote += toread;
	}
	return wrote;
}

s32 GCDumper::ReadDiscHeader(void)
{
	if(!ReadBuffer)
		return -1;

	s32 result = 0;

	struct discHdr *gcheader = (struct discHdr *) memalign(32, ALIGN32(sizeof(struct discHdr)));
	if(!gcheader)
		return -1;

	s32 ret = Disc_ReadHeader(gcheader);
	if(ret < 0) {
		free(gcheader);
		return ret;
	}

	if(memcmp(gcheader->id, "GCOPDV", 6) == 0)
	{
		while(result == 0)
		{
			__ReadDVDPlain(ReadBuffer, 0x10, 0x40+(gameOffsets.size()*4));
			u64 MultiGameOffset = ((u64)(*(u32*)ReadBuffer)) << 2ULL;
			if(!MultiGameOffset)
				break;

			ret = __ReadDVDPlain(gcheader, sizeof(struct discHdr), MultiGameOffset);
			if(ret < 0)
				result = -3;

			if(ReadDiscInfo(MultiGameOffset) < 0)
				result = -4;
			discHeaders.push_back(*gcheader);
			gameOffsets.push_back(MultiGameOffset);
		}
	}
	else
	{
		discHeaders.push_back(*gcheader);
		gameOffsets.push_back(0);

		if(ReadDiscInfo(0) < 0)
			result = -5;
	}

	free(gcheader);
	return result;
}

int GCDumper::ReadDiscInfo(const u64 &game_offset)
{
	if(!ReadBuffer)
		return -1;

	s32 ret = __ReadDVDPlain(ReadBuffer, 0x440, game_offset);
	if(ret < 0)
		return -2;

	u32 FSTOffset = *(u32*)(ReadBuffer+0x424);
	u32 FSTSize = *(u32*)(ReadBuffer+0x428);
	u32 GamePartOffset = *(u32*)(ReadBuffer+0x434);
	u32 DataSize = *(u32*)(ReadBuffer+0x438);
	u32 DiscSize =  DataSize + GamePartOffset;

	u32 installSize = 0;

	if(!compressed)
	{
		installSize += DiscSize;
	}
	else
	{
		u8 *FSTBuffer = (u8 *)memalign(32, ALIGN32(FSTSize));

		ret = __ReadDVDPlain(FSTBuffer, ALIGN32(FSTSize), game_offset+FSTOffset);
		if(ret < 0)
		{
			free(FSTBuffer);
			return -3;
		}

		u8 *FSTable = (u8*)FSTBuffer;
		u32 FSTEnt = *(u32*)(FSTable+0x08);
		FST *fst = (FST *)(FSTable);

		installSize += (FSTOffset + FSTSize);

		u32 i;
		u32 correction;
		u32 align;

		for( i=1; i < FSTEnt; ++i )
		{
			if( fst[i].Type ) {
				continue;
			}
			else
			{
				for(align = 0x8000; align > 2; align/=2)
				{
					if((fst[i].FileOffset & (align-1)) == 0 || force_align32)
					{
						correction = 0;
						while(((installSize+correction) & (align-1)) != 0)
							correction++;
						installSize += correction;
						break;
					}
				}
				installSize += fst[i].FileLength;
			}
		}
		free(FSTBuffer);
	}

	gameSizes.push_back(installSize);
	return 0;
}

s32 GCDumper::InstallGame(const char *installpath, u32 game, const char *installedGamePath)
{
	if(!ReadBuffer || game >= discHeaders.size() || game >= gameOffsets.size() || game >= gameSizes.size())
		return -1;

	const u64 &game_offset = gameOffsets[game];
	const struct discHdr &gcheader = discHeaders[game];

	discWrote = 0;
	discTotal = gameSizes[game];

	//! check for enough free space
	{
		struct statvfs sd_vfs;
		if(statvfs(installpath, &sd_vfs) != 0)
		{
			ShowError(tr("Could not get free device space for game."));
			return -102;
		}

		if(((u64)sd_vfs.f_frsize * (u64)sd_vfs.f_bfree) < discTotal)
		{
			ShowError(tr("Not enough free space on device."));
			return -103;
		}
	}

	s32 ret = __ReadDVDPlain(ReadBuffer, 0x440, game_offset);
	if(ret < 0) {
		ShowError(tr("Disc read error."));
		return -2;
	}

	u32 Disc = *(u8*)(ReadBuffer+0x06);
	u32 ApploaderSize = *(u32*)(ReadBuffer+0x400);
	u32 DOLOffset = *(u32*)(ReadBuffer+0x420);
	u32 FSTOffset = *(u32*)(ReadBuffer+0x424);
	u32 FSTSize = *(u32*)(ReadBuffer+0x428);
	u32 GamePartOffset = *(u32*)(ReadBuffer+0x434);
	u32 DataSize = *(u32*)(ReadBuffer+0x438);
	u32 DOLSize = FSTOffset - DOLOffset;
	u32 DiscSize = DataSize + GamePartOffset;

	u8 *FSTBuffer = (u8 *)memalign(32, ALIGN32(FSTSize));
	if(!FSTBuffer) {
		ShowError(tr("Not enough memory for FST."));
		return -3;
	}

	ret = __ReadDVDPlain(FSTBuffer, ALIGN32(FSTSize), game_offset+FSTOffset);
	if(ret < 0)
	{
		free(FSTBuffer);
		ShowError(tr("Disc read error."));
		return -3;
	}

	char gametitle[65];
	snprintf(gametitle, sizeof(gametitle), "%s", gcheader.title);
	Wbfs_Fat::CleanTitleCharacters(gametitle);

	char gamepath[512];
	// snprintf(gamepath, sizeof(gamepath), "%s%s [%.6s]%s/", installpath, gametitle, gcheader.id, Disc ? "2" : ""); // Disc2 currently needs to be on the same folder.
	snprintf(gamepath, sizeof(gamepath), "%s%s [%.6s]/", installpath, gametitle, gcheader.id);

	// If another Disc from the same gameID already exists, let's use that path
	if(strlen((char *)installedGamePath) != 0)
		snprintf(gamepath, sizeof(gamepath), "%s/", installedGamePath);

	CreateSubfolder(gamepath);

	// snprintf(gamepath, sizeof(gamepath), "%s%s [%.6s]%s/game.iso", installpath, gametitle, gcheader.id, Disc ? "2" : ""); // Disc2 currently needs to be on the same folder.
	snprintf(gamepath, sizeof(gamepath), "%s%s [%.6s]/%s.iso", installpath, gametitle, gcheader.id, Disc ? "disc2" : "game");

	FILE *f = fopen(gamepath, "wb");
	if(!f)
	{
		free(FSTBuffer);
		ShowError(tr("Can't open file for write: %s"), gamepath);
		return -4;
	}

	u8 *FSTable = (u8*)FSTBuffer;
	u32 FSTEnt = *(u32*)(FSTable+0x08);

	FST *fst = (FST *)(FSTable);

	gprintf("Dumping: %s %s\n", gcheader.title, compressed ? "compressed" : "full");

	gprintf("Apploader size : %d\n", ApploaderSize);
	gprintf("DOL offset	 : 0x%08x\n", DOLOffset);
	gprintf("DOL size	   : %d\n", DOLSize);
	gprintf("FST offset	 : 0x%08x\n", FSTOffset);
	gprintf("FST size	   : %d\n", FSTSize);
	gprintf("Num FST entries: %d\n", FSTEnt);
	gprintf("Data Offset	: 0x%08x\n", FSTOffset+FSTSize);
	gprintf("Disc size	  : %d\n", DiscSize);
	if(compressed)
		gprintf("Compressed size: %d\n", discTotal);


	gprintf("Writing %s\n", gamepath);

	s32 result = 0;

	ProgressCancelEnable(true);
	StartProgress(tr("Installing Game Cube Game..."), gcheader.title, 0, true, true);

	if(compressed)
	{
		u32 align;
		u32 correction;
		u32 toread;
		u32 wrote = 0;

		ret = CopyDiscData(f, game_offset, (FSTOffset + FSTSize), ReadBuffer);
		if(ret < 0)
			result = -3;

		wrote += (FSTOffset + FSTSize);

		for(u32 i = 1; (result == 0) && (i < FSTEnt); ++i)
		{
			if(ProgressCanceled()) {
				result = PROGRESS_CANCELED;
				break;
			}

			if( fst[i].Type ) {
				continue;
			}
			else
			{
				for(align = 0x8000; align > 2; align/=2)
				{
					if((fst[i].FileOffset & (align-1)) == 0 || force_align32)
					{
						correction = 0;
						while(((wrote+correction) & (align-1)) != 0)
							correction++;

						wrote += correction;
						while(correction)
						{
							toread = std::min(correction, BUF_SIZE);
							memset(ReadBuffer, 0, toread);
							fwrite(ReadBuffer, 1, toread, f);
							correction -= toread;
						}
						break;
					}
				}
				ret = CopyDiscData(f, game_offset+fst[i].FileOffset, fst[i].FileLength, ReadBuffer);
				if(ret < 0) {
					result = -2;
					break;
				}

				fst[i].FileOffset = wrote;
				wrote += ret;
			}
		}

		fseek(f, FSTOffset, SEEK_SET);
		fwrite(fst, 1, FSTSize, f);

		gprintf("Done!! Disc old size: %d, disc new size: %d, saved: %d\n", DiscSize, wrote, DiscSize - wrote);
	}
	else
	{
		ret = CopyDiscData(f, game_offset, discTotal, ReadBuffer);
		if( ret < 0 )
			result = -2;
		else
			gprintf("Done!! Disc size: %d\n", DiscSize);
	}

	// Stop progress
	ProgressStop();
	ProgressCancelEnable(false);

	free(FSTBuffer);
	fclose(f);

	if(result < 0)
	{
		RemoveFile(gamepath);
		if(strlen((char *)installedGamePath) == 0) // If no other disc is installed in that folder, delete it.
		{
			char *pathPtr = strrchr(gamepath, '/');
			if(pathPtr) *pathPtr = 0;
			RemoveFile(gamepath);
		}

		if(result != PROGRESS_CANCELED)
			ShowError(tr("Disc read error."));
	}

	return result;
}
