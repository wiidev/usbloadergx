/*
	Code by Oddx @ GBAtemp.net
*/
#include <fstream>
#include <dirent.h>
#include <zlib.h>

#include "cache.hpp"
#include "usbloader/disc.h"
#include "settings/CSettings.h"
#include "FileOperations/fileops.h"
#include "memory/memory.h"
#include "Channels/channels.h"
#include "usbloader/GameList.h"
#include "GameCube/GCGames.h"
#include "FileOperations/DirList.h"
#include "wad/nandtitle.h"
#include "Controls/DeviceHandler.hpp"
#include "gecko.h"

void ResetGameHeaderCache()
{
	RemoveDirectory(Settings.GameHeaderCachePath);
}

void GetDirectoryList(const char *path, std::string &list)
{
	DirList dir(path, 0, DirList::Files | DirList::Dirs);
	for (int i = 0; i < dir.GetFilecount(); ++i)
		list.append(dir.GetFilepath(i));
}


void GetListWBFS(std::string &list)
{
	char drive[11];
	int portPart;
	u16 partitions = DeviceHandler::GetUSBPartitionCount();
	for (u16 i = 0; i < partitions; ++i)
	{
		PartitionHandle *usb = DeviceHandler::Instance()->GetUSBHandleFromPartition(i);
		if (!usb)
			continue;
		portPart = DeviceHandler::PartitionToPortPartition(i);
		snprintf(drive, sizeof(drive), "%s:/wbfs", usb->MountName(portPart));
		gprintf("Partition: %d - %s\n", i, drive);
		GetDirectoryList(drive, list);
	}
}

bool isCacheCurrent()
{
	if (!Settings.CacheTitles)
		return true;

	char filepath[256] = {};
	std::string list;

	// GameCube
	if (!Settings.SDMode)
	{
		snprintf(filepath, sizeof(filepath), "%s", Settings.GameCubePath);
		GetDirectoryList(filepath, list);
	}
	snprintf(filepath, sizeof(filepath), "%s", Settings.GameCubeSDPath);
	GetDirectoryList(filepath, list);

	// Wii
	if (Settings.SDMode)
	{
		snprintf(filepath, sizeof(filepath), "sd:/wbfs");
		GetDirectoryList(filepath, list);
	}
	else
		GetListWBFS(list);

	// EmuNAND
	snprintf(filepath, sizeof(filepath), "%s/title/00010001", Settings.NandEmuChanPath);
	GetDirectoryList(filepath, list);
	snprintf(filepath, sizeof(filepath), "%s/title/00010002", Settings.NandEmuChanPath);
	GetDirectoryList(filepath, list);
	snprintf(filepath, sizeof(filepath), "%s/title/00010004", Settings.NandEmuChanPath);
	GetDirectoryList(filepath, list);

	// NAND
	u32 types[3] = {0x00010001, 0x00010002, 0x00010004};
	for (u32 i = 0; i < 3; ++i)
	{
		u32 num_titles = NandTitles.SetType(types[i]);
		for (u32 x = 0; x < num_titles; ++x)
		{
			u64 tid = NandTitles.Next();
			if (!tid)
				break;
			snprintf(filepath, sizeof(filepath), "%016llx", tid);
			list.append(filepath);
		}
	}

	// Generate a CRC-32 hash
	u32 crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (u8 *)list.c_str(), list.length());

	if (Settings.CacheCheckCRC == crc)
		return true;
	gprintf("Resetting cache\n");
	Settings.CacheCheckCRC = crc;
	return false;
}

// EmuNAND
void SaveGameHeaderCache(std::vector<struct discHdr> &list)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + EMUNAND_HEADER_CACHE_FILE;
	if (list.empty())
		RemoveFile(path.c_str());

	CreateSubfolder(Settings.GameHeaderCachePath);

	FILE *cache = fopen(path.c_str(), "wb");
	if (!cache)
		return;

	fwrite((void *)&list[0], 1, list.size() * sizeof(struct discHdr), cache);
	fclose(cache);
}

