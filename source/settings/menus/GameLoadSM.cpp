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
#include <gccore.h>
#include "settings/CSettings.h"
#include "themes/CTheme.h"
#include "prompts/PromptWindows.h"
#include "prompts/DiscBrowser.h"
#include "usbloader/AlternateDOLOffsets.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "GameLoadSM.hpp"

static const char * OnOffText[] =
{
    trNOOP( "OFF" ),
    trNOOP( "ON" )
};

static const char * VideoModeText[] =
{
    trNOOP( "System Default" ),
    trNOOP( "Disc Default" ),
    trNOOP( "Force PAL50" ),
    trNOOP( "Force PAL60" ),
    trNOOP( "Force NTSC" ),
    trNOOP( "Region Patch" ),
    trNOOP( "Force PAL480p" ),
    trNOOP( "Force NTSC480p" ),
};

static const char * LanguageText[] =
{
    trNOOP( "Japanese" ),
    trNOOP( "English" ),
    trNOOP( "German" ),
    trNOOP( "French" ),
    trNOOP( "Spanish" ),
    trNOOP( "Italian" ),
    trNOOP( "Dutch" ),
    trNOOP( "SChinese" ),
    trNOOP( "TChinese" ),
    trNOOP( "Korean" ),
    trNOOP( "Console Default" ),
};

static const char * Error002Text[] =
{
    trNOOP( "No" ),
    trNOOP( "Yes" ),
    trNOOP( "Anti" )
};

static const char * ParentalText[] =
{
    trNOOP( "0 (Everyone)" ),
    trNOOP( "1 (Child 7+)" ),
    trNOOP( "2 (Teen 12+)" ),
    trNOOP( "3 (Mature 16+)" ),
    trNOOP( "4 (Adults Only 18+)" )
};

static const char * AlternateDOLText[] =
{
    trNOOP( "OFF" ),
    trNOOP( "Select a DOL from Game" ),
    trNOOP( "Load From SD/USB" ),
    trNOOP( "List on Gamelaunch" ),
    trNOOP( "Default" ),
};

