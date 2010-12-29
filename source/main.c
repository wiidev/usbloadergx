 /****************************************************************************
 * Copyright 2009 The Lemon Man and thanks to luccax, Wiipower, Aurelio and crediar
 * Copyright 2010 Dimok
 *
 * Original forwarder source by
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
#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>

#include "video.h"
#include "background_image.h"
#include "dolloader.h"
#include "filelist.h"
#include "devicemounter.h"
#include "cfg.h"

void __exception_setreload(int t);

static FILE * open_file(const char * dev, char * filepath)
{
    sprintf(filepath, "%s:/apps/usbloader_gx/boot.dol", dev);

    FILE * exeFile = fopen(filepath ,"rb");
    if (exeFile == NULL)
    {
        sprintf(filepath, "%s:/apps/usbloader_gx/boot.dol", dev);
        exeFile = fopen(filepath ,"rb");
    }
    if (exeFile == NULL)
    {
        sprintf(filepath, "%s:/apps/usbloadergx/boot.dol", dev);
        exeFile = fopen(filepath ,"rb");
    }

    return exeFile;
}

static bool FindConfigPath(const char * device, char *cfgpath)
{
    bool result = false;
	sprintf(cfgpath, "%s:/config/GXGlobal.cfg", device);
	result = cfg_parsefile(cfgpath, &cfg_set);

	if(!result)
	{
        sprintf(cfgpath, "%s:/apps/usbloader_gx/GXGlobal.cfg", device);
        result = cfg_parsefile(cfgpath, &cfg_set);
	}
	if(!result)
	{
        sprintf(cfgpath, "%s:/apps/usbloadergx/GXGlobal.cfg", device);
        result = cfg_parsefile(cfgpath, &cfg_set);
	}

	return result;
}

int main(int argc, char **argv)
{
	u32 cookie;
	FILE *exeFile = NULL;
	void * exeBuffer = (void *)EXECUTABLE_MEM_ADDR;
	u32 exeSize = 0;
	u32 exeEntryPointAddress = 0;
	entrypoint exeEntryPoint;
	__exception_setreload(0);

	/* int videomod */
	InitVideo();
	/* get imagedata */
	u8 * imgdata = GetImageData();
	fadein(imgdata);

    char filepath[200];

	// try SD Card First
	SDCard_Init();

	if(FindConfigPath(DeviceName[SD], filepath))
	{
	    strcat(filepath, "boot.dol");
        exeFile = fopen(filepath, "rb");
	}
    if(!exeFile)
        exeFile = open_file(DeviceName[SD], filepath);
	// if app not found on SD Card try USB
	if (exeFile == NULL)
	{
		USBDevice_Init();
		int dev;
		for(dev = USB1; dev < MAXDEVICES; ++dev)
		{
            if(FindConfigPath(DeviceName[dev], filepath))
            {
                strcat(filepath, "boot.dol");
                exeFile = fopen(filepath, "rb");
            }
		    if(!exeFile)
                exeFile = open_file(DeviceName[dev], filepath);
		}
	}

    // if nothing found exiting
    if (exeFile == NULL)
    {
        fadeout(imgdata);
        fclose (exeFile);
        SDCard_deInit();
        USBDevice_deInit();
        StopGX();
        free(imgdata);
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

	fseek (exeFile, 0, SEEK_END);
	exeSize = ftell(exeFile);
	rewind (exeFile);

	if(fread (exeBuffer, 1, exeSize, exeFile) != exeSize)
	{
		fadeout(imgdata);
        fclose (exeFile);
        SDCard_deInit();
        USBDevice_deInit();
        StopGX();
        free(imgdata);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}
	fclose (exeFile);

	/* load entry point */
	struct __argv args;
	bzero(&args, sizeof(args));
	args.argvMagic = ARGV_MAGIC;
	args.length = strlen(filepath) + 2;
	args.commandLine = (char*)malloc(args.length);
	if (!args.commandLine) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	strcpy(args.commandLine, filepath);
	args.commandLine[args.length - 1] = '\0';
	args.argc = 1;
	args.argv = &args.commandLine;
	args.endARGV = args.argv + 1;

	u8 * appboot_buff = (u8 *) malloc(app_booter_dol_size);
	if(!appboot_buff)
	{
		fadeout(imgdata);
        SDCard_deInit();
        USBDevice_deInit();
        StopGX();
        free(imgdata);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}

    memcpy(appboot_buff, app_booter_dol, app_booter_dol_size);

	exeEntryPointAddress = load_dol_image(appboot_buff, &args);

    if(appboot_buff)
        free(appboot_buff);

	fadeout(imgdata);
	SDCard_deInit();
	USBDevice_deInit();
	StopGX();
	free(imgdata);

    //! Reset HBC stub so we can leave correct from usb loader
    memset((char *) 0x80001804, 0, 8);

	if (exeEntryPointAddress == 0)
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

	exeEntryPoint = (entrypoint) exeEntryPointAddress;
	/* cleaning up and load dol */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable (cookie);
	__exception_closeall ();
	exeEntryPoint ();
	_CPU_ISR_Restore (cookie);
	return 0;
}
