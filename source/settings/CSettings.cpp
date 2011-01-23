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
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CSettings.h"
#include "CGameSettings.h"
#include "CGameStatistics.h"
#include "Controls/DeviceHandler.hpp"
#include "language/gettext.h"
#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "utils/encrypt.h"
#include "svnrev.h"

#define VALID_CONFIG_REV    1031

CSettings Settings;

CSettings::CSettings()
{
    CONF_Init();
    strcpy(BootDevice, "sd:");
    snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/", BootDevice);
    this->SetDefault();
}

CSettings::~CSettings()
{
}

void CSettings::SetDefault()
{
    snprintf(covers_path, sizeof(covers_path), "%simages/", ConfigPath);
    snprintf(covers2d_path, sizeof(covers2d_path), "%simages/2D/", ConfigPath);
    snprintf(disc_path, sizeof(disc_path), "%simages/disc/", ConfigPath);
    snprintf(titlestxt_path, sizeof(titlestxt_path), "%s", ConfigPath);
    snprintf(languagefiles_path, sizeof(languagefiles_path), "%slanguage/", ConfigPath);
    snprintf(update_path, sizeof(update_path), "%s/apps/usbloader_gx/", BootDevice);
    snprintf(homebrewapps_path, sizeof(homebrewapps_path), "%s/apps/", BootDevice);
    snprintf(Cheatcodespath, sizeof(Cheatcodespath), "%s/codes/", BootDevice);
    snprintf(TxtCheatcodespath, sizeof(TxtCheatcodespath), "%s/txtcodes/", BootDevice);
    snprintf(BcaCodepath, sizeof(BcaCodepath), "%s/bca/", BootDevice);
    snprintf(WipCodepath, sizeof(WipCodepath), "%s/wip/", BootDevice);
    snprintf(WDMpath, sizeof(WDMpath), "%s/wdm/", BootDevice);
    snprintf(theme_path, sizeof(theme_path), "%stheme/", ConfigPath);
    snprintf(dolpath, sizeof(dolpath), "%s/", BootDevice);
    strcpy(theme, "");
    strcpy(language_path, "");
    strcpy(ogg_path, "");
    strcpy(unlockCode, "");
    strcpy(db_language, "");
    strcpy(returnTo, "");

    godmode = 1;
    videomode = VIDEO_MODE_SYSDEFAULT;
    videopatch = OFF;
    language = CONSOLE_DEFAULT;
    ocarina = OFF;
    hddinfo = CLOCK_HR12;
    sinfo = ON;
    rumble = ON;
    GameSort = SORT_ABC;
    volume = 80;
    sfxvolume = 80;
    gamesoundvolume = 80;
    tooltips = ON;
    gamesound = ON;
    parentalcontrol = PARENTAL_LVL_ADULT;
    cios = 249;
    gridRows = 3;
    error002 = 2;
    partition = -1;
    discart = DISCARTS_ORIGINALS_CUSTOMS;
    xflip = XFLIP_NO;
    quickboot = OFF;
    wiilight = WIILIGHT_ON;
    autonetwork = OFF;
    patchcountrystrings = OFF;
    titlesOverride = ON;
    screensaver = SCREENSAVER_10_MIN;
    musicloopmode = ON;
    marknewtitles = ON;
    ShowFreeSpace = ON;
    PlaylogUpdate = ON;
    UseIOS58 = OFF;
    ParentalBlocks = BLOCK_ALL;
    InstallToDir = INSTALL_TO_NAME_GAMEID;
    GameSplit = GAMESPLIT_4GB;
    InstallPartitions = ONLY_GAME_PARTITION;
    widescreen = (CONF_GetAspectRatio() == CONF_ASPECT_16_9);
    HomeMenu = HOME_MENU_DEFAULT;
}

bool CSettings::Load()
{
    FindConfig();
    //! Reset default path variables to the right device
    SetDefault();

    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%sGXGlobal.cfg", ConfigPath);

    FILE * file = fopen(filepath, "r");
    if (!file) return false;

    if(!ValidVersion(file))
    {
        fclose(file);
        return false;
    }

    char line[1024];

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#') continue;

        this->ParseLine(line);
    }
    fclose(file);

    return true;
}

