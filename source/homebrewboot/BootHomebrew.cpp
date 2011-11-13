#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <string>
#include "Controls/DeviceHandler.hpp"
#include "settings/CSettings.h"
#include "lstub.h"
#include "sys.h"
#include "gecko.h"

#define EXECUTE_ADDR	((u8 *) 0x92000000)
#define BOOTER_ADDR		((u8 *) 0x93000000)
#define ARGS_ADDR		((u8 *) 0x93200000)

extern const u8 app_booter_bin[];
extern const u32 app_booter_bin_size;

typedef void (*entrypoint) (void);
extern "C" { void __exception_closeall(); }

static u8 *homebrewbuffer = EXECUTE_ADDR;
static u32 homebrewsize = 0;
static std::vector<std::string> Arguments;

void AddBootArgument(const char * argv)
{
	std::string arg(argv);
	Arguments.push_back(arg);
}

int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len)
{
	homebrewsize += len;
	memcpy((homebrewbuffer) + pos, temp, len);

	return 1;
}

void FreeHomebrewBuffer()
{
	homebrewbuffer = EXECUTE_ADDR;
	homebrewsize = 0;

	Arguments.clear();
}

static int SetupARGV(struct __argv * args)
{
	if (!args) return -1;

	bzero(args, sizeof(struct __argv));
	args->argvMagic = ARGV_MAGIC;

	u32 argc = 0;
	u32 position = 0;
	u32 stringlength = 1;

	/** Append Arguments **/
	for (u32 i = 0; i < Arguments.size(); i++)
	{
		stringlength += Arguments[i].size() + 1;
	}

	args->length = stringlength;
	//! Put the argument into mem2 too, to avoid overwriting it
	args->commandLine = (char *) ARGS_ADDR + sizeof(struct __argv);

	/** Append Arguments **/
	for (u32 i = 0; i < Arguments.size(); i++)
	{
		strcpy(&args->commandLine[position], Arguments[i].c_str());
		position += Arguments[i].size() + 1;
		argc++;
	}

	args->argc = argc;

	args->commandLine[args->length - 1] = '\0';
	args->argv = &args->commandLine;
	args->endARGV = args->argv + 1;

	Arguments.clear();

	return 0;
}

static int RunAppbooter()
{
	if (homebrewsize == 0) return -1;

	ExitApp();

	if(Settings.EntryIOS != IOS_GetVersion())
		IOS_ReloadIOS(Settings.EntryIOS);

	struct __argv args;
	SetupARGV(&args);

	u32 cpu_isr;

	memcpy(BOOTER_ADDR, app_booter_bin, app_booter_bin_size);
	DCFlushRange(BOOTER_ADDR, app_booter_bin_size);

	entrypoint entry = (entrypoint) BOOTER_ADDR;

	if (args.argvMagic == ARGV_MAGIC)
	{
		memmove(ARGS_ADDR, &args, sizeof(args));
		DCFlushRange(ARGS_ADDR, sizeof(args) + args.length);
	}

	u64 currentStub = getStubDest();
	loadStub();

	if (Set_Stub_Split(0x00010001, "UNEO") < 0)
	{
		if (Set_Stub_Split(0x00010001, "ULNR") < 0)
		{
			if (!currentStub) currentStub = 0x100000002ULL;

			Set_Stub(currentStub);
		}
	}

	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable( cpu_isr );
	__exception_closeall();
	entry();
	_CPU_ISR_Restore( cpu_isr );

	return 0;
}

int BootHomebrew(const char * filepath)
{
	FILE *file = fopen(filepath, "rb");
	if (!file) return -1;

	fseek(file, 0, SEEK_END);
	u32 filesize = ftell(file);
	rewind(file);

	if (fread(homebrewbuffer, 1, filesize, file) != filesize)
	{
		fclose(file);
		DeviceHandler::DestroyInstance();
		Sys_BackToLoader();
	}

	homebrewsize = filesize;

	fclose(file);

	AddBootArgument(filepath);
	return RunAppbooter();
}

int BootHomebrewFromMem()
{
	return RunAppbooter();
}
