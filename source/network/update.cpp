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
#include "language/gettext.h"
#include "language/UpdateLanguage.h"
#include "homebrewboot/BootHomebrew.h"
#include "utils/StringTools.h"
#include "utils/ShowError.h"
#include "prompts/PromptWindows.h"
#include "FileOperations/fileops.h"
#include "xml/GameTDB.hpp"
#include "wad/nandtitle.h"
#include "wad/wad.h"
#include "sys.h"
#include "svnrev.h"

static const char * GameTDB_URL = "http://www.gametdb.com/wiitdb.zip";

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

static bool CheckNewGameTDBVersion(const char *url)
{
	u64 Version = 0;

	char * HEAD_Responde = HEAD_Request(url);
	if(!HEAD_Responde)
		return false;

	char * version_ptr = strstr(HEAD_Responde, "X-GameTDB-Timestamp: ");
	if(version_ptr)
	{
		version_ptr += strlen("X-GameTDB-Timestamp: ");
		Version = strtoull(version_ptr, NULL, 10);
	}

	free(HEAD_Responde);

	std::string Title;
	std::string Filepath = Settings.titlestxt_path;
	if(Settings.titlestxt_path[Filepath.size()-1] != '/')
		Filepath += '/';
	Filepath += "wiitdb.xml";

	GameTDB XML_DB;

	if(!XML_DB.OpenFile((Filepath.c_str())))
		return true;	//! If no file exists we need the file

	u64 ExistingVersion = XML_DB.GetGameTDBVersion();

	gprintf("Existing GameTDB Version: %llu Online GameTDB Version: %llu\n", ExistingVersion, Version);

	return (ExistingVersion != Version);
}

int UpdateGameTDB()
{
	if(CheckNewGameTDBVersion(GameTDB_URL) == false)
	{
		gprintf("Not updating GameTDB: Version is the same\n");
		return -1;
	}

	gprintf("Updating GameTDB...\n");

	string ZipPath = Settings.titlestxt_path;
	if(Settings.titlestxt_path[ZipPath.size()-1] != '/')
		ZipPath += '/';

	ZipPath += "wiitdb.zip";

	int filesize = DownloadFileToPath(GameTDB_URL, ZipPath.c_str(), false);

	if(filesize <= 0)
		return -1;

	ZipFile zFile(ZipPath.c_str());

	bool result = zFile.ExtractAll(Settings.titlestxt_path);

	//! The zip file is not needed anymore so we can remove it
	remove(ZipPath.c_str());

	//! Reload all titles and reload cached titles because the file changed now.
	GameTitles.SetDefault();
	GameTitles.LoadTitlesFromGameTDB(Settings.titlestxt_path);

	return (result ? filesize : -1);
}

static void UpdateIconPng()
{
	char iconpath[200];
	struct block file = downloadfile("http://svn.code.sf.net/p/usbloadergx/code/branches/updates/icon.png");
	if (file.data != NULL)
	{
		snprintf(iconpath, sizeof(iconpath), "%sicon.png", Settings.update_path);
		FILE * pfile = fopen(iconpath, "wb");
		if(pfile)
		{
			fwrite(file.data, 1, file.size, pfile);
			fclose(pfile);
		}
		free(file.data);
	}
}

static void UpdateMetaXml()
{
	char xmlpath[200];
	struct block file = downloadfile("http://svn.code.sf.net/p/usbloadergx/code/branches/updates/meta.xml");
	// if not working, use this url form: http://sourceforge.net/p/usbloadergx/code/1254/tree//branches/updates/meta.xml?format=raw
	if (file.data != NULL)
	{
		snprintf(xmlpath, sizeof(xmlpath), "%smeta.xml", Settings.update_path);
		FILE *pfile = fopen(xmlpath, "wb");
		if(pfile)
		{
			fwrite(file.data, 1, file.size, pfile);
			fclose(pfile);
		}
		free(file.data);
	}
}

int CheckUpdate()
{
	if (!IsNetworkInit())
		return -1;

	int revnumber = 0;
	int currentrev = atoi(GetRev());

#ifdef FULLCHANNEL
	struct block file = downloadfile( "http://svn.code.sf.net/p/usbloadergx/code/branches/updates/update_wad.txt" );
#else
	struct block file = downloadfile("http://svn.code.sf.net/p/usbloadergx/code/branches/updates/update_dol.txt");
#endif

	if (file.data != NULL)
	{
		revnumber = atoi((char *) file.data);
		free(file.data);
	}

	if (revnumber > currentrev)
		return revnumber;

	return -1;
}