bool CSettings::ValidVersion(FILE * file)
{
    if(!file) return false;

    char line[255];
    int revision = 0;

    while (fgets(line, sizeof(line), file))
    {
        const char * ptr = strcasestr(line, "USB Loader GX R");
        if(ptr)
        {
            ptr += strlen("USB Loader GX R");
            revision = atoi(ptr);
            break;
        }
    }

    rewind(file);

    return revision >= VALID_CONFIG_REV;
}

bool CSettings::Reset()
{
    this->SetDefault();

    if (this->Save()) return true;

    return false;
}

bool CSettings::Save()
{
    if (!FindConfig()) return false;

    char filedest[300];
    snprintf(filedest, sizeof(filedest), "%sGXGlobal.cfg", ConfigPath);

    if(!CreateSubfolder(ConfigPath))	return false;

    FILE * file = fopen(filedest, "w");
    if (!file) return false;

    fprintf(file, "# USB Loader GX R%s - Main settings file\n", GetRev());
    fprintf(file, "# Note: This file is automatically generated\n ");
    fprintf(file, "godmode = %d\n ", godmode);
    fprintf(file, "videomode = %d\n ", videomode);
    fprintf(file, "videopatch = %d\n ", videopatch);
    fprintf(file, "language = %d\n ", language);
    fprintf(file, "ocarina = %d\n ", ocarina);
    fprintf(file, "hddinfo = %d\n ", hddinfo);
    fprintf(file, "sinfo = %d\n ", sinfo);
    fprintf(file, "rumble = %d\n ", rumble);
    fprintf(file, "volume = %d\n ", volume);
    fprintf(file, "sfxvolume = %d\n ", sfxvolume);
    fprintf(file, "gamesoundvolume = %d\n ", gamesoundvolume);
    fprintf(file, "tooltips = %d\n ", tooltips);
    char EncryptedTxt[50];
    EncryptString(unlockCode, EncryptedTxt);
    fprintf(file, "password = %s\n ", EncryptedTxt);
    fprintf(file, "GameSort = %d\n ", GameSort);
    fprintf(file, "cios = %d\n ", cios);
    fprintf(file, "keyset = %d\n ", keyset);
    fprintf(file, "xflip = %d\n ", xflip);
    fprintf(file, "gridRows = %d\n ", gridRows);
    fprintf(file, "quickboot = %d\n ", quickboot);
    fprintf(file, "wsprompt = %d\n ", wsprompt);
    fprintf(file, "parentalcontrol = %d\n ", parentalcontrol);
    fprintf(file, "covers_path = %s\n ", covers_path);
    fprintf(file, "covers2d_path = %s\n ", covers2d_path);
    fprintf(file, "theme_path = %s\n ", theme_path);
    fprintf(file, "theme = %s\n ", theme);
    fprintf(file, "disc_path = %s\n ", disc_path);
    fprintf(file, "language_path = %s\n ", language_path);
    fprintf(file, "languagefiles_path = %s\n ", languagefiles_path);
    fprintf(file, "TxtCheatcodespath = %s\n ", TxtCheatcodespath);
    fprintf(file, "titlestxt_path = %s\n ", titlestxt_path);
    fprintf(file, "gamesound = %d\n ", gamesound);
    fprintf(file, "dolpath = %s\n ", dolpath);
    fprintf(file, "ogg_path = %s\n ", ogg_path);
    fprintf(file, "wiilight = %d\n ", wiilight);
    fprintf(file, "gameDisplay = %d\n ", gameDisplay);
    fprintf(file, "update_path = %s\n ", update_path);
    fprintf(file, "homebrewapps_path = %s\n ", homebrewapps_path);
    fprintf(file, "Cheatcodespath = %s\n ", Cheatcodespath);
    fprintf(file, "BcaCodepath = %s\n ", BcaCodepath);
    fprintf(file, "WipCodepath = %s\n ", WipCodepath);
    fprintf(file, "WDMpath = %s\n ", WDMpath);
    fprintf(file, "titlesOverride = %d\n ", titlesOverride);
    fprintf(file, "patchcountrystrings = %d\n ", patchcountrystrings);
    fprintf(file, "screensaver = %d\n ", screensaver);
    fprintf(file, "musicloopmode = %d\n ", musicloopmode);
    fprintf(file, "error002 = %d\n ", error002);
    fprintf(file, "autonetwork = %d\n ", autonetwork);
    fprintf(file, "discart = %d\n ", discart);
    fprintf(file, "partition = %d\n ", partition);
    fprintf(file, "marknewtitles = %d\n ", marknewtitles);
    fprintf(file, "ShowFreeSpace = %d\n ", ShowFreeSpace);
    fprintf(file, "InstallToDir = %d\n ", InstallToDir);
    fprintf(file, "GameSplit = %d\n ", GameSplit);
    fprintf(file, "InstallPartitions = %08X\n ", InstallPartitions);
    fprintf(file, "PlaylogUpdate = %d\n ", PlaylogUpdate);
    fprintf(file, "UseIOS58 = %d\n ", UseIOS58);
    fprintf(file, "ParentalBlocks = %08X\n ", ParentalBlocks);
    fprintf(file, "returnTo = %s\n ", returnTo);
    fprintf(file, "HomeMenu = %d\n ", HomeMenu);
    fclose(file);

    return true;
}

