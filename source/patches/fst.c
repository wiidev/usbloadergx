/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <malloc.h>
#include <sys/unistd.h>
#include <sdcard/wiisd_io.h>
#include <ogc/ipc.h>

#include "settings/cfg.h"
#include "fst.h"
#include "dvd_broadway.h"
#include "wpad.h"
#include "fatmounter.h"
#include "sys.h"
#include "../gecko.h"
#include "mload/mload.h"
#include "mload/dip_plugin.h"

extern struct SSettings Settings;

// Pre-allocate the buffer size for ocarina codes
u8 filebuff[MAX_GCT_SIZE];

u32 do_sd_code(char *filename)
{
gprintf("\ndo_sd_code(%s)",filename);

	FILE *fp;
	u8 *filebuff;
	u32 filesize;
	u32 ret;
	char filepath[150];

    //SDCard_Init();
	//USBDevice_Init();

	sprintf(filepath, "%s%s", Settings.Cheatcodespath, filename);
	filepath[strlen(Settings.Cheatcodespath)+6] = 0x2E;
	filepath[strlen(Settings.Cheatcodespath)+7] = 0x67;
	filepath[strlen(Settings.Cheatcodespath)+8] = 0x63;
	filepath[strlen(Settings.Cheatcodespath)+9] = 0x74;
	filepath[strlen(Settings.Cheatcodespath)+10] = 0;

	fp = fopen(filepath, "rb");
	if (!fp) {
        USBDevice_deInit();
        SDCard_deInit();
		gprintf("\n\tcan't open %s",filepath);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	if(filesize <= 16){
		fclose(fp);
		sleep(2);
        USBDevice_deInit();
        SDCard_deInit();
		gprintf("\n\tError.  size = %d",filesize);
		return 0;
	}
	fseek(fp, 0, SEEK_SET);
	

	ret = fread(&filebuff, 1, filesize, fp);
	if(ret != filesize){
		fclose(fp);
        USBDevice_deInit();
        SDCard_deInit();
		gprintf("\n\tError. ret != size");
		return 0;
	}
	fclose(fp);
	//USBDevice_deInit();
    //SDCard_deInit();

    memcpy((void*)0x800027E8, &filebuff,filesize);
    *(vu8*)0x80001807 = 0x01;
	//gprintf("\n\tDe-init SD & USB");
	

	
	gprintf("\n\tDone");

	return 1;
}

u32 do_bca_code(u8 *gameid)
{
	if (IOS_GetVersion() == 222 || IOS_GetVersion() == 223)
	{
		FILE *fp;
		u32 filesize;
		char filepath[150];
		memset(filepath, 0, 150);
		u8 bcaCode[64] ATTRIBUTE_ALIGN(32);

		sprintf(filepath, "%s%6s", Settings.BcaCodepath, gameid);
		filepath[strlen(Settings.BcaCodepath)+6] = '.';
		filepath[strlen(Settings.BcaCodepath)+7] = 'b';
		filepath[strlen(Settings.BcaCodepath)+8] = 'c';
		filepath[strlen(Settings.BcaCodepath)+9] = 'a';

		fp = fopen(filepath, "rb");
		if (!fp) {
			memset(filepath, 0, 150);
			sprintf(filepath, "%s%3s", Settings.BcaCodepath, gameid + 1);
			filepath[strlen(Settings.BcaCodepath)+3] = '.';
			filepath[strlen(Settings.BcaCodepath)+4] = 'b';
			filepath[strlen(Settings.BcaCodepath)+5] = 'c';
			filepath[strlen(Settings.BcaCodepath)+6] = 'a';
			fp = fopen(filepath, "rb");
			
			if (!fp) {
				// Set default bcaCode
				memset(bcaCode, 0, 64);
				bcaCode[0x33] = 1;
			}
		}

		if (fp) {
			u32 ret = 0;

			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			
			if (filesize == 64) {			
				fseek(fp, 0, SEEK_SET);
				ret = fread(bcaCode, 1, 64, fp);
			}
			fclose(fp);

			if (ret != 64) {
				// Set default bcaCode
				memset(bcaCode, 0, 64);
				bcaCode[0x33] = 1;
			}
		}
		
		mload_seek(*((u32 *) (dip_plugin+15*4)), SEEK_SET);	// offset 15 (bca_data area)
		mload_write(bcaCode, 64);
		mload_close();
	}
	return 0;
}