void LoadGameHeaderCache(std::vector<struct discHdr> &list)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + EMUNAND_HEADER_CACHE_FILE;

	FILE *cache = fopen(path.c_str(), "rb");
	if (!cache)
		return;

	struct discHdr tmp;
	fseek(cache, 0, SEEK_END);
	u64 fileSize = ftell(cache);
	fseek(cache, 0, SEEK_SET);

	u32 count = (u32)(fileSize / sizeof(struct discHdr));

	list.reserve(count + list.size());
	for (u32 i = 0; i < count; ++i)
	{
		fseek(cache, i * sizeof(struct discHdr), SEEK_SET);
		fread((void *)&tmp, 1, sizeof(struct discHdr), cache);
		list.push_back(tmp);
	}

	fclose(cache);
}

// Wii
void SaveGameHeaderCache(std::vector<struct discHdr> &list, std::vector<int> &plist)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + WII_HEADER_CACHE_FILE;
	if (list.empty() || plist.empty())
		RemoveFile(path.c_str());

	std::vector<struct wiiCache> wiictmp;
	struct wiiCache gtmp;

	for (u32 i = 0; i < list.size(); ++i)
	{
		memset(&gtmp, 0, sizeof(struct wiiCache));
		gtmp.header = list[i];
		gtmp.part = plist[i];
		wiictmp.push_back(gtmp);
	}

	CreateSubfolder(Settings.GameHeaderCachePath);

	FILE *cache = fopen(path.c_str(), "wb");
	if (!cache)
		return;

	fwrite((void *)&wiictmp[0], 1, wiictmp.size() * sizeof(struct wiiCache), cache);
	fclose(cache);
}

void LoadGameHeaderCache(std::vector<struct discHdr> &list, std::vector<int> &plist)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + WII_HEADER_CACHE_FILE;

	FILE *cache = fopen(path.c_str(), "rb");
	if (!cache)
		return;

	struct wiiCache wiictmp;
	fseek(cache, 0, SEEK_END);
	u64 fileSize = ftell(cache);
	fseek(cache, 0, SEEK_SET);

	u32 count = (u32)(fileSize / sizeof(struct wiiCache));

	list.reserve(count + list.size());
	plist.reserve(count + plist.size());
	for (u32 i = 0; i < count; ++i)
	{
		fseek(cache, i * sizeof(struct wiiCache), SEEK_SET);
		fread((void *)&wiictmp, 1, sizeof(struct wiiCache), cache);
		list.push_back(wiictmp.header);
		plist.push_back(wiictmp.part);
	}

	fclose(cache);
}

// GameCube
void SaveGameHeaderCache(std::vector<struct discHdr> &list, std::vector<std::string> &plist)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + GAMECUBE_HEADER_CACHE_FILE;
	if (list.empty() || plist.empty())
		RemoveFile(path.c_str());

	std::vector<struct gcCache> gcctmp;
	struct gcCache gtmp;

	for (u32 i = 0; i < list.size(); ++i)
	{
		memset(&gtmp, 0, sizeof(gcCache));
		gtmp.header = list[i];

		strcpy((char *)gtmp.path, plist[i].c_str());

		gcctmp.push_back(gtmp);
	}

	CreateSubfolder(Settings.GameHeaderCachePath);

	FILE *cache = fopen(path.c_str(), "wb");
	if (!cache)
		return;

	fwrite((void *)&gcctmp[0], 1, gcctmp.size() * sizeof(struct gcCache), cache);
	fclose(cache);
}

void LoadGameHeaderCache(std::vector<struct discHdr> &list, std::vector<std::string> &plist)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + GAMECUBE_HEADER_CACHE_FILE;

	FILE *cache = fopen(path.c_str(), "rb");
	if (!cache)
		return;

	struct gcCache gcctmp;
	fseek(cache, 0, SEEK_END);
	u64 fileSize = ftell(cache);
	fseek(cache, 0, SEEK_SET);

	u32 count = (u32)(fileSize / sizeof(struct gcCache));

	list.reserve(count + list.size());
	plist.reserve(count + plist.size());
	for (u32 i = 0; i < count; ++i)
	{
		fseek(cache, i * sizeof(struct gcCache), SEEK_SET);
		fread((void *)&gcctmp, 1, sizeof(struct gcCache), cache);
		list.push_back(gcctmp.header);

		std::string tmp((char *)gcctmp.path);
		plist.push_back(tmp);
	}

	fclose(cache);
}

bool isCacheFile(std::string filename)
{
	std::string path = std::string(Settings.GameHeaderCachePath) + filename;
	return CheckFile(path.c_str());
}
