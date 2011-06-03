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
#include "prompts/PromptWindows.h"
#include "prompts/CategorySelectPrompt.hpp"
#include "settings/GameTitles.h"
#include "usbloader/GameList.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "cheats/cheatmenu.h"
#include "GameLoadSM.hpp"
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

int GameSettingsMenu::Show(GameBrowseMenu *parent, struct discHdr * header)
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

    //! Categories
    else if(menuNr == Idx++)
    {
        HideMenu();
        titleTxt = new GuiText(MenuTitle.c_str(), 28, ( GXColor ) {0, 0, 0, 255});
        titleTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
        titleTxt->SetPosition(0, 40);
        titleTxt->SetMaxWidth(310, SCROLL_HORIZONTAL);
        Append(titleTxt);
        Remove(backBtn);
        ResumeGui();
        SetState(STATE_DISABLED);
        CategorySelectPrompt promptMenu(DiscHeader);
        promptMenu.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
        promptMenu.SetEffect(EFFECT_FADE, 20);
        mainWindow->Append(&promptMenu);

        promptMenu.Show();

        promptMenu.SetEffect(EFFECT_FADE, -20);
        while(promptMenu.GetEffect() > 0) usleep(100);
        mainWindow->Remove(&promptMenu);
        if(promptMenu.categoriesChanged())
        {
            wString oldFilter(gameList.GetCurrentFilter());
            gameList.FilterList(oldFilter.c_str());
            browserMenu->ReloadBrowser();
        }
        SetState(STATE_DEFAULT);
        Remove(titleTxt);
        delete titleTxt;
        titleTxt = NULL;
        Append(backBtn);
        ShowMenu();
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
