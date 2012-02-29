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
#include <dirent.h>
#include <sys/stat.h>
#include "GCGames.h"
#include "FileOperations/fileops.h"
#include "settings/GameTitles.h"
#include "settings/CSettings.h"
#include "usbloader/wbfs/wbfs_fat.h"
#include "utils/tools.h"

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

u32 GCGames::LoadGameList(const string &path)
{
	PathList.clear();
	HeaderList.clear();

	struct discHdr tmpHdr;
	struct stat st;
	u8 id[8];
	char fpath[1024];
	char fname_title[64];
	DIR *dir_iter;
	struct dirent *dirent;

	dir_iter = opendir(path.c_str());
	if (!dir_iter) return 0;

	while ((dirent = readdir(dir_iter)) != 0)
	{
		const char *dirname = dirent->d_name;
		if(!dirname)
			continue;

		if (dirname[0] == '.') continue;

		snprintf(fpath, sizeof(fpath), "%s/%s/game.iso", path.c_str(), dirname);

		if(stat(fpath, &st) != 0)
			continue;

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
					snprintf(fname_title, sizeof(fname_title), &dirname[7]);
				}
			}
		}

		if(!lay_a && !lay_b)
			memset(id, 0, sizeof(id));

		// if we have titles.txt entry use that
		const char *title = GameTitles.GetTitle(id);
		// if no titles.txt get title from dir or file name
		if (strlen(title) == 0 && !Settings.ForceDiscTitles && strlen(fname_title) > 0)
			title = fname_title;

		if (*id != 0 && strlen(title) > 0)
		{
			memset(&tmpHdr, 0, sizeof(tmpHdr));
			memcpy(tmpHdr.id, id, 6);
			strncpy(tmpHdr.title, title, sizeof(tmpHdr.title)-1);
			tmpHdr.magic = GCGames::MAGIC;
			tmpHdr.type = TYPE_GAME_GC_IMG;
			HeaderList.push_back(tmpHdr);
			PathList.push_back(dirname);
			continue;
		}

		// else read it from file directly
		// iso file
		FILE *fp = fopen(fpath, "rb");
		if (fp != NULL)
		{
			fread(&tmpHdr, sizeof(struct discHdr), 1, fp);
			fclose(fp);

			if (tmpHdr.gc_magic == GCGames::MAGIC)
			{
				tmpHdr.magic = tmpHdr.gc_magic;
				tmpHdr.type = TYPE_GAME_GC_IMG;
				HeaderList.push_back(tmpHdr);
				PathList.push_back(dirname);

				// Save title for next start
				GameTitles.SetGameTitle(tmpHdr.id, tmpHdr.title);
			}
		}
	}

	closedir(dir_iter);

	return HeaderList.size();
}

bool GCGames::RemoveGame(const char *gameID)
{
	const char *path = GetPath(gameID);
	if(*path == 0)
		return false;

	char filepath[512];
	int result = 0;
	int ret;

	// Remove game iso
	snprintf(filepath, sizeof(filepath), "%s%s/game.iso", Settings.GameCubePath, path);
	ret = RemoveFile(filepath);
	if(ret != 0)
		result = -1;

	// Remove path
	snprintf(filepath, sizeof(filepath), "%s%s", Settings.GameCubePath, path);
	ret = RemoveFile(filepath);
	if(ret != 0)
		result = -1;

	return (result == 0);
}

float GCGames::GetGameSize(const char *gameID)
{
	const char *path = GetPath(gameID);
	if(*path == 0)
		return 0.0f;

	struct stat st;
	char filepath[512];
	snprintf(filepath, sizeof(filepath), "%s%s/game.iso", Settings.GameCubePath, path);

	if(stat(filepath, &st) != 0)
		return 0.0f;

	return ((float) st.st_size / GB_SIZE);
}
