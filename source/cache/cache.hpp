/*
	Code by Oddx @ GBAtemp.net
*/
#include "usbloader/disc.h"
#include "settings/CSettings.h"

#define WII_HEADER_CACHE_FILE "WII.cache"
#define GAMECUBE_HEADER_CACHE_FILE "GAMECUBE.cache"
#define EMUNAND_HEADER_CACHE_FILE "EMUNAND.cache"

struct gameHdr
{
	/* Game ID */
	u8 id[6];

	/* Padding */
	u8 unused3[2];
};

struct wiiCache
{
	struct discHdr header;
	int part;
};

struct gcCache
{
	struct discHdr header;
	u8 path[200];
};

void ResetGameHeaderCache();
void GetDirectoryList(const char *path, std::string &list);
void GetListWBFS(std::string &list);
bool isCacheCurrent();
bool isCacheFile(std::string filename);

// EmuNAND
void SaveGameHeaderCache(std::vector<struct discHdr> &list);
void LoadGameHeaderCache(std::vector<struct discHdr> &list);

// Wii
void SaveGameHeaderCache(std::vector<struct discHdr> &list, std::vector<int> &plist);
void LoadGameHeaderCache(std::vector<struct discHdr> &list, std::vector<int> &plist);

// GameCube
void SaveGameHeaderCache(std::vector<struct discHdr> &list, std::vector<std::string> &plist);
void LoadGameHeaderCache(std::vector<struct discHdr> &list, std::vector<std::string> &plist);

bool isCacheFile(std::string filename);
