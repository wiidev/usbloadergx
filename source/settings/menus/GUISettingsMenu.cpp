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
#include "GUISettingsMenu.hpp"
#include "Controls/DeviceHandler.hpp"
#include "settings/CSettings.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "settings/SettingsPrompts.h"
#include "settings/GameTitles.h"
#include "settings/CGameCategories.hpp"
#include "xml/xml.h"
#include "usbloader/GameList.h"
#include "usbloader/wbfs.h"
#include "utils/tools.h"

static const char * OnOffText[MAX_ON_OFF] =
{
    trNOOP( "OFF" ),
    trNOOP( "ON" )
};

static const char * WiilightText[WIILIGHT_MAX] =
{
    trNOOP( "OFF" ),
    trNOOP( "ON" ),
    trNOOP( "Only for Install" )
};

static const char * GameInfoText[GAMEINFO_MAX] =
{
    trNOOP( "Game ID" ),
    trNOOP( "Game Region" ),
    trNOOP( "Both" ),
    trNOOP( "Neither" )
};

static const char * FlipXText[XFLIP_MAX][3] =
{
    { trNOOP( "Right" ), "/", trNOOP( "Next" ) },
    { trNOOP( "Left" ), "/", trNOOP( "Prev" ) },
    { trNOOP( "Like SysMenu" ), "", "" },
    { trNOOP( "Right" ), "/", trNOOP( "Prev" ) },
    { trNOOP( "DiskFlip" ), "", "" }
};

static const char * PromptButtonsText[MAX_ON_OFF] =
{
    trNOOP( "Normal" ),
    trNOOP( "Widescreen Fix" ),
};

static const char * KeyboardText[KEYBOARD_MAX] =
{
    "QWERTY",
    "DVORAK",
    "QWERTZ",
    "AZERTY",
    "QWERTY 2"
};

static const char * DiscArtDownloadText[DISCARTS_MAX_CHOICE] =
{
    trNOOP( "Original/Customs" ),
    trNOOP( "Customs/Original" )
};

static const char *ScreensaverText[SCREENSAVER_MAX] =
{
    trNOOP( "OFF" ),
    trNOOP( "3 min" ),
    trNOOP( "5 min" ),
    trNOOP( "10 min" ),
    trNOOP( "20 min" ),
    trNOOP( "30 min" ),
    trNOOP( "1 hour" )
};

static const char * HomeMenuText[HOME_MENU_MAX_CHOICE] =
{
    trNOOP( "System Default" ),
    trNOOP( "Full Menu" ),
    trNOOP( "Default" )
};

GuiSettingsMenu::GuiSettingsMenu()
    : SettingsMenu(tr("GUI Settings"), &GuiOptions, MENU_NONE)
{
    int Idx = 0;
    Options->SetName(Idx++, "%s", tr( "App Language" ));
    Options->SetName(Idx++, "%s", tr( "Display" ));
    Options->SetName(Idx++, "%s", tr( "Clock" ));
    Options->SetName(Idx++, "%s", tr( "Tooltips" ));
    Options->SetName(Idx++, "%s", tr( "Flip-X" ));
    Options->SetName(Idx++, "%s", tr( "Prompts Buttons" ));
    Options->SetName(Idx++, "%s", tr( "Widescreen Factor" ));
    Options->SetName(Idx++, "%s", tr( "Font Scale Factor" ));
    Options->SetName(Idx++, "%s", tr( "Keyboard" ));
    Options->SetName(Idx++, "%s", tr( "Disc Artwork Download" ));
    Options->SetName(Idx++, "%s", tr( "Wiilight" ));
    Options->SetName(Idx++, "%s", tr( "Rumble" ));
    Options->SetName(Idx++, "%s", tr( "AutoInit Network" ));
    Options->SetName(Idx++, "%s", tr( "Titles from WiiTDB" ));
    Options->SetName(Idx++, "%s", tr( "Cache Titles" ));
    Options->SetName(Idx++, "%s", tr( "Screensaver" ));
    Options->SetName(Idx++, "%s", tr( "Mark new games" ));
    Options->SetName(Idx++, "%s", tr( "Show Free Space" ));
    Options->SetName(Idx++, "%s", tr( "HOME Menu" ));
    Options->SetName(Idx++, "%s", tr( "Import categories from WiiTDB" ));

    SetOptionValues();

    OldTitlesOverride = Settings.titlesOverride;
}

GuiSettingsMenu::~GuiSettingsMenu()
{
    if (Settings.titlesOverride != OldTitlesOverride)
    {
        GameTitles.LoadTitlesFromWiiTDB(Settings.titlestxt_path, true);
        if(!Settings.titlesOverride)
            gameList.ReadGameList();
    }
}

