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
#include "ParentalControlSM.hpp"
#include "settings/CSettings.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "utils/PasswordCheck.h"

static const char * OnOffText[MAX_ON_OFF] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" )
};

static const char * LockModeText[] =
{
	trNOOP( "Locked" ),
	trNOOP( "Unlocked" )
};

static const char * ParentalText[5] =
{
	trNOOP( "0 (Everyone)" ),
	trNOOP( "1 (Child 7+)" ),
	trNOOP( "2 (Teen 12+)" ),
	trNOOP( "3 (Mature 16+)" ),
	trNOOP( "4 (Adults Only 18+)" )
};

ParentalControlSM::ParentalControlSM()
	: SettingsMenu(tr("Parental Control"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;
	Options->SetName(Idx++, "%s", tr( "Console" ));
	Options->SetName(Idx++, "%s", tr( "Password" ));
	Options->SetName(Idx++, "%s", tr( "Controllevel" ));
	Options->SetName(Idx++, "%s", tr( "Remember Unlock" ));
	Options->SetName(Idx++, "%s", tr( "Block Global Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Gui Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Loader Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Hard Drive Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Feature Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Parental Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Sound Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Theme Downloader" ));
	Options->SetName(Idx++, "%s", tr( "Block Theme Menu" ));
	Options->SetName(Idx++, "%s", tr( "Block Custom Paths" ));
	Options->SetName(Idx++, "%s", tr( "Block Updates" ));
	Options->SetName(Idx++, "%s", tr( "Block Reset Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block Game Settings" ));
	Options->SetName(Idx++, "%s", tr( "Block HBC Menu" ));
	Options->SetName(Idx++, "%s", tr( "Block Title Launcher" ));
	Options->SetName(Idx++, "%s", tr( "Block Cover Downloads" ));
	Options->SetName(Idx++, "%s", tr( "Block Game Install" ));
	Options->SetName(Idx++, "%s", tr( "Block GameID Change" ));
	Options->SetName(Idx++, "%s", tr( "Block Categories Menu" ));
	Options->SetName(Idx++, "%s", tr( "Block Categories Modify" ));
	Options->SetName(Idx++, "%s", tr( "Block SD Reload Button" ));
	Options->SetName(Idx++, "%s", tr( "Block Priiloader Override" ));
	Options->SetName(Idx++, "%s", tr( "Block Loader Mode Button" ));
	Options->SetName(Idx++, "%s", tr( "Block Loader Layout Button" ));

	SetOptionValues();
}

void ParentalControlSM::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Console
	Options->SetValue(Idx++, "%s", tr( LockModeText[Settings.godmode] ));

	if(!Settings.godmode)
	{
		for(int i = Idx; i < Options->GetLength(); ++i)
			Options->SetValue(i, "********");

		return;
	}

	//! Settings: Password
	if (strcmp(Settings.unlockCode, "") == 0)
		Options->SetValue(Idx++, "%s", tr( "not set" ));
	else
		Options->SetValue(Idx++, Settings.unlockCode);

	//! Settings: Controllevel
	Options->SetValue(Idx++, "%s", tr(ParentalText[Settings.parentalcontrol]));

	//! Settings: Remember Unlock
	Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.RememberUnlock]));

	//! Settings: Block Global Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_GLOBAL_SETTINGS) != 0)]));

	//! Settings: Block Gui Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_GUI_SETTINGS) != 0)]));

	//! Settings: Block Loader Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_LOADER_SETTINGS) != 0)]));

	//! Settings: Block Hard Drive Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_HARD_DRIVE_SETTINGS) != 0)]));

	//! Settings: Block Feature Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_FEATURE_SETTINGS) != 0)]));

	//! Settings: Block Parental Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_PARENTAL_SETTINGS) != 0)]));

	//! Settings: Block Sound Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_SOUND_SETTINGS) != 0)]));

	//! Settings: Block Theme Downloader
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_THEME_DOWNLOADER) != 0)]));

	//! Settings: Block Theme Menu
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_THEME_MENU) != 0)]));

	//! Settings: Block Custom Paths
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_CUSTOMPATH_SETTINGS) != 0)]));

	//! Settings: Block Updates
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_UPDATES) != 0)]));

	//! Settings: Block Reset Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_RESET_SETTINGS) != 0)]));

	//! Settings: Block Game Settings
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_GAME_SETTINGS) != 0)]));

	//! Settings: Block HBC Menu
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_HBC_MENU) != 0)]));

	//! Settings: Block Title Launcher
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_TITLE_LAUNCHER_MENU) != 0)]));

	//! Settings: Block Cover Downloads
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_COVER_DOWNLOADS) != 0)]));

	//! Settings: Block Game Install
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_GAME_INSTALL) != 0)]));

	//! Settings: Block GameID Change
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_GAMEID_CHANGE) != 0)]));

	//! Settings: Block Categories Menu
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_CATEGORIES_MENU) != 0)]));

	//! Settings: Block Categories Modify
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD) != 0)]));

	//! Settings: Block SD Reload Button
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_SD_RELOAD_BUTTON) != 0)]));

	//! Settings: Block Priiloader Override
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_PRIILOADER_OVERRIDE) != 0)]));

	//! Settings: Block Loader Mode Button
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_LOADER_MODE_BUTTON) != 0)]));

	//! Settings: Block Loader Layout Button
	Options->SetValue(Idx++, "%s", tr(OnOffText[((Settings.ParentalBlocks & BLOCK_LOADER_LAYOUT_BUTTON) != 0)]));
}

