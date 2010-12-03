/***************************************************************************
 * Copyright (C) 2009
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
 *
 * update.cpp
 *
 * Update operations
 * for Wii-Xplorer 2009
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <string>

#include "gecko.h"
#include "ZipFile.h"
#include "http.h"
#include "networkops.h"
#include "HTML_Stream.h"
#include "FileDownloader.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "xml/WiiTDB.hpp"

static const char * WiiTDB_URL = "http://wiitdb.com/wiitdb.zip";

/****************************************************************************
 * Checking if an Update is available
 ***************************************************************************/
int CheckForBetaUpdate()
{
    int revnumber = 0;

    HTML_Stream HTML("http://code.google.com/p/usbloader-gui/downloads/list");

    const char * HTML_Pos = NULL;

    do
    {
        HTML_Pos = HTML.FindStringEnd("href='");
        char * tmpLink = HTML.CopyString("'\"");
        if (tmpLink)
        {
            char *fileext = strrchr(tmpLink, '.');
            if (fileext)
            {
                if (strcasecmp(fileext, ".dol") == 0)
                {
                    char revtxt[80];
                    char *filename = strrchr(tmpLink, '/') + 2;
                    u8 n = 0;
                    for (n = 0; n < strlen(filename) - 2; n++)
                        revtxt[n] = filename[n];
                    revtxt[n] = 0;
                    int fileRev = atoi(revtxt);

                    if (fileRev > revnumber)
                    {
                        revnumber = fileRev;
                    }
                }
            }
            free(tmpLink);
        }
    } while (HTML_Pos != NULL);

    return revnumber;
}

static bool CheckNewWiiTDBVersion(const char *url)
{
    u64 Version = 0;

    char * HEAD_Responde = HEAD_Request(url);
    if(!HEAD_Responde)
        return false;

    char * version_ptr = strstr(HEAD_Responde, "X-WiiTDB-Timestamp: ");
    if(version_ptr)
    {
        version_ptr += strlen("X-WiiTDB-Timestamp: ");
        Version = strtoull(version_ptr, NULL, 10);
    }

    free(HEAD_Responde);

    std::string Title;
    std::string Filepath = Settings.titlestxt_path;
    if(Settings.titlestxt_path[Filepath.size()-1] != '/')
        Filepath += '/';
    Filepath += "wiitdb.xml";

    WiiTDB XML_DB;

    if(!XML_DB.OpenFile((Filepath.c_str())))
        return true;    //! If no file exists we need the file

    u64 ExistingVersion = XML_DB.GetWiiTDBVersion();

    gprintf("Existing WiiTDB Version: %llu Online WiiTDB Version: %llu\n", ExistingVersion, Version);

    return (ExistingVersion != Version);
}

int UpdateWiiTDB()
{
    if(CheckNewWiiTDBVersion(WiiTDB_URL) == false)
    {
        gprintf("Not updating WiiTDB: Version is the same\n");
        return -1;
    }

    gprintf("Updating WiiTDB...\n");

    string ZipPath = Settings.titlestxt_path;
    if(Settings.titlestxt_path[ZipPath.size()-1] != '/')
        ZipPath += '/';

    ZipPath += "wiitdb.zip";

    int filesize = DownloadFileToPath(WiiTDB_URL, ZipPath.c_str(), false);

    if(filesize <= 0)
        return -1;

    ZipFile zFile(ZipPath.c_str());

    bool result = zFile.ExtractAll(Settings.titlestxt_path);

    //! The zip file is not needed anymore so we can remove it
    remove(ZipPath.c_str());

    //! Reload all titles because the file changed now.
    GameTitles.LoadTitlesFromWiiTDB(Settings.titlestxt_path);

    return (result ? filesize : -1);
}