void GuiSettingsMenu::SetOptionValues()
{
    int Idx = 0;

    //! Settings: App Language
    const char * language = strrchr(Settings.language_path, '/');
    if(language)
        language += 1;
    if (!language || strcmp(Settings.language_path, "") == 0)
        Options->SetValue(Idx++, "%s", tr( "Default" ));
    else
        Options->SetValue(Idx++, "%s", language);

    //! Settings: Display
    Options->SetValue(Idx++, "%s", tr( GameInfoText[Settings.sinfo] ));

    //! Settings: Clock
    if (Settings.hddinfo == CLOCK_HR12)
        Options->SetValue(Idx++, "12 %s", tr( "Hour" ));
    else if (Settings.hddinfo == CLOCK_HR24)
        Options->SetValue(Idx++, "24 %s", tr( "Hour" ));
    else if (Settings.hddinfo == OFF)
        Options->SetValue(Idx++, "%s", tr( "OFF" ));

    //! Settings: Tooltips
    Options->SetValue(Idx++, "%s", tr(OnOffText[Settings.tooltips]));

    //! Settings: Flip-X
    Options->SetValue(Idx++, "%s%s%s", tr(FlipXText[Settings.xflip][0]),
                FlipXText[Settings.xflip][1], tr( FlipXText[Settings.xflip][2] ));

    //! Settings: Prompts Buttons
    Options->SetValue(Idx++, "%s", tr( PromptButtonsText[Settings.wsprompt] ));

    //! Settings: Widescreen Factor
    Options->SetValue(Idx++, "%0.3f", Settings.WSFactor);

    //! Settings: Font Scale Factor
    Options->SetValue(Idx++, "%0.3f", Settings.FontScaleFactor);

    //! Settings: Keyboard
    Options->SetValue(Idx++, "%s", KeyboardText[Settings.keyset]);

    //! Settings: Disc Artwork Download
    Options->SetValue(Idx++, "%s", tr( DiscArtDownloadText[Settings.discart] ));

    //! Settings: Wiilight
    Options->SetValue(Idx++, "%s", tr( WiilightText[Settings.wiilight] ));

    //! Settings: Rumble
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.rumble] ));

    //! Settings: AutoInit Network
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.autonetwork] ));

    //! Settings: Titles from WiiTDB
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.titlesOverride] ));

    //! Settings: Cache Titles
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.CacheTitles] ));

    //! Settings: Screensaver
    Options->SetValue(Idx++, "%s", tr( ScreensaverText[Settings.screensaver] ));

    //! Settings: Mark new games
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.marknewtitles] ));

    //! Settings: Show Free Space
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.ShowFreeSpace] ));

    //! Settings: Home Menu style
    Options->SetValue(Idx++, "%s", tr( HomeMenuText[Settings.HomeMenu] ));

    //! Settings: Import categories from WiiTDB
    Options->SetValue(Idx++, " ");
}

