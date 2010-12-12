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
#include "UninstallSM.hpp"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/GameTitles.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "usbloader/wbfs.h"
#include "usbloader/GameList.h"
#include "wstring.hpp"

extern int mountMethod;

UninstallSM::UninstallSM(struct discHdr * header)
    : SettingsMenu(tr("Uninstall Menu"), &GuiOptions, MENU_NONE)
{
    DiscHeader = header;

    int Idx = 0;

    Options->SetName(Idx++, "%s", tr( "Uninstall Game" ));
    Options->SetName(Idx++, "%s", tr( "Reset Playcounter" ));
    Options->SetName(Idx++, "%s", tr( "Delete Cover Artwork" ));
    Options->SetName(Idx++, "%s", tr( "Delete Disc Artwork" ));
    Options->SetName(Idx++, "%s", tr( "Delete Cheat TXT" ));
    Options->SetName(Idx++, "%s", tr( "Delete Cheat GCT" ));

    SetOptionValues();
}

void UninstallSM::SetOptionValues()
{
    int Idx = 0;

    //! Settings: Uninstall Game
    Options->SetValue(Idx++, " ");

    //! Settings: Reset Playcounter
    Options->SetValue(Idx++, " ");

    //! Settings: Delete Cover Artwork
    Options->SetValue(Idx++, " ");

    //! Settings: Delete Disc Artwork
    Options->SetValue(Idx++, " ");

    //! Settings: Delete Cheat TXT
    Options->SetValue(Idx++, " ");

    //! Settings: Delete Cheat GCT
    Options->SetValue(Idx++, " ");
}

int UninstallSM::GetMenuInternal()
{
    int ret = optionBrowser->GetClickedOption();

    if (ret < 0)
        return MENU_NONE;

    int Idx = -1;

    //! Settings: Uninstall Game
    if (ret == ++Idx)
    {
        int choice = WindowPrompt(tr( "Do you really want to delete:" ), GameTitles.GetTitle(DiscHeader), tr( "Yes" ), tr( "Cancel" ));
        if (choice == 1)
        {
            std::string Title = GameTitles.GetTitle(DiscHeader);
            GameSettings.Remove(DiscHeader->id);
            GameSettings.Save();
            GameStatistics.Remove(DiscHeader->id);
            GameStatistics.Save();
            int ret = 0;
            if(!mountMethod)
                ret = WBFS_RemoveGame(DiscHeader->id);

            if(ret >= 0)
            {
                wString oldFilter(gameList.GetCurrentFilter());
                gameList.ReadGameList();
                gameList.FilterList(oldFilter.c_str());
            }

            if (ret < 0)
                WindowPrompt(tr( "Can't delete:" ), Title.c_str(), tr( "OK" ));
            else
                WindowPrompt(tr( "Successfully deleted:" ), Title.c_str(), tr( "OK" ));

            return MENU_DISCLIST;
        }
    }

    //! Settings: Reset Playcounter
    else if (ret == ++Idx)
    {
        int result = WindowPrompt(tr( "Are you sure?" ), 0, tr( "Yes" ), tr( "Cancel" ));
        if (result == 1)
        {
            GameStatistics.SetPlayCount(DiscHeader->id, 0);
            GameStatistics.Save();
        }
    }

    //! Settings: Delete Cover Artwork
    else if (ret == ++Idx)
    {
        char GameID[7];
        snprintf(GameID, sizeof(GameID), "%s", (char *) DiscHeader->id);
        char filepath[200];
        snprintf(filepath, sizeof(filepath), "%s%s.png", Settings.covers_path, GameID);

        int choice = WindowPrompt(tr( "Delete" ), filepath, tr( "Yes" ), tr( "No" ));
        if (choice == 1)
            if (CheckFile(filepath)) remove(filepath);
    }

    //! Settings: Delete Disc Artwork
    else if (ret == ++Idx)
    {
        char GameID[7];
        snprintf(GameID, sizeof(GameID), "%s", (char *) DiscHeader->id);
        char filepath[200];
        snprintf(filepath, sizeof(filepath), "%s%s.png", Settings.disc_path, GameID);

        int choice = WindowPrompt(tr( "Delete" ), filepath, tr( "Yes" ), tr( "No" ));
        if (choice == 1)
            if (CheckFile(filepath)) remove(filepath);
    }

    //! Settings: Delete Cheat TXT
    else if (ret == ++Idx)
    {
        char GameID[7];
        snprintf(GameID, sizeof(GameID), "%s", (char *) DiscHeader->id);
        char filepath[200];
        snprintf(filepath, sizeof(filepath), "%s%s.txt", Settings.TxtCheatcodespath, GameID);

        int choice = WindowPrompt(tr( "Delete" ), filepath, tr( "Yes" ), tr( "No" ));
        if (choice == 1)
            if (CheckFile(filepath)) remove(filepath);
    }

    //! Settings: Delete Cheat GCT
    else if (ret == ++Idx)
    {
        char GameID[7];
        snprintf(GameID, sizeof(GameID), "%s", (char *) DiscHeader->id);
        char filepath[200];
        snprintf(filepath, sizeof(filepath), "%s%s.gct", Settings.Cheatcodespath, GameID);

        int choice = WindowPrompt(tr( "Delete" ), filepath, tr( "Yes" ), tr( "No" ));
        if (choice == 1)
            if (CheckFile(filepath)) remove(filepath);
    }

    SetOptionValues();

    return MENU_NONE;
}
