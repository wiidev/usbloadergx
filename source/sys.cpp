#include <gctypes.h>
#include <ogc/system.h>
#include <wiiuse/wpad.h>

#include "mload/mload.h"
#include "banner/BannerAsync.h"
#include "Controls/DeviceHandler.hpp"
#include "FileOperations/fileops.h"
#include "homebrewboot/BootHomebrew.h"
#include "homebrewboot/HomebrewXML.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "settings/newtitles.h"
#include "settings/meta.h"
#include "language/gettext.h"
#include "network/networkops.h"
#include "utils/ResourceManager.h"
#include "usbloader/playlog.h"
#include "usbloader/wbfs.h"
#include "GameCube/GCGames.h"
#include "themes/CTheme.h"
#include "SoundOperations/SoundHandler.hpp"
#include "utils/ThreadedTask.hpp"
#include "audio.h"
#include "lstub.h"
#include "menu.h"
#include "video.h"
#include "gecko.h"
#include "wad/nandtitle.h"

extern "C"
{
	extern s32 MagicPatches(s32);
}

//Wiilight stuff
void wiilight(int enable) // Toggle wiilight (thanks Bool for wiilight source)
{
	static vu32 *_wiilight_reg = (u32*) 0xCD0000C0;
	u32 val = (*_wiilight_reg & ~0x20);
	if (enable && Settings.wiilight) val |= 0x20;
	*_wiilight_reg = val;
}

/* Variables */
u8 shutdown = 0;
u8 reset = 0;

void __Sys_ResetCallback(void)
{
	/* Reboot console */
	reset = 1;
}

void __Sys_PowerCallback(void)
{
	/* Poweroff console */
	shutdown = 1;
}