GameLoadSM::GameLoadSM(const char * GameID)
    : SettingsMenu(tr("Game Load"), &GuiOptions, MENU_NONE)
{
    memcpy(&GameConfig, GameSettings.GetGameCFG(GameID), sizeof(GameCFG));

    if(!btnOutline)
        btnOutline = Resources::GetImageData("button_dialogue_box.png");
    if(!trigA)
        trigA = new GuiTrigger();
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    saveBtnTxt = new GuiText(tr( "Save" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    saveBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
    saveBtnImg = new GuiImage (btnOutline);
    if (Settings.wsprompt == ON)
    {
        saveBtnTxt->SetWidescreen(Settings.widescreen);
        saveBtnImg->SetWidescreen(Settings.widescreen);
    }
    saveBtn = new GuiButton(saveBtnImg, saveBtnImg, 2, 3, 180, 400, trigA, btnSoundOver, btnSoundClick2, 1);
    saveBtn->SetLabel(saveBtnTxt);
    Append(saveBtn);

    SetOptionNames();
    SetOptionValues();
}

GameLoadSM::~GameLoadSM()
{
    HaltGui();
    //! The rest is destroyed in SettingsMenu.cpp
    Remove(saveBtn);
    delete saveBtnTxt;
    delete saveBtnImg;
    delete saveBtn;
    ResumeGui();
}

void GameLoadSM::SetDefaultConfig()
{
    char id[7];
    snprintf(id, sizeof(id), GameConfig.id);
    memcpy(&GameConfig, GameSettings.GetDefault(), sizeof(GameCFG));
    snprintf(GameConfig.id, sizeof(GameConfig.id), id);
}

void GameLoadSM::SetOptionNames()
{
    int Idx = 0;

    Options->SetName(Idx++, "%s", tr( "Video Mode" ));
    Options->SetName(Idx++, "%s", tr( "VIDTV Patch" ));
    Options->SetName(Idx++, "%s", tr( "Game Language" ));
    Options->SetName(Idx++, "%s", tr( "Patch Country Strings" ));
    Options->SetName(Idx++, "%s", tr( "Ocarina" ));
    Options->SetName(Idx++, "%s", tr( "Game IOS" ));
    Options->SetName(Idx++, "%s", tr( "Parental Control" ));
    Options->SetName(Idx++, "%s", tr( "Error 002 fix" ));
    Options->SetName(Idx++, "%s", tr( "Return To" ));
    Options->SetName(Idx++, "%s", tr( "Alternate DOL" ));
    Options->SetName(Idx++, "%s", tr( "Select DOL Offset" ));
    Options->SetName(Idx++, "%s", tr( "Block IOS Reload" ));
    Options->SetName(Idx++, "%s", tr( "Game Lock" ));
}

void GameLoadSM::SetOptionValues()
{
    int Idx = 0;

    //! Settings: Video Mode
    if(GameConfig.video == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%s", tr(VideoModeText[GameConfig.video]));

    //! Settings: VIDTV Patch
    if(GameConfig.vipatch == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.vipatch]));

    //! Settings: Game Language
    if(GameConfig.language == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%s", tr(LanguageText[GameConfig.language]));

    //! Settings: Patch Country Strings
    if(GameConfig.patchcountrystrings == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.patchcountrystrings]));

    //! Settings: Ocarina
    if(GameConfig.ocarina == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.ocarina]));

    //! Settings: Game IOS
    if(GameConfig.ios == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%i", GameConfig.ios);

    //! Settings: Parental Control
    Options->SetValue(Idx++, "%s", tr(ParentalText[GameConfig.parentalcontrol]));

    //! Settings: Error 002 fix
    if(GameConfig.errorfix002 == INHERIT)
        Options->SetValue(Idx++, tr("Use global"));
    else
        Options->SetValue(Idx++, "%s", tr(Error002Text[GameConfig.errorfix002]));

    //! Settings: Return To
    if(GameConfig.returnTo)
    {
        const char* TitleName = NULL;
        int haveTitle = NandTitles.FindU32(Settings.returnTo);
        if (haveTitle >= 0)
            TitleName = NandTitles.NameFromIndex(haveTitle);
        Options->SetValue(Idx++, "%s", TitleName ? TitleName : strlen(Settings.returnTo) > 0 ?
                                        Settings.returnTo : tr( OnOffText[0] ));
    }
    else
    {
        Options->SetValue(Idx++, "%s", tr( OnOffText[0] ));
    }

    //! Settings: Alternate DOL
    Options->SetValue(Idx++, "%s", tr( AlternateDOLText[GameConfig.loadalternatedol] ));

    //! Settings: Select DOL Offset
    if(GameConfig.loadalternatedol != 1)
        Options->SetValue(Idx++, tr("Not required"));
    else
    {
        if(strcmp(GameConfig.alternatedolname, "") != 0)
            Options->SetValue(Idx++, "%i <%s>", GameConfig.alternatedolstart, GameConfig.alternatedolname);
        else
            Options->SetValue(Idx++, "%i", GameConfig.alternatedolstart);
    }

    //! Settings: Block IOS Reload
    Options->SetValue(Idx++, "%s", tr( OnOffText[GameConfig.iosreloadblock]) );

    //! Settings: Game Lock
    Options->SetValue(Idx++, "%s", tr( OnOffText[GameConfig.Locked] ));
}

