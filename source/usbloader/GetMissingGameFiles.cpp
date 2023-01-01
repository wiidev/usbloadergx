#include <stdio.h>
#include <string.h>
#include <vector>
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "settings/SettingsEnums.h"
#include "usbloader/GameList.h"
#include "wstring.hpp"
#include "gecko.h"

extern struct discHdr *dvdheader;

/**************************************************************************************
 * FindMissingFiles
 * Inputs:
 * path - Path to search in with. example "SD:/covers/"
 * fileext - the file extension. example ".png"
 * MissingFilesList - string vector where the IDs of missing game files will be put in.
 **************************************************************************************/
int GetMissingGameFiles(const char * path, const char * fileext, std::vector<std::string> & MissingFilesList)
{
	char gameID[7];
	char filepath[512];
	MissingFilesList.clear();

	for (int i = 0; i < gameList.size(); ++i)
	{
		struct discHdr* header = gameList[i];

		// Only GameCube games might have missing banners
		if (strcmp(fileext, ".bnr") == 0 && !(header->type >= TYPE_GAME_GC_IMG && header->type <= TYPE_GAME_GC_EXTRACTED))
			continue;
		
		// NAND and EmuNAND games don't have disc artwork
		if (strcmp(path, Settings.disc_path) == 0 && (header->type == TYPE_GAME_NANDCHAN || header->type == TYPE_GAME_EMUNANDCHAN))
			continue;

		snprintf(gameID, sizeof(gameID), "%s", (char *) header->id);
		snprintf(filepath, sizeof(filepath), "%s/%s%s", path, gameID, fileext);

		if (CheckFile(filepath))
			continue;

		//! Not found. Try 4 ID path.
		gameID[4] = '\0';
		snprintf(filepath, sizeof(filepath), "%s/%s%s", path, gameID, fileext);

		if (CheckFile(filepath))
			continue;

		//! Not found. Try 3 ID path.
		gameID[3] = '\0';
		snprintf(filepath, sizeof(filepath), "%s/%s%s", path, gameID, fileext);

		if (CheckFile(filepath))
			continue;

		//! Not found add to missing list
		snprintf(gameID, sizeof(gameID), "%s", (char *) header->id);
		MissingFilesList.push_back(std::string(gameID));
	}

	if(dvdheader)
	{
		snprintf(gameID, sizeof(gameID), "%s", (char *) dvdheader->id);
		snprintf(filepath, sizeof(filepath), "%s/%s%s", path, gameID, fileext);

		if (!CheckFile(filepath)) {
			//! Not found, add missing dvd header to list
			MissingFilesList.push_back(std::string(gameID));
		}
	}

	gprintf(" = %i\n", MissingFilesList.size());

	return MissingFilesList.size();
}
