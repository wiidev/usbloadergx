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
#include "GameLoadSM.hpp"
#include "settings/CSettings.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "prompts/TitleBrowser.h"
#include "usbloader/GameList.h"
#include "usbloader/wbfs.h"
#include "usbloader/utils.h"
#include "system/IosLoader.h"
#include "settings/GameTitles.h"
#include "xml/xml.h"
#include "menu.h"

extern PartList partitions;
extern char game_partition[6];
extern u8 load_from_fs;

static const char * OnOffText[MAX_ON_OFF] =
{
    trNOOP( "OFF" ),
    trNOOP( "ON" )
};

static const char * VideoModeText[VIDEO_MODE_MAX] =
{
    trNOOP( "Disc Default" ),
    trNOOP( "System Default" ),
    trNOOP( "AutoPatch" ),
    trNOOP( "Force PAL50" ),
    trNOOP( "Force PAL60" ),
    trNOOP( "Force NTSC" )
};

static const char * LanguageText[MAX_LANGUAGE] =
{
    trNOOP( "Disc Default" ),
    trNOOP( "Console Default" ),
    trNOOP( "Japanese" ),
    trNOOP( "English" ),
    trNOOP( "German" ),
    trNOOP( "French" ),
    trNOOP( "Spanish" ),
    trNOOP( "Italian" ),
    trNOOP( "Dutch" ),
    trNOOP( "SChinese" ),
    trNOOP( "TChinese" ),
    trNOOP( "Korean" )
};

static const char * InstallToText[INSTALL_TO_MAX] =
{
    trNOOP( "None" ),
    trNOOP( "GAMEID_Gamename" ),
    trNOOP( "Gamename [GAMEID]" )
};

static const char * Error002Text[3] =
{
    trNOOP( "No" ),
    trNOOP( "Yes" ),
    trNOOP( "Anti" )
};

static const char * InstPartitionsText[3] =
{
    trNOOP( "Game partition" ),
    trNOOP( "All partitions" ),
    trNOOP( "Remove update" )
};

static inline bool IsValidPartition(int fs_type, int cios)
{
    if (IosLoader::IsWaninkokoIOS() && NandTitles.VersionOf(TITLE_ID(1, cios)) < 18)
    {
        return fs_type == FS_TYPE_WBFS;
    }
    else
    {
        return fs_type == FS_TYPE_WBFS || fs_type == FS_TYPE_FAT32 || fs_type == FS_TYPE_NTFS;
    }
}

GameLoadSM::GameLoadSM()
    : SettingsMenu(tr("Game Load"), &GuiOptions, MENU_NONE)
{
    int Idx = 0;

    Options->SetName(Idx++, "%s", tr( "Video Mode" ));
    Options->SetName(Idx++, "%s", tr( "VIDTV Patch" ));
    Options->SetName(Idx++, "%s", tr( "Game Language" ));
    Options->SetName(Idx++, "%s", tr( "Patch Country Strings" ));
    Options->SetName(Idx++, "%s", tr( "Ocarina" ));
    Options->SetName(Idx++, "%s", tr( "Boot/Standard" ));
    Options->SetName(Idx++, "%s", tr( "Partition" ));
    Options->SetName(Idx++, "%s", tr( "FAT: Use directories" ));
    Options->SetName(Idx++, "%s", tr( "Quick Boot" ));
    Options->SetName(Idx++, "%s", tr( "Error 002 fix" ));
    Options->SetName(Idx++, "%s", tr( "Install partitions" ));
    Options->SetName(Idx++, "%s", tr( "Install 1:1 Copy" ));
    Options->SetName(Idx++, "%s", tr( "Return To" ));

    SetOptionValues();

    OldSettingsPartition = Settings.partition;
}

GameLoadSM::~GameLoadSM()
{
    // if partition has changed, Reinitialize it
    if (Settings.partition != OldSettingsPartition)
    {
        PartInfo pinfo = partitions.pinfo[Settings.partition];
        partitionEntry pentry = partitions.pentry[Settings.partition];
        WBFS_OpenPart(pinfo.part_fs, pinfo.index, pentry.sector, pentry.size, (char *) &game_partition);
        load_from_fs = pinfo.part_fs;
        CloseXMLDatabase();
        GameTitles.SetDefault();
        OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride, true);
        gameList.ReadGameList();
    }
}

