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
#include <gccore.h>
#include <ogc/machine/processor.h>
#include <unistd.h>

#include "FeatureSettingsMenu.hpp"
#include "Channels/channels.h"
#include "GameCube/GCGames.h"
#include "settings/CGameCategories.hpp"
#include "settings/CGameSettings.h"
#include "settings/GameTitles.h"
#include "settings/CSettings.h"
#include "settings/SettingsPrompts.h"
#include "network/update.h"
#include "network/Wiinnertag.h"
#include "network/networkops.h"
#include "FileOperations/fileops.h"
#include "FileOperations/DirList.h"
#include "utils/StringTools.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "prompts/filebrowser.h"
#include "usbloader/GameList.h"
#include "usbloader/neek.hpp"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "wad/wad.h"
#include "sys.h"
#include "cache/cache.hpp"

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

static const char * TitleTypeText[] =
{
	trNOOP( "Folder" ),
	trNOOP( "Disc" ),
	trNOOP( "GameTDB" )
};

FeatureSettingsMenu::FeatureSettingsMenu()
	: SettingsMenu(tr("Features Settings"), &GuiOptions, MENU_NONE)
{
	SetOptionNames();
	SetOptionValues();
	OldTitlesType = Settings.TitlesType;
	OldCacheTitles = Settings.CacheTitles;
}

FeatureSettingsMenu::~FeatureSettingsMenu()
{
	if (Settings.TitlesType != OldTitlesType || Settings.CacheTitles != OldCacheTitles)
	{
		//! Remove cached titles and reload new titles
		GameTitles.Reset();
		gameList.clear();
		GCGames::Instance()->clear();
		Channels::Instance()->clear();

		if ((Settings.CacheTitles && (Settings.TitlesType == TITLETYPE_FORCED_DISC || OldTitlesType == TITLETYPE_FORCED_DISC)) || (!Settings.CacheTitles && OldCacheTitles))
			ResetGameHeaderCache();

		gameList.LoadUnfiltered();
	}
	//! EmuNAND contents might of changed
	isCacheCurrent();
}

void FeatureSettingsMenu::SetOptionNames()
{
	int Idx = 0;

	Options->SetName(Idx++, "%s", tr( "Titles From" ));
	Options->SetName(Idx++, "%s", tr( "Cache Titles" ));
	Options->SetName(Idx++, "%s", tr( "Wiilight" ));
	Options->SetName(Idx++, "%s", tr( "Rumble" ));
	Options->SetName(Idx++, "%s", tr( "AutoInit Network" ));
	Options->SetName(Idx++, "%s", tr( "System Proxy Settings" ));
	Options->SetName(Idx++, "%s", tr( "Messageboard Update" ));
	Options->SetName(Idx++, "%s", tr( "Wiinnertag" ));
	Options->SetName(Idx++, "%s", tr( "Import Categories" ));
	Options->SetName(Idx++, "%s", tr( "Export All Saves to EmuNAND" ));
	Options->SetName(Idx++, "%s", tr( "Export Miis to EmuNAND" ));
	Options->SetName(Idx++, "%s", tr( "Export SYSCONF to EmuNAND" ));
	Options->SetName(Idx++, "%s", tr( "Dump NAND to EmuNAND" ));
	Options->SetName(Idx++, "%s", tr( "EmuNAND WAD Manager" ));
	Options->SetName(Idx++, "%s", tr( "Update Nintendont" ));
	Options->SetName(Idx++, "%s", tr( "WiiU Widescreen" ));
	Options->SetName(Idx++, "%s", tr( "Boot Neek System Menu" ));
	Options->SetName(Idx++, "%s", tr( "Reset All Game Settings" ));
	if (Settings.CacheTitles)
		Options->SetName(Idx++, "%s", tr( "Reset Cached Titles" ));
}