int GameLoadSM::GetMenuInternal()
{
    if (saveBtn->GetState() == STATE_CLICKED)
    {
        if (GameSettings.AddGame(GameConfig) && GameSettings.Save())
        {
            WindowPrompt(tr( "Successfully Saved" ), 0, tr( "OK" ));
        }
        else
            WindowPrompt(tr( "Save Failed. No device inserted?" ), 0, tr( "OK" ));

        saveBtn->ResetState();
    }

    int ret = optionBrowser->GetClickedOption();

    if (ret < 0)
        return MENU_NONE;

    int Idx = -1;

    //! Settings: Video Mode
    if (ret == ++Idx)
    {
        if (++GameConfig.video >= VIDEO_MODE_MAX) GameConfig.video = INHERIT;
    }

    //! Settings: VIDTV Patch
    else if (ret == ++Idx)
    {
        if (++GameConfig.vipatch >= MAX_ON_OFF) GameConfig.vipatch = INHERIT;
    }

    //! Settings: Game Language
    else if (ret == ++Idx)
    {
        if (++GameConfig.language >= MAX_LANGUAGE) GameConfig.language = INHERIT;
    }

    //! Settings: Patch Country Strings
    else if (ret == ++Idx)
    {
        if (++GameConfig.patchcountrystrings >= MAX_ON_OFF) GameConfig.patchcountrystrings = INHERIT;
    }

    //! Settings: Ocarina
    else if (ret == ++Idx)
    {
        if (++GameConfig.ocarina >= MAX_ON_OFF) GameConfig.ocarina = INHERIT;
    }

    //! Settings: Game IOS
    else if (ret == ++Idx)
    {
        char entered[4];
        snprintf(entered, sizeof(entered), "%i", GameConfig.ios);
        if(OnScreenKeyboard(entered, sizeof(entered), 0))
        {
            GameConfig.ios = atoi(entered) & 0xFF;
            if(GameConfig.ios < 200) GameConfig.ios = 200;

            if(NandTitles.IndexOf(TITLE_ID(1, GameConfig.ios)) < 0)
            {
                WindowPrompt(tr("Warning:"), tr("This IOS was not found on the titles list. If you are sure you have it installed than ignore this warning."), tr("OK"));
            }
            else if(GameConfig.ios == 254)
            {
                WindowPrompt(tr("Warning:"), tr("This IOS is the BootMii ios. If you are sure it is not BootMii and you have something else installed there than ignore this warning."), tr("OK"));
            }
        }
    }

    //! Settings: Parental Control
    else if (ret == ++Idx)
    {
        if (++GameConfig.parentalcontrol >= 5) GameConfig.parentalcontrol = 0;
    }

    //! Settings: Error 002 fix
    else if (ret == ++Idx)
    {
        if (++GameConfig.errorfix002 >= 3) GameConfig.errorfix002 = INHERIT;
    }

    //! Settings: Return To
    else if (ret == ++Idx)
    {
        if (++GameConfig.returnTo >= MAX_ON_OFF) GameConfig.returnTo = 0;
    }

    //! Settings: Alternate DOL
    else if (ret == ++Idx)
    {
        if (++GameConfig.loadalternatedol >= ALT_DOL_MAX_CHOICE)
            GameConfig.loadalternatedol = 0;
    }

    //! Settings: Select DOL Offset from Game
    else if (ret == ++Idx && GameConfig.loadalternatedol == 1)
    {
        GuiWindow * parentWindow = (GuiWindow *) parentElement;
        if(parentWindow) parentWindow->SetState(STATE_DISABLED);
        //alt dol menu for games that require more than a single alt dol
        int autodol = autoSelectDolPrompt((char *) GameConfig.id);
        if(autodol == 0)
        {
            if(parentWindow) parentWindow->SetState(STATE_DEFAULT);
            return MENU_NONE; //Cancel Button pressed
        }

        if (autodol > 0)
        {
            GameConfig.alternatedolstart = autodol;
            snprintf(GameConfig.alternatedolname, sizeof(GameConfig.alternatedolname), "%s <%i>", tr( "AUTO" ), autodol);
            SetOptionValues();
            if(parentWindow) parentWindow->SetState(STATE_DEFAULT);
            return MENU_NONE;
        }

        int res = DiscBrowse(GameConfig.id, GameConfig.alternatedolname, sizeof(GameConfig.alternatedolname));
        if (res >= 0)
        {
            GameConfig.alternatedolstart = res;
            char tmp[170];
            snprintf(tmp, sizeof(tmp), "%s %.6s - %i", tr( "It seems that you have some information that will be helpful to us. Please pass this information along to the DEV team." ), (char *) GameConfig.id, GameConfig.alternatedolstart);
            WindowPrompt(0, tmp, tr( "OK" ));
        }

        if(GameConfig.alternatedolstart == 0)
            GameConfig.loadalternatedol = 0;
        if(parentWindow) parentWindow->SetState(STATE_DEFAULT);
    }

    //! Settings: Block IOS Reload
    else if (ret == ++Idx)
    {
        if(++GameConfig.iosreloadblock >= MAX_ON_OFF) GameConfig.iosreloadblock = 0;
    }

    //! Settings: Game Lock
    else if (ret == ++Idx)
    {
        if (++GameConfig.Locked >= MAX_ON_OFF) GameConfig.Locked = 0;
    }

    SetOptionValues();

    return MENU_NONE;
}

