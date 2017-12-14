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
#include "GameSettingsMenu.hpp"
#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "prompts/CategorySelectPrompt.hpp"
#include "settings/GameTitles.h"
#include "usbloader/GameList.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "cheats/cheatmenu.h"
#include "GameLoadSM.hpp"
#include "GCGameLoadSM.hpp"
#include "UninstallSM.hpp"

GameSettingsMenu::GameSettingsMenu(GameBrowseMenu *parent, struct discHdr * header)
	: FlyingButtonsMenu(GameTitles.GetTitle(header)), browserMenu(parent)
{
	DiscHeader = header;
	//! Don't switch menu's by default but return to disc window.
	ParentMenu = -2;
}

GameSettingsMenu::~GameSettingsMenu()
{
}

int GameSettingsMenu::Execute(GameBrowseMenu *parent, struct discHdr * header)
{
	GameSettingsMenu * Menu = new GameSettingsMenu(parent, header);
	mainWindow->Append(Menu);

	Menu->ShowMenu();

	int returnMenu = MENU_NONE;

	while((returnMenu = Menu->MainLoop()) == MENU_NONE);

	delete Menu;

	return returnMenu;
}

void GameSettingsMenu::SetupMainButtons()
{
	int pos = 0;

	SetMainButton(pos++, tr( "Game Load" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Ocarina" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Categories" ), MainButtonImgData, MainButtonImgOverData);
	if(		DiscHeader->type == TYPE_GAME_WII_IMG
		||	DiscHeader->type == TYPE_GAME_WII_DISC
		||	DiscHeader->type == TYPE_GAME_NANDCHAN)
	{
		SetMainButton(pos++, tr( "Extract Save to EmuNand" ), MainButtonImgData, MainButtonImgOverData);
	}
	SetMainButton(pos++, tr( "Default Gamesettings" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Uninstall Menu" ), MainButtonImgData, MainButtonImgOverData);
}

void GameSettingsMenu::CreateSettingsMenu(int menuNr)
{
	if(CurrentMenu)
		return;

	int Idx = 0;

	//! Game Load
	if(menuNr == Idx++)
	{
		HideMenu();
		ResumeGui();
		if(   DiscHeader->type == TYPE_GAME_GC_IMG
		   || DiscHeader->type == TYPE_GAME_GC_DISC
		   || DiscHeader->type == TYPE_GAME_GC_EXTRACTED)
		{
			CurrentMenu = new GCGameLoadSM(DiscHeader);
		}
		else
		{
			CurrentMenu = new GameLoadSM(DiscHeader);
		}
		Append(CurrentMenu);
	}

	//! Ocarina
	else if(menuNr == Idx++)
	{
		char ID[7];
		snprintf(ID, sizeof(ID), "%s", (char *) DiscHeader->id);
		CheatMenu(ID);
	}

	//! Categories
	else if(menuNr == Idx++)
	{
		if (!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MENU))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}
		HideMenu();
		Remove(backBtn);
		ResumeGui();
		mainWindow->SetState(STATE_DISABLED);
		CategorySelectPrompt promptMenu(DiscHeader);
		promptMenu.SetAlignment(thAlign("center - category game prompt align hor"), thAlign("middle - category game prompt align ver"));
		promptMenu.SetPosition(thInt("0 - category game prompt pos x"), thInt("0 - category game prompt pos y"));
		promptMenu.SetEffect(EFFECT_FADE, 20);
		mainWindow->Append(&promptMenu);

		promptMenu.Show();

		promptMenu.SetEffect(EFFECT_FADE, -20);
		while(promptMenu.GetEffect() > 0) usleep(100);
		mainWindow->Remove(&promptMenu);
		if(promptMenu.categoriesChanged())
		{
			gameList.FilterList();
			browserMenu->ReloadBrowser();
		}
		mainWindow->SetState(STATE_DEFAULT);
		Append(backBtn);
		ShowMenu();
	}

	//! Extract Save to EmuNand
	else if(	(DiscHeader->type == TYPE_GAME_WII_IMG
			||	 DiscHeader->type == TYPE_GAME_WII_DISC
			||	 DiscHeader->type == TYPE_GAME_NANDCHAN)
			&&	menuNr == Idx++)
	{
		int choice = WindowPrompt(tr( "Do you want to extract the save game?" ), tr("The save game will be extracted to your emu nand path."), tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			char filePath[512];
			char nandPath[512];
			if(DiscHeader->tid != 0) //! Channels
			{
				snprintf(nandPath, sizeof(nandPath), "/title/%08x/%08x/data", (unsigned int) (DiscHeader->tid >> 32), (unsigned int) DiscHeader->tid);
				snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuChanPath, nandPath);
			}
			else //! Wii games
			{
				snprintf(nandPath, sizeof(nandPath), "/title/00010000/%02x%02x%02x%02x", DiscHeader->id[0], DiscHeader->id[1], DiscHeader->id[2], DiscHeader->id[3]);
				snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
			}

			ProgressCancelEnable(true);
			StartProgress(tr("Extracting file:"), 0, 0, true, false);
			int ret = NandTitle::ExtractDir(nandPath, filePath);

			if(ret < 0) //! Games with installable channels: Mario Kart, Wii Fit, etc.
			{
				snprintf(nandPath, sizeof(nandPath), "/title/00010004/%02x%02x%02x%02x", DiscHeader->id[0], DiscHeader->id[1], DiscHeader->id[2], DiscHeader->id[3]);
				snprintf(filePath, sizeof(filePath), "%s%s", Settings.NandEmuPath, nandPath);
				ret = NandTitle::ExtractDir(nandPath, filePath);
			}

			//! extract the Mii file if not yet done
			snprintf(nandPath, sizeof(nandPath), "/shared2/menu/FaceLib/RFL_DB.dat");
			snprintf(filePath, sizeof(filePath), "%s%s", (DiscHeader->tid != 0) ? Settings.NandEmuChanPath : Settings.NandEmuPath, nandPath);
			if(!CheckFile(filePath))
				NandTitle::ExtractDir(nandPath, filePath);

			ProgressStop();
			ProgressCancelEnable(false);

			if(ret < 0)
				WindowPrompt(tr("Error:"), tr("Failed to extract all files. Savegame might not exist."), tr("OK"));
			else
				WindowPrompt(tr("Files extracted successfully."), 0, tr("OK"));
		}
	}

	//! Default Gamesettings
	else if(menuNr == Idx++)
	{
		int choice = WindowPrompt(tr( "Are you sure?" ), 0, tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			GameSettings.Remove(DiscHeader->id);
			GameSettings.Save();
		}
	}

	//! Uninstall Menu
	else if(menuNr == Idx++)
	{
		HideMenu();
		ResumeGui();
		CurrentMenu = new UninstallSM(DiscHeader);
		Append(CurrentMenu);
	}
}

void GameSettingsMenu::DeleteSettingsMenu()
{
	delete CurrentMenu;
	CurrentMenu = NULL;
}