void FeatureSettingsMenu::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Titles From
	Options->SetValue(Idx++, "%s", tr( TitleTypeText[Settings.TitlesType] ));

	//! Settings: Cache Titles
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.CacheTitles] ));

	//! Settings: Wiilight
	Options->SetValue(Idx++, "%s", tr( WiilightText[Settings.wiilight] ));

	//! Settings: Rumble
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.rumble] ));

	//! Settings: AutoInit Network
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.autonetwork] ));

	//! Settings: System Proxy Settings
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.ProxyUseSystem] ));

	//! Settings: Messageboard Update
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.PlaylogUpdate] ));

	//! Settings: Wiinnertag
	Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.Wiinnertag] ));

	//! Settings: Import categories from GameTDB
	Options->SetValue(Idx++, " ");

	//! Settings: Export Savegames to EmuNAND
	Options->SetValue(Idx++, " ");

	//! Settings: Export Miis to EmuNAND
	Options->SetValue(Idx++, " ");

	//! Settings: Export SYSCONF to EmuNAND
	Options->SetValue(Idx++, " ");

	//! Settings: Dump NAND to EmuNAND
	Options->SetValue(Idx++, " ");

	//! Settings: EmuNAND WAD Manager
	Options->SetValue(Idx++, " ");

	//! Settings: Update Nintendont
	Options->SetValue(Idx++, " ");

	//! Settings: WiiU Widescreen
	Options->SetValue(Idx++, " ");

	//! Settings: Neek boot
	Options->SetValue(Idx++, " ");

}

