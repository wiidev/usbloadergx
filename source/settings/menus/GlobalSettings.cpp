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
#include "GlobalSettings.hpp"
#include "themes/CTheme.h"
#include "prompts/PromptWindows.h"
#include "network/update.h"
#include "language/gettext.h"
#include "GUISettingsMenu.hpp"
#include "LoaderSettings.hpp"
#include "ParentalControlSM.hpp"
#include "SoundSettingsMenu.hpp"
#include "CustomPathsSM.hpp"
#include "FeatureSettingsMenu.hpp"
#include "HardDriveSM.hpp"
#include "BannerSettingsMenu.hpp"

GlobalSettings::GlobalSettings()
	: FlyingButtonsMenu(tr("Global Settings"))
{
	creditsImgData = Resources::GetImageData("credits_button.png");
	creditsImgOverData = Resources::GetImageData("credits_button_over.png");
}

GlobalSettings::~GlobalSettings()
{
	Settings.Save();

	delete creditsImgData;
	delete creditsImgOverData;
}

int GlobalSettings::Execute()
{
	GlobalSettings * Menu = new GlobalSettings();
	mainWindow->Append(Menu);

	Menu->ShowMenu();

	int returnMenu = MENU_NONE;

	while((returnMenu = Menu->MainLoop()) == MENU_NONE);

	delete Menu;

	return returnMenu;
}

void GlobalSettings::SetupMainButtons()
{
	int pos = 0;

	SetMainButton(pos++, tr( "GUI Settings" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Loader Settings" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Hard Drive Settings" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Features" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Banner Animation Settings" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Sound" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Parental Control" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Custom Paths" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Theme Menu" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Theme Downloader" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Update" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Default Settings" ), MainButtonImgData, MainButtonImgOverData);
	SetMainButton(pos++, tr( "Credits" ), creditsImgData, creditsImgOverData);
}

void GlobalSettings::CreateSettingsMenu(int menuNr)
{
	if(CurrentMenu)
		return;

	int Idx = 0;

	//! GUI Settings
	if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_GUI_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new GuiSettingsMenu();
		Append(CurrentMenu);
	}
	//! Loader Settings
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_LOADER_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new LoaderSettings();
		Append(CurrentMenu);
	}
	//! Hard Drive Settings
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_HARD_DRIVE_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new HardDriveSM();
		Append(CurrentMenu);
	}
	//! Feature
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_FEATURE_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new FeatureSettingsMenu();
		Append(CurrentMenu);
	}
	//! Banner Animation Settings
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_BANNER_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new BannerSettingsMenu();
		Append(CurrentMenu);
	}
	//! Sound
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_SOUND_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new SoundSettingsMenu();
		Append(CurrentMenu);
	}
	//! Parental Control
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_PARENTAL_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new ParentalControlSM();
		Append(CurrentMenu);
	}
	//! Custom Paths
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CUSTOMPATH_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		ResumeGui();
		CurrentMenu = new CustomPathsSM();
		Append(CurrentMenu);
	}
	//! Theme Menu
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_THEME_MENU))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		returnMenu = MENU_THEMEMENU;
	}
	//! Theme Downloader
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_THEME_DOWNLOADER))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}
		
		WindowPrompt(tr( "Warning:" ), tr( "Sorry, the theme downloader menu is not working anymore because http://wii.spiffy360.com now requires user registration." ), tr( "OK" ));
			return;
		
		// returnMenu = MENU_THEMEDOWNLOADER;
	}
	//! Update
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_UPDATES))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		HideMenu();
		Remove(backBtn);
		ResumeGui();
		int ret = UpdateApp();
		if (ret < 0)
			WindowPrompt(tr( "Update failed" ), 0, tr( "OK" ));
		Append(backBtn);
		ShowMenu();
	}
	//! Default Settings
	else if(menuNr == Idx++)
	{
		if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_RESET_SETTINGS))
		{
			WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
			return;
		}

		int choice = WindowPrompt(tr( "Are you sure you want to reset?" ), 0, tr( "Yes" ), tr( "Cancel" ));
		if (choice == 1)
		{
			HaltGui();
			gettextCleanUp();
			Settings.Reset();
			returnMenu = MENU_SETTINGS;
			ResumeGui();
		}
	}
	//! Credits
	else if(menuNr == Idx++)
	{
		HideMenu();
		Remove(backBtn);
		ResumeGui();
		WindowCredits();
		Append(backBtn);
		ShowMenu();
	}
}

void GlobalSettings::DeleteSettingsMenu()
{
	delete CurrentMenu;
	CurrentMenu = NULL;
}
