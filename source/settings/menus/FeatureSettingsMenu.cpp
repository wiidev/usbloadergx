/****************************************************************************
 * Copyright (C) 2011
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
#include "FeatureSettingsMenu.hpp"
#include "Channels/channels.h"
#include "settings/CGameCategories.hpp"
#include "settings/GameTitles.h"
#include "settings/CSettings.h"
#include "settings/SettingsPrompts.h"
#include "network/Wiinnertag.h"
#include "network/networkops.h"
#include "FileOperations/fileops.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "prompts/filebrowser.h"
#include "usbloader/GameList.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "wad/wad.h"

static const char * OnOffText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" )
};

static const char * WiilightText[WIILIGHT_MAX] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Only for Install" )
};

FeatureSettingsMenu::FeatureSettingsMenu()
	: SettingsMenu(tr("Features Settings"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;
	Options->SetName(Idx++, "%s", tr( "Titles from GameTDB" ));
	Options->SetName(Idx++, "%s", tr( "Cache Titles" ));
	Options->SetName(Idx++, "%s", tr( "Wiilight" ));
	Options->SetName(Idx++, "%s", tr( "Rumble" ));
	Options->SetName(Idx++, "%s", tr( "AutoInit Network" ));
	Options->SetName(Idx++, "%s", tr( "Messageboard Update" ));
	Options->SetName(Idx++, "%s", tr( "Wiinnertag" ));
	Options->SetName(Idx++, "%s", tr( "Import Categories" ));
	Options->SetName(Idx++, "%s", tr( "Export All Saves to EmuNand" ));
	Options->SetName(Idx++, "%s", tr( "Dump NAND to EmuNand" ));
	Options->SetName(Idx++, "%s", tr( "Install WAD to EmuNand" ));

	OldTitlesOverride = Settings.titlesOverride;
	OldCacheTitles = Settings.CacheTitles;

	SetOptionValues();
}

FeatureSettingsMenu::~FeatureSettingsMenu()
{
	if (   Settings.titlesOverride != OldTitlesOverride
		|| Settings.CacheTitles != OldCacheTitles)
	{
		//! Remove cached titles and reload new titles
		GameTitles.SetDefault();
		if(Settings.titlesOverride) {
			GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path);
		}
		else
		{
			//! Don't override titles, in other words read them from disc header or directory names
			gameList.ReadGameList();
			gameList.LoadUnfiltered();
		}
	}
}

void FeatureSettingsMenu::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Titles from GameTDB
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.titlesOverride] ));

	//! Settings: Cache Titles
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.CacheTitles] ));

	//! Settings: Wiilight
	Options->SetValue(Idx++, "%s", tr( WiilightText[Settings.wiilight] ));

	//! Settings: Rumble
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.rumble] ));

	//! Settings: AutoInit Network
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.autonetwork] ));

	//! Settings: Messageboard Update
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.PlaylogUpdate] ));

	//! Settings: Wiinnertag
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.Wiinnertag] ));

	//! Settings: Import categories from GameTDB
	Options->SetValue(Idx++, " ");

	//! Settings: Export Savegames to EmuNand
	Options->SetValue(Idx++, " ");

	//! Settings: Dump NAND to EmuNand
	Options->SetValue(Idx++, " ");

	//! Settings: Install WAD to EmuNand
	Options->SetValue(Idx++, " ");
}

int FeatureSettingsMenu::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;


	//! Settings: Titles from GameTDB
	if (ret == ++Idx)
	{
		if (++Settings.titlesOverride >= MAX_ON_OFF) Settings.titlesOverride = 0;
	}

	//! Settings: Cache Titles
	else if (ret == ++Idx)
	{
		if (++Settings.CacheTitles >= MAX_ON_OFF) Settings.CacheTitles = 0;
	}

	//! Settings: Wiilight
	else if (ret == ++Idx)
	{
		if (++Settings.wiilight >= WIILIGHT_MAX) Settings.wiilight = 0;
	}

	//! Settings: Rumble
	else if (ret == ++Idx)
	{
		if (++Settings.rumble >= MAX_ON_OFF) Settings.rumble = 0; //RUMBLE
	}

	//! Settings: AutoInit Network
	else if (ret == ++Idx)
	{
		if (++Settings.autonetwork >= MAX_ON_OFF) Settings.autonetwork = 0;
	}

	//! Settings: Messageboard Update
	else if (ret == ++Idx )
	{
		if (++Settings.PlaylogUpdate >= MAX_ON_OFF) Settings.PlaylogUpdate = 0;
	}

	//! Settings: Winnertag
	else if (ret == ++Idx)
	{
		if (++Settings.Wiinnertag >= MAX_ON_OFF) Settings.Wiinnertag = 0;

		if(Settings.Wiinnertag == ON && !Settings.autonetwork)
		{
			int choice = WindowPrompt(tr("Warning"), tr("Wiinnertag requires you to enable automatic network connect on application start. Do you want to enable it now?"), tr("Yes"), tr("Cancel"));
			if(choice)
			{
				Settings.autonetwork = ON;
				if(!IsNetworkInit())
					Initialize_Network();
			}
		}

		char filepath[200];
		snprintf(filepath, sizeof(filepath), "%sWiinnertag.xml", Settings.WiinnertagPath);

		if(Settings.Wiinnertag == ON && !CheckFile(filepath))
		{
			int choice = WindowPrompt(tr("Warning"), tr("No Wiinnertag.xml found in the config path. Do you want an example file created?"), tr("Yes"), tr("No"));
			if(choice)
			{
				if(Wiinnertag::CreateExample(Settings.WiinnertagPath))
				{
					char text[200];
					snprintf(text, sizeof(text), "%s %s", tr("An example file was created here:"), filepath);
					WindowPrompt(tr("Success"), text, tr("OK"));
				}
				else
				{
					char text[200];
					snprintf(text, sizeof(text), "%s %s", tr("Could not write to:"), filepath);
					WindowPrompt(tr("Failed"), text, tr("OK"));
				}
			}
		}
	}

	//! Settings: Import categories from GameTDB
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr("Import Categories"), tr("Are you sure you want to import game categories from GameTDB?"), tr("Yes"), tr("Cancel"));
		if(choice)
		{
			char xmlpath[300];
			snprintf(xmlpath, sizeof(xmlpath), "%swiitdb.xml", Settings.titlestxt_path);
			if(!GameCategories.ImportFromGameTDB(xmlpath))
			{
				WindowPrompt(tr("Error"), tr("Could not open the WiiTDB.xml file."), tr("OK"));
			}
			else
			{
				GameCategories.Save();
				GameCategories.CategoryList.goToFirst();
				WindowPrompt(tr("Import Categories"), tr("Import operation successfully completed."), tr("OK"));
			}
		}
	}

	//! Settings: Export Savegames to EmuNand
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "Do you want to extract all the save games?" ), tr("The save games will be extracted to your emu nand path. Attention: All existing saves will be overwritten."), tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			ProgressCancelEnable(true);
			StartProgress(tr("Extracting files:"), 0, 0, true, false);
			char filePath[512];
			char nandPath[ISFS_MAXPATH];
			bool noErrors = true;
			bool skipErrors = false;
			wString filter(gameList.GetCurrentFilter());
			gameList.LoadUnfiltered();

			for(int i = 0; i < gameList.size(); ++i)
			{
				if(   gameList[i]->type != TYPE_GAME_WII_IMG
				   && gameList[i]->type != TYPE_GAME_NANDCHAN)
					continue;

				if(gameList[i]->tid != 0) //! Channels
				{
					snprintf(nandPath, sizeof(nandPath), "/title/%08x/%08x/data", (u32) (gameList[i]->tid  >> 32), (u32) gameList[i]->tid );
					snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuChanPath, nandPath);
				}
				else //! Wii games
				{
					snprintf(nandPath, sizeof(nandPath), "/title/00010000/%02x%02x%02x%02x", gameList[i]->id[0], gameList[i]->id[1], gameList[i]->id[2], gameList[i]->id[3]);
					snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
				}

				ShowProgress(tr("Extracting files:"), GameTitles.GetTitle(gameList[i]), 0, 0, -1, true, false);

				int ret = NandTitle::ExtractDir(nandPath, filePath);
				if(ret == PROGRESS_CANCELED)
				{
					break;
				}
				else if(ret < 0) //! Games with installable channels: Mario Kart, Wii Fit, etc.
				{
					snprintf(nandPath, sizeof(nandPath), "/title/00010004/%02x%02x%02x%02x", gameList[i]->id[0], gameList[i]->id[1], gameList[i]->id[2], gameList[i]->id[3]);
					snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
					ret = NandTitle::ExtractDir(nandPath, filePath);
				}
				if(ret < 0 && !skipErrors)
				{
					noErrors = false;
					char text[200];
					snprintf(text, sizeof(text), "%s %s. %s. %s", tr("Could not extract files for:"), GameTitles.GetTitle(gameList[i]), tr("Savegame might not exist for this game."), tr("Continue?"));

					ProgressStop();
					int ret = WindowPrompt(tr("Error"), text, tr("Yes"), tr("No"), tr("Skip Errors"));
					if(ret == 0)
						skipErrors = true;
					else if(ret == 2)
						break;
				}
			}

			ProgressStop();
			ProgressCancelEnable(false);

			if(ret != PROGRESS_CANCELED)
			{
				if(noErrors)
					WindowPrompt(tr("Success."), tr("All files extracted."), tr("OK"));
				else
					WindowPrompt(tr("Process finished."), tr("Errors occured."), tr("OK"));
			}
			gameList.FilterList(filter.c_str());
		}
	}

	//! Settings: Dump NAND to EmuNand
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "What to extract from NAND?" ), tr("The files will be extracted to your emu nand path. Attention: All existing files will be overwritten."), tr( "Everything" ), tr("Enter Path"), tr( "Cancel" ));
		if (choice)
		{
			char filePath[255];
			char *nandPath = (char *) memalign(32, ISFS_MAXPATH);
			if(!nandPath)
			{
				WindowPrompt(tr("Error"), tr("Not enough memory."), tr("OK"));
				return MENU_NONE;
			}

			snprintf(nandPath, sizeof(nandPath), "/");

			if(choice == 2)
			{
				choice = OnScreenKeyboard(nandPath, ISFS_MAXPATH, 1);

				if(strlen(nandPath) > 1 && nandPath[strlen(nandPath)-1] == '/')
					nandPath[strlen(nandPath)-1] = 0;
			}

			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);

			if(choice)
			{
				u32 dummy;
				int ret = -1;
				ProgressCancelEnable(true);
				StartProgress(tr("Extracting nand files:"), 0, 0, true, false);
				ShowProgress(tr("Extracting nand files:"), 0, 0, -1, true, false);

				if(ISFS_ReadDir(nandPath, NULL, &dummy) < 0)
					ret = NandTitle::ExtractFile(nandPath, filePath);
				else
					ret = NandTitle::ExtractDir(nandPath, filePath);

				ProgressStop();
				ProgressCancelEnable(false);

				if(ret != PROGRESS_CANCELED)
				{
					if(ret < 0)
						WindowPrompt(tr("Process finished."), tr("Errors occured."), tr("OK"));
					else
						WindowPrompt(tr("Success."), tr("All files extracted."), tr("OK"));
				}
			}
			free(nandPath);
		}
	}

	//! Settings: Install WAD to EmuNand
	else if (ret == ++Idx)
	{
		GuiWindow * parent = (GuiWindow *) parentElement;
		if(parent) parent->SetState(STATE_DISABLED);
		this->SetState(STATE_DEFAULT);
		this->Remove(optionBrowser);

		char wadpath[150];
		sprintf(wadpath, "%s/wad/", Settings.BootDevice);

		int result = BrowseDevice(wadpath, sizeof(wadpath), FB_DEFAULT);
		if(result)
		{
			int choice = WindowPrompt(tr("WAD Installation"), tr("What do you want to do?"), tr("Install"), tr("Uninstall"), tr("Cancel"));
			if(choice == 1)
			{
				Wad wadFile(wadpath);
				wadFile.Install(Settings.NandEmuChanPath);
				Channels::Instance()->GetEmuChannelList();
				GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path);
			}
			else if(choice == 2)
			{
				Wad wadFile(wadpath);
				wadFile.UnInstall(Settings.NandEmuChanPath);
				Channels::Instance()->GetEmuChannelList();
			}
		}

		if(parent) parent->SetState(STATE_DEFAULT);
		this->Append(optionBrowser);
	}

	SetOptionValues();

	return MENU_NONE;
}
