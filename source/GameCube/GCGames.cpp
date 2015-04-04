/****************************************************************************
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "GCGames.h"
#include "FileOperations/fileops.h"
#include "settings/GameTitles.h"
#include "settings/CSettings.h"
#include "prompts/GCDeleteMenu.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "language/gettext.h"
#include "usbloader/wbfs/wbfs_fat.h"
#include "utils/tools.h"
#include "system/IosLoader.h"
#include "menu.h"
#include "gecko.h"

GCGames *GCGames::instance = NULL;

inline bool isGameID(const u8 *id)
{
	for (int i = 0; i < 6; i++)
		if (!isalnum((int) id[i]))
			return false;

	return true;
}

const char *GCGames::GetPath(const char *gameID) const
{
	if(!gameID)
		return "";

	for(u32 i = 0; i < HeaderList.size(); i++)
	{
		if(strncasecmp((const char *) HeaderList[i].id, gameID, 6) == 0)
			return PathList[i].c_str();
	}

	return "";
}

void GCGames::LoadGameList(const string &path, vector<struct discHdr> &headerList, vector<string> &pathList)
{
	struct discHdr tmpHdr;
	struct stat st;
	u8 id[8];
	u8 disc_number = 0;
	char fpath[1024];
	char fname_title[64];
	DIR *dir_iter;
	struct dirent *dirent;

	dir_iter = opendir(path.c_str());
	if (!dir_iter) return;

	while ((dirent = readdir(dir_iter)) != 0)
	{
		const char *dirname = dirent->d_name;
		if(!dirname)
			continue;

		if (dirname[0] == '.') continue;

		// reset id and title
		memset(id, 0, sizeof(id));
		*fname_title = 0;

		bool lay_a = false;
		bool lay_b = false;
		int len = strlen(dirname);
		if (len >= 8)
		{
			if (Wbfs_Fat::CheckLayoutB((char *) dirname, len, id, fname_title))
			{
				// path/TITLE[GAMEID]/game.iso
				lay_b = true;
			}
			else if (dirname[6] == '_')
			{
				// path/GAMEID_TITLE/game.iso
				memcpy(id, dirname, 6);

				if(isGameID(id))
				{
					lay_a = true;
					snprintf(fname_title, sizeof(fname_title), "%s", &dirname[7]);
				}
			}
		}
		else if(len == 6 && isGameID((u8*)dirname)) {
			memcpy(id, dirname, 6);
			lay_a = true;
		}

		if(!lay_a && !lay_b)
			memset(id, 0, sizeof(id));

		bool found = false;
		bool extracted = false;

		for(int i = 0; i < 4; i++)
		{
			char name[50];
			snprintf(name, sizeof(name), "%.6s.%s", (i % 2) == 0 ? "game" : (char *) id, i >= 2 ? "gcm" : "iso");
			snprintf(fpath, sizeof(fpath), "%s%s/%s", path.c_str(), dirname, name);
			if((found = (stat(fpath, &st) == 0)) == true)
			 break;
		}

		// Check if only disc2.iso is present
		if(!found)
		{
			for(int i = 0; i < 2; i++)
			{
				char name[50];
				snprintf(name, sizeof(name), "disc2.%s", (i % 2) == 0 ? "gcm" : "iso"); // allow gcm, though DM(L) require "disc2.iso" filename
				snprintf(fpath, sizeof(fpath), "%s%s/%s", path.c_str(), dirname, name);
				if((found = (stat(fpath, &st) == 0)) == true)
				{
					disc_number = 1;
					break;
				}
			}
		}

		if(!found)
		{
			snprintf(fpath, sizeof(fpath), "%s%s/sys/boot.bin", path.c_str(), dirname);
			if(stat(fpath, &st) != 0)
				continue;
			// this game is extracted
			extracted = true;
		}

		//! GAMEID was not found
		if(!lay_a && !lay_b) {
			// read game ID and title from disc header
			// iso file
			FILE *fp = fopen(fpath, "rb");
			if (fp != NULL)
			{
				memset(&tmpHdr, 0, sizeof(tmpHdr));
				fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
				fclose(fp);

				if (tmpHdr.gc_magic == GCGames::MAGIC)
				{
					memcpy(id, tmpHdr.id, 6);
					snprintf(fname_title, sizeof(fname_title), "%s", tmpHdr.title);
				}
			}
		}

		// if we have titles.txt entry use that
		const char *title = GameTitles.GetTitle(id);

		// if no titles.txt get title from dir or file name
		if (strlen(title) == 0 && !Settings.ForceDiscTitles && strlen(fname_title) > 0)
			title = fname_title;

		if (*id != 0 && strlen(title) > 0)
		{
			string gamePath = string(path) + dirname + (extracted ? "/" : strrchr(fpath, '/'));
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title)-1);
			tmpHdr.magic = GCGames::MAGIC;
			tmpHdr.type = extracted ? TYPE_GAME_GC_EXTRACTED : TYPE_GAME_GC_IMG;
			tmpHdr.disc_no = disc_number;
			headerList.push_back(tmpHdr);
			pathList.push_back(gamePath);
			continue;
		}

		// else read it from file directly
		// iso file
		FILE *fp = fopen(fpath, "rb");
		if (fp != NULL)
		{
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
			fclose(fp);

			if (tmpHdr.gc_magic == GCGames::MAGIC)
			{
				string gamePath = string(path) + dirname + (extracted ? "/" : strrchr(fpath, '/'));
				tmpHdr.magic = tmpHdr.gc_magic;
				tmpHdr.type = extracted ? TYPE_GAME_GC_EXTRACTED : TYPE_GAME_GC_IMG;
				headerList.push_back(tmpHdr);
				pathList.push_back(gamePath);

				// Save title for next start
				GameTitles.SetGameTitle(tmpHdr.id, tmpHdr.title);
			}
		}
	}

	closedir(dir_iter);
}

u32 GCGames::LoadAllGames(void)
{
	PathList.clear();
	HeaderList.clear();
	sdGCList.clear();
	sdGCPathList.clear();

	if(strcmp(Settings.GameCubePath, Settings.GameCubeSDPath) == 0 || Settings.GameCubeSource != GC_SOURCE_SD)
		LoadGameList(Settings.GameCubePath, HeaderList, PathList);

	if(strcmp(Settings.GameCubePath, Settings.GameCubeSDPath) != 0 && (Settings.GameCubeSource != GC_SOURCE_MAIN))
	{
		LoadGameList(Settings.GameCubeSDPath, sdGCList, sdGCPathList);

		for(u32 i = 0; i < sdGCList.size(); ++i)
		{
			if(Settings.GameCubeSource != GC_SOURCE_SD)
			{
				u32 n;
				for(n = 0; n < HeaderList.size(); ++n)
				{
					//! Display only one game if it is present on both SD and USB.
					if(memcmp(HeaderList[n].id, sdGCList[i].id, 6) == 0)
					{
						if((Settings.GameCubeSource == GC_SOURCE_MAIN_SD) ||
						   (Settings.GameCubeSource == GC_SOURCE_AUTO && (IosLoader::GetMIOSInfo() == DIOS_MIOS || IosLoader::GetMIOSInfo() == QUADFORCE_USB))) // DIOS MIOS - Show the game on USB in priority
						{
							break;
						}
						else // replace the one loaded from USB with the same games on SD
						{
							memcpy(&HeaderList[n], &sdGCList[i], sizeof(struct discHdr));
							PathList[n] = sdGCPathList[i];
							break;
						}
					}
				}

				// Not available in the main GC path
				if(n == HeaderList.size()) {
					HeaderList.push_back(sdGCList[i]);
					PathList.push_back(sdGCPathList[i]);
				}
			}
			else // GC_SOURCE_SD
			{
 				HeaderList.push_back(sdGCList[i]);
 				PathList.push_back(sdGCPathList[i]);
 			}
		}
	}

	return HeaderList.size();
}

bool GCGames::RemoveGame(const char *gameID)
{
	const char *path = GetPath(gameID);
	if(*path == 0)
		return false;

	RemoveSDGame(gameID);

	if(strcmp(Settings.GameCubePath, Settings.GameCubeSDPath) == 0)
		return true;

	struct discHdr *header = NULL;
	for(u32 i = 0; i < HeaderList.size(); ++i)
	{
		if(strncmp(gameID, (char*)HeaderList[i].id, 6) == 0)
		{
			header = &HeaderList[i];
			break;
		}
	}
	if(!header)
		return false;

	char filepath[512];
	int result = 0;
	int ret;

	// the main path is the SD path as it is prefered, now delete USB
	char cIsoPath[256];
	snprintf(cIsoPath, sizeof(cIsoPath), "%s", path + strlen(Settings.GameCubeSDPath));

	if(header->type == TYPE_GAME_GC_IMG)
	{
		// Remove game iso
		snprintf(filepath, sizeof(filepath), "%s%s", Settings.GameCubePath, cIsoPath);
		ret = RemoveFile(filepath);
		if(ret != 0)
			result = -1;

		// Remove path
		char *pathPtr = strrchr(filepath, '/');
		if(pathPtr) *pathPtr = 0;
		ret = RemoveFile(filepath);
		if(ret != 0)
			result = -1;
	}
	else if(header->type == TYPE_GAME_GC_EXTRACTED)
	{
		//! remove extracted gamecube game
		snprintf(filepath, sizeof(filepath), "%s%s", Settings.GameCubePath, cIsoPath);
		ret = RemoveDirectory(path);
		if(ret < 0)
			result = -1;
	}

	return (result == 0);
}

bool GCGames::RemoveSDGame(const char *gameID)
{
	const char *path = GetPath(gameID);
	if(*path == 0)
		return false;

	struct discHdr *header = NULL;
	for(u32 i = 0; i < HeaderList.size(); ++i)
	{
		if(strncmp(gameID, (char*)HeaderList[i].id, 6) == 0)
		{
			header = &HeaderList[i];
			break;
		}
	}
	if(!header)
		return false;

	char filepath[512];
	int result = 0;
	int ret;

	if(header->type == TYPE_GAME_GC_IMG)
	{
		// Remove game iso
		snprintf(filepath, sizeof(filepath), "%s", path);
		ret = RemoveFile(filepath);
		if(ret != 0)
			result = -1;

		// Remove path
		char *pathPtr = strrchr(filepath, '/');
		if(pathPtr) *pathPtr = 0;
		ret = RemoveFile(filepath);
		if(ret != 0)
			result = -1;
	}
	else if(header->type == TYPE_GAME_GC_EXTRACTED)
	{
		//! remove extracted gamecube game
		ret = RemoveDirectory(path);
		if(ret < 0)
			result = -1;
	}

	return (result == 0);
}

float GCGames::GetGameSize(const char *gameID)
{
	const char *path = GetPath(gameID);
	if(*path == 0)
		return 0.0f;

	struct stat st;

	if(stat(path, &st) != 0)
		return 0.0f;

	return ((float) st.st_size / GB_SIZE);
}

bool GCGames::IsInstalled(const char *gameID, u8 disc_number) const
{
	for(u32 n = 0; n < HeaderList.size(); n++)
	{
		if(memcmp(HeaderList[n].id, gameID, 6) == 0)
		{
			if(HeaderList[n].type == TYPE_GAME_GC_EXTRACTED || Settings.GCInstallCompressed)
				return true; // Multi-disc games in extracted form are currently unsupported by DML, no need to check further.
			
			if(HeaderList[n].disc_no == disc_number) // Disc number already in headerList. If Disc2 is loaded in headerList, then Disc1 is not installed yet
			{
				return true;
			}
			else if(disc_number == 1) // Check if the second Game Disc exists in the same folder than Disc1.
			{
				char filepath[512];
				snprintf(filepath, sizeof(filepath), "%s", GetPath(gameID));
				char *pathPtr = strrchr(filepath, '/');
				if(pathPtr) *pathPtr = 0;
				snprintf(filepath, sizeof(filepath), "%s/disc2.iso", filepath);
				
				if(CheckFile(filepath))
					return true;
			}
		}
	}
	return false;
}

bool GCGames::CopyUSB2SD(const struct discHdr *header)
{
	const char *path = GetPath((char*)header->id);
	int oldGameCubeSource = Settings.GameCubeSource;
	if(*path == 0)
		return false;

	int choice = WindowPrompt(tr("The game is on USB."), tr("Do you want to copy the game to SD or delete a game on SD?"), tr("Copy"), tr("Show SD"), tr("Cancel"));
	if(choice == 0)
		return false;

	const char *cpTitle = GameTitles.GetTitle(header);
	
	if(choice == 2)
	{
		// Load Games from SD card only
		Settings.GameCubeSource = GC_SOURCE_SD;
		GCGames::Instance()->LoadAllGames();

		GCDeleteMenu gcDeleteMenu;
		gcDeleteMenu.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
		gcDeleteMenu.SetEffect(EFFECT_FADE, 20);
		mainWindow->SetState(STATE_DISABLED);
		mainWindow->Append(&gcDeleteMenu);

		gcDeleteMenu.Show();

		gcDeleteMenu.SetEffect(EFFECT_FADE, -20);
		while(gcDeleteMenu.GetEffect() > 0) usleep(1000);

		mainWindow->Remove(&gcDeleteMenu);
		mainWindow->SetState(STATE_DEFAULT);

		// Reload user's gameCubeSource setting
		Settings.GameCubeSource = oldGameCubeSource;
		GCGames::Instance()->LoadAllGames();

		if(!WindowPrompt(tr("Do you want to copy now?"), cpTitle, tr("Yes"), tr("Cancel")))
			return false;
	}

	struct statvfs sd_vfs;
	if(statvfs(Settings.GameCubeSDPath, &sd_vfs) != 0)
	{
		WindowPrompt(tr("Error:"), tr("SD Card could not be accessed."), tr("OK"));
		return false;
	}

	u64 filesize = 0;

	if(header->type == TYPE_GAME_GC_IMG) {
		filesize = FileSize(path);
	}
	else if(header->type == TYPE_GAME_GC_EXTRACTED) {
		StartProgress(tr("Getting game folder size..."), tr("Please wait"), 0, true, true);
		ShowProgress(0, 1);
		filesize = FileSize(path);
		ProgressStop();
	}

	while(((u64)sd_vfs.f_frsize * (u64)sd_vfs.f_bfree) < filesize)
	{
		choice = WindowPrompt(tr("Error: Not enough space on SD."), tr("Do you want to delete a game on SD?"), tr("Yes"), tr("Cancel"));
		if(choice == 0)
			return false;

		GCDeleteMenu gcDeleteMenu;
		gcDeleteMenu.SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
		gcDeleteMenu.SetEffect(EFFECT_FADE, 20);
		mainWindow->SetState(STATE_DISABLED);
		mainWindow->Append(&gcDeleteMenu);

		gcDeleteMenu.Show();

		gcDeleteMenu.SetEffect(EFFECT_FADE, -20);
		while(gcDeleteMenu.GetEffect() > 0) usleep(1000);

		mainWindow->Remove(&gcDeleteMenu);
		mainWindow->SetState(STATE_DEFAULT);

		statvfs(Settings.GameCubeSDPath, &sd_vfs);
	}

	const char *cIsoPath = path + strlen(Settings.GameCubePath);
	char destPath[512];
	snprintf(destPath, sizeof(destPath), "%s%s", Settings.GameCubeSDPath, cIsoPath);

	int res = -1;

	if(header->type == TYPE_GAME_GC_IMG)
	{
		ProgressCancelEnable(true);
		StartProgress(tr("Copying GC game..."), cpTitle, 0, true, true);

		char *ptr = strrchr(destPath, '/');
		if(ptr) *ptr = 0;

		CreateSubfolder(destPath);

		snprintf(destPath, sizeof(destPath), "%s%s", Settings.GameCubeSDPath, cIsoPath);

		res = CopyFile(path, destPath);
	}
	else if(header->type == TYPE_GAME_GC_EXTRACTED)
	{
		res = CopyDirectory(path, destPath);
	}

	// Refresh list
	GCGames::Instance()->LoadAllGames();

	ProgressStop();
	ProgressCancelEnable(false);

	if(res == PROGRESS_CANCELED)
	{
		if(header->type == TYPE_GAME_GC_IMG)
		{
			// remove file and path
			RemoveFile(destPath);
			char *ptr = strrchr(destPath, '/');
			if(ptr) *ptr = 0;
			RemoveFile(destPath);
		}

		WindowPrompt(tr("Copying Canceled"), 0, tr("OK"));
		return false;
	}
	else if(res < 0)
	{
		if(header->type == TYPE_GAME_GC_IMG)
		{
			// remove file and path
			RemoveFile(destPath);
			char *ptr = strrchr(destPath, '/');
			if(ptr) *ptr = 0;
			RemoveFile(destPath);
		}

		WindowPrompt(tr("Error:"), tr("Failed copying file"), tr("OK"));
		return false;
	}
	else
	{
		return WindowPrompt(tr("Successfully copied"), tr("Do you want to start the game now?"), tr("Yes"), tr("Cancel"));
	}
}

int nintendontBuildDate(const char *NIN_loader_path, char *NINBuildDate)
{

	char NIN_loader[100];
	snprintf(NIN_loader, sizeof(NIN_loader), "%sboot.dol", NIN_loader_path);
	if(!CheckFile(NIN_loader))
		snprintf(NIN_loader, sizeof(NIN_loader), "%sloader.dol", NIN_loader_path);
	if(CheckFile(NIN_loader))
	{
		u8 *buffer = NULL;
		u32 filesize = 0;
		const char* str = "Nintendont Loader";
		bool found = false;
		if(LoadFileToMem(NIN_loader, &buffer, &filesize))
		{
			for(u32 i = 0; i < filesize-100; ++i)
			{
				if( memcmp(buffer+i, str, strlen(str)) == 0)
				{
					// Write buffer in NINheader
					char NINHeader[100];
					for(u8 j = 0 ; j < 99 ; j++)
						NINHeader[j] = *(u8*)(buffer+i+j) == 0 ? ' ' : *(u8*)(buffer+i+j); // replace \0 with a space.
					NINHeader[99] = '\0';

					// Search month string start position in header
					char *dateStart = NULL;
					const char *month[] = {"Jan ", "Feb ", "Mar ", "Apr ", "May ", "Jun ", "Jui ", "Aug ", "Sep ", "Oct ", "Nov ", "Dec "};
					for(u8 m = 0 ; m < 12 ; m++) 
					{
						dateStart = strstr(NINHeader, month[m]);
						if(dateStart != NULL)
							break;
					}
					if(dateStart == NULL)
						break;
					
					dateStart[20] = '\0';
					
					sprintf(NINBuildDate, "%.20s", dateStart);
					gprintf("Nintendont Build date : %.20s \n", dateStart);
					
					found = true;
					break;
				}
			}
			free(buffer);
		}
		if(found)
			return 1;
	}
	
	return 0;
}

int nintendontVersion(const char *NIN_loader_path, char *NINVersion, int len)
{
	char NIN_loader[100];
	u32 NINRev = 0;
	snprintf(NIN_loader, sizeof(NIN_loader), "%sboot.dol", NIN_loader_path);
	if(!CheckFile(NIN_loader))
		snprintf(NIN_loader, sizeof(NIN_loader), "%sloader.dol", NIN_loader_path);
	if(CheckFile(NIN_loader))
	{
		u8 *buffer = NULL;
		u32 filesize = 0;
		const char* str = "$$Version:";
		if(LoadFileToMem(NIN_loader, &buffer, &filesize))
		{
			for(u32 i = 0; i < filesize; i += 32)
			{
				if(memcmp( buffer+i, str, strlen(str)) == 0)
				{
					// Write buffer in NINVersion
					snprintf(NINVersion, len, "%s", buffer+i+strlen(str));
					NINRev = atoi(strchr(NINVersion, '.')+1);
					break;
				}
			}
			free(buffer);
		}
	}
	
	return NINRev;
}
