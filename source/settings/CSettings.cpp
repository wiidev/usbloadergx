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
#include "language/gettext.h"
#include "themes/CTheme.h"
#include "listfiles.h"

#define DEFAULT_APP_PATH    "apps/usbloader_gx/"
#define CONFIGPATH          "config/"
#define CONFIGNAME          "GXGlobal.cfg"

CSettings Settings;

CSettings::CSettings()
{
    CONF_Init();
    strcpy(BootDevice, "SD:");
    this->SetDefault();
}

CSettings::~CSettings()
{
}

void CSettings::SetDefault()
{
    snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/GXGlobal.cfg", BootDevice);
    snprintf(covers_path, sizeof(covers_path), "%s/images/", BootDevice);
    snprintf(covers2d_path, sizeof(covers2d_path), "%s/images/2D/", BootDevice);
    snprintf(disc_path, sizeof(disc_path), "%s/images/disc/", BootDevice);
    snprintf(titlestxt_path, sizeof(titlestxt_path), "%s/config/", BootDevice);
    snprintf(language_path, sizeof(language_path), "notset");
    snprintf(languagefiles_path, sizeof(languagefiles_path), "%s/config/language/", BootDevice);
    snprintf(update_path, sizeof(update_path), "%s/apps/usbloader_gx/", BootDevice);
    snprintf(theme_downloadpath, sizeof(theme_downloadpath), "%s/config/themes/", BootDevice);
    snprintf(homebrewapps_path, sizeof(homebrewapps_path), "%s/apps/", BootDevice);
    snprintf(Cheatcodespath, sizeof(Cheatcodespath), "%s/codes/", BootDevice);
    snprintf(TxtCheatcodespath, sizeof(TxtCheatcodespath), "%s/txtcodes/", BootDevice);
    snprintf(BcaCodepath, sizeof(BcaCodepath), "%s/bca/", BootDevice);
    snprintf(WipCodepath, sizeof(WipCodepath), "%s/wip/", BootDevice);
    snprintf(theme_path, sizeof(theme_path), "%s/theme/", BootDevice);
    snprintf(dolpath, sizeof(dolpath), "%s/", BootDevice);
    strcpy(ogg_path, "");
    strcpy(unlockCode, "");

    videomode = discdefault;
    videopatch = off;
    language = ConsoleLangDefault;
    ocarina = off;
    hddinfo = hr12;
    sinfo = on;
    rumble = RumbleOn;
    volume = 80;
    sfxvolume = 80;
    gamesoundvolume = 80;
    tooltips = TooltipsOn;
    gamesound = 1;
    parentalcontrol = 0;
    cios = 249;
    xflip = no;
    qboot = no;
    wiilight = 1;
    autonetwork = 0;
    discart = 0;
    patchcountrystrings = 0;
    gridRows = 3;
    error002 = 2;
    titlesOverride = 1;
    db_JPtoEN = 0;
    screensaver = 3;
    musicloopmode = 1;
    partition = -1;
    marknewtitles = 1;
    FatInstallToDir = 0;
    partitions_to_install = install_game_only;
    fullcopy = 0;
    beta_upgrades = 0;
    strcpy(db_url, "");
    strcpy(db_language, "");
    strcpy(unlockCode, "");
    strcpy(returnTo, "");

    memset(&Parental, 0, sizeof(Parental));

    char buf[0x4a];
    s32 res = CONF_Get("IPL.PC", buf, 0x4A);
    if (res > 0)
    {
        if (buf[2] != 0x14)
        {
            Parental.enabled = 1;
            Parental.rating = buf[2];
        }
        Parental.question = buf[7];
        memcpy(Parental.pin, buf + 3, 4);
        memcpy(Parental.answer, buf + 8, 32);
    }
    widescreen = (CONF_GetAspectRatio() == CONF_ASPECT_16_9);
    godmode = (Parental.enabled == 0) ? 1 : 0;

    Theme.SetDefault(); //! We need to move this later
}

