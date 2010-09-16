#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <string>

#include "usbloader/usbstorage2.h"
#include "dolloader.h"
#include "fatmounter.h"
#include "sys.h"

extern const u8	app_booter_dol[];
extern const u32 app_booter_dol_size;

static u8 *homebrewbuffer = (u8 *) 0x92000000;
static int homebrewsize = 0;
static std::vector<std::string> Arguments;

void AddBootArgument(const char * argv)
{
    std::string arg(argv);
    Arguments.push_back(arg);
}

int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len)
{
    homebrewsize += len;
    memcpy((homebrewbuffer)+pos, temp, len);

    return 1;
}

void FreeHomebrewBuffer()
{
    homebrewbuffer = (u8 *)0x92000000;
    homebrewsize = 0;
}

static int SetupARGV(struct __argv * args)
{
    if(!args)
        return -1;

    bzero(args, sizeof(struct __argv));
    args->argvMagic = ARGV_MAGIC;

    u32 stringlength = 1;

    /** Append Arguments **/
    for(u32 i = 0; i < Arguments.size(); i++)
    {
        stringlength += Arguments[i].size()+1;
    }

    args->length = stringlength;
    args->commandLine = (char*) malloc(args->length);
    if (!args->commandLine)
        return -1;

    u32 argc = 0;
    u32 position = 0;

    /** Append Arguments **/
    for(u32 i = 0; i < Arguments.size(); i++)
    {
        strcpy(&args->commandLine[position], Arguments[i].c_str());
        position += Arguments[i].size()+1;
        argc++;
    }

    args->argc = argc;

    args->commandLine[args->length - 1] = '\0';
    args->argv = &args->commandLine;
    args->endARGV = args->argv + 1;

    Arguments.clear();

    return 0;
}

int BootHomebrew()
{
    if(homebrewsize <= 0)
        Sys_BackToLoader();

    SDCard_deInit();
    USBDevice_deInit();
    USBStorage2_Deinit();

    struct __argv args;
    SetupARGV(&args);

    u32 cpu_isr;

    entrypoint entry = (entrypoint) load_dol(app_booter_dol, &args);

    if (!entry)
        Sys_BackToLoader();

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable (cpu_isr);
    __exception_closeall();
    entry();
    _CPU_ISR_Restore (cpu_isr);

    return 0;
}

int BootHomebrew(const char * filepath)
{
    FILE * file = fopen(filepath, "rb");
    if(!file)
        Sys_BackToLoader();

    fseek(file, 0, SEEK_END);

    int size = ftell(file);
    rewind(file);

    homebrewsize = fread(homebrewbuffer, 1, size, file);
    fclose(file);

    AddBootArgument(filepath);

    return BootHomebrew();
}
