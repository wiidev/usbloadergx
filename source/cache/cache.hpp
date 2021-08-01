/*
    Code by Oddx @ GBAtemp.net
    Loosely based on emuNAND caching by geoGolem.
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

// emuNAND
void SaveGameHeaderCache(std::vector<struct discHdr> &list);
void LoadGameHeaderCache(std::vector<struct discHdr> &list);

// Wii
void SaveGameHeaderCache(std::vector<struct discHdr> &list, std::vector<int> &plist);
void LoadGameHeaderCache(std::vector<struct discHdr> &list, std::vector<int> &plist);

// GameCube
void SaveGameHeaderCache(std::vector<struct discHdr> &list, std::vector<std::string> &plist);
void LoadGameHeaderCache(std::vector<struct discHdr> &list, std::vector<std::string> &plist);

void ResetGameHeaderCache();

void SaveFilteredListCache(std::vector<struct discHdr *> &list, const wchar_t *gameFilter);
void LoadFilteredListCache(std::vector<struct discHdr *> &list, const wchar_t *gameFilter);

std::string FilteredListCacheFileName(const wchar_t *gameFilter);
std::string FilteredListCacheFileName();
bool isCacheFile(std::string filename);