bool CSettings::SetSetting(char *name, char *value)
{
    int i = 0;

    if (strcmp(name, "godmode") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            godmode = i;
        }
        return true;
    }
    else if (strcmp(name, "videomode") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            videomode = i;
        }
        return true;
    }
    else if (strcmp(name, "videopatch") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            videopatch = i;
        }
        return true;
    }
    else if (strcmp(name, "language") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            language = i;
        }
        return true;
    }
    else if (strcmp(name, "ocarina") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            ocarina = i;
        }
        return true;
    }
    else if (strcmp(name, "hddinfo") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            hddinfo = i;
        }
        return true;
    }
    else if (strcmp(name, "sinfo") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            sinfo = i;
        }
        return true;
    }
    else if (strcmp(name, "rumble") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            rumble = i;
        }
        return true;
    }
    else if (strcmp(name, "volume") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            volume = i;
        }
        return true;
    }
    else if (strcmp(name, "sfxvolume") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            sfxvolume = i;
        }
        return true;
    }
    else if (strcmp(name, "gamesoundvolume") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            gamesoundvolume = i;
        }
        return true;
    }
    else if (strcmp(name, "tooltips") == 0)
    {
        if (sscanf(value, "%d", &i) == 1)
        {
            tooltips = i;
        }
        return true;
    }
    else if (strcmp(name, "password") == 0)
    {
        char EncryptedTxt[50];
        strcpy(EncryptedTxt, value);
        DecryptString(EncryptedTxt, unlockCode);
        return true;
    }
    else if (strcmp(name, "GameSort") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) GameSort = i;
        return true;
    }
    else if (strcmp(name, "cios") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) cios = i;
        return true;
    }
    else if (strcmp(name, "keyset") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) keyset = i;
        return true;
    }
    else if (strcmp(name, "xflip") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) xflip = i;
        return true;
    }
    else if (strcmp(name, "gridRows") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) gridRows = i;
        return true;
    }
    else if (strcmp(name, "quickboot") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) quickboot = i;
        return true;
    }
    else if (strcmp(name, "partition") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) partition = i;
        return true;
    }
    else if (strcmp(name, "wsprompt") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) wsprompt = i;
        return true;
    }
    else if (strcmp(name, "gameDisplay") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) gameDisplay = i;
        return true;
    }
    else if (strcmp(name, "parentalcontrol") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) parentalcontrol = i;
        return true;
    }
    else if (strcmp(name, "screensaver") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) screensaver = i;
        return true;
    }
    else if (strcmp(name, "titlesOverride") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) titlesOverride = i;
        return true;
    }
    else if (strcmp(name, "musicloopmode") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) musicloopmode = i;
        return true;
    }
    else if (strcmp(name, "gamesound") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) gamesound = i;
        return true;
    }
    else if (strcmp(name, "wiilight") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) wiilight = i;
        return true;
    }
    else if (strcmp(name, "marknewtitles") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) marknewtitles = i;
        return true;
    }
    else if (strcmp(name, "ShowFreeSpace") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) ShowFreeSpace = i;
        return true;
    }
    else if (strcmp(name, "HomeMenu") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) HomeMenu = i;
        return true;
    }
    else if (strcmp(name, "patchcountrystrings") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) patchcountrystrings = i;
        return true;
    }
    else if (strcmp(name, "discart") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) discart = i;
        return true;
    }
    else if (strcmp(name, "error002") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) error002 = i;
        return true;
    }
    else if (strcmp(name, "autonetwork") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) autonetwork = i;
        return true;
    }
    else if (strcmp(name, "InstallToDir") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) InstallToDir = i;
        return true;
    }
    else if (strcmp(name, "GameSplit") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) GameSplit = i;
        return true;
    }
    else if (strcmp(name, "PlaylogUpdate") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) PlaylogUpdate = i;
        return true;
    }
    else if (strcmp(name, "UseIOS58") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) UseIOS58 = i;
        return true;
    }
    else if (strcmp(name, "InstallPartitions") == 0)
    {
        InstallPartitions = strtoul(value, 0, 16);
        return true;
    }
    else if (strcmp(name, "ParentalBlocks") == 0)
    {
        ParentalBlocks = strtoul(value, 0, 16);
        return true;
    }
    else if (strcmp(name, "covers_path") == 0)
    {
        strcpy(covers_path, value);
        return true;
    }
    else if (strcmp(name, "covers2d_path") == 0)
    {
        strcpy(covers2d_path, value);
        return true;
    }
    else if (strcmp(name, "theme_path") == 0)
    {
        strcpy(theme_path, value);
        return true;
    }
    else if (strcmp(name, "theme") == 0)
    {
        strcpy(theme, value);
        return true;
    }
    else if (strcmp(name, "disc_path") == 0)
    {
        strcpy(disc_path, value);
        return true;
    }
    else if (strcmp(name, "language_path") == 0)
    {
        strcpy(language_path, value);
        return true;
    }
    else if (strcmp(name, "languagefiles_path") == 0)
    {
        strcpy(languagefiles_path, value);
        return true;
    }
    else if (strcmp(name, "TxtCheatcodespath") == 0)
    {
        strcpy(TxtCheatcodespath, value);
        return true;
    }
    else if (strcmp(name, "titlestxt_path") == 0)
    {
        strcpy(titlestxt_path, value);
        return true;
    }
    else if (strcmp(name, "dolpath") == 0)
    {
        strcpy(dolpath, value);
        return true;
    }
    else if (strcmp(name, "ogg_path") == 0)
    {
        strcpy(ogg_path, value);
        return true;
    }
    else if (strcmp(name, "update_path") == 0)
    {
        strcpy(update_path, value);
        return true;
    }
    else if (strcmp(name, "homebrewapps_path") == 0)
    {
        strcpy(homebrewapps_path, value);
        return true;
    }
    else if (strcmp(name, "Cheatcodespath") == 0)
    {
        strcpy(Cheatcodespath, value);
        return true;
    }
    else if (strcmp(name, "BcaCodepath") == 0)
    {
        strcpy(BcaCodepath, value);
        return true;
    }
    else if (strcmp(name, "WipCodepath") == 0)
    {
        strcpy(WipCodepath, value);
        return true;
    }
    else if (strcmp(name, "WDMpath") == 0)
    {
        strcpy(WDMpath, value);
        return true;
    }
    else if (strcmp(name, "returnTo") == 0)
    {
        strcpy(returnTo, value);
        return true;
    }

    return false;
}

