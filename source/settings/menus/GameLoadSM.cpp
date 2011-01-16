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
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "GameLoadSM.hpp"

static const char * OnOffText[MAX_ON_OFF] =
{
    trNOOP( "OFF" ),
    trNOOP( "ON" )
};

static const char * VideoModeText[VIDEO_MODE_MAX] =
{
    trNOOP( "System Default" ),
    trNOOP( "Disc Default" ),
    trNOOP( "Force PAL50" ),
    trNOOP( "Force PAL60" ),
    trNOOP( "Force NTSC" ),
    trNOOP( "Region Patch" ),
};

static const char * LanguageText[MAX_LANGUAGE] =
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

static const char * Error002Text[3] =
{
    trNOOP( "No" ),
    trNOOP( "Yes" ),
    trNOOP( "Anti" )
};

static const char * ParentalText[5] =
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
};

GameLoadSM::GameLoadSM(const char * GameID)
    : SettingsMenu(tr("Game Load"), &GuiOptions, MENU_NONE)
{
    //! Setup default settings from global settings
    snprintf(GameConfig.id, sizeof(GameConfig.id), "%s", (char *) GameID);
    SetDefaultConfig();

    GameCFG * existCFG = GameSettings.GetGameCFG(GameID);

    //! Overwrite with existing if available
	if (existCFG)
	    memcpy(&GameConfig, existCFG, sizeof(GameCFG));

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
    GameConfig.video = Settings.videomode;
    GameConfig.language = Settings.language;
    GameConfig.ocarina = Settings.ocarina;
    GameConfig.vipatch = Settings.videopatch;
    GameConfig.ios = Settings.cios;
    GameConfig.parentalcontrol = 0;
    GameConfig.errorfix002 = Settings.error002;
    GameConfig.patchcountrystrings = Settings.patchcountrystrings;
    GameConfig.loadalternatedol = OFF;
    GameConfig.alternatedolstart = 0;
    GameConfig.iosreloadblock = OFF;
    strcpy(GameConfig.alternatedolname, "");
    GameConfig.returnTo = 1;
    GameConfig.Locked = 0;
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
    Options->SetValue(Idx++, "%s", tr(VideoModeText[GameConfig.video]));

    //! Settings: VIDTV Patch
    Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.vipatch]));

    //! Settings: Game Language
    Options->SetValue(Idx++, "%s", tr(LanguageText[GameConfig.language]));

    //! Settings: Patch Country Strings
    Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.patchcountrystrings]));

    //! Settings: Ocarina
    Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.ocarina]));

    //! Settings: Game IOS
    Options->SetValue(Idx++, "%i", GameConfig.ios);

    //! Settings: Parental Control
    Options->SetValue(Idx++, "%s", tr(ParentalText[GameConfig.parentalcontrol]));

    //! Settings: Error 002 fix
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
    Options->SetValue(Idx++, "%s", tr( OnOffText[GameConfig.iosreloadblock] ));

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
        if (++GameConfig.video >= VIDEO_MODE_MAX) GameConfig.video = 0;
    }

    //! Settings: VIDTV Patch
    else if (ret == ++Idx)
    {
        if (++GameConfig.vipatch >= MAX_ON_OFF) GameConfig.vipatch = 0;
    }

    //! Settings: Game Language
    else if (ret == ++Idx)
    {
        if (++GameConfig.language >= MAX_LANGUAGE) GameConfig.language = 0;
    }

    //! Settings: Patch Country Strings
    else if (ret == ++Idx)
    {
        if (++GameConfig.patchcountrystrings >= MAX_ON_OFF) GameConfig.patchcountrystrings = 0;
    }

    //! Settings: Ocarina
    else if (ret == ++Idx)
    {
        if (++GameConfig.ocarina >= MAX_ON_OFF) GameConfig.ocarina = 0;
    }

    //! Settings: Game IOS
    else if (ret == ++Idx)
    {
        char entered[4];
        snprintf(entered, sizeof(entered), "%i", GameConfig.ios);
        if(OnScreenKeyboard(entered, sizeof(entered), 0))
        {
            GameConfig.ios = atoi(entered);
            if(GameConfig.ios < 200) GameConfig.ios = 200;
            else if(GameConfig.ios > 255) GameConfig.ios = 255;

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
        if (++GameConfig.errorfix002 >= 3) GameConfig.errorfix002 = 0;
    }

    //! Settings: Return To
    else if (ret == ++Idx)
    {
        if (++GameConfig.returnTo >= MAX_ON_OFF) GameConfig.returnTo = 0;
    }

    //! Settings: Alternate DOL
    else if (ret == ++Idx)
    {
        if (++GameConfig.loadalternatedol > 3)
            GameConfig.loadalternatedol = 0;
    }

    //! Settings: Select DOL Offset from Game
    else if (ret == ++Idx && GameConfig.loadalternatedol == 1)
    {
        char filename[10];
        snprintf(filename, 7, "%s", GameConfig.id);

        //alt dol menu for games that require more than a single alt dol
        int autodol = autoSelectDolMenu(filename, false);

        if (autodol > 0)
        {
            GameConfig.alternatedolstart = autodol;
            snprintf(GameConfig.alternatedolname, sizeof(GameConfig.alternatedolname), "%s <%i>", tr( "AUTO" ), autodol);
            SetOptionValues();
            return MENU_NONE;
        }
        else if (autodol == 0)
        {
            GameConfig.loadalternatedol = 0;
            SetOptionValues();
            return MENU_NONE;
        }

        //check to see if we already know the offset of the correct dol
        autodol = autoSelectDol(filename, false);
        //if we do know that offset ask if they want to use it
        if (autodol > 0)
        {
            int dolchoice = WindowPrompt(0, tr( "Do you want to use the alternate DOL that is known to be correct?" ),
                                            tr( "Yes" ), tr( "Pick from a list" ), tr( "Cancel" ));
            if (dolchoice == 1)
            {
                GameConfig.alternatedolstart = autodol;
                snprintf(GameConfig.alternatedolname, sizeof(GameConfig.alternatedolname), "%s <%i>", tr( "AUTO" ), autodol);
            }
            else if (dolchoice == 2) //they want to search for the correct dol themselves
            {
                int res = DiscBrowse(GameConfig.id, GameConfig.alternatedolname, sizeof(GameConfig.alternatedolname));
                if (res >= 0)
                    GameConfig.alternatedolstart = res;
            }
        }
        else
        {
            int res = DiscBrowse(GameConfig.id, GameConfig.alternatedolname, sizeof(GameConfig.alternatedolname));
            if (res >= 0)
            {
                GameConfig.alternatedolstart = res;
                char tmp[170];
                snprintf(tmp, sizeof(tmp), "%s %s - %i", tr( "It seems that you have some information that will be helpful to us. Please pass this information along to the DEV team." ), filename, GameConfig.alternatedolstart);
                WindowPrompt(0, tmp, tr( "OK" ));
            }
        }

        if(GameConfig.alternatedolstart == 0)
            GameConfig.loadalternatedol = 0;
    }

    //! Settings: Block IOS Reload
    else if (ret == ++Idx)
    {
        if (++GameConfig.iosreloadblock >= MAX_ON_OFF) GameConfig.iosreloadblock = 0;
    }

    //! Settings: Game Lock
    else if (ret == ++Idx)
    {
        if (++GameConfig.Locked >= MAX_ON_OFF) GameConfig.Locked = 0;
    }

    SetOptionValues();

    return MENU_NONE;
}