int ParentalControlSM::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: Console
	if (ret == ++Idx)
	{
		if (!Settings.godmode)
		{
			//password check to unlock Install,Delete and Format
			SetState(STATE_DISABLED);
			int result = PasswordCheck(Settings.unlockCode);
			SetState(STATE_DEFAULT);
			if (result > 0)
			{
				if(result == 1)
					WindowPrompt( tr( "Correct Password" ), tr( "All the features of USB Loader GX are unlocked." ), tr( "OK" ));
				Settings.godmode = 1;
			}
			else if(result < 0)
				WindowPrompt(tr( "Wrong Password" ), tr( "USB Loader GX is protected" ), tr( "OK" ));
		}
		else
		{
			int choice = WindowPrompt(tr( "Lock Console" ), tr( "Are you sure?" ), tr( "Yes" ), tr( "No" ));
			if (choice == 1)
			{
				WindowPrompt(tr( "Console Locked" ), tr( "USB Loader GX is protected" ), tr( "OK" ));
				Settings.godmode = 0;
			}
		}
	}

	//! General permission check for all following
	else if(!Settings.godmode)
	{
		WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
	}

	//! Settings: Password
	else if (ret == ++Idx)
	{
		char entered[20];
		SetState(STATE_DISABLED);
		snprintf(entered, sizeof(entered), Settings.unlockCode);
		int result = OnScreenKeyboard(entered, 20, 0);
		SetState(STATE_DEFAULT);
		if (result == 1)
		{
			snprintf(Settings.unlockCode, sizeof(Settings.unlockCode), entered);
			WindowPrompt(tr( "Password Changed" ), tr( "Password has been changed" ), tr( "OK" ));
		}
	}

	//! Settings: Controllevel
	else if (ret == ++Idx)
	{
		if (++Settings.parentalcontrol >= 5) Settings.parentalcontrol = 0;
	}

	//! Settings: Remember Unlock
	else if (ret == ++Idx)
	{
		if (++Settings.RememberUnlock >= MAX_ON_OFF) Settings.RememberUnlock = 0;
	}

	//! Settings: Block Global Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_GLOBAL_SETTINGS;
	}

	//! Settings: Block Gui Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_GUI_SETTINGS;
	}

	//! Settings: Block Loader Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_LOADER_SETTINGS;
	}

	//! Settings: Hard Drive Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_HARD_DRIVE_SETTINGS;
	}

	//! Settings: Block Feature Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_FEATURE_SETTINGS;
	}

	//! Settings: Block Parental Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_PARENTAL_SETTINGS;
	}

	//! Settings: Block Sound Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_SOUND_SETTINGS;
	}

	//! Settings: Block Theme Downloader
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_THEME_DOWNLOADER;
	}

	//! Settings: Block Theme Menu
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_THEME_MENU;
	}

	//! Settings: Block Custom Paths
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_CUSTOMPATH_SETTINGS;
	}

	//! Settings: Block Updates
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_UPDATES;
	}

	//! Settings: Block Reset Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_RESET_SETTINGS;
	}

	//! Settings: Block Game Settings
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_GAME_SETTINGS;
	}

	//! Settings: Block HBC Menu
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_HBC_MENU;
	}

	//! Settings: Block Title Launcher
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_TITLE_LAUNCHER_MENU;
	}

	//! Settings: Block Cover Downloads
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_COVER_DOWNLOADS;
	}

	//! Settings: Block Game Install
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_GAME_INSTALL;
	}

	//! Settings: Block GameID Change
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_GAMEID_CHANGE;
	}

	//! Settings: Block Categories Menu
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_CATEGORIES_MENU;
	}

	//! Settings: Block Categories Modify
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_CATEGORIES_MOD;
	}

	//! Settings: Block SD Reload Button
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_SD_RELOAD_BUTTON;
	}

	//! Settings: Block Priiloader Override
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_PRIILOADER_OVERRIDE;
	}

	//! Settings: Block Loader Mode Button
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_LOADER_MODE_BUTTON;
	}

	//! Settings: Block Loader Layout Button
	else if (ret == ++Idx)
	{
		Settings.ParentalBlocks ^= BLOCK_LOADER_LAYOUT_BUTTON;
	}

	SetOptionValues();

	return MENU_NONE;
}
