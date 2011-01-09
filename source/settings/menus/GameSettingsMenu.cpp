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
#include "GameSettingsMenu.hpp"
#include "themes/CTheme.h"
#include "prompts/PromptWindows.h"
#include "settings/GameTitles.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "cheats/cheatmenu.h"
#include "GameLoadSM.hpp"
#include "UninstallSM.hpp"

GameSettingsMenu::GameSettingsMenu(struct discHdr * header)
    : FlyingButtonsMenu(GameTitles.GetTitle(header))
{
    DiscHeader = header;
    //! Don't switch menu's by default but return to disc window.
	ParentMenu = -2;
}

GameSettingsMenu::~GameSettingsMenu()
{
}

void GameSettingsMenu::SetupMainButtons()
{
    int pos = 0;

    SetMainButton(pos++, tr( "Game Load" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Ocarina" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Uninstall Menu" ), MainButtonImgData, MainButtonImgOverData);
    SetMainButton(pos++, tr( "Default Gamesettings" ), MainButtonImgData, MainButtonImgOverData);
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
        CurrentMenu = new GameLoadSM((const char *) DiscHeader->id);
        Append(CurrentMenu);
    }

    //! Ocarina
    else if(menuNr == Idx++)
    {
        char ID[7];
        snprintf(ID, sizeof(ID), "%s", (char *) DiscHeader->id);
        CheatMenu(ID);
    }

    //! Uninstall Menu
    else if(menuNr == Idx++)
    {
        HideMenu();
        ResumeGui();
        CurrentMenu = new UninstallSM(DiscHeader);
        Append(CurrentMenu);
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
}

void GameSettingsMenu::DeleteSettingsMenu()
{
    if(!CurrentMenu)
        return;

    int type = CurrentMenu->GetType();

    switch(type)
    {
        case CGameLoadSM:
            delete ((GameLoadSM *) CurrentMenu);
            break;
        case CUninstallSM:
            delete ((UninstallSM *) CurrentMenu);
            break;
        case CSettingsMenu:
        default:
            delete CurrentMenu;
            break;
    }

    CurrentMenu = NULL;
}
