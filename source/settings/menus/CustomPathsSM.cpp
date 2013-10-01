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
#include "Channels/channels.h"
#include "Controls/DeviceHandler.hpp"
#include "GameCube/GCGames.h"
#include "CustomPathsSM.hpp"
#include "settings/SettingsPrompts.h"
#include "settings/CSettings.h"
#include "settings/SettingsEnums.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "prompts/filebrowser.h"
#include "themes/CTheme.h"
#include "gecko.h"

CustomPathsSM::CustomPathsSM()
	: SettingsMenu(tr("Custom Paths"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;
	Options->SetName(Idx++, tr("3D Cover Path"));
	Options->SetName(Idx++, tr("2D Cover Path"));
	Options->SetName(Idx++, tr("Full Cover Path"));
	Options->SetName(Idx++, tr("Disc Artwork Path"));
	Options->SetName(Idx++, tr("Theme Path"));
	Options->SetName(Idx++, tr("GameTDB Path"));
	Options->SetName(Idx++, tr("Update Path"));
	Options->SetName(Idx++, tr("GCT Cheatcodes Path"));
	Options->SetName(Idx++, tr("TXT Cheatcodes Path"));
	Options->SetName(Idx++, tr("DOL Path"));
	Options->SetName(Idx++, tr("Homebrew Apps Path"));
	Options->SetName(Idx++, tr("BCA Codes Path"));
	Options->SetName(Idx++, tr("WIP Patches Path"));
	Options->SetName(Idx++, tr("Languagefiles Path"));
	Options->SetName(Idx++, tr("WDM Files Path"));
	Options->SetName(Idx++, tr("Wiinnertag Path"));
	Options->SetName(Idx++, tr("Nand Emu Path"));
	Options->SetName(Idx++, tr("Nand Emu Channel Path"));
	Options->SetName(Idx++, tr("Main GameCube Path"));
	Options->SetName(Idx++, tr("SD GameCube Path"));
	Options->SetName(Idx++, tr("Devolution Loader Path"));
	Options->SetName(Idx++, tr("Nintendont Loader Path"));
	Options->SetName(Idx++, tr("Cache BNR Files Path"));

	SetOptionValues();
}

void CustomPathsSM::SetOptionValues()
{
	int Idx = 0;

	//! Settings: 3D Cover Path
	Options->SetValue(Idx++, Settings.covers_path);

	//! Settings: 2D Cover Path
	Options->SetValue(Idx++, Settings.covers2d_path);

	//! Settings: Full Cover Path
	Options->SetValue(Idx++, Settings.coversFull_path);

	//! Settings: Disc Artwork Path
	Options->SetValue(Idx++, Settings.disc_path);

	//! Settings: Theme Path
	Options->SetValue(Idx++, Settings.theme_path);

	//! Settings: GameTDB Path
	Options->SetValue(Idx++, Settings.titlestxt_path);

	//! Settings: Update Path
	Options->SetValue(Idx++, Settings.update_path);

	//! Settings: GCT Cheatcodes Path
	Options->SetValue(Idx++, Settings.Cheatcodespath);

	//! Settings: TXT Cheatcodes Path
	Options->SetValue(Idx++, Settings.TxtCheatcodespath);

	//! Settings: DOL Path
	Options->SetValue(Idx++, Settings.dolpath);

	//! Settings: Homebrew Apps Path
	Options->SetValue(Idx++, Settings.homebrewapps_path);

	//! Settings: BCA Codes Path
	Options->SetValue(Idx++, Settings.BcaCodepath);

	//! Settings: WIP Patches Path
	Options->SetValue(Idx++, Settings.WipCodepath);

	//! Settings: Languagefiles Path
	Options->SetValue(Idx++, Settings.languagefiles_path);

	//! Settings: WDM Files Path
	Options->SetValue(Idx++, Settings.WDMpath);

	//! Settings: Wiinnertag Path
	Options->SetValue(Idx++, Settings.WiinnertagPath);

	//! Settings: Nand Emu Path
	Options->SetValue(Idx++, Settings.NandEmuPath);

	//! Settings: Nand Emu Channel Path
	Options->SetValue(Idx++, Settings.NandEmuChanPath);

	//! Settings: GameCube Games Path
	Options->SetValue(Idx++, Settings.GameCubePath);

	//! Settings: SD GameCube Games Path
	Options->SetValue(Idx++, Settings.GameCubeSDPath);

	//! Settings: GameCube Devolution loader.bin Path
	Options->SetValue(Idx++, Settings.DEVOLoaderPath);

	//! Settings: GameCube Nintendont boot.dol Path
	Options->SetValue(Idx++, Settings.NINLoaderPath);

	//! Settings: Cache BNR Files Path
	Options->SetValue(Idx++, Settings.BNRCachePath);
}

int CustomPathsSM::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: 3D Cover Path
	if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "3D Cover Path" ));
		ChangePath(Settings.covers_path, sizeof(Settings.covers_path));
	}

	//! Settings: 2D Cover Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "2D Cover Path" ));
		ChangePath(Settings.covers2d_path, sizeof(Settings.covers2d_path));
	}

	//! Settings: Full Cover Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Full Cover Path" ));
		ChangePath(Settings.coversFull_path, sizeof(Settings.coversFull_path));
	}

	//! Settings: Disc Artwork Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Disc Artwork Path" ));
		ChangePath(Settings.disc_path, sizeof(Settings.disc_path));
	}

	//! Settings: Theme Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Theme Path" ));
		ChangePath(Settings.theme_path, sizeof(Settings.theme_path));
	}

	//! Settings: GameTDB Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "GameTDB Path" ));
		ChangePath(Settings.titlestxt_path, sizeof(Settings.titlestxt_path));
	}

	//! Settings: Update Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Update Path" ));
		ChangePath(Settings.update_path, sizeof(Settings.update_path));
	}

	//! Settings: GCT Cheatcodes Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "GCT Cheatcodes Path" ));
		ChangePath(Settings.Cheatcodespath, sizeof(Settings.Cheatcodespath));
	}

	//! Settings: TXT Cheatcodes Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "TXT Cheatcodes Path" ));
		ChangePath(Settings.TxtCheatcodespath, sizeof(Settings.TxtCheatcodespath));
	}

	//! Settings: DOL Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "DOL Path" ));
		ChangePath(Settings.dolpath, sizeof(Settings.dolpath));
	}

	//! Settings: Homebrew Apps Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Homebrew Apps Path" ));
		ChangePath(Settings.homebrewapps_path, sizeof(Settings.homebrewapps_path));
	}

	//! Settings: BCA Codes Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "BCA Codes Path" ));
		ChangePath(Settings.BcaCodepath, sizeof(Settings.BcaCodepath));
	}

	//! Settings: WIP Patches Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "WIP Patches Path" ));
		ChangePath(Settings.WipCodepath, sizeof(Settings.WipCodepath));
	}

	//! Settings: Languagefiles Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Languagefiles Path" ));
		ChangePath(Settings.languagefiles_path, sizeof(Settings.languagefiles_path));
	}

	//! Settings: WDM Files Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "WDM Files Path" ));
		ChangePath(Settings.WDMpath, sizeof(Settings.WDMpath));
	}

	//! Settings: Wiinnertag Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Wiinnertag Path" ));
		ChangePath(Settings.WiinnertagPath, sizeof(Settings.WiinnertagPath));
	}

	//! Settings: Nand Emu Path
	else if (ret == ++Idx)
	{
		char oldPath[sizeof(Settings.NandEmuPath)];
		snprintf(oldPath, sizeof(oldPath), Settings.NandEmuPath);

		titleTxt->SetText(tr( "Nand Emu Path" ));
		ChangePath(Settings.NandEmuPath, sizeof(Settings.NandEmuPath));
		if(strncasecmp(DeviceHandler::PathToFSName(Settings.NandEmuPath), "FAT", 3) != 0)
		{
			snprintf(Settings.NandEmuPath, sizeof(Settings.NandEmuPath), oldPath);
			WindowPrompt(tr("Error:"), tr("Nand Emulation only works on FAT/FAT32 partitions!"), tr("OK"));
		}
	}

	//! Settings: Nand Emu Channel Path
	else if (ret == ++Idx)
	{
		char oldPath[sizeof(Settings.NandEmuChanPath)];
		snprintf(oldPath, sizeof(oldPath), Settings.NandEmuChanPath);

		titleTxt->SetText(tr( "Nand Emu Channel Path" ));
		int result = ChangePath(Settings.NandEmuChanPath, sizeof(Settings.NandEmuChanPath));
		if(strncasecmp(DeviceHandler::PathToFSName(Settings.NandEmuChanPath), "FAT", 3) != 0)
		{
			snprintf(Settings.NandEmuChanPath, sizeof(Settings.NandEmuChanPath), oldPath);
			WindowPrompt(tr("Error:"), tr("Nand Emulation only works on FAT/FAT32 partitions!"), tr("OK"));
		}
		else if(result == 1)
		{
			Channels::Instance()->GetEmuChannelList();
		}
	}

	//! Settings: GameCube Games Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Main GameCube Games Path" ));
		if(ChangePath(Settings.GameCubePath, sizeof(Settings.GameCubePath)))
		{
			GCGames::Instance()->LoadAllGames();
		}
	}

	//! Settings: SD GameCube Games Path
	else if (ret == ++Idx)
	{
		char tmp_path[sizeof(Settings.GameCubeSDPath)];
		snprintf(tmp_path, sizeof(tmp_path), "%s", Settings.GameCubeSDPath);

		titleTxt->SetText(tr( "SD GameCube Games Path" ));
		if(ChangePath(tmp_path, sizeof(tmp_path)))
		{
			if(strncmp(tmp_path, "sd", 2) != 0)
			{
				WindowPrompt(tr("Error:"), tr("This path must be on SD!"), tr("OK"));
			}
			else
			{
				snprintf(Settings.GameCubeSDPath, sizeof(Settings.GameCubeSDPath), "%s", tmp_path);
				GCGames::Instance()->LoadAllGames();
			}
		}
	}

	//! Settings: GameCube Devolution loader.bin path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Devolution Loader Path" ));
		ChangePath(Settings.DEVOLoaderPath, sizeof(Settings.DEVOLoaderPath));
	}

	//! Settings: GameCube Nintendont boot.dol path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Nintendont Loader Path" ));
		ChangePath(Settings.NINLoaderPath, sizeof(Settings.NINLoaderPath));
	}

	//! Settings: Cache BNR Files Path
	else if (ret == ++Idx)
	{
		titleTxt->SetText(tr( "Cache BNR Files Path" ));
		ChangePath(Settings.BNRCachePath, sizeof(Settings.BNRCachePath));
	}

	//! Global set back of the titleTxt after a change
	titleTxt->SetText(tr( "Custom Paths" ));
	SetOptionValues();

	return MENU_NONE;
}

int CustomPathsSM::ChangePath(char * SettingsPath, int SizeOfPath)
{
	char entered[300];
	snprintf(entered, sizeof(entered), SettingsPath);

	HaltGui();
	GuiWindow * parent = (GuiWindow *) parentElement;
	if(parent) parent->SetState(STATE_DISABLED);
	this->SetState(STATE_DEFAULT);
	this->Remove(optionBrowser);
	ResumeGui();

	int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);

	if(parent) parent->SetState(STATE_DEFAULT);
	this->Append(optionBrowser);

	if (result == 1)
	{
		if (entered[strlen(entered)-1] != '/')
			strcat(entered, "/");

		snprintf(SettingsPath, SizeOfPath, entered);
		WindowPrompt(tr( "Path Changed" ), 0, tr( "OK" ));
	}

	return result;
}