bool CSettings::Load()
{
    FindConfig();

    char line[1024];
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s", ConfigPath);

    file = fopen(filepath, "r");
    if (!file) return false;

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#') continue;

        this->ParseLine(line);
    }
    fclose(file);

    //!The following needs to be moved later
    CFG_LoadGameNum();

    snprintf(filepath, sizeof(filepath), "%sGXtheme.cfg", theme_path);
    Theme.Load(filepath);

    return true;

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

    char filedest[100];
    snprintf(filedest, sizeof(filedest), "%s", ConfigPath);

    char * tmppath = strrchr(filedest, '/');
    if (tmppath)
    {
        tmppath++;
        tmppath[0] = '\0';
    }

    subfoldercreate(filedest);

    file = fopen(ConfigPath, "w");
    if (!file) return false;

    fprintf(file, "# USB Loader global settings file\n");
    fprintf(file, "# Note: This file is automatically generated\n ");
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
    fprintf(file, "password = %s\n ", unlockCode);
    fprintf(file, "sort = %d\n ", sort);
    fprintf(file, "fave = %d\n ", fave);
    fprintf(file, "cios = %d\n ", cios);
    fprintf(file, "keyset = %d\n ", keyset);
    fprintf(file, "xflip = %d\n ", xflip);
    fprintf(file, "gridRows = %d\n ", gridRows);
    fprintf(file, "qboot = %d\n ", qboot);
    fprintf(file, "wsprompt = %d\n ", wsprompt);
    fprintf(file, "parentalcontrol = %d\n ", parentalcontrol);
    fprintf(file, "cover_path = %s\n ", covers_path);
    fprintf(file, "cover2d_path = %s\n ", covers2d_path);
    fprintf(file, "theme_path = %s\n ", theme_path);
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
    fprintf(file, "theme_downloadpath = %s\n ", theme_downloadpath);
    fprintf(file, "homebrewapps_path = %s\n ", homebrewapps_path);
    fprintf(file, "Cheatcodespath = %s\n ", Cheatcodespath);
    fprintf(file, "BcaCodepath = %s\n ", BcaCodepath);
    fprintf(file, "WipCodepath = %s\n ", WipCodepath);
    fprintf(file, "titlesOverride = %d\n ", titlesOverride);
    fprintf(file, "patchcountrystrings = %d\n ", patchcountrystrings);
    fprintf(file, "screensaver = %d\n ", screensaver);
    fprintf(file, "musicloopmode = %d\n ", musicloopmode);
    fprintf(file, "error002 = %d\n ", error002);
    fprintf(file, "autonetwork = %d\n ", autonetwork);
    fprintf(file, "discart = %d\n ", discart);
    fprintf(file, "partition = %d\n ", partition);
    fprintf(file, "marknewtitles = %d\n ", marknewtitles);
    fprintf(file, "fatInstallToDir = %d\n ", FatInstallToDir);
    fprintf(file, "partitions = %d\n ", partitions_to_install);
    fprintf(file, "fullcopy = %d\n ", fullcopy);
    fprintf(file, "beta_upgrades = %d\n ", beta_upgrades);
    fprintf(file, "returnTo = %s\n ", returnTo);
    fclose(file);

    return true;
}

