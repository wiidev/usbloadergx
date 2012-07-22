/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "prompts/TitleBrowser.h"
#include "system/IosLoader.h"
#include "usbloader/wbfs.h"
#include "usbloader/GameList.h"
#include "utils/tools.h"
#include "menu.h"

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

static const char * Error002Text[] =
{
	trNOOP( "No" ),
	trNOOP( "Yes" ),
	trNOOP( "Anti" )
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
};

static const char * DMLVerText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "v1.2 -> v2.1" ),
	trNOOP( "v2.2+" ),
};

static const char * DMLVideoText[] =
{
	trNOOP( "DML Auto" ),
	trNOOP( "Use Game Settings" ),
	trNOOP( "DML None" ),
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

LoaderSettings::LoaderSettings()
	: SettingsMenu(tr("Loader Settings"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;

	Options->SetName(Idx++, "%s", tr( "Video Mode" ));
	Options->SetName(Idx++, "%s", tr( "VIDTV Patch" ));
	Options->SetName(Idx++, "%s", tr( "Sneek Video Patch" ));
	Options->SetName(Idx++, "%s", tr( "Aspect Ratio" ));
	Options->SetName(Idx++, "%s", tr( "Game Language" ));
	Options->SetName(Idx++, "%s", tr( "Patch Country Strings" ));
	Options->SetName(Idx++, "%s", tr( "Ocarina" ));
	Options->SetName(Idx++, "%s", tr( "Boot/Standard" ));
	Options->SetName(Idx++, "%s", tr( "Quick Boot" ));
	Options->SetName(Idx++, "%s", tr( "Error 002 fix" ));
	Options->SetName(Idx++, "%s", tr( "Block IOS Reload" ));
	Options->SetName(Idx++, "%s", tr( "Return To" ));
	Options->SetName(Idx++, "%s", tr( "Nand Saves Emulation" ));
	Options->SetName(Idx++, "%s", tr( "Nand Chan. Emulation" ));
	Options->SetName(Idx++, "%s", tr( "Hooktype" ));
	Options->SetName(Idx++, "%s", tr( "Wiird Debugger" ));
	Options->SetName(Idx++, "%s", tr( "Debugger Paused Start" ));
	Options->SetName(Idx++, "%s", tr( "Channel Launcher" ));
	Options->SetName(Idx++, "%s", tr( "GameCube Mode" ));
	Options->SetName(Idx++, "%s", tr( "DML Installed Version" ));
	Options->SetName(Idx++, "%s", tr( "DML Video Mode" ));
	Options->SetName(Idx++, "%s", tr( "DML Progressive Patch" ));
	Options->SetName(Idx++, "%s", tr( "DML NMM Mode" ));
	Options->SetName(Idx++, "%s", tr( "DML LED Activity" ));
	Options->SetName(Idx++, "%s", tr( "DML PAD Hook" ));
	Options->SetName(Idx++, "%s", tr( "DML No Disc" ));
	Options->SetName(Idx++, "%s", tr( "DML No Disc+" ));
	Options->SetName(Idx++, "%s", tr( "DML Force Widescreen" ));
	Options->SetName(Idx++, "%s", tr( "DML Debug" ));
	Options->SetName(Idx++, "%s", tr( "DEVO MemCard Emulation" ));

	SetOptionValues();

	oldLoaderMode = Settings.LoaderMode;
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
}

void LoaderSettings::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Video Mode
	Options->SetValue(Idx++, "%s", tr(VideoModeText[Settings.videomode]));

	//! Settings: VIDTV Patch
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.videopatch] ));

	//! Settings: Sneek Video Patch
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.sneekVideoPatch] ));

	//! Settings: Aspect Ratio
	Options->SetValue(Idx++, "%s", tr( AspectText[Settings.GameAspectRatio] ));

	//! Settings: Game Language
	Options->SetValue(Idx++, "%s", tr( LanguageText[Settings.language] ));

	//! Settings: Patch Country Strings
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.patchcountrystrings] ));

	//! Settings: Ocarina
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.ocarina] ));

	//! Settings: Boot/Standard
	if (Settings.godmode)
		Options->SetValue(Idx++, "IOS %i", Settings.cios);
	else
		Options->SetValue(Idx++, "********");

	//! Settings: Quick Boot
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.quickboot] ));

	//! Settings: Error 002 fix
	Options->SetValue(Idx++, "%s", tr( Error002Text[Settings.error002] ));

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

	//! Settings: GameCube Mode
	Options->SetValue(Idx++, "%s", tr(GCMode[Settings.GameCubeMode]));

	//! Settings: DML Config Version
	Options->SetValue(Idx++, "%s", tr(DMLVerText[Settings.DMLConfigVersion]));

	//! Settings: DML Video Mode
	Options->SetValue(Idx++, "%s", tr(DMLVideoText[Settings.DMLVideo]));

	//! Settings: DML Progressive Patch
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLProgPatch]));

	//! Settings: DML NMM Mode
	Options->SetValue(Idx++, "%s", tr(DMLNMMMode[Settings.DMLNMM]));

	//! Settings: DML LED Activity
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLActivityLED]));

	//! Settings: DML PAD Hook
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLPADHOOK]));

	//! Settings: DML No Disc
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLNoDisc]));

	//! Settings: DML Extended No Disc
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLNoDisc2]));

	//! Settings: DML Force Widescreen
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.DMLWidescreen]));

	//! Settings: DML Debug
	Options->SetValue(Idx++, "%s", tr(DMLDebug[Settings.DMLDebug]));

	//! Settings: DEVO Memory Card Emulation
	Options->SetValue(Idx++, "%s", tr(DEVOMCText[Settings.DEVOMCEmulation]));
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

	//! Settings: VIDTV Patch
	else if (ret == ++Idx)
	{
		if (++Settings.videopatch >= MAX_ON_OFF) Settings.videopatch = 0;
	}

	//! Settings: Sneek Video Patch
	else if (ret == ++Idx )
	{
		if (++Settings.sneekVideoPatch >= MAX_ON_OFF) Settings.sneekVideoPatch = 0;
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

	//! Settings: Boot/Standard
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

	//! Settings: Error 002 fix
	else if (ret == ++Idx )
	{
		if (++Settings.error002 >= 3) Settings.error002 = 0;
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
		if(!IosLoader::IsD2X())
			WindowPrompt(tr("Error:"), tr("Nand Emulation is only available on D2X cIOS!"), tr("OK"));
		else if (++Settings.NandEmuMode >= 3) Settings.NandEmuMode = 0;
	}

	//! Settings: Nand Chan. Emulation
	else if (ret == ++Idx )
	{
		if(!IosLoader::IsD2X())
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

	//! Settings: GameCube Mode
	else if (ret == ++Idx)
	{
		if (++Settings.GameCubeMode >= CG_MODE_MAX_CHOICE) Settings.GameCubeMode = 0;
	}

	//! Settings: DML Config Version
	else if (ret == ++Idx)
	{
		if (++Settings.DMLConfigVersion > DML_VERSION) Settings.DMLConfigVersion = 1;
	}

	//! Settings: DML Video Mode
	else if (ret == ++Idx)
	{
		if (++Settings.DMLVideo >= DML_VIDEO_MAX_CHOICE) Settings.DMLVideo = 0;
	}

	//! Settings: DML Progressive Patch
	else if (ret == ++Idx)
	{
		if (++Settings.DMLProgPatch >= MAX_ON_OFF) Settings.DMLProgPatch = 0;
	}

	//! Settings: DML NMM Mode
	else if (ret == ++Idx)
	{
		if (++Settings.DMLNMM >= 3) Settings.DMLNMM = 0;
	}

	//! Settings: DML LED Activity
	else if (ret == ++Idx)
	{
		if (++Settings.DMLActivityLED >= MAX_ON_OFF) Settings.DMLActivityLED = 0;
	}

	//! Settings: DML PAD Hook
	else if (ret == ++Idx)
	{
		if (++Settings.DMLPADHOOK >= MAX_ON_OFF) Settings.DMLPADHOOK = 0;
	}

	//! Settings: DML No Disc
	else if (ret == ++Idx)
	{
		if (++Settings.DMLNoDisc >= MAX_ON_OFF) Settings.DMLNoDisc = 0;
	}

	//! Settings: DML Extended No Disc
	else if (ret == ++Idx)
	{
		if (++Settings.DMLNoDisc2 >= MAX_ON_OFF) Settings.DMLNoDisc2 = 0;
	}

	//! Settings: DML Force Widescreen
	else if (ret == ++Idx)
	{
		if (++Settings.DMLWidescreen >= MAX_ON_OFF) Settings.DMLWidescreen = 0;
	}

	//! Settings: DML Debug
	else if (ret == ++Idx)
	{
		if (++Settings.DMLDebug >= 3) Settings.DMLDebug = 0;
	}

	//! Settings: DEVO Memory Card Emulation
	else if (ret == ++Idx)
	{
		if (++Settings.DEVOMCEmulation >= DEVO_MC_MAX_CHOICE) Settings.DEVOMCEmulation = 0;
	}

	SetOptionValues();

	return MENU_NONE;
}

