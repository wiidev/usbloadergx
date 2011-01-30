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
#include "CustomPathsSM.hpp"
#include "settings/SettingsPrompts.h"
#include "settings/CSettings.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "prompts/filebrowser.h"
#include "themes/CTheme.h"

CustomPathsSM::CustomPathsSM()
    : SettingsMenu(tr("Custom Paths"), &GuiOptions, MENU_NONE)
{
    int Idx = 0;
    Options->SetName(Idx++, tr("3D Cover Path"));
    Options->SetName(Idx++, tr("2D Cover Path"));
    Options->SetName(Idx++, tr("Full Cover Path"));
    Options->SetName(Idx++, tr("Disc Artwork Path"));
    Options->SetName(Idx++, tr("Theme Path"));
    Options->SetName(Idx++, tr("WiiTDB Path"));
    Options->SetName(Idx++, tr("Update Path"));
    Options->SetName(Idx++, tr("GCT Cheatcodes Path"));
    Options->SetName(Idx++, tr("TXT Cheatcodes Path"));
    Options->SetName(Idx++, tr("DOL Path"));
    Options->SetName(Idx++, tr("Homebrew Apps Path"));
    Options->SetName(Idx++, tr("BCA Codes Path"));
    Options->SetName(Idx++, tr("WIP Patches Path"));
    Options->SetName(Idx++, tr("Languagefiles Path"));
    Options->SetName(Idx++, tr("WDM Files Path"));

    SetOptionValues();
}

void CustomPathsSM::SetOptionValues()
{
    int Idx = 0;

    //! Settings: 3D Cover Path
    Options->SetValue(Idx++, Settings.covers_path);

    //! Settings: 2D Cover Path
    Options->SetValue(Idx++, Settings.covers2d_path);

    //! Settings: Full Cover Path
    Options->SetValue(Idx++, Settings.coversFull_path);

    //! Settings: Disc Artwork Path
    Options->SetValue(Idx++, Settings.disc_path);

    //! Settings: Theme Path
    Options->SetValue(Idx++, Settings.theme_path);

    //! Settings: WiiTDB Path
    Options->SetValue(Idx++, Settings.titlestxt_path);

    //! Settings: Update Path
    Options->SetValue(Idx++, Settings.update_path);

    //! Settings: GCT Cheatcodes Path
    Options->SetValue(Idx++, Settings.Cheatcodespath);

    //! Settings: TXT Cheatcodes Path
    Options->SetValue(Idx++, Settings.TxtCheatcodespath);

    //! Settings: DOL Path
    Options->SetValue(Idx++, Settings.dolpath);

    //! Settings: Homebrew Apps Path
    Options->SetValue(Idx++, Settings.homebrewapps_path);

    //! Settings: BCA Codes Path
    Options->SetValue(Idx++, Settings.BcaCodepath);

    //! Settings: WIP Patches Path
    Options->SetValue(Idx++, Settings.WipCodepath);

    //! Settings: Languagefiles Path
    Options->SetValue(Idx++, Settings.languagefiles_path);

    //! Settings: WDM Files Path
    Options->SetValue(Idx++, Settings.WDMpath);
}

int CustomPathsSM::GetMenuInternal()
{
    int ret = optionBrowser->GetClickedOption();

    if (ret < 0)
        return MENU_NONE;

    int Idx = -1;

    //! Settings: 3D Cover Path
    if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "3D Cover Path" ));
        ChangePath(Settings.covers_path, sizeof(Settings.covers_path));
    }

    //! Settings: 2D Cover Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "2D Cover Path" ));
        ChangePath(Settings.covers2d_path, sizeof(Settings.covers2d_path));
    }

    //! Settings: Full Cover Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "Full Cover Path" ));
        ChangePath(Settings.coversFull_path, sizeof(Settings.coversFull_path));
    }

    //! Settings: Disc Artwork Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "Disc Artwork Path" ));
        ChangePath(Settings.disc_path, sizeof(Settings.disc_path));
    }

    //! Settings: Theme Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "Theme Path" ));
        HaltGui();
        GuiWindow * parent = (GuiWindow *) parentElement;
        if(parent) parent->SetState(STATE_DISABLED);
        this->SetState(STATE_DEFAULT);
        this->Remove(optionBrowser);
        ResumeGui();
        int res = MenuThemeSelect();
        if(parent) parent->SetState(STATE_DEFAULT);
        this->Append(optionBrowser);
        if (res == 1)
            return MENU_SETTINGS;
    }

    //! Settings: WiiTDB Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "WiiTDB Path" ));
        ChangePath(Settings.titlestxt_path, sizeof(Settings.titlestxt_path));
    }

    //! Settings: Update Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "Update Path" ));
        ChangePath(Settings.update_path, sizeof(Settings.update_path));
    }

    //! Settings: GCT Cheatcodes Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "GCT Cheatcodes Path" ));
        ChangePath(Settings.Cheatcodespath, sizeof(Settings.Cheatcodespath));
    }

    //! Settings: TXT Cheatcodes Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "TXT Cheatcodes Path" ));
        ChangePath(Settings.TxtCheatcodespath, sizeof(Settings.TxtCheatcodespath));
    }

    //! Settings: DOL Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "DOL Path" ));
        ChangePath(Settings.dolpath, sizeof(Settings.dolpath));
    }

    //! Settings: Homebrew Apps Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "Homebrew Apps Path" ));
        ChangePath(Settings.homebrewapps_path, sizeof(Settings.homebrewapps_path));
    }

    //! Settings: BCA Codes Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "BCA Codes Path" ));
        ChangePath(Settings.BcaCodepath, sizeof(Settings.BcaCodepath));
    }

    //! Settings: WIP Patches Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "WIP Patches Path" ));
        ChangePath(Settings.WipCodepath, sizeof(Settings.WipCodepath));
    }

    //! Settings: Languagefiles Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "Languagefiles Path" ));
        ChangePath(Settings.languagefiles_path, sizeof(Settings.languagefiles_path));
    }

    //! Settings: WDM Files Path
    else if (ret == ++Idx)
    {
        titleTxt->SetText(tr( "WDM Files Path" ));
        ChangePath(Settings.WDMpath, sizeof(Settings.WDMpath));
    }

    //! Global set back of the titleTxt after a change
    titleTxt->SetText(tr( "Custom Paths" ));
    SetOptionValues();

    return MENU_NONE;
}

int CustomPathsSM::ChangePath(char * SettingsPath, int SizeOfPath)
{
    char entered[300];
    snprintf(entered, sizeof(entered), SettingsPath);

    HaltGui();
    GuiWindow * parent = (GuiWindow *) parentElement;
    if(parent) parent->SetState(STATE_DISABLED);
    this->SetState(STATE_DEFAULT);
    this->Remove(optionBrowser);
    ResumeGui();

    int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);

    if(parent) parent->SetState(STATE_DEFAULT);
    this->Append(optionBrowser);

    if (result == 1)
    {
        if (entered[strlen(entered)-1] != '/')
            strcat(entered, "/");

        snprintf(SettingsPath, SizeOfPath, entered);
        WindowPrompt(tr( "Path Changed" ), 0, tr( "OK" ));
    }

    return result;
}