void Sys_Init(void)
{
	/* Initialize video subsytem */
	//VIDEO_Init();

	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void AppCleanUp(void)
{
	static bool app_clean = false;
	if(app_clean)
		return;

	app_clean = true;

	BannerAsync::ThreadExit();

	if(Settings.CacheTitles)
		GameTitles.WriteCachedTitles(Settings.titlestxt_path);
	Settings.Save();

	ExitGUIThreads();
	StopGX();
	wiilight(0);

	delete btnSoundClick;
	delete btnSoundOver;
	delete btnSoundClick2;
	delete bgMusic;
	delete background;
	delete bgImg;
	delete mainWindow;
	for (int i = 0; i < 4; i++)
		delete pointer[i];

	gettextCleanUp();
	Theme::CleanUp();
	NewTitles::DestroyInstance();
	ThreadedTask::DestroyInstance();
	SoundHandler::DestroyInstance();
	GCGames::DestroyInstance();
	DeinitNetwork();
	GameTitles.SetDefault();

	ShutdownAudio();

	ResourceManager::DestroyInstance();

	WPAD_Shutdown();
	ISFS_Deinitialize();
}

void ExitApp(void)
{
	AppCleanUp();
	WBFS_CloseAll();
	DeviceHandler::DestroyInstance();
	USB_Deinitialize();
	if(Settings.PlaylogUpdate)
		Playlog_Delete(); // Don't show USB Loader GX in the Wii message board

	MagicPatches(0);
}

void Sys_Reboot(void)
{
	/* Restart console */
	ExitApp();
	STM_RebootSystem();
}

#define ShutdownToDefault   0
#define ShutdownToIdle	  1
#define ShutdownToStandby   2

static void _Sys_Shutdown(int SHUTDOWN_MODE)
{
	ExitApp();

	/* Poweroff console */
	if ((CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE && SHUTDOWN_MODE != ShutdownToStandby) || SHUTDOWN_MODE
			== ShutdownToIdle)
	{
		s32 ret;

		/* Set LED mode */
		ret = CONF_GetIdleLedMode();
		if (ret >= 0 && ret <= 2) STM_SetLedMode(ret);

		/* Shutdown to idle */
		STM_ShutdownToIdle();
	}
	else
	{
		/* Shutdown to standby */
		STM_ShutdownToStandby();
	}
}

void Sys_Shutdown(void)
{
	_Sys_Shutdown(ShutdownToDefault);
}

void Sys_ShutdownToIdle(void)
{
	_Sys_Shutdown(ShutdownToIdle);
}
void Sys_ShutdownToStandby(void)
{
	_Sys_Shutdown(ShutdownToStandby);
}

void Sys_LoadMenu(void)
{
	ExitApp();

	// Priiloader shutup
	if (Settings.godmode || !(Settings.ParentalBlocks & BLOCK_PRIILOADER_OVERRIDE))
	{
		*(u32 *)0x8132fffb = 0x50756e65;
		*(u32 *)0x817feff0 = 0x50756e65;	// priiloader 0.8 beta 4+
		DCFlushRange((u32 *)0x8132fffb, 4);
		DCFlushRange((u32 *)0x817feff0, 4);
	}

	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void Sys_BackToLoader(void)
{
	ExitApp();

	if (hbcStubAvailable())
		exit(0);
	// Channel Version
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

#define HBC_HAXX	0x0001000148415858LL
#define HBC_JODI	0x000100014A4F4449LL
#define HBC_1_0_7   0x00010001AF1BF516LL
#define HBC_LULZ	0x000100014c554c5aLL

void Sys_LoadHBC(void)
{
	ExitApp();

	WII_Initialize();

	// Try launching all known HBC titles in reversed released order
	WII_LaunchTitle(HBC_LULZ);
	WII_LaunchTitle(HBC_1_0_7);
	WII_LaunchTitle(HBC_JODI);
	WII_LaunchTitle(HBC_HAXX);

	//Back to system menu if all fails
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

bool RebootApp(void)
{
	// be sure to use current settings as arguments
	editMetaArguments();
	
#ifdef FULLCHANNEL
	ExitApp();
	WII_Initialize();
	return !(WII_LaunchTitle(TITLE_ID(0x00010001, 0x554c4e52)) < 0);
#else

	// Load meta.xml arguments
	char filepath[255];
	HomebrewXML MetaXML;
	snprintf(filepath, sizeof(filepath), "%s/meta.xml", Settings.update_path);
	MetaXML.LoadHomebrewXMLData(filepath);

	u8 *buffer = NULL;
	u32 filesize = 0;
	snprintf(filepath, sizeof(filepath), "%s/boot.dol", Settings.update_path);
	LoadFileToMem(filepath, &buffer, &filesize);
	if(!buffer)
	{
		return false;
	}
	FreeHomebrewBuffer();
	CopyHomebrewMemory(buffer, 0, filesize);

	AddBootArgument(filepath);

	for(u32 i = 0; i < MetaXML.GetArguments().size(); ++i)
	{
		AddBootArgument(MetaXML.GetArguments().at(i).c_str());
	}

	return !(BootHomebrewFromMem() <0);
#endif
}

void ScreenShot()
{
	time_t rawtime;
	struct tm * timeinfo;
	char filename[100];	// Filename, with current date/time.
	char fullPath[300];	// Full pathname: ConfigPath + filename.

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	// Create the filename with the current date/time.
	// Format: USBLoader_GX_ScreenShot-Month_Day_Hour_Minute_Second_Year.png
	int ret = strftime(filename, sizeof(filename), "USBLoader_GX_ScreenShot-%b%d%H%M%S%y.png", timeinfo);
	if (ret == 0)
	{
		// Error formatting the time.
		// Use the raw time in seconds as a fallback.
		snprintf(filename, sizeof(filename), "USBLoader_GX_ScreenShot-%ld.png", rawtime);
	}

	// Create the full pathname.
	snprintf(fullPath, sizeof(fullPath), "%s%s", Settings.ConfigPath, filename);

	if(!CreateSubfolder(Settings.ConfigPath))
	{
		gprintf("Can't create screenshot folder\n");
		return;
	}

	TakeScreenshot(fullPath);
}

/*
 * Check if the current console is a Wii or WiiU
 * Thanks to Crediar
 */
bool isWiiU()
{
	return ((*(vu32*)(0xCD8005A0) >> 16 ) == 0xCAFE);
}