void GameLoadSM::SetOptionValues()
{
    int Idx = 0;

    //! Settings: Video Mode
    Options->SetValue(Idx++, "%s", tr(VideoModeText[Settings.videomode]));

    //! Settings: VIDTV Patch
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.videopatch] ));

    //! Settings: Game Language
    Options->SetValue(Idx++, "%s", tr( LanguageText[Settings.language] ));

    //! Settings: Patch Country Strings
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.patchcountrystrings] ));

    //! Settings: Ocarina
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.ocarina] ));

    //! Settings: Boot/Standard
    if (Settings.godmode)
        Options->SetValue(Idx++, "IOS %i", Settings.cios);
    else
        Options->SetValue(Idx++, "********");

    //! Settings: Partition
    PartInfo pInfo = partitions.pinfo[Settings.partition];
    f32 partition_size = partitions.pentry[Settings.partition].size
            * (partitions.sector_size / GB_SIZE);

    // Get the partition name and it's size in GB's
    Options->SetValue(Idx++, "%s%d (%.2fGB)", pInfo.fs_type == FS_TYPE_FAT32 ? "FAT"
            : pInfo.fs_type == FS_TYPE_NTFS ? "NTFS" : "WBFS", pInfo.index, partition_size);

    //! Settings: FAT: Use directories
    Options->SetValue(Idx++, "%s", tr( InstallToText[Settings.FatInstallToDir] ));

    //! Settings: Quick Boot
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.quickboot] ));

    //! Settings: Error 002 fix
    Options->SetValue(Idx++, "%s", tr( Error002Text[Settings.error002] ));

    //! Settings: Install partitions
    Options->SetValue(Idx++, "%s", tr( InstPartitionsText[Settings.InstallPartitions] ));

    //! Settings: Install 1:1 Copy
    Options->SetValue(Idx++, "%s", tr( OnOffText[Settings.fullcopy] ));

    //! Settings: Return To
    const char* TitleName = NULL;
    int haveTitle = NandTitles.FindU32(Settings.returnTo);
    if (haveTitle >= 0)
        TitleName = NandTitles.NameFromIndex(haveTitle);
    Options->SetValue(Idx++, "%s", TitleName ? TitleName : strlen(Settings.returnTo) > 0 ?
                                    Settings.returnTo : tr( OnOffText[0] ));
}

int GameLoadSM::GetMenuInternal()
{
    int ret = optionBrowser->GetClickedOption();

    if (ret < 0)
        return MENU_NONE;

    int Idx = -1;

    //! Settings: Video Mode
    if (ret == ++Idx)
    {
        if (++Settings.videomode >= VIDEO_MODE_MAX) Settings.videomode = 0;
    }

    //! Settings: VIDTV Patch
    else if (ret == ++Idx)
    {
        if (++Settings.videopatch >= MAX_ON_OFF) Settings.videopatch = 0;
    }

    //! Settings: Game Language
    else if (ret == ++Idx)
    {
        if (++Settings.language >= MAX_LANGUAGE) Settings.language = 0;
    }

    //! Settings: Patch Country Strings
    else if (ret == ++Idx)
    {
        if (++Settings.patchcountrystrings >= MAX_ON_OFF) Settings.patchcountrystrings = 0;
    }

    //! Settings: Ocarina
    else if (ret == ++Idx)
    {
        if (++Settings.ocarina >= MAX_ON_OFF) Settings.ocarina = 0;
    }

    //! Settings: Boot/Standard
    else if (ret == ++Idx)
    {
        if(!Settings.godmode)
            return MENU_NONE;

        char entered[4];
        snprintf(entered, sizeof(entered), "%i", Settings.cios);
        if(OnScreenKeyboard(entered, sizeof(entered), 0))
        {
            Settings.cios = atoi(entered);
            if(Settings.cios < 200) Settings.cios = 200;
            else if(Settings.cios > 255) Settings.cios = 255;

            if(NandTitles.IndexOf(TITLE_ID(1, Settings.cios)) < 0)
            {
                WindowPrompt(tr("Warning:"), tr("This IOS was not found on the titles list. If you are sure you have it installed than ignore this warning."), tr("OK"));
            }
            else if(Settings.cios == 254)
            {
                WindowPrompt(tr("Warning:"), tr("This IOS is the BootMii ios. If you are sure it is not BootMii and you have something else installed there than ignore this warning."), tr("OK"));
            }
        }
    }

    //! Settings: Partition
    else if (ret == ++Idx)
    {
        // Select the next valid partition, even if that's the same one
        int fs_type = partitions.pinfo[Settings.partition].fs_type;
        int ios = IOS_GetVersion();
        do
        {
            Settings.partition = (Settings.partition + 1) % partitions.num;
            fs_type = partitions.pinfo[Settings.partition].fs_type;
        }
        while (!IsValidPartition(fs_type, ios));
    }

    //! Settings: FAT: Use directories
    else if (ret == ++Idx)
    {
        if (++Settings.FatInstallToDir >= INSTALL_TO_MAX) Settings.FatInstallToDir = 0;
    }

    //! Settings: Quick Boot
    else if (ret == ++Idx)
    {
        if (++Settings.quickboot >= MAX_ON_OFF) Settings.quickboot = 0;
    }

    //! Settings: Error 002 fix
    else if (ret == ++Idx )
    {
        if (++Settings.error002 >= 3) Settings.error002 = 0;
    }

    //! Settings: Install partitions
    else if (ret == ++Idx)
    {
        if (++Settings.InstallPartitions >= 3) Settings.InstallPartitions = 0;
    }

    //! Settings: Install 1:1 Copy
    else if (ret == ++Idx)
    {
        if (++Settings.fullcopy >= MAX_ON_OFF) Settings.fullcopy = 0;
    }

    //! Settings: Return To
    else if (ret == ++Idx)
    {
        char tidChar[10];
        bool getChannel = TitleSelector(tidChar);
        if (getChannel)
            snprintf(Settings.returnTo, sizeof(Settings.returnTo), "%s", tidChar);
    }

    SetOptionValues();

    return MENU_NONE;
}