int GuiSettingsMenu::GetMenuInternal()
{
    int ret = optionBrowser->GetClickedOption();

    if (ret < 0)
        return MENU_NONE;

    int Idx = -1;

    //! Settings: App Language
    if (ret == ++Idx)
    {
        if (!Settings.godmode)
        {
            WindowPrompt(tr( "Language change:" ), tr( "Console should be unlocked to modify it." ), tr( "OK" ));
            return MENU_NONE;
        }
        SetEffect(EFFECT_FADE, -20);
        while (GetEffect() > 0) usleep(100);
        HaltGui();
        if(parentElement)
        {
            ((GuiWindow *) parentElement)->Remove(this);
            ((GuiWindow *) parentElement)->SetState(STATE_DISABLED);
        }
        ResumeGui();

        int returnhere = 1;
        while (returnhere == 1)
            returnhere = MenuLanguageSelect();

        if (returnhere == 2)
        {
            //! Language changed. Reload game titles with new lang code.
            GameTitles.LoadTitlesFromWiiTDB(Settings.titlestxt_path);
            return MENU_SETTINGS;
        }

        HaltGui();
        if(parentElement)
        {
            ((GuiWindow *) parentElement)->Append(this);
            ((GuiWindow *) parentElement)->SetState(STATE_DEFAULT);
        }
        SetEffect(EFFECT_FADE, 20);
        ResumeGui();
        while (GetEffect() > 0) usleep(100);
    }

    //! Settings: Display
    else if (ret == ++Idx)
    {
        if (++Settings.sinfo >= GAMEINFO_MAX) Settings.sinfo = 0;
    }

    //! Settings: Clock
    else if (ret == ++Idx)
    {
        if (++Settings.hddinfo >= CLOCK_MAX) Settings.hddinfo = 0; //CLOCK
    }

    //! Settings: Tooltips
    else if (ret == ++Idx)
    {
        if (++Settings.tooltips >= MAX_ON_OFF) Settings.tooltips = 0;
    }

    //! Settings: Flip-X
    else if (ret == ++Idx)
    {
        if (++Settings.xflip >= XFLIP_MAX) Settings.xflip = 0;
    }

    //! Settings: Prompts Buttons
    else if (ret == ++Idx)
    {
        if (++Settings.wsprompt >= MAX_ON_OFF) Settings.wsprompt = 0;
    }

    //! Settings: Widescreen Factor
    else if (ret == ++Idx)
    {
        char entrie[20];
        snprintf(entrie, sizeof(entrie), "%0.3f", Settings.WSFactor);
        int ret = OnScreenKeyboard(entrie, sizeof(entrie), 0);
        if(ret)
        {
            for(u32 i = 0; i < sizeof(entrie); ++i)
            {
                if(entrie[i] == ',')
                    entrie[i] = '.';
            }

            Settings.WSFactor = LIMIT(atof(entrie), 0.01f, 1.5f);
        }
    }

    //! Settings: Font Scale Factor
    else if (ret == ++Idx)
    {
        char entrie[20];
        snprintf(entrie, sizeof(entrie), "%0.3f", Settings.FontScaleFactor);
        int ret = OnScreenKeyboard(entrie, sizeof(entrie), 0);
        if(ret)
        {
            for(u32 i = 0; i < sizeof(entrie); ++i)
            {
                if(entrie[i] == ',')
                    entrie[i] = '.';
            }

            Settings.FontScaleFactor = LIMIT(atof(entrie), 0.01f, 1.5f);
        }
    }

    //! Settings: Keyboard
    else if (ret == ++Idx)
    {
        if (++Settings.keyset >= KEYBOARD_MAX) Settings.keyset = 0;
    }

    //! Settings: Disc Artwork Download
    else if (ret == ++Idx)
    {
        if (++Settings.discart >= DISCARTS_MAX_CHOICE) Settings.discart = 0;
    }

    //! Settings: Wiilight
    else if (ret == ++Idx)
    {
        if (++Settings.wiilight >= WIILIGHT_MAX) Settings.wiilight = 0;
    }

    //! Settings: Rumble
    else if (ret == ++Idx)
    {
        if (++Settings.rumble >= MAX_ON_OFF) Settings.rumble = 0; //RUMBLE
    }

    //! Settings: AutoInit Network
    else if (ret == ++Idx)
    {
        if (++Settings.autonetwork >= MAX_ON_OFF) Settings.autonetwork = 0;
    }

    //! Settings: Titles from WiiTDB
    else if (ret == ++Idx)
    {
        if (++Settings.titlesOverride >= MAX_ON_OFF) Settings.titlesOverride = 0;
    }

    //! Settings: Cache Titles
    else if (ret == ++Idx)
    {
        if (++Settings.CacheTitles >= MAX_ON_OFF) Settings.CacheTitles = 0;

        if(Settings.CacheTitles) //! create new cache file
            GameTitles.LoadTitlesFromWiiTDB(Settings.titlestxt_path);
    }

    //! Settings: Screensaver
    else if (ret == ++Idx)
    {
        if (++Settings.screensaver >= SCREENSAVER_MAX) Settings.screensaver = 0;

        SetWPADTimeout();
    }

    //! Settings: Mark new games
    else if (ret == ++Idx)
    {
        if (++Settings.marknewtitles >= MAX_ON_OFF) Settings.marknewtitles = 0;
    }

    //! Settings: Show Free Space
    else if (ret == ++Idx)
    {
        if (++Settings.ShowFreeSpace >= MAX_ON_OFF) Settings.ShowFreeSpace = 0;
    }

    //! Settings: Home Menu Style
    else if (ret == ++Idx)
    {
        if (++Settings.HomeMenu >= HOME_MENU_MAX_CHOICE) Settings.HomeMenu = 0;
    }

    //! Settings: Import categories from WiiTDB
    else if (ret == ++Idx)
    {
        int choice = WindowPrompt(tr("Import categories"), tr("Are you sure you want to import game categories from WiiTDB?"), tr("Yes"), tr("Cancel"));
        if(choice)
        {
            char xmlpath[300];
            snprintf(xmlpath, sizeof(xmlpath), "%swiitdb.xml", Settings.titlestxt_path);
            if(!GameCategories.ImportFromWiiTDB(xmlpath))
            {
                WindowPrompt(tr("Error"), tr("Could not open the WiiTDB.xml file."), tr("OK"));
            }
            else
            {
                GameCategories.Save();
                GameCategories.CategoryList.goToFirst();
                WindowPrompt(tr("Import categories"), tr("Import operation successfully completed."), tr("OK"));
            }
        }
    }

    SetOptionValues();

    return MENU_NONE;
}
