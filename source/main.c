 /****************************************************************************
 * Copyright 2009 The Lemon Man and thanks to luccax, Wiipower, Aurelio and crediar
 * Copyright 2010-2011 Dimok
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
#include "filelist.h"
#include "devicemounter.h"
#include "cfg.h"

#define EXECUTE_ADDR	((u8 *) 0x92000000)
#define BOOTER_ADDR		((u8 *) 0x93000000)
#define ARGS_ADDR		((u8 *) 0x93200000)

typedef void (*entrypoint) (void);
extern void __exception_setreload(int t);
extern void __exception_closeall();

static int GetXMLArguments(const char *path, struct __argv *args)
{
	char xmlpath[200];
	snprintf(xmlpath, sizeof(xmlpath), "%s", path);

	char *ptr = strrchr(xmlpath, '/');
	if(ptr)
		ptr[1] = '\0';
	else
		return -1; // only take full path

	strncat(xmlpath, "meta.xml", sizeof(xmlpath));

	FILE *fp = fopen(xmlpath, "rb");
	if(!fp)
		return -1;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);

	char *fileBuf = (char *) malloc(size+1);
	if(!fileBuf)
	{
		fclose(fp);
		return -1;
	}

	fread(fileBuf, 1, size, fp);
	fclose(fp);

	fileBuf[size] = '\0';

	const int max_len = 1024;
	char *entry = (char *) malloc(max_len);
	ptr = fileBuf;

	while(ptr)
	{
		char *comment = strstr(ptr, "<!--");
		ptr = strstr(ptr, "<arg>");
		if(!ptr)
			break;

		if(comment && comment < ptr)
		{
			ptr = strstr(ptr, "-->");
			continue;
		}

		ptr += strlen("<arg>");
		int len;
		for(len = 0; *ptr != '\0' && !(ptr[0] == '<' && ptr[1]  == '/') && len < max_len-1; ptr++, len++)
			entry[len] = *ptr;

		entry[len] = '\0';

		strcpy(args->commandLine + args->length - 1, entry);
		args->length += len + 1;
	}

	free(entry);
	free(fileBuf);

	return 0;
}

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

static bool FindConfigPath(const char * device, char *cfgpath, int maxsize)
{
    bool result = false;
	snprintf(cfgpath, maxsize, "%s:/config/GXGlobal.cfg", device);
	result = cfg_parsefile(cfgpath, maxsize);

	if(!result)
	{
        snprintf(cfgpath, maxsize, "%s:/apps/usbloader_gx/GXGlobal.cfg", device);
        result = cfg_parsefile(cfgpath, maxsize);
	}
	if(!result)
	{
        snprintf(cfgpath, maxsize, "%s:/apps/usbloadergx/GXGlobal.cfg", device);
        result = cfg_parsefile(cfgpath, maxsize);
	}

	return result;
}

int main(int argc, char **argv)
{
	u32 cookie;
	FILE *exeFile = NULL;
	u32 exeSize = 0;
	void * exeBuffer = (void *)EXECUTE_ADDR;
	entrypoint exeEntryPoint = (entrypoint) BOOTER_ADDR;
	__exception_setreload(0);

	/* int videomod */
	InitVideo();
	/* get imagedata */
	u8 * imgdata = GetImageData();
	fadein(imgdata);

    char filepath[200];

	// try SD Card First
	SDCard_Init();

	if(FindConfigPath(DeviceName[SD], filepath, sizeof(filepath)))
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
            if(FindConfigPath(DeviceName[dev], filepath, sizeof(filepath)))
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

	//! Setup argument struct
	struct __argv args;
	bzero(&args, sizeof(args));
	args.argvMagic = ARGV_MAGIC;
	args.length = strlen(filepath) + 2;
	//! Put the argument into mem2 too, to avoid overwriting it
	args.commandLine = (char *) ARGS_ADDR + sizeof(args);
	//! Add executable file path as first argument
	strcpy(args.commandLine, filepath);
	//! Append the arguments from the meta.xml
	GetXMLArguments(filepath, &args);
	//! Finishing sequence "\0\0"
	args.commandLine[args.length - 1] = '\0';
	args.argc = 1;
	args.argv = &args.commandLine;
	args.endARGV = args.argv + 1;

	//! Put the booter code to it's expected address
	memcpy(BOOTER_ADDR, app_booter_bin, app_booter_bin_size);
	DCFlushRange(BOOTER_ADDR, app_booter_bin_size);

	//! Put the argument struct to it's expected address
	memcpy(ARGS_ADDR, &args, sizeof(args));
	DCFlushRange(ARGS_ADDR, sizeof(args) + args.length);

    //! Reset HBC stub so we can leave correct from USB Loader GX to System Menu
    memset((char *) 0x80001804, 0, 8);
    DCFlushRange((char *) 0x80001804, 8);

	/* cleaning up and load booter */
	fadeout(imgdata);
	SDCard_deInit();
	USBDevice_deInit();
	StopGX();
	free(imgdata);

	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable (cookie);
	__exception_closeall ();
	exeEntryPoint ();
	_CPU_ISR_Restore (cookie);
	return 0;
}
