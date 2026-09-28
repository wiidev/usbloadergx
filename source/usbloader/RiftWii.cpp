/****************************************************************************
 * Copyright (C) 2026 KakarottoCake
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
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <malloc.h>
#include "settings/CSettings.h"
#include "settings/SettingsEnums.h"
#include "FileOperations/fileops.h"
#include "homebrewboot/BootHomebrew.h"
#include "RiftWii.hpp"
#include "gecko.h"

#define RIIVOLUTION_PATH "sd:/riivolution/"

std::string RiftWii::LoaderPath()
{
	std::string path = Settings.homebrewapps_path;
	if (path.size() && path.back() != '/')
		path += '/';
	path += "riftwii/boot.dol";
	if (CheckFile(path.c_str()))
		return path;
	if (CheckFile("sd:/apps/riftwii/boot.dol"))
		return "sd:/apps/riftwii/boot.dol";
	return "";
}

//! A pack names its game in <id game="RMC"> (three to six characters)
static bool PackIsForGame(const char *filepath, const char *gameID)
{
	FILE *file = fopen(filepath, "rb");
	if (!file)
		return false;

	char head[4096];
	size_t len = fread(head, 1, sizeof(head) - 1, file);
	fclose(file);
	head[len] = 0;

	const char *id = strstr(head, "<id");
	const char *game = id ? strstr(id, "game=\"") : NULL;
	if (!game)
		return false;
	game += 6;

	size_t n = 0;
	while (game[n] && game[n] != '"')
		n++;
	return n >= 3 && n <= 6 && strncmp(game, gameID, n) == 0;
}

void RiftWii::ListPacks(const char *gameID, std::vector<std::string> &packs)
{
	packs.clear();
	DIR *dir = opendir(RIIVOLUTION_PATH);
	if (!dir)
		return;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		const char *name = entry->d_name;
		size_t len = strlen(name);
		//! Hidden files are macOS leftovers; ';' would end the entry in GameSettings.cfg
		if (name[0] == '.' || len < 5 || strcasecmp(name + len - 4, ".xml") != 0 || strchr(name, ';'))
			continue;
		std::string path = std::string(RIIVOLUTION_PATH) + name;
		if (PackIsForGame(path.c_str(), gameID))
			packs.push_back(name);
	}
	closedir(dir);
}

int RiftWii::BootGame(const struct discHdr &gameHeader, const std::string &pack, u8 videoChoice, u8 languageChoice,
					  u8 ocarinaChoice, u8 deflicker, u16 videoWidth, u8 privateServer, const char *customAddress,
					  u8 autoIOS, s32 iosChoice, u64 returnTo)
{
	static const char *videoModes[VIDEO_MODE_MAX] = { "system", "game", "pal50", "pal60", "ntsc", "system", "480p", "480p" };
	static const char *deflickers[DEFLICKER_MAX] = { "game", "off", "off", "low", "medium", "high" };
	static const char *languages[CONSOLE_DEFAULT + 1] = { "ja", "en", "de", "fr", "es", "it", "nl", "zh-hans", "zh-hant", "ko", "console" };
	static const char *servers[PRIVSERV_MAX_CHOICE] = { "off", "off", "wiimmfi", "altwfc", "custom" };

	std::string loader = LoaderPath();
	if (loader.empty())
		return -1;

	std::vector<std::string> args;
	args.push_back("--launch");
	args.push_back(std::string("game=") + std::string((const char *)gameHeader.id, 6));
	if (gameHeader.type == TYPE_GAME_WII_DISC)
		args.push_back("from=disc");
	else
		args.push_back(Settings.SDMode ? "from=sd" : "from=usb");
	args.push_back("xml=" RIIVOLUTION_PATH + pack);

	args.push_back(std::string("video_mode=") + videoModes[videoChoice]);
	args.push_back(std::string("deflicker=") + deflickers[deflicker]);
	args.push_back(videoWidth == WIDTH_FRAMEBUFFER ? "video_width=framebuffer" : "video_width=game");
	args.push_back(std::string("language=") + languages[languageChoice]);
	args.push_back(std::string("server=") + servers[privateServer]);
	if (privateServer == PRIVSERV_CUSTOM && *customAddress)
		args.push_back(std::string("wfc_domain=") + customAddress);

	if (autoIOS == GAME_IOS_CUSTOM && iosChoice >= 248 && iosChoice <= 252)
		args.push_back("cios=" + std::to_string(iosChoice));
	else
		args.push_back("cios=auto");

	//! RiftWii reads codes from the SD card only
	char gct[160];
	snprintf(gct, sizeof(gct), "%s%.6s.gct", Settings.Cheatcodespath, (const char *)gameHeader.id);
	if (ocarinaChoice == OCARINA_ON && strncmp(gct, "sd:/", 4) == 0 && CheckFile(gct))
		args.push_back(std::string("gct=") + gct);
	else
		args.push_back("gct=none");

	if (returnTo)
	{
		char title[17];
		snprintf(title, sizeof(title), "%016llx", returnTo);
		args.push_back(std::string("return_to=") + title);
	}
	else
		args.push_back("return_to=menu");

	u8 *buffer = NULL;
	u32 filesize = 0;
	LoadFileToMem(loader.c_str(), &buffer, &filesize);
	if (!buffer)
		return -1;
	FreeHomebrewBuffer();
	CopyHomebrewMemory(buffer, 0, filesize);
	free(buffer);

	gprintf("RiftWii: %s", loader.c_str());
	AddBootArgument(loader.c_str());
	for (size_t i = 0; i < args.size(); i++)
	{
		gprintf(" %s", args[i].c_str());
		AddBootArgument(args[i].c_str());
	}
	gprintf("\n");

	return BootHomebrewFromMem();
}