static int ApplicationDownload(void)
{
	std::string DownloadURL;
	int newrev = 0;
	int currentrev = atoi(GetRev());

#ifdef FULLCHANNEL
	struct block file = downloadfile( "http://svn.code.sf.net/p/usbloadergx/code/branches/updates/update_wad.txt" );
#else
	struct block file = downloadfile("http://svn.code.sf.net/p/usbloadergx/code/branches/updates/update_dol.txt");
#endif

	if (file.data != NULL)
	{
		// first line of the text file is the revisionc
		newrev = atoi((char *) file.data);
		// 2nd line of the text file is the url
		char *ptr = strchr((char *)file.data, '\n');
		while(ptr && (*ptr == '\r' || *ptr == '\n' || *ptr == ' '))
			ptr++;
		while(ptr && *ptr != '\0' && *ptr != '\r' && *ptr != '\n')
		{
			DownloadURL.push_back(*ptr);
			ptr++;
		}

		free(file.data);
	}

	if (newrev <= currentrev)
	{
		WindowPrompt(tr( "No new updates." ), 0, tr( "OK" ));
		return 0;
	}

	bool update_error = false;
	char tmppath[250];

	#ifdef FULLCHANNEL
		snprintf(tmppath, sizeof(tmppath), "%s/ULNR.wad", Settings.BootDevice);
	#else
		char realpath[250];
		snprintf(realpath, sizeof(realpath), "%sboot.dol", Settings.update_path);
		snprintf(tmppath, sizeof(tmppath), "%sboot.tmp", Settings.update_path);
	#endif

	int update_choice = WindowPrompt(fmt("Rev%i %s.", newrev, tr( "available" )), tr( "How do you want to update?" ), tr( "Update DOL" ), tr( "Update All" ), tr( "Cancel" ));
	if (update_choice == 0)
		return 0;

	int ret = DownloadFileToPath(DownloadURL.c_str(), tmppath, false);
	if(ret < 1024*1024)
	{
		remove(tmppath);
		WindowPrompt(tr("Failed updating"), tr("Error while downloding file"), tr( "OK" ));
		if(update_choice == 1)
			return -1;

		update_error = true;
	}
	else
	{
	#ifdef FULLCHANNEL
		FILE * wadFile = fopen(tmppath, "rb");
		if(!wadFile)
		{
			update_error = true;
			WindowPrompt(tr("Failed updating"), tr("Error opening downloaded file"), tr( "OK" ));
			return -1;
		}

		int error = Wad_Install( wadFile );
		if(error)
		{
			update_error = true;
			ShowError(tr( "The wad installation failed with error %i" ), error);
		}
		else
			WindowPrompt(tr( "Success" ), tr( "The wad file was installed" ), tr( "OK" ));

		RemoveFile(tmppath);
	#else
		gprintf("%s\n%s\n", realpath, tmppath);
		RemoveFile(realpath);
		if(!RenameFile(tmppath, realpath))
			update_error = true;
	#endif
	}

	if (update_choice == 2)
	{
		UpdateIconPng();
		UpdateMetaXml();
		UpdateGameTDB();
		DownloadAllLanguageFiles(newrev);
	}

	if(update_error)
	{
		ShowError(tr( "Error while updating USB Loader GX." ));
		return -1;
	}

	if (update_choice > 0)
	{
		WindowPrompt(tr( "Successfully updated." ), tr( "Restarting..." ), 0, 0, 0, 0, 150);
		RebootApp();
	}

	return 0;
}

int UpdateApp()
{
	if (!IsNetworkInit() && !NetworkInitPrompt())
	{
		WindowPrompt(tr("Error:"), tr("Could not initialize network!"), tr("OK"));
		return -1;
	}

	if (!CreateSubfolder(Settings.update_path))
	{
		WindowPrompt(tr("Error:"), tr("Can't create directory"), tr("OK"));
		return -1;
	}

	int choice = WindowPrompt(tr( "What do you want to update?" ), 0, "USB Loader GX", tr( "WiiTDB.xml" ), tr( "Language Files" ), tr( "Cancel" ));
	if(choice == 0)
		return 0;

	if(choice == 1)
	{
		return ApplicationDownload();
	}
	else if (choice == 2)
	{
		if(UpdateGameTDB() < 0)
		{
			WindowPrompt(fmt("%s", tr( "WiiTDB.xml is up to date." )), 0, tr("OK"));
			return 1;
		}
		else
		{
			WindowPrompt(tr( "Successfully Updated" ), 0, tr( "OK" ));
			return 1;
		}
	}
	else if (choice == 3)
	{
		if(UpdateLanguageFiles() > 0)
			WindowPrompt(tr( "Successfully Updated" ), 0, tr( "OK" ));
	}

	return 1;
}
