/*
 *      loadMii loader v0.3
 *      main.c
 *      http://code.google.com/p/loadmii
 *
 *      Copyright 2009 The Lemon Man
 *      Thanks to luccax, Wiipower, Aurelio and crediar
 *      usbGecko powered by Nuke
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ogc/machine/processor.h>
//shit i added
#include <string.h>


#include "pngu/pngu.h"
#include "video.h"
#include "filelist.h"
#include "dolloader.h"
#include "elfloader.h"


PNGUPROP imgProp;
IMGCTX ctx;


u8 * GetImageData(void) {

	u8 * data = NULL;

	int ret;

	ctx = PNGU_SelectImageFromBuffer(background_png);
	if (!ctx)
		return NULL;

	ret = PNGU_GetImageProperties(ctx, &imgProp);
	if (ret != PNGU_OK)
		return NULL;

    int len = imgProp.imgWidth * imgProp.imgHeight * 4;

    if(len%32) len += (32-len%32);
    data = (u8 *)memalign (32, len);
    ret = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);
	DCFlushRange(data, len);

	PNGU_ReleaseImageContext(ctx);

    return data;
}

void Background_Show(int x, int y, int z, u8 * data, int angle, int scaleX, int scaleY, int alpha)
{
	/* Draw image */
	Menu_DrawImg(x, y, z, imgProp.imgWidth, imgProp.imgHeight, data, angle, scaleX, scaleY, alpha);
}

char thePath[100];
static char *cfg_name, *cfg_val;


char* strcopy(char *dest, char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

char* trimcopy(char *dest, char *src, int size)
{
	int len;
	while (*src == ' ') src++;
	len = strlen(src);
	// trim trailing " \r\n"
	while (len > 0 && strchr(" \r\n", src[len-1])) len--;
	if (len >= size) len = size-1;
	strncpy(dest, src, len);
	dest[len] = 0;
	return dest;
}



void cfg_parseline(char *line, void (*set_func)(char*, char*))
{
	// split name = value
	char tmp[200], name[100], val[100];
	strcopy(tmp, line, sizeof(tmp));
	char *eq = strchr(tmp, '=');
	if (!eq) return;
	*eq = 0;
	trimcopy(name, tmp, sizeof(name));
	trimcopy(val, eq+1, sizeof(val));
	set_func(name, val);
}



bool cfg_parsefile(char *fname, void (*set_func)(char*, char*))
{
	FILE *f;
	char line[200];

	f = fopen(fname, "r");
	if (!f) {
		return false;
	}
	while (fgets(line, sizeof(line), f)) {
		// lines starting with # are comments
		if (line[0] == '#') continue;
		cfg_parseline(line, set_func);
	}
	fclose(f);
	return true;
}


char pathname[200];
void cfg_set(char *name, char *val)
{
	cfg_name = name;
	cfg_val = val;

	if (strcmp(name, "update_path") == 0) {
		strcopy(thePath, val, sizeof(thePath));
		return;
	}
	
}


int main(int argc, char **argv) {

	u32 cookie;
	FILE *exeFile;
	void *exeBuffer          = (void *)EXECUTABLE_MEM_ADDR;
	int exeSize              = 0;
	u32 exeEntryPointAddress = 0;
	entrypoint exeEntryPoint;
	
	
	
    /* int videomod */
    InitVideo();

    /* get imagedata */
    u8 * imgdata = GetImageData();

    /* fadein of image */
    for(int i = 0; i < 255; i = i+10) {
    if(i>255) i = 255;
	Background_Show(0, 0, 0, imgdata, 0, 1, 1, i);
    Menu_Render();
    }
    /* check devices */
    __io_wiisd.startup();
	fatMount("SD", &__io_wiisd, 0, 32, 128);

    /* write default path*/
	snprintf(thePath, sizeof(thePath), "SD:/apps/usbloader_gx/");
	
	// check to see if there is a path in the GXglobal file 
	snprintf(pathname, sizeof(pathname), "SD:/config/GXGlobal.cfg");
	cfg_parsefile(pathname, &cfg_set);
	
	//add boot.dol to our path
	snprintf(pathname, sizeof(pathname), "%sboot.dol", thePath);
	
	/* Open dol File and check exist */
	exeFile = fopen (pathname ,"rb");
	
	//boot.dol wasn't found.  Try for the elf.
	if (exeFile==NULL) {
	    fclose(exeFile);
		snprintf(pathname, sizeof(pathname), "%sboot.elf", thePath);
		exeFile = fopen (pathname ,"rb");
		if (exeFile==NULL) {
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		}
	}

	fseek (exeFile, 0, SEEK_END);
	exeSize = ftell(exeFile);
	fseek (exeFile, 0, SEEK_SET);
	if(fread (exeBuffer, 1, exeSize, exeFile) != (unsigned int) exeSize) {
		printf("Can't open DOL File...\n");
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);;
	}

	fclose (exeFile);

    /* load entry point */
	struct __argv args[10];

	int ret = valid_elf_image(exeBuffer);
	if (ret == 1) {
        exeEntryPointAddress = load_elf_image(exeBuffer);
	} else {
        exeEntryPointAddress = load_dol_image(exeBuffer, args);
	}

    /* fadeout of image */
    for(int i = 255; i > 1; i = i-7) {
        if(i < 0) i = 0;
        Background_Show(0, 0, 0, imgdata, 0, 1, 1, i);
        Menu_Render();
	}

	fatUnmount("SD");
    __io_wiisd.shutdown();
    StopGX();

	if (exeEntryPointAddress == 0) {
		printf("EntryPointAddress failed...\n");
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);;
	}

	exeEntryPoint = (entrypoint) exeEntryPointAddress;

    /* cleaning up and load dol */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable (cookie);
	__exception_closeall ();
	exeEntryPoint ();
	_CPU_ISR_Restore (cookie);
	return 0;

}