bool CSettings::FindConfig()
{
    bool found = false;
    char CheckDevice[10];
    char CheckPath[300];

    for (int i = SD; i < MAXDEVICES; ++i)
    {
        sprintf(CheckDevice, "%s:", DeviceName[i]);

        if(!found)
        {
            strcpy(BootDevice, CheckDevice);
            snprintf(ConfigPath, sizeof(ConfigPath), "%s/apps/usbloader_gx/", BootDevice);
            snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
            found = CheckFile(CheckPath);
        }
        if(!found)
        {
            strcpy(BootDevice, CheckDevice);
            snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/", BootDevice);
            snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
            found = CheckFile(CheckPath);
        }
    }

    if (!found)
    {
        FILE * testFp = NULL;
        //! No existing config so try to find a place where we can write it too
        for (int i = SD; i < MAXDEVICES; ++i)
        {
            sprintf(CheckDevice, "%s:", DeviceName[i]);

            if (!found)
            {
                strcpy(BootDevice, CheckDevice);
                snprintf(ConfigPath, sizeof(ConfigPath), "%s/apps/usbloader_gx/", BootDevice);
                snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
                testFp = fopen(CheckPath, "wb");
                found = (testFp != NULL);
                fclose(testFp);
            }
            if (!found)
            {
                strcpy(BootDevice, CheckDevice);
                snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/", BootDevice);
                CreateSubfolder(ConfigPath);
                snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
                testFp = fopen(CheckPath, "wb");
                found = (testFp != NULL);
                fclose(testFp);
            }
        }
    }

    return found;
}

