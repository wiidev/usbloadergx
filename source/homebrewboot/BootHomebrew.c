#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>

#include "fatmounter.h"
#include "dolloader.h"
#include "elfloader.h"

static void *innetbuffer = NULL;

int AllocHomebrewMemory(u32 filesize) {

    innetbuffer = malloc(filesize);

    if(!innetbuffer)
        return -1;

    return 1;

}

void FreeHomebrewBuffer() {
    free(innetbuffer);
    innetbuffer = NULL;
}

void CopyHomebrewMemory(u32 read, u8 *temp, u32 len) {

    memcpy(((u8 *) innetbuffer)+read, temp, len);

}

int BootHomebrew(char * path)
{
	void *buffer = NULL;
	u32 filesize = 0;
	entrypoint entry;
    u32 cpu_isr;

    FILE * file = fopen(path, "rb");
    if(!file) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

    fseek (file, 0, SEEK_END);
	filesize = ftell(file);
	rewind(file);

	buffer = malloc(filesize);

	if(fread (buffer, 1, filesize, file) != filesize)
	{
        fclose (file);
        free(buffer);
        SDCard_deInit();
        USBDevice_deInit();
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}
	fclose (file);

	struct __argv args;
	bzero(&args, sizeof(args));
	args.argvMagic = ARGV_MAGIC;
	args.length = strlen(path) + 2;
	args.commandLine = (char*)malloc(args.length);
	if (!args.commandLine) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	strcpy(args.commandLine, path);
	args.commandLine[args.length - 1] = '\0';
	args.argc = 1;
	args.argv = &args.commandLine;
	args.endARGV = args.argv + 1;

	int ret = valid_elf_image(buffer);
	if (ret == 1)
		entry = (entrypoint) load_elf_image(buffer);
	else
		entry = (entrypoint) load_dol(buffer, &args);

    free(buffer);

    if(!entry) {
        SDCard_deInit();
        USBDevice_deInit();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    SDCard_deInit();
    USBDevice_deInit();

    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable (cpu_isr);
	__exception_closeall();
	entry();
	_CPU_ISR_Restore (cpu_isr);

	return 0;
}

int BootHomebrewFromMem()
{
	entrypoint entry;
    u32 cpu_isr;

    if(!innetbuffer) {
        SDCard_deInit();
        USBDevice_deInit();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

	struct __argv args;

	int ret = valid_elf_image(innetbuffer);
	if (ret == 1)
		entry = (entrypoint) load_elf_image(innetbuffer);
	else
		entry = (entrypoint) load_dol(innetbuffer, &args);

    free(innetbuffer);

    if(!entry) {
        SDCard_deInit();
        USBDevice_deInit();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    SDCard_deInit();
    USBDevice_deInit();

    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable (cpu_isr);
	__exception_closeall();
	entry();
	_CPU_ISR_Restore (cpu_isr);

	return 0;
}