bool CSettings::SetSetting(char *name, char *value)
{
    int i = 0;

    if (strcmp(name, "videomode") == 0)
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
    else if (strcmp(name, "unlockCode") == 0)
    {
        strcpy(unlockCode, value);
        return true;
    }
    else if (strcmp(name, "sort") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) sort = i;
        return true;
    }
    else if (strcmp(name, "fave") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) fave = i;
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
    else if (strcmp(name, "qboot") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) qboot = i;
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
    else if (strcmp(name, "patchcountrystrings") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) patchcountrystrings = i;
        return true;
    }
    else if (strcmp(name, "fullcopy") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) fullcopy = i;
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
    else if (strcmp(name, "FatInstallToDir") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) FatInstallToDir = i;
        return true;
    }
    else if (strcmp(name, "beta_upgrades") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) beta_upgrades = i;
        return true;
    }
    else if (strcmp(name, "partitions_to_install") == 0)
    {
        if (sscanf(value, "%d", &i) == 1) partitions_to_install = i;
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
    else if (strcmp(name, "theme_downloadpath") == 0)
    {
        strcpy(theme_downloadpath, value);
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
    strcpy(BootDevice, "SD:");

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) strcpy(BootDevice, "USB:");

        snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/GXGlobal.cfg", BootDevice);
        if ((found = checkfile(ConfigPath))) break;

        snprintf(ConfigPath, sizeof(ConfigPath), "%s/apps/usbloader_gx/GXGlobal.cfg", BootDevice);
        if ((found = checkfile(ConfigPath))) break;
    }

    if (!found)
    {
        FILE * testFp = NULL;
        strcpy(BootDevice, "SD:");
        //! No existing config so try to find a place where we can write it too
        for (int i = 0; i < 2; ++i)
        {
            if (i == 1) strcpy(BootDevice, "USB:");
            if (!found)
            {
                snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/GXGlobal.cfg", BootDevice);
                testFp = fopen(ConfigPath, "wb");
                found = (testFp != NULL);
                fclose(testFp);
            }
            if (!found)
            {
                snprintf(ConfigPath, sizeof(ConfigPath), "%s/apps/usbloader_gx/GXGlobal.cfg", BootDevice);
                testFp = fopen(ConfigPath, "wb");
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

static inline const char * GetLangCode(int langid)
{
    switch (langid)
    {
        case CONF_LANG_JAPANESE:
            return "JA";
        case CONF_LANG_ENGLISH:
            return "EN";
        case CONF_LANG_GERMAN:
            return "DE";
        case CONF_LANG_FRENCH:
            return "FR";
        case CONF_LANG_SPANISH:
            return "ES";
        case CONF_LANG_ITALIAN:
            return "IT";
        case CONF_LANG_DUTCH:
            return "NL";
        case CONF_LANG_SIMP_CHINESE:
            return "ZHCN";
        case CONF_LANG_TRAD_CHINESE:
            return "ZHTW";
        case CONF_LANG_KOREAN:
            return "KO";
        default:
            return "EN";
    }
}

bool CSettings::LoadLanguage(const char *path, int language)
{
    bool ret = false;

    if (language >= 0 || !path)
    {
        if (language < 0) return false;

        char filepath[150];
        char langpath[150];
        snprintf(langpath, sizeof(langpath), "%s", language_path);
        if (langpath[strlen(langpath) - 1] != '/')
        {
            char * ptr = strrchr(langpath, '/');
            if (ptr)
            {
                ptr++;
                ptr[0] = '\0';
            }
        }

        if (language == APP_DEFAULT)
        {
            strcpy(language_path, langpath);
            gettextCleanUp();
            return true;
        }
        else if (language == CONSOLE_DEFAULT)
        {
            return LoadLanguage(NULL, CONF_GetLanguage() + 2);
        }
        else if (language == JAPANESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/japanese.lang", langpath);
        }
        else if (language == ENGLISH)
        {
            snprintf(filepath, sizeof(filepath), "%s/english.lang", langpath);
        }
        else if (language == GERMAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/german.lang", langpath);
        }
        else if (language == FRENCH)
        {
            snprintf(filepath, sizeof(filepath), "%s/french.lang", langpath);
        }
        else if (language == SPANISH)
        {
            snprintf(filepath, sizeof(filepath), "%s/spanish.lang", langpath);
        }
        else if (language == ITALIAN)
        {
            snprintf(filepath, sizeof(filepath), "%s/italian.lang", langpath);
        }
        else if (language == DUTCH)
        {
            snprintf(filepath, sizeof(filepath), "%s/dutch.lang", langpath);
        }
        else if (language == S_CHINESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/s_chinese.lang", langpath);
        }
        else if (language == T_CHINESE)
        {
            snprintf(filepath, sizeof(filepath), "%s/t_chinese.lang", langpath);
        }
        else if (language == KOREAN)
        {
            snprintf(filepath, sizeof(filepath), "%s%s/korean.lang", BootDevice, langpath);
        }

        ret = gettextLoadLanguage(filepath);
        if (ret) strncpy(language_path, filepath, sizeof(language_path));

        strcpy(db_language, GetLangCode(language));
    }
    else if (strlen(path) < 3)
    {
        return LoadLanguage(NULL, CONF_GetLanguage() + 2);
    }
    else
    {
        ret = gettextLoadLanguage(path);
        if (ret) strncpy(language_path, path, sizeof(language_path));
    }

    return ret;
}