void CSettings::ParseLine(char *line)
{
    char temp[1024], name[1024], value[1024];

    strncpy(temp, line, sizeof(temp));

    char * eq = strchr(temp, '=');

    if (!eq) return;

    *eq = 0;

    this->TrimLine(name, temp, sizeof(name));
    this->TrimLine(value, eq + 1, sizeof(value));

    this->SetSetting(name, value);
}

void CSettings::TrimLine(char *dest, char *src, int size)
{
    int len;
    while (*src == ' ')
        src++;
    len = strlen(src);
    while (len > 0 && strchr(" \r\n", src[len - 1]))
        len--;
    if (len >= size) len = size - 1;
    strncpy(dest, src, len);
    dest[len] = 0;
}

//! Get language code from the selected language file
//! eg. german.lang = DE and default to EN
static inline const char * GetLangCode(const char * langpath)
{
    if(strcasestr(langpath, "japanese"))
        return "JA";

    else if(strcasestr(langpath, "german"))
        return "DE";

    else if(strcasestr(langpath, "french"))
        return "FR";

    else if(strcasestr(langpath, "spanish"))
        return "ES";

    else if(strcasestr(langpath, "italian"))
        return "IT";

    else if(strcasestr(langpath, "dutch"))
        return "NL";

    else if(strcasestr(langpath, "schinese"))
        return "ZHCN";

    else if(strcasestr(langpath, "tchinese"))
        return "ZHTW";

    else if(strcasestr(langpath, "korean"))
        return "KO";

    return "EN";
}

bool CSettings::LoadLanguage(const char *path, int lang)
{
    bool ret = false;

    if (path && strlen(path) > 3)
    {
        ret = gettextLoadLanguage(path);
        if (ret)
        {
            snprintf(language_path, sizeof(language_path), path);
            strcpy(db_language, GetLangCode(language_path));
        }
        else
            return LoadLanguage(NULL, CONSOLE_DEFAULT);
    }
    else if (lang >= 0)
    {
        char filepath[150];
        char langpath[150];
        snprintf(langpath, sizeof(langpath), "%s", languagefiles_path);
        if (langpath[strlen(langpath) - 1] != '/')
        {
            char * ptr = strrchr(langpath, '/');
            if (ptr)
            {
                ptr++;
                ptr[0] = '\0';
            }
        }

        if (lang == CONSOLE_DEFAULT)
        {
            return LoadLanguage(NULL, CONF_GetLanguage());
        }
        else if (lang == JAPANESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/japanese.lang", langpath);
        }
        else if (lang == ENGLISH)
        {
            snprintf(filepath, sizeof(filepath), "%s/english.lang", langpath);
        }
        else if (lang == GERMAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/german.lang", langpath);
        }
        else if (lang == FRENCH)
        {
            snprintf(filepath, sizeof(filepath), "%s/french.lang", langpath);
        }
        else if (lang == SPANISH)
        {
            snprintf(filepath, sizeof(filepath), "%s/spanish.lang", langpath);
        }
        else if (lang == ITALIAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/italian.lang", langpath);
        }
        else if (lang == DUTCH)
        {
            snprintf(filepath, sizeof(filepath), "%s/dutch.lang", langpath);
        }
        else if (lang == S_CHINESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/schinese.lang", langpath);
        }
        else if (lang == T_CHINESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/tchinese.lang", langpath);
        }
        else if (lang == KOREAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/korean.lang", langpath);
        }

        strcpy(db_language, GetLangCode(filepath));
        ret = gettextLoadLanguage(filepath);
        if (ret)
            snprintf(language_path, sizeof(language_path), filepath);
    }

    return ret;
}
