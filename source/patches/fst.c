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

extern struct SSettings Settings;

u32 do_sd_code(char *filename)
{
	FILE *fp;
	u8 *filebuff;
	u32 filesize;
	u32 ret;
	char filepath[150];

    SDCard_Init();
	USBDevice_Init();

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
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	if(filesize <= 16){
		fclose(fp);
		sleep(2);
        USBDevice_deInit();
        SDCard_deInit();
		return 0;
	}
	fseek(fp, 0, SEEK_SET);

	filebuff = (u8*) malloc (filesize);
	if(filebuff == 0){
		fclose(fp);
		sleep(2);
        USBDevice_deInit();
        SDCard_deInit();
		return 0;
	}

	ret = fread(filebuff, 1, filesize, fp);
	if(ret != filesize){
		free(filebuff);
		fclose(fp);
        USBDevice_deInit();
        SDCard_deInit();
		return 0;
	}

    memcpy((void*)0x800027E8,filebuff,filesize);
    *(vu8*)0x80001807 = 0x01;

	free(filebuff);
	fclose(fp);

	USBDevice_deInit();
    SDCard_deInit();

	return 1;
}


