/****************************************************************************
 * Copyright (C) 2012-2014 Cyan
 * Copyright (C) 2010 by Dimok
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
#include <unistd.h>
#include "LoaderSettings.hpp"
#include "usbloader/usbstorage2.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "settings/meta.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "prompts/TitleBrowser.h"
#include "system/IosLoader.h"
#include "usbloader/wbfs.h"
#include "usbloader/GameList.h"
#include "utils/tools.h"
#include "menu.h"
#include "GameCube/GCGames.h"

static const char * OnOffText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Auto" )
};

static const char * AspectText[] =
{
	trNOOP( "Force 4:3" ),
	trNOOP( "Force 16:9" ),
	trNOOP( "System Default" )
};

static const char * VideoModeText[] =
{
	trNOOP( "System Default" ),
	trNOOP( "Disc Default" ),
	trNOOP( "Force PAL50" ),
	trNOOP( "Force PAL60" ),
	trNOOP( "Force NTSC" ),
	trNOOP( "Region Patch" ),
	trNOOP( "Force PAL480p" ),
	trNOOP( "Force NTSC480p" ),
};

static const char * VideoPatchDolText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "Region Patch" ),
	trNOOP( "ON" ),
	trNOOP( "All" ),
};

static const char * LanguageText[] =
{
	trNOOP( "Japanese" ),
	trNOOP( "English" ),
	trNOOP( "German" ),
	trNOOP( "French" ),
	trNOOP( "Spanish" ),
	trNOOP( "Italian" ),
	trNOOP( "Dutch" ),
	trNOOP( "SChinese" ),
	trNOOP( "TChinese" ),
	trNOOP( "Korean" ),
	trNOOP( "Console Default" )
};

static const char * NandEmuText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "Partial" ),
	trNOOP( "Full" )
};

static const char * HooktypeText[] =
{
	trNOOP( "None" ),
	trNOOP( "VBI (Default)" ),
	trNOOP( "KPAD Read" ),
	trNOOP( "Joypad" ),
	trNOOP( "GXDraw" ),
	trNOOP( "GXFlush" ),
	trNOOP( "OSSleepThread" ),
	trNOOP( "AXNextFrame" ),
};

static const char * ChannelLaunchText[] =
{
	trNOOP( "Main DOL" ),
	trNOOP( "Boot Content" ),
};

static const char * GCMode[] =
{
	trNOOP( "MIOS (Default & Customs)" ),
	trNOOP( "Devolution" ),
	trNOOP( "Nintendont" ),
};

static const char * GCSourceText[][3] =
{
	{ trNOOP( "Main Path" ), "", "" },
	{ trNOOP( "SD Path" ), "", "" },
	{ trNOOP( "Auto" ), "", "" },
	{ trNOOP( "Main Path" ), "/", trNOOP( "SD Path" ) },
	{ trNOOP( "SD Path" ), "/", trNOOP( "Main Path" ) },
};

static const char * DMLVideoText[] =
{
	trNOOP( "Auto" ),
	trNOOP( "System Default" ),
	trNOOP( "Disc Default" ),
	trNOOP( "Force PAL50" ),
	trNOOP( "Force PAL60" ),
	trNOOP( "Force NTSC" ),
	"", // unused
	trNOOP( "Force PAL480p" ),
	trNOOP( "Force NTSC480p" ),
	trNOOP( "None" ),
};

static const char * DMLNMMMode[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Debug" ),
};

static const char * DMLDebug[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Debug Wait" ),
};

static const char * DEVOMCText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Individual" ),
};

static const char * NINMCText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "Individual" ),
	trNOOP( "ON (Multi)" ),
};

static const char * NINCfgText[] =
{
	trNOOP( "Delete" ),
	trNOOP( "Create" ),
	trNOOP( "No change" ),
};

static const char * PrivServText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "NoSSL only" ),
	trNOOP( "Wiimmfi.de" ),
};

LoaderSettings::LoaderSettings()
	: SettingsMenu(tr("Loader Settings"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;

	Options->SetName(Idx++, "%s", tr( "Video Mode" ));
	Options->SetName(Idx++, "%s", tr( "Dol Video Patch" ));
	Options->SetName(Idx++, "%s", tr( "Sneek Video Patch" ));
	Options->SetName(Idx++, "%s", tr( "VIDTV Patch" ));
	Options->SetName(Idx++, "%s", tr( "Aspect Ratio" ));
	Options->SetName(Idx++, "%s", tr( "Game Language" ));
	Options->SetName(Idx++, "%s", tr( "Patch Country Strings" ));
	Options->SetName(Idx++, "%s", tr( "Ocarina" ));
	Options->SetName(Idx++, "%s", tr( "Private Server" ));
	Options->SetName(Idx++, "%s", tr( "Loader's IOS" ));
	Options->SetName(Idx++, "%s", tr( "Game's IOS" ));
	Options->SetName(Idx++, "%s", tr( "Quick Boot" ));
	Options->SetName(Idx++, "%s", tr( "Block IOS Reload" ));
	Options->SetName(Idx++, "%s", tr( "Return To" ));
	Options->SetName(Idx++, "%s", tr( "Nand Saves Emulation" ));
	Options->SetName(Idx++, "%s", tr( "Nand Chan. Emulation" ));
	Options->SetName(Idx++, "%s", tr( "Hooktype" ));
	Options->SetName(Idx++, "%s", tr( "Wiird Debugger" ));
	Options->SetName(Idx++, "%s", tr( "Debugger Paused Start" ));
	Options->SetName(Idx++, "%s", tr( "Channel Launcher" ));
	Options->SetName(Idx++, "%s", tr( "=== GameCube Settings" ));
	Options->SetName(Idx++, "%s", tr( "GameCube Source" ));
	Options->SetName(Idx++, "%s", tr( "GameCube Mode" ));
	Options->SetName(Idx++, "%s", tr( "--==  DM(L) + Nintendont" ));
	Options->SetName(Idx++, "%s", tr( "Video Mode" ));
	Options->SetName(Idx++, "%s", tr( "Progressive Patch" ));
	Options->SetName(Idx++, "%s", tr( "Force Widescreen" ));
	Options->SetName(Idx++, "%s", tr( "Debug" ));
	Options->SetName(Idx++, "%s", tr( "Disc-Select Prompt" ));
	Options->SetName(Idx++, "%s", tr( "--==   DIOS MIOS (Lite) " ));
	Options->SetName(Idx++, "%s", tr( "NMM Mode" ));
	Options->SetName(Idx++, "%s", tr( "PAD Hook" ));
	Options->SetName(Idx++, "%s", tr( "No Disc+" ));
	Options->SetName(Idx++, "%s", tr( "Screenshot" ));
	Options->SetName(Idx++, "%s", tr( "LED Activity" ));
	Options->SetName(Idx++, "%s", tr( "Japanese Patch" ));
	Options->SetName(Idx++, "%s", tr( "--==       Nintendont" ));
	Options->SetName(Idx++, "%s", tr( "Auto Boot" ));
	Options->SetName(Idx++, "%s", tr( "Settings File" ));
	Options->SetName(Idx++, "%s", tr( "Video Deflicker" ));
	Options->SetName(Idx++, "%s", tr( "Memory Card Emulation" ));
	Options->SetName(Idx++, "%s", tr( "Memory Card Blocks Size" ));
	Options->SetName(Idx++, "%s", tr( "USB-HID Controller" ));
	Options->SetName(Idx++, "%s", tr( "GameCube Controller" ));
	Options->SetName(Idx++, "%s", tr( "Native Controller" ));
	Options->SetName(Idx++, "%s", tr( "WiiU Widescreen" ));
	Options->SetName(Idx++, "%s", tr( "LED Activity" ));
	Options->SetName(Idx++, "%s", tr( "OSReport" ));
	Options->SetName(Idx++, "%s", tr( "Log to file" ));
	Options->SetName(Idx++, "%s", tr( "--==       Devolution" ));
	Options->SetName(Idx++, "%s", tr( "Memory Card Emulation" ));
	Options->SetName(Idx++, "%s", tr( "Force Widescreen" ));
	Options->SetName(Idx++, "%s", tr( "LED Activity" ));
	Options->SetName(Idx++, "%s", tr( "F-Zero AX" ));
	Options->SetName(Idx++, "%s", tr( "Timer Fix" ));
	Options->SetName(Idx++, "%s", tr( "D Buttons" ));
	Options->SetName(Idx++, "%s", tr( "Crop Overscan" ));
	Options->SetName(Idx++, "%s", tr( "Disc Read Delay" ));

	SetOptionValues();

	oldLoaderMode = Settings.LoaderMode;
	oldGameCubeSource = Settings.GameCubeSource;
	oldLoaderIOS = Settings.LoaderIOS;
}

LoaderSettings::~LoaderSettings()
{
	if(oldLoaderMode != Settings.LoaderMode)
	{
		if(Settings.LoaderMode & MODE_WIIGAMES && (gameList.GameCount() == 0))
		{
			WBFS_ReInit(WBFS_DEVICE_USB);
			gameList.ReadGameList();
		}

		gameList.LoadUnfiltered();
		GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path, false);
	}
	
	if(oldGameCubeSource != Settings.GameCubeSource)
	{
		GCGames::Instance()->LoadAllGames();
	}
	
	if(oldLoaderIOS != Settings.LoaderIOS)
	{
		// edit meta.xml arguments
		editMetaArguments();
	}
}

void LoaderSettings::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Video Mode
	Options->SetValue(Idx++, "%s", tr(VideoModeText[Settings.videomode]));

	//! Settings: Dol Video Patch
	Options->SetValue(Idx++, "%s", tr( VideoPatchDolText[Settings.videoPatchDol] ));

	//! Settings: Sneek Video Patch
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.sneekVideoPatch] ));

	//! Settings: VIDTV Patch
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.videopatch] ));

	//! Settings: Aspect Ratio
	Options->SetValue(Idx++, "%s", tr( AspectText[Settings.GameAspectRatio] ));

	//! Settings: Game Language
	Options->SetValue(Idx++, "%s", tr( LanguageText[Settings.language] ));

	//! Settings: Patch Country Strings
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.patchcountrystrings] ));

	//! Settings: Ocarina
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.ocarina] ));

	//! Settings: Private Server
	Options->SetValue(Idx++, "%s", tr( PrivServText[Settings.PrivateServer] ));

	//! Settings: Loader's IOS
	if (Settings.godmode)
		Options->SetValue(Idx++, "IOS %i", Settings.LoaderIOS);
	else
		Options->SetValue(Idx++, "********");

	//! Settings: Game's IOS
	if (Settings.godmode)
		Options->SetValue(Idx++, "IOS %i", Settings.cios);
	else
		Options->SetValue(Idx++, "********");

	//! Settings: Quick Boot
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.quickboot] ));

	//! Settings: Block IOS Reload
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.BlockIOSReload] ));

	//! Settings: Return To
	const char* TitleName = NULL;
	u64 tid = NandTitles.FindU32(Settings.returnTo);
	if (tid > 0)
		TitleName = NandTitles.NameOf(tid);
	TitleName = TitleName ? TitleName : strlen(Settings.returnTo) > 0 ? Settings.returnTo : tr(OnOffText[0]);
	Options->SetValue(Idx++, "%s", TitleName);

	//! Settings: Nand Emulation
	Options->SetValue(Idx++, "%s", tr( NandEmuText[Settings.NandEmuMode] ));

	//! Settings: Nand Chan. Emulation
	Options->SetValue(Idx++, "%s", tr( NandEmuText[Settings.NandEmuChanMode] ));

	//! Settings: Hooktype
	Options->SetValue(Idx++, "%s", tr( HooktypeText[Settings.Hooktype] ));

	//! Settings: Wiird Debugger
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.WiirdDebugger] ));

	//! Settings: Wiird Debugger Pause on Start
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.WiirdDebuggerPause] ));

	//! Settings: Channel Launcher
	Options->SetValue(Idx++, "%s", tr( ChannelLaunchText[Settings.UseChanLauncher] ));

	//! Settings: TITLE - GameCube Settings
	Options->SetValue(Idx++, "=======");

	//! Settings: GameCube Source
	Options->SetValue(Idx++, "%s%s%s", tr(GCSourceText[Settings.GameCubeSource][0]),
	                GCSourceText[Settings.GameCubeSource][1], tr(GCSourceText[Settings.GameCubeSource][2]));

	//! Settings: GameCube Mode
	Options->SetValue(Idx++, "%s", tr(GCMode[Settings.GameCubeMode]));

	//! Settings: TITLE - GameCube DIOS MIOS (Lite) + Nintendont
	Options->SetValue(Idx++, "==--   ");

	//! Settings: DML + NIN Video Mode
	Options->SetValue(Idx++, "%s", tr(DMLVideoText[Settings.DMLVideo]));

	//! Settings: DML + NIN Progressive Patch
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLProgPatch]));

	//! Settings: DML + NIN Force Widescreen
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLWidescreen]));

	//! Settings: DML + NIN Debug
	Options->SetValue(Idx++, "%s", tr(DMLDebug[Settings.DMLDebug]));

	//! Settings: DML + NIN MultiDiscPrompt
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.MultiDiscPrompt]));

	//! Settings: TITLE - GameCube DIOS MIOS (Lite)
	Options->SetValue(Idx++, "==--   ");

	//! Settings: DML NMM Mode
	Options->SetValue(Idx++, "%s", tr(DMLNMMMode[Settings.DMLNMM]));

	//! Settings: DML PAD Hook
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLPADHOOK]));

	//! Settings: DML Extended No Disc
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLNoDisc2]));

	//! Settings: DML Screenshot
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLScreenshot]));

	//! Settings: DML LED Activity
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLActivityLED]));

	//! Settings: DML Japanese Patch
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLJPNPatch]));

	//! Settings: TITLE - Nintendont
	Options->SetValue(Idx++, "==--   ");

	//! Settings: NIN Auto Boot
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINAutoboot]));

	//! Settings: NIN Nincfg.bin file
	Options->SetValue(Idx++, "%s", tr(NINCfgText[Settings.NINSettings]));

	//! Settings: NIN Video Deflicker
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINDeflicker]));

	//! Settings: NIN Memory Card Emulation
	Options->SetValue(Idx++, "%s", tr(NINMCText[Settings.NINMCEmulation]));

	//! Settings: NIN Memory Card Blocks Size
	Options->SetValue(Idx++, "%d", MEM_CARD_BLOCKS(Settings.NINMCSize));

	//! Settings: NIN USB-HID controller
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINUSBHID]));

	//! Settings: NIN MaxPads - Number of GameCube controllers
	Options->SetValue(Idx++, "%i", Settings.NINMaxPads);

	//! Settings: NIN Native Controller
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINNativeSI]));

	//! Settings: WiiU Widescreen
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINWiiUWide]));

	//! Settings: NIN LED Activity
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINLED]));

	//! Settings: NIN OS Report
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINOSReport]));

	//! Settings: NIN Log to file
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.NINLog]));

	//! Settings: TITLE - Devolution
	Options->SetValue(Idx++, "==--   ");

	//! Settings: DEVO Memory Card Emulation
	Options->SetValue(Idx++, "%s", tr(DEVOMCText[Settings.DEVOMCEmulation]));

	//! Settings: DEVO Widescreen Patch
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVOWidescreen]));

	//! Settings: DEVO Activity LED
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVOActivityLED]));

	//! Settings: DEVO F-Zero AX unlock patch
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVOFZeroAX]));

	//! Settings: DEVO Timer Fix
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVOTimerFix]));

	//! Settings: DEVO Direct Button Mapping
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVODButtons]));

	//! Settings: DEVO Crop Overscan
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVOCropOverscan]));

	//! Settings: DEVO Disc Read Delay
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DEVODiscDelay]));

}

int LoaderSettings::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: Video Mode
	if (ret == ++Idx)
	{
		if (++Settings.videomode >= VIDEO_MODE_MAX) Settings.videomode = 0;
	}

	//! Settings: Dol Video Patch
	if (ret == ++Idx)
	{
		if (++Settings.videoPatchDol >= VIDEO_PATCH_DOL_MAX) Settings.videoPatchDol = 0;
	}

	//! Settings: Sneek Video Patch
	else if (ret == ++Idx )
	{
		if (++Settings.sneekVideoPatch >= MAX_ON_OFF) Settings.sneekVideoPatch = 0;
	}

	//! Settings: VIDTV Patch
	else if (ret == ++Idx)
	{
		if (++Settings.videopatch >= MAX_ON_OFF) Settings.videopatch = 0;
	}

	//! Settings: Aspect Ratio
	else if (ret == ++Idx )
	{
		if (++Settings.GameAspectRatio >= ASPECT_MAX) Settings.GameAspectRatio = 0;
	}

	//! Settings: Game Language
	else if (ret == ++Idx)
	{
		if (++Settings.language >= MAX_LANGUAGE) Settings.language = 0;
	}

	//! Settings: Patch Country Strings
	else if (ret == ++Idx)
	{
		if (++Settings.patchcountrystrings >= MAX_ON_OFF) Settings.patchcountrystrings = 0;
	}

	//! Settings: Ocarina
	else if (ret == ++Idx)
	{
		if (++Settings.ocarina >= MAX_ON_OFF) Settings.ocarina = 0;
	}

	//! Settings: Private Server
	else if (ret == ++Idx)
	{
		if (++Settings.PrivateServer >= PRIVSERV_MAX_CHOICE) Settings.PrivateServer = 0;
	}

	//! Settings: Loader's IOS
	else if (ret == ++Idx)
	{
		if(!Settings.godmode)
			return MENU_NONE;

		char entered[4];
		snprintf(entered, sizeof(entered), "%i", Settings.LoaderIOS);
		if(OnScreenNumpad(entered, sizeof(entered)))
		{
			if(atoi(entered) == 58) // allow only IOS58 for IOS <200
				Settings.LoaderIOS = 58;
			else
				Settings.LoaderIOS = LIMIT(atoi(entered), 200, 255);

			if(NandTitles.IndexOf(TITLE_ID(1, Settings.LoaderIOS)) < 0)
			{
				WindowPrompt(tr("Warning:"), tr("This IOS was not found on the titles list. If you are sure you have it installed than ignore this warning."), tr("OK"));
			}
			else if(Settings.LoaderIOS == 254)
			{
				WindowPrompt(tr("Warning:"), tr("This IOS is the BootMii ios. If you are sure it is not BootMii and you have something else installed there than ignore this warning."), tr("OK"));
			}
		}
	}

	//! Settings: Game's IOS
	else if (ret == ++Idx)
	{
		if(!Settings.godmode)
			return MENU_NONE;

		char entered[4];
		snprintf(entered, sizeof(entered), "%i", Settings.cios);
		if(OnScreenNumpad(entered, sizeof(entered)))
		{
			Settings.cios = LIMIT(atoi(entered), 200, 255);

			if(NandTitles.IndexOf(TITLE_ID(1, Settings.cios)) < 0)
			{
				WindowPrompt(tr("Warning:"), tr("This IOS was not found on the titles list. If you are sure you have it installed than ignore this warning."), tr("OK"));
			}
			else if(Settings.cios == 254)
			{
				WindowPrompt(tr("Warning:"), tr("This IOS is the BootMii ios. If you are sure it is not BootMii and you have something else installed there than ignore this warning."), tr("OK"));
			}
		}
	}

	//! Settings: Quick Boot
	else if (ret == ++Idx)
	{
		if (++Settings.quickboot >= MAX_ON_OFF) Settings.quickboot = 0;
	}

	//! Settings: Block IOS Reload
	else if (ret == ++Idx )
	{
		if (++Settings.BlockIOSReload >= 3) Settings.BlockIOSReload = 0;
	}

	//! Settings: Return To
	else if (ret == ++Idx)
	{
		char tidChar[10];
		bool getChannel = TitleSelector(tidChar);
		if (getChannel)
			snprintf(Settings.returnTo, sizeof(Settings.returnTo), "%s", tidChar);
	}

	//! Settings: Nand Emulation
	else if (ret == ++Idx )
	{
		if(!IosLoader::IsD2X(Settings.cios))
			WindowPrompt(tr("Error:"), tr("Nand Emulation is only available on D2X cIOS!"), tr("OK"));
		else if (++Settings.NandEmuMode >= 3) Settings.NandEmuMode = 0;
	}

	//! Settings: Nand Chan. Emulation
	else if (ret == ++Idx )
	{
		// If NandEmuPath is on root of the first FAT32 partition, allow rev17-21 cIOS for EmuNAND Channels
		bool NandEmu_compatible = false;
		NandEmu_compatible = IosLoader::is_NandEmu_compatible(Settings.NandEmuChanPath);

		if(!IosLoader::IsD2X(Settings.cios) && !NandEmu_compatible)
			WindowPrompt(tr("Error:"), tr("Nand Emulation is only available on D2X cIOS!"), tr("OK"));
		else if (++Settings.NandEmuChanMode >= 3) Settings.NandEmuChanMode = 1;
	}

	//! Settings: Hooktype
	else if (ret == ++Idx )
	{
		if (++Settings.Hooktype >= 8) Settings.Hooktype = 0;
	}

	//! Settings: Wiird Debugger
	else if (ret == ++Idx )
	{
		if (++Settings.WiirdDebugger >= MAX_ON_OFF) Settings.WiirdDebugger = 0;
	}

	//! Settings: Wiird Debugger Pause on Start
	else if (ret == ++Idx )
	{
		if (++Settings.WiirdDebuggerPause >= MAX_ON_OFF) Settings.WiirdDebuggerPause = 0;
	}

	//! Settings: Channel Launcher
	else if (ret == ++Idx )
	{
		if (++Settings.UseChanLauncher >= MAX_ON_OFF) Settings.UseChanLauncher = 0;
	}

	//! Settings: TITLE - GameCube Settings
	else if (ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: GameCube Source
	else if (ret == ++Idx)
	{
		if (++Settings.GameCubeSource >= CG_SOURCE_MAX_CHOICE) Settings.GameCubeSource = 0;
	}

	//! Settings: GameCube Mode
	else if (ret == ++Idx)
	{
		if (++Settings.GameCubeMode >= CG_MODE_MAX_CHOICE) Settings.GameCubeMode = 0;
	}

	//! Settings: TITLE - GameCube DM(L) + Nintendont
	else if (ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: DML + NIN Video Mode
	else if (ret == ++Idx)
	{
		Settings.DMLVideo++;
		if(Settings.DMLVideo == DML_VIDEO_FORCE_PATCH) // Skip Force Patch
			Settings.DMLVideo++;
		if(Settings.DMLVideo >= DML_VIDEO_MAX_CHOICE) Settings.DMLVideo = 0;
	}

	//! Settings: DML + NIN Progressive Patch
	else if (ret == ++Idx)
	{
		if (++Settings.DMLProgPatch >= MAX_ON_OFF) Settings.DMLProgPatch = 0;
	}

	//! Settings: DML + NIN Force Widescreen
	else if (ret == ++Idx)
	{
		if (++Settings.DMLWidescreen >= MAX_ON_OFF) Settings.DMLWidescreen = 0;
	}

	//! Settings: DML + NIN Debug
	else if (ret == ++Idx)
	{
		if (++Settings.DMLDebug >= 3) Settings.DMLDebug = 0;
	}

	//! Settings: DML + NIN MultiDiscPrompt
	else if (ret == ++Idx)
	{
		if (++Settings.MultiDiscPrompt >= MAX_ON_OFF) Settings.MultiDiscPrompt = 0;
	}

	//! Settings: TITLE - GameCube DIOS MIOS (Lite)
	else if (ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: DML + NIN NMM Mode
	else if (ret == ++Idx)
	{
		if (++Settings.DMLNMM >= 3) Settings.DMLNMM = 0;
	}

	//! Settings: DML PAD Hook
	else if (ret == ++Idx)
	{
		if (++Settings.DMLPADHOOK >= MAX_ON_OFF) Settings.DMLPADHOOK = 0;
	}

	//! Settings: DML Extended No Disc
	else if (ret == ++Idx)
	{
		if (++Settings.DMLNoDisc2 >= MAX_ON_OFF) Settings.DMLNoDisc2 = 0;
	}

	//! Settings: DML Screenshot
	else if (ret == ++Idx)
	{
		if (++Settings.DMLScreenshot >= MAX_ON_OFF) Settings.DMLScreenshot = 0;
	}

	//! Settings: DML LED Activity
	else if (ret == ++Idx)
	{
		if (++Settings.DMLActivityLED >= MAX_ON_OFF) Settings.DMLActivityLED = 0;
	}

	//! Settings: DML Japanese Patch
	else if (ret == ++Idx)
	{
		if (++Settings.DMLJPNPatch >= MAX_ON_OFF) Settings.DMLJPNPatch = 0;
	}

	//! Settings: TITLE - Nintendont
	else if (ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: NIN Auto Boot
	else if (ret == ++Idx)
	{
		if (++Settings.NINAutoboot >= MAX_ON_OFF) Settings.NINAutoboot = 0;
	}

	//! Settings: NIN Nincfg.bin file
	else if (ret == ++Idx)
	{
		if (++Settings.NINSettings > AUTO) Settings.NINSettings = 0;
	}

	//! Settings: NIN Video Deflicker
	else if (ret == ++Idx)
	{
		if (++Settings.NINDeflicker >= MAX_ON_OFF) Settings.NINDeflicker = 0;
	}

	//! Settings: NIN Memory Card Emulation
	else if (ret == ++Idx)
	{
		if (++Settings.NINMCEmulation >= NIN_MC_MAX_CHOICE) Settings.NINMCEmulation = 0;
	}

	//! Settings: NIN Memory Card Blocks Size
	else if (ret == ++Idx)
	{
		if (++Settings.NINMCSize >= 6) Settings.NINMCSize = 0;
		if (Settings.NINMCSize == 5)
			WindowPrompt(tr("Warning:"), tr("Memory Card with 2043 blocs has issues with Nintendont. Use at your own risk."), tr("Ok"));
	}

	//! Settings: NIN USB-HID controller
	else if (ret == ++Idx)
	{
		if (++Settings.NINUSBHID >= MAX_ON_OFF) Settings.NINUSBHID = 0;
	}

	//! Settings: NIN MaxPads - Number of Gamecube controllers
	else if (ret == ++Idx)
	{
		if (++Settings.NINMaxPads >= 5) Settings.NINMaxPads = 0;
	}

	//! Settings: NIN Native Controller
	else if (ret == ++Idx)
	{
		if (++Settings.NINNativeSI >= MAX_ON_OFF) Settings.NINNativeSI = 0;
	}

	//! Settings: WiiU Widescreen
	else if (ret == ++Idx)
	{
		if (++Settings.NINWiiUWide >= MAX_ON_OFF) Settings.NINWiiUWide = 0;
	}

	//! Settings: NIN LED Activity
	else if (ret == ++Idx)
	{
		if (++Settings.NINLED >= MAX_ON_OFF) Settings.NINLED = 0;
	}

	//! Settings: NIN OS Report
	else if (ret == ++Idx)
	{
		if (++Settings.NINOSReport >= MAX_ON_OFF) Settings.NINOSReport = 0;
	}

	//! Settings: NIN Log to file
	else if (ret == ++Idx)
	{
		if (++Settings.NINLog >= MAX_ON_OFF) Settings.NINLog = 0;
	}

	//! Settings: TITLE - Devolution
	else if (ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: DEVO Memory Card Emulation
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOMCEmulation >= DEVO_MC_MAX_CHOICE) Settings.DEVOMCEmulation = 0;
	}

	//! Settings: DEVO Widescreen Patch
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOWidescreen >= MAX_ON_OFF) Settings.DEVOWidescreen = 0;
	}

	//! Settings: DEVO Activity LED
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOActivityLED >= MAX_ON_OFF) Settings.DEVOActivityLED = 0;
	}

	//! Settings: DEVO F-Zero AX unlock patch
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOFZeroAX >= MAX_ON_OFF) Settings.DEVOFZeroAX = 0;
	}

	//! Settings: DEVO Timer Fix
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOTimerFix >= MAX_ON_OFF) Settings.DEVOTimerFix = 0;
	}

	//! Settings: DEVO Direct Button Mapping
	else if (ret == ++Idx)
	{
		if (++Settings.DEVODButtons >= MAX_ON_OFF) Settings.DEVODButtons = 0;
	}

	//! Settings: DEVO Crop Overscan
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOCropOverscan >= MAX_ON_OFF) Settings.DEVOCropOverscan = 0;
	}

	//! Settings: DEVO Disc Read Delay
	else if (ret == ++Idx)
	{
		if (++Settings.DEVODiscDelay >= MAX_ON_OFF) Settings.DEVODiscDelay = 0;
	}

	SetOptionValues();

	return MENU_NONE;
}

