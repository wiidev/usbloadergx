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
#include "menu.h"

static const char * loaderModeText[] =
{
	trNOOP( "None" ),
	trNOOP( "Wii Games" ),
	trNOOP( "Nand Channels" ),
	trNOOP( "EmuNand Channels" ),
	trNOOP( "All" )
};

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

LoaderSettings::LoaderSettings()
	: SettingsMenu(tr("Loader Settings"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;

	Options->SetName(Idx++, "%s", tr( "Loader Mode" ));
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

	//! Settings: Loader Mode
	if(Settings.LoaderMode == MODE_NONE) Options->SetValue(Idx++, "%s", tr(loaderModeText[0]));
	else if(Settings.LoaderMode == MODE_ALL) Options->SetValue(Idx++, "%s", tr(loaderModeText[4]));
	else if(Settings.LoaderMode & MODE_WIIGAMES) Options->SetValue(Idx++, "%s", tr(loaderModeText[1]));
	else if(Settings.LoaderMode & MODE_NANDCHANNELS) Options->SetValue(Idx++, "%s", tr(loaderModeText[2]));
	else if(Settings.LoaderMode & MODE_EMUCHANNELS) Options->SetValue(Idx++, "%s", tr(loaderModeText[3]));

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
}

int LoaderSettings::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: Loader Mode
	if (ret == ++Idx)
	{
		if (Settings.LoaderMode == MODE_ALL) Settings.LoaderMode = MODE_NONE;
		else if (Settings.LoaderMode & MODE_WIIGAMES) Settings.LoaderMode = MODE_NANDCHANNELS;
		else if (Settings.LoaderMode & MODE_NANDCHANNELS) Settings.LoaderMode = MODE_EMUCHANNELS;
		else if (Settings.LoaderMode & MODE_EMUCHANNELS) Settings.LoaderMode = MODE_ALL;
		else Settings.LoaderMode = MODE_WIIGAMES;
	}

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
		if(OnScreenKeyboard(entered, sizeof(entered), 0))
		{
			Settings.cios = atoi(entered);
			if(Settings.cios < 200) Settings.cios = 200;
			else if(Settings.cios > 255) Settings.cios = 255;

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

	SetOptionValues();

	return MENU_NONE;
}

