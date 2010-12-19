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

static const char * LockedGamesText[2] =
{
    trNOOP( "0 (Locked and Unlocked Games)" ),
    trNOOP( "1 (Unlocked Games Only)" )
};

ParentalControlSM::ParentalControlSM()
    : SettingsMenu(tr("Parental Control"), &GuiOptions, MENU_NONE)
{
    int Idx = 0;
    Options->SetName(Idx++, "%s", tr( "Console" ));
    Options->SetName(Idx++, "%s", tr( "Password" ));
    Options->SetName(Idx++, "%s", tr( "Controllevel" ));
    Options->SetName(Idx++, "%s", tr( "GamesLevel" ));

    SetOptionValues();
}

void ParentalControlSM::SetOptionValues()
{
    int Idx = 0;

    //! Settings: Console
    Options->SetValue(Idx++, "%s", tr( LockModeText[Settings.godmode] ));

    //! Settings: Password
    if (!Settings.godmode)
        Options->SetValue(Idx++, "********");
    else if (strcmp(Settings.unlockCode, "") == 0)
        Options->SetValue(Idx++, "%s", tr( "not set" ));
    else
        Options->SetValue(Idx++, Settings.unlockCode);

    //! Settings: Controllevel
    if (Settings.godmode)
        Options->SetValue(Idx++, "%s", tr( ParentalText[Settings.parentalcontrol] ));
    else
        Options->SetValue(Idx++, "********");


    //! Settings: GamesLevel
    if (Settings.godmode)
        Options->SetValue(Idx++, "%s", tr( LockedGamesText[Settings.lockedgames] ));
    else
        Options->SetValue(Idx++, "********");
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

    //! Settings: Password
    else if (ret == ++Idx)
    {
        if (Settings.godmode)
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
        else
        {
            WindowPrompt(tr( "Password Changed" ), tr( "Console should be unlocked to modify it." ), tr( "OK" ));
        }
    }

    //! Settings: Controllevel
    else if (ret == ++Idx)
    {
        if (Settings.godmode && ++Settings.parentalcontrol >= 5) Settings.parentalcontrol = 0;
    }

    //! Settings: GamesLevel
    else if (ret == ++Idx)
    {
        if (Settings.godmode && ++Settings.lockedgames >= 2) Settings.lockedgames = 0;
    }

    SetOptionValues();

    return MENU_NONE;
}