int FeatureSettingsMenu::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;


	//! Settings: Titles From
	if (ret == ++Idx)
	{
		if (++Settings.TitlesType >= 3) Settings.TitlesType = 0;
	}

	//! Settings: Cache Titles
	else if (ret == ++Idx)
	{
		if (++Settings.CacheTitles >= MAX_ON_OFF) Settings.CacheTitles = 0;
		Options->ClearList();
		SetOptionNames();
		SetOptionValues();
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

	//! Settings: System Proxy Settings
	else if (ret == ++Idx)
	{
		if (++Settings.ProxyUseSystem >= MAX_ON_OFF) Settings.ProxyUseSystem = 0;
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
					char text[300];
					snprintf(text, sizeof(text), "%s %s", tr("An example file was created here:"), filepath);
					WindowPrompt(tr("Success"), text, tr("OK"));
				}
				else
				{
					char text[300];
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

	//! Settings: Export Savegames to EmuNAND
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "Do you want to extract all the save games?" ), tr("The save games will be extracted to your EmuNAND save and channel path. Attention: All existing files will be overwritten."), tr( "Yes" ), tr( "Cancel" ));
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

			//! extract the Mii file
			snprintf(nandPath, sizeof(nandPath), "/shared2/menu/FaceLib/RFL_DB.dat");
			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuChanPath, nandPath);
			if(!CheckFile(filePath))
				NandTitle::ExtractDir(nandPath, filePath);
			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
			if(!CheckFile(filePath))
				NandTitle::ExtractDir(nandPath, filePath);

			for(int i = 0; i < gameList.size(); ++i)
			{
				if(gameList[i]->type != TYPE_GAME_WII_IMG && gameList[i]->type != TYPE_GAME_NANDCHAN)
					continue;

				if(gameList[i]->tid != 0) //! Channels
				{
					snprintf(nandPath, sizeof(nandPath), "/title/%08x/%08x/data", (unsigned int) (gameList[i]->tid  >> 32), (unsigned int) gameList[i]->tid );
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

	//! Settings: Export Miis to EmuNAND
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "Extract Miis to the EmuNAND?" ), tr("The Miis will be extracted to your EmuNAND path and EmuNAND channel path. Attention: All existing files will be overwritten."), tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			char filePath[512];
			char nandPath[ISFS_MAXPATH];
			bool Error = false;
			ProgressCancelEnable(true);
			StartProgress(tr("Extracting file:"), 0, 0, true, false);

			//! extract the Mii file
			snprintf(nandPath, sizeof(nandPath), "/shared2/menu/FaceLib/RFL_DB.dat");
			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuChanPath, nandPath);
			if(NandTitle::ExtractFile(nandPath, filePath) < 0)
			   Error = true;
			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
			if(NandTitle::ExtractFile(nandPath, filePath) < 0)
				Error = true;

			ProgressStop();
			ProgressCancelEnable(false);

			if(Error)
				WindowPrompt(tr("Process finished."), tr("Errors occured."), tr("OK"));
			else
				WindowPrompt(tr("Success."), tr("All files extracted."), tr("OK"));
		}
	}

	//! Settings: Export SYSCONF to EmuNAND
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "Extract SYSCONF to the EmuNAND?" ), tr("The SYSCONF file will be extracted to your EmuNAND path and EmuNAND channel path. Attention: All existing files will be overwritten."), tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			char filePath[512];
			char nandPath[ISFS_MAXPATH];
			bool Error = false;
			ProgressCancelEnable(true);
			StartProgress(tr("Extracting file:"), 0, 0, true, false);

			//! extract the Mii file
			snprintf(nandPath, sizeof(nandPath), "/shared2/sys/SYSCONF");
			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuChanPath, nandPath);
			if(NandTitle::ExtractFile(nandPath, filePath) < 0)
			   Error = true;
			snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
			if(NandTitle::ExtractFile(nandPath, filePath) < 0)
				Error = true;

			ProgressStop();
			ProgressCancelEnable(false);

			if(Error)
				WindowPrompt(tr("Process finished."), tr("Errors occured."), tr("OK"));
			else
				WindowPrompt(tr("Success."), tr("All files extracted."), tr("OK"));
		}
	}

	//! Settings: Dump NAND to EmuNAND
	else if (ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "What to extract from NAND?" ), tr("The files will be extracted to your EmuNAND save and channel path. Attention: All existing files will be overwritten."), tr( "Everything" ), tr("Enter Path"), tr( "Cancel" ));
		if (choice)
		{
			char filePath[255];
			char *nandPath = (char *) memalign(32, ISFS_MAXPATH);
			if(!nandPath)
			{
				WindowPrompt(tr("Error"), tr("Not enough memory."), tr("OK"));
				return MENU_NONE;
			}

			strcpy(nandPath, "/");

			if(choice == 2)
			{
				choice = OnScreenKeyboard(nandPath, ISFS_MAXPATH, 1);

				if(strlen(nandPath) > 1 && nandPath[strlen(nandPath)-1] == '/')
					nandPath[strlen(nandPath)-1] = 0;
			}

			char extractPath[255];
			snprintf(extractPath, sizeof(extractPath), "%s", Settings.NandEmuPath);
			if( strlen(Settings.NandEmuPath) != strlen(Settings.NandEmuChanPath) || strcmp(Settings.NandEmuPath, Settings.NandEmuChanPath) != 0 )
			{
				if(WindowPrompt(tr( "Where to dump NAND?" ), tr("Select the EmuNAND path to use."), tr( "Channel Path" ), tr("Save Path")) == 1)
					snprintf(extractPath, sizeof(extractPath), "%s", Settings.NandEmuChanPath);
			}
			snprintf(filePath, sizeof(filePath), "%s%s", extractPath, nandPath);

			if(choice)
			{
				u32 dummy;
				int ret = -1;
				ProgressCancelEnable(true);
				StartProgress(tr("Extracting NAND files:"), 0, 0, true, false);
				ShowProgress(tr("Extracting NAND files:"), 0, 0, -1, true, false);

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

	//! Settings: EmuNAND WAD Manager
	else if (ret == ++Idx)
	{
		GuiWindow * parent = (GuiWindow *) parentElement;
		if(parent) parent->SetState(STATE_DISABLED);
		this->SetState(STATE_DEFAULT);
		this->Remove(optionBrowser);

		char wadpath[150];
		snprintf(wadpath, sizeof(wadpath), "%s/wad/", Settings.BootDevice);

		int choice = WindowPrompt(tr("EmuNAND WAD Manager"), tr("Which mode do you want to use?"), tr("File"), tr("Folder"), tr("Cancel"));
		if(choice == 1) 			// File mode
		{
			int result = BrowseDevice(wadpath, sizeof(wadpath), FB_DEFAULT );
			if(result)
			{
				choice = WindowPrompt(tr("EmuNAND WAD Manager"), tr("What do you want to do?"), tr("Install"), tr("Uninstall"), tr("Cancel"));
				if(choice == 1) 	// File install
				{
					Wad wadFile(wadpath);
					if(!wadFile.Install(Settings.NandEmuChanPath))
					{
						// install error - Try to cleanup any partially installed WAD data
						WindowPrompt(tr("EmuNAND WAD Manager"), tr("Install error - Cleaning incomplete data."), tr( "OK" ));
						//gprintf("Error   : %s\n", wadpath);
						wadFile.UnInstall(Settings.NandEmuChanPath);
					}
				}
				else if(choice == 2) // File uninstall
				{
					Wad wadFile(wadpath);
					wadFile.UnInstall(Settings.NandEmuChanPath);
				}

				// Refresh new EmuNAND content
				Channels::Instance()->GetEmuChannelList();
				GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path);
			}
		}
		else if(choice == 2)		// Folder mode
		{
			int result = BrowseDevice(wadpath, sizeof(wadpath), FB_DEFAULT, noFILES );
			if(result)
			{
				DirList* wadList = new DirList(wadpath, ".wad", DirList::Files);
				if(wadList->GetFilecount())
				{
					char found[20];
					snprintf(found, sizeof(found), fmt(tr("%i WAD found."), wadList->GetFilecount()));
					choice = WindowPrompt(tr("EmuNAND WAD Manager"), fmt("%s %s", found, tr("What do you want to do?")), tr("Install"), tr("Uninstall"), tr("Cancel"));
					if(choice == 1) // Folder install
					{
						for(int i = 0; i < wadList->GetFilecount(); i++)
						{
							Wad wadFile(wadList->GetFilepath(i), false);
							if(wadFile.Install(Settings.NandEmuChanPath))
							{
								//gprintf("Success : %s\n", wadList->GetFilepath(i));
								wadList->RemoveEntrie(i);
								--i;
							}
							else 	// install error - Try to cleanup any partially installed wad data
							{
								//gprintf("Error   : %s\n", wadList->GetFilepath(i));
								wadFile.UnInstall(Settings.NandEmuChanPath);
							}
						}
					}
					if(choice == 2) // Folder uninstall
					{
						if(WindowPrompt(tr("EmuNAND WAD Manager"), tr("Attention: All savegames will be deleted."), tr("Uninstall"), tr("Cancel")))
						{
							for(int i = 0; i < wadList->GetFilecount(); i++)
							{
								Wad wadFile(wadList->GetFilepath(i), false);
								if(wadFile.UnInstall(Settings.NandEmuChanPath))
								{
									//gprintf("uninst. : %s\n", wadList->GetFilepath(i));
									wadList->RemoveEntrie(i);
									--i;
								}
							}
						}
						else
							choice = 0;
					}

					// check if there is any remaining unprocessed WAD
					if(choice != 0)
					{
						if(wadList->GetFilecount() == 0)
							WindowPrompt(tr("EmuNAND WAD Manager"), tr("All WAD files processed successfully."), tr( "OK" ));
						else
						{
							if(WindowPrompt(tr( "EmuNAND WAD Manager" ), fmt(tr("%i WAD file(s) not processed!"), wadList->GetFilecount()), tr("Save List"), tr( "OK" )))
							{
								char path[200];
								snprintf(path, sizeof(path), "%s/wad_manager_errors.txt", Settings.ConfigPath);

								FILE *f = fopen(path, "a");
								if(f)
								{
									time_t rawtime = time(NULL);
									char theTime[11];
									theTime[0] = 0;
									strftime(theTime, sizeof(theTime), "%Y-%m-%d", localtime(&rawtime));
									fprintf(f, "\r\n\r\nEmuNAND WAD Manager - %10s\r\n--------------------------------\r\n", theTime);
									fprintf(f, "%s %s\r\n", choice == 1 ? "Error installing to" : "Error uninstalling from", Settings.NandEmuChanPath);
									fprintf(f, "%s\r\n", choice == 1 ? "List of user canceled installation or bad WAD files." : "Titles not on EmuNAND or weren't correctly installed.");
									
									for(int i = 0; i < wadList->GetFilecount(); i++)
									{
										fprintf(f, "%s\r\n", wadList->GetFilepath(i));
										//gprintf("%s\n", wadList->GetFilepath(i));
									}

									fclose(f);
								}
								else
									WindowPrompt(tr( "EmuNAND WAD Manager" ), tr("Error writing the data."), tr( "OK" ));
							}
						}
					}

					// Refresh new EmuNAND content
					Channels::Instance()->GetEmuChannelList();
					GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path);
				}
				else
				{
					WindowPrompt(tr( "EmuNAND WAD Manager" ), tr("No WAD file found in this folder."), tr( "OK" ));
				}

				delete wadList;
			}
		}

		if(parent) parent->SetState(STATE_DEFAULT);
		this->Append(optionBrowser);
	}

	//! Settings: Update Nintendont
	else if (ret == ++Idx)
	{
		char NINUpdatePath[120];
		snprintf(NINUpdatePath, sizeof(NINUpdatePath), "%sboot.dol", Settings.NINLoaderPath);
		char NINUpdatePathBak[120];
		snprintf(NINUpdatePathBak, sizeof(NINUpdatePathBak), "%sboot.bak", Settings.NINLoaderPath);

		int choice = WindowPrompt(tr( "Do you want to update this file?" ), NINUpdatePath, tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			if (!IsNetworkInit() && !NetworkInitPrompt())
			{
				WindowPrompt(tr("Error:"), tr("Could not initialize network!"), tr("OK"));
			}
			else
			{
				// Create the directory if it doesn't exist
				CreateSubfolder(Settings.NINLoaderPath);
				// Rename existing boot.dol file to boot.bak
				if (CheckFile(NINUpdatePath))
					RenameFile(NINUpdatePath, NINUpdatePathBak);
				
				if (DownloadFileToPath("https://raw.githubusercontent.com/FIX94/Nintendont/master/loader/loader.dol", NINUpdatePath) > 0)
				{
					// Remove existing loader.dol file if found as it has priority over boot.dol, and boot.bak
					snprintf(NINUpdatePath, sizeof(NINUpdatePath), "%s/loader.dol", Settings.NINLoaderPath);
					RemoveFile(NINUpdatePath);
					RemoveFile(NINUpdatePathBak);
					WindowPrompt(tr("Successfully Updated"), 0, tr("OK"));
				}
				else
				{
					// Restore backup file if found
					RemoveFile(NINUpdatePath);
					if (CheckFile(NINUpdatePathBak))
						RenameFile(NINUpdatePathBak, NINUpdatePath);
					WindowPrompt(tr("Update Failed"), 0, tr("OK"));
				}
			}
		}
	}

	// WiiU Aspect switcher (Thanks Tueidj)
	else if (ret == ++Idx)
	{
		if(isWiiU()) // vWii only
		{
			if( read32(0xd8006a0) == 0x30000004)
			{
				write32(0xd8006a0, 0x30000002), mask32(0xd8006a8, 0, 2); // Set 4:3
				Settings.widescreen = OFF;
			}
			else
			{
				write32(0xd8006a0, 0x30000004), mask32(0xd8006a8, 0, 2); // Set 16:9
				Settings.widescreen = ON;
			}
		}
	}

	// Neek: Boot neek system menu with current EmuNAND channel path
	else if (ret == ++Idx)
	{
		if(neek2oSetNAND(Settings.NandEmuChanPath) < 0) // set current path as default
		{
			WindowPrompt(tr("Error:"), tr("Neek NAND path selection failed."), tr("OK"));
		}
		else
		{
			if(neekLoadKernel(Settings.NandEmuChanPath) == false)
			{
				WindowPrompt(tr("Error:"), tr("Neek kernel loading failed."), tr("OK"));
			}
			else
			{
				ExitApp();
				NEEK_CFG *neek_config = (NEEK_CFG *) NEEK_CONFIG_ADDRESS;
				neek2oSetBootSettings(neek_config, 0 /* TitleID */ , 0 /* Magic */, 0 /* Returnto TitleID */, Settings.NandEmuChanPath /* Full EmuNAND path */);

				if(neekBoot() == -1)
					Sys_BackToLoader();
				return MENU_NONE;
			}
		}
	}

	//! Reset All Game Settings
	else if(ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "Are you sure you want to reset?" ), 0, tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
			GameSettings.RemoveAll();
	}

	//! Reset Cached Titles
	else if(Settings.CacheTitles && ret == ++Idx)
	{
		int choice = WindowPrompt(tr( "Are you sure you want to reset?" ), 0, tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			gameList.clear();
			GCGames::Instance()->clear();
			Channels::Instance()->clear();
			ResetGameHeaderCache();
			gameList.LoadUnfiltered();
		}
	}
	SetOptionValues();

	return MENU_NONE;
}
