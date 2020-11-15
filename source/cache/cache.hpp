/*
    Code by Oddx @ GBAtemp.net
    Loosely based on emuNAND caching by geoGolem.
*/
#include "usbloader/disc.h"
#include "settings/CSettings.h"

#define WII_HEADER_CACHE_FILE "WII.cache"
#define GAMECUBE_HEADER_CACHE_FILE "GAMECUBE.cache"
#define EMUNAND_HEADER_CACHE_FILE "EMUNAND.cache"

using namespace std;

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
void SaveGameHeaderCache(vector<struct discHdr> &list);
void LoadGameHeaderCache(vector<struct discHdr> &list);

// Wii
void SaveGameHeaderCache(vector<struct discHdr> &list, vector<int> &plist);
void LoadGameHeaderCache(vector<struct discHdr> &list, vector<int> &plist);

// GameCube
void SaveGameHeaderCache(vector<struct discHdr> &list, vector<string> &plist);
void LoadGameHeaderCache(vector<struct discHdr> &list, vector<string> &plist);

void ResetGameHeaderCache();

void SaveFilteredListCache(vector<struct discHdr *> &list, const wchar_t *gameFilter);
void LoadFilteredListCache(vector<struct discHdr *> &list, const wchar_t *gameFilter);

string FilteredListCacheFileName(const wchar_t *gameFilter);
string FilteredListCacheFileName();
bool isCacheFile(string filename);
