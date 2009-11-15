/****************************************************************************
 * USB Loader GX Team
 * banner.c
 * 
 * Dump opening.bnr thanks to Wiipower
 ***************************************************************************/

#include <gctypes.h>
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <fat.h>

#include "fatmounter.h"
#include "usbloader/wdvd.h"
#include "usbloader/disc.h"
#include "banner.h"
#include "patches/fst.h"
#include "usbloader/fstfile.h"

s32 dump_banner(const u8* discid,const char * dest)
{
	// Mount the disc
	//Disc_SetWBFS(1, (u8*)discid);
	Disc_SetUSB(discid);

	Disc_Open();

	u64 offset;
	s32 ret;

	ret = __Disc_FindPartition(&offset);
	if (ret < 0)
		return ret;

		ret = WDVD_OpenPartition(offset);

	if (ret < 0) {
		//printf("ERROR: OpenPartition(0x%llx) %d\n", offset, ret);
		return ret;
	}

	// Read where to find the fst.bin
	u32 *buffer = memalign(32, 0x20);

	if (buffer == NULL)
	{
		//Out of memory
		return -1;
	}

	ret = WDVD_Read(buffer, 0x20, 0x420);
	if (ret < 0)
		return ret;

	// Read fst.bin
	void *fstbuffer = memalign(32, buffer[2]*4);
	FST_ENTRY *fst = (FST_ENTRY *)fstbuffer;

	if (fst == NULL)
	{
		//Out of memory
		free(buffer);
		return -1;
	}

	ret = WDVD_Read(fstbuffer, buffer[2]*4, buffer[1]*4);
	if (ret < 0)
		return ret;

	free(buffer);

	// Search the fst.bin
	u32 count = fst[0].filelen;
	int i;
	u32 index = 0;

	for (i=1;i<count;i++)
	{
		if (strstr(fstfiles(fst, i), "opening.bnr") != NULL)
		{
			index = i;
		}
	}

	if (index == 0)
	{
		//opening.bnr not found
		free(fstbuffer);
		return -1;
	}

	// Load the .bnr
	u8 *banner = memalign(32, fst[index].filelen);

	if (banner == NULL)
	{
		//Out of memory
		free(fstbuffer);
		return -1;
	}

	ret = WDVD_Read((void *)banner, fst[index].filelen, fst[index].fileoffset * 4);
	if (ret < 0)
		return ret;

    WDVD_Reset();
    WDVD_ClosePartition();
	//fatInitDefault();
	//SDCard_Init();
	WDVD_SetUSBMode(NULL, 0);
	FILE *fp = fopen(dest, "wb");
	if(fp)
	{
		fwrite(banner, 1, fst[index].filelen, fp);
		fclose(fp);
	}
	free(fstbuffer);
	free(banner);

	return 1;
}

