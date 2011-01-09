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

void GlobalSettings::SetupMainButtons()
{
    int pos = 0;

    SetMainButton(pos++, tr( "GUI Settings" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Loader Settings" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Parental Control" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Sound" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Custom Paths" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Update" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Default Settings" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Credits" ), creditsImgData, creditsImgOverData);
    SetMainButton(pos++, tr( "Theme Downloader" ), MainButtonImgData, MainButtonImgOverData);
}

void GlobalSettings::CreateSettingsMenu(int menuNr)
{
    if(CurrentMenu)
        return;

    int Idx = 0;

    //! GUI Settings
    if(menuNr == Idx++)
    {
        HideMenu();
        ResumeGui();
        CurrentMenu = new GuiSettingsMenu();
        Append(CurrentMenu);
    }
    //! Game Load
    else if(menuNr == Idx++)
    {
        HideMenu();
        ResumeGui();
        CurrentMenu = new LoaderSettings();
        Append(CurrentMenu);
    }
    //! Parental Control
    else if(menuNr == Idx++)
    {
        HideMenu();
        ResumeGui();
        CurrentMenu = new ParentalControlSM();
        Append(CurrentMenu);
    }
    //! Sound
    else if(menuNr == Idx++)
    {
        HideMenu();
        ResumeGui();
        CurrentMenu = new SoundSettingsMenu();
        Append(CurrentMenu);
    }
    //! Custom Paths
    else if(menuNr == Idx++)
    {
        if(Settings.godmode)
        {
            HideMenu();
            ResumeGui();
            CurrentMenu = new CustomPathsSM();
            Append(CurrentMenu);
        }
        else
            WindowPrompt(tr( "Console Locked" ), tr( "Unlock console to use this option." ), tr( "OK" ));

    }
    //! Update
    else if(menuNr == Idx++)
    {
        if (Settings.godmode)
        {
            HideMenu();
            Remove(backBtn);
            ResumeGui();
            int ret = UpdateApp();
            if (ret < 0)
                WindowPrompt(tr( "Update failed" ), 0, tr( "OK" ));
            Append(backBtn);
            ShowMenu();
        }
        else
            WindowPrompt(tr( "Console Locked" ), tr( "Unlock console to use this option." ), tr( "OK" ));
    }
    //! Default Settings
    else if(menuNr == Idx++)
    {
        if (Settings.godmode)
        {
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
        else
            WindowPrompt(tr( "Console Locked" ), tr( "Unlock console to use this option." ), tr( "OK" ));
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
    //! Theme Downloader
    else if(menuNr == Idx++)
    {
        returnMenu = MENU_THEMEDOWNLOADER;
    }
}

void GlobalSettings::DeleteSettingsMenu()
{
    if(!CurrentMenu)
        return;

    int type = CurrentMenu->GetType();

    switch(type)
    {
        case CGUISettingsMenu:
            delete ((GuiSettingsMenu *) CurrentMenu);
            break;
        case CLoaderSettings:
            delete ((LoaderSettings *) CurrentMenu);
            break;
        case CParentalControlSM:
            delete ((ParentalControlSM *) CurrentMenu);
            break;
        case CSoundSettingsMenu:
            delete ((SoundSettingsMenu *) CurrentMenu);
            break;
        case CCustomPathsSM:
            delete ((CustomPathsSM *) CurrentMenu);
            break;
        case CSettingsMenu:
        default:
            delete CurrentMenu;
            break;
    }

    CurrentMenu = NULL;
}
