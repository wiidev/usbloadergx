/*
    Code by Oddx @ GBAtemp.net
    Loosely based on emuNAND caching by geoGolem.
*/
#include <fstream>
#include <dirent.h>

#include "cache.hpp"
#include "usbloader/disc.h"
#include "settings/CSettings.h"
#include "FileOperations/fileops.h"
#include "memory/memory.h"
#include "Channels/channels.h"
#include "usbloader/GameList.h"
#include "GameCube/GCGames.h"

void ResetGameHeaderCache()
{
    RemoveDirectory(Settings.GameHeaderCachePath);
    return;
}

// emuNAND
void SaveGameHeaderCache(vector<struct discHdr> &list)
{
    string path = string(Settings.GameHeaderCachePath) + EMUNAND_HEADER_CACHE_FILE;

    if (!CheckFile(Settings.GameHeaderCachePath))
        CreateSubfolder(Settings.GameHeaderCachePath);

    FILE *cache = fopen(path.c_str(), "wb");

    if (!cache)
        return;

    fwrite((void *)&list[0], 1, list.size() * sizeof(struct discHdr), cache);

    fclose(cache);
}

void LoadGameHeaderCache(vector<struct discHdr> &list)
{
    string path = string(Settings.GameHeaderCachePath) + EMUNAND_HEADER_CACHE_FILE;

    FILE *cache = fopen(path.c_str(), "rb");

    if (!cache)
        return;

    struct discHdr tmp;
    fseek(cache, 0, SEEK_END);
    u64 fileSize = ftell(cache);
    fseek(cache, 0, SEEK_SET);

    u32 count = (u32)(fileSize / sizeof(struct discHdr));

    list.reserve(count + list.size());
    for (u32 i = 0; i < count; i++)
    {
        fseek(cache, i * sizeof(struct discHdr), SEEK_SET);
        fread((void *)&tmp, 1, sizeof(struct discHdr), cache);
        list.push_back(tmp);
    }

    fclose(cache);
}

// Wii
void SaveGameHeaderCache(vector<struct discHdr> &list, vector<int> &plist)
{
    vector<struct wiiCache> wiictmp;
    struct wiiCache gtmp;

    for (u32 i = 0; i < list.size(); ++i)
    {
        memset(&gtmp, 0, sizeof(struct wiiCache));
        gtmp.header = list[i];
        gtmp.part = plist[i];
        wiictmp.push_back(gtmp);
    }

    string path = string(Settings.GameHeaderCachePath) + WII_HEADER_CACHE_FILE;

    if (!CheckFile(Settings.GameHeaderCachePath))
        CreateSubfolder(Settings.GameHeaderCachePath);

    FILE *cache = fopen(path.c_str(), "wb");

    if (!cache)
        return;

    fwrite((void *)&wiictmp[0], 1, wiictmp.size() * sizeof(struct wiiCache), cache);

    fclose(cache);
}

void LoadGameHeaderCache(vector<struct discHdr> &list, vector<int> &plist)
{
    string path = string(Settings.GameHeaderCachePath) + WII_HEADER_CACHE_FILE;

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
    for (u32 i = 0; i < count; i++)
    {
        fseek(cache, i * sizeof(struct wiiCache), SEEK_SET);
        fread((void *)&wiictmp, 1, sizeof(struct wiiCache), cache);
        list.push_back(wiictmp.header);
        plist.push_back(wiictmp.part);
    }

    fclose(cache);
}

// GameCube
void SaveGameHeaderCache(vector<struct discHdr> &list, vector<string> &plist)
{
    vector<struct gcCache> gcctmp;
    struct gcCache gtmp;

    for (u32 i = 0; i < list.size(); ++i)
    {
        memset(&gtmp, 0, sizeof(gcCache));
        gtmp.header = list[i];

        strcpy((char *)gtmp.path, plist[i].c_str());

        gcctmp.push_back(gtmp);
    }

    string path = string(Settings.GameHeaderCachePath) + GAMECUBE_HEADER_CACHE_FILE;

    if (!CheckFile(Settings.GameHeaderCachePath))
        CreateSubfolder(Settings.GameHeaderCachePath);

    FILE *cache = fopen(path.c_str(), "wb");

    if (!cache)
        return;

    fwrite((void *)&gcctmp[0], 1, gcctmp.size() * sizeof(struct gcCache), cache);

    fclose(cache);
}

void LoadGameHeaderCache(vector<struct discHdr> &list, vector<string> &plist)
{
    string path = string(Settings.GameHeaderCachePath) + GAMECUBE_HEADER_CACHE_FILE;

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
    for (u32 i = 0; i < count; i++)
    {
        fseek(cache, i * sizeof(struct gcCache), SEEK_SET);
        fread((void *)&gcctmp, 1, sizeof(struct gcCache), cache);
        list.push_back(gcctmp.header);

        string tmp((char *)gcctmp.path);
        plist.push_back(tmp);
    }
    fclose(cache);
}

void SaveFilteredListCache(vector<struct discHdr *> &list, const wchar_t *gameFilter)
{
    string path = string(Settings.GameHeaderCachePath) + FilteredListCacheFileName(gameFilter);

    if (!CheckFile(Settings.GameHeaderCachePath))
        CreateSubfolder(Settings.GameHeaderCachePath);

    FILE *cache = fopen(path.c_str(), "wb");

    if (!cache)
        return;

    vector<struct gameHdr> tmplist;
    struct gameHdr tmp;

    for (u32 i = 0; i < list.size(); ++i)
    {
        memcpy(tmp.id, list[i]->id, 6);
        tmplist.push_back(tmp);
    }

    fwrite((void *)&tmplist[0], 1, tmplist.size() * sizeof(struct gameHdr), cache);
    fclose(cache);
}

void LoadFilteredListCache(vector<struct discHdr *> &list, const wchar_t *gameFilter)
{
    string path = string(Settings.GameHeaderCachePath) + FilteredListCacheFileName(gameFilter);

    if (!CheckFile(Settings.GameHeaderCachePath))
        CreateSubfolder(Settings.GameHeaderCachePath);

    FILE *cache = fopen(path.c_str(), "rb");

    if (!cache)
        return;

    struct gameHdr tmp;

    fseek(cache, 0, SEEK_END);
    u64 fileSize = ftell(cache);
    fseek(cache, 0, SEEK_SET);

    u32 count = (u32)(fileSize / sizeof(struct gameHdr));

    list.reserve(count + list.size());
    for (u32 i = 0; i < count; i++)
    {
        bool found = false;
        fseek(cache, i * sizeof(struct gameHdr), SEEK_SET);
        fread((void *)&tmp, 1, sizeof(struct gameHdr), cache);

        if (!found)
        {
            vector<struct discHdr> &tmplist = gameList.GetFullGameList();
            for (u32 c = 0; c < tmplist.size(); ++c)
            {
                struct discHdr *header = &tmplist[c];
                if (strncasecmp((const char *)tmp.id, (const char *)header->id, 6) == 0)
                {
                    list.push_back(header);
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            vector<struct discHdr> &tmplist = GCGames::Instance()->GetHeaders();
            for (u32 c = 0; c < tmplist.size(); ++c)
            {
                struct discHdr *header = &tmplist[c];
                if (strncasecmp((const char *)tmp.id, (const char *)header->id, 6) == 0)
                {
                    list.push_back(header);
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            vector<struct discHdr> &tmplist = Channels::Instance()->GetNandHeaders();
            for (u32 c = 0; c < tmplist.size(); ++c)
            {
                struct discHdr *header = &tmplist[c];
                if (strncasecmp((const char *)tmp.id, (const char *)header->id, 6) == 0)
                {
                    list.push_back(header);
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            vector<struct discHdr> &tmplist = Channels::Instance()->GetEmuHeaders();
            for (u32 c = 0; c < tmplist.size(); ++c)
            {
                struct discHdr *header = &tmplist[c];
                if (strncasecmp((const char *)tmp.id, (const char *)header->id, 6) == 0)
                {
                    list.push_back(header);
                    found = true;
                    break;
                }
            }
        }
    }

    fclose(cache);
}

string FilteredListCacheFileName(const wchar_t *gameFilter)
{
    string tmp;
    tmp = "FL";
    tmp += "_" + to_string(Settings.LoaderMode);
    tmp += "_" + to_string(Settings.GameSort);
    if (gameFilter)
    {
        wstring ws(gameFilter);
        string gf(ws.begin(), ws.end());
        if ((gf.length()) > 0)
            tmp += "_" + gf;
    }
    tmp += ".cache";
    return tmp;
}

string FilteredListCacheFileName()
{
    string tmp;
    tmp = "FL";
    tmp += "_" + to_string(Settings.LoaderMode);
    tmp += "_" + to_string(Settings.GameSort);
    tmp += ".cache";
    return tmp;
}

bool isCacheFile(string filename)
{
    string path = string(Settings.GameHeaderCachePath) + filename;

    if (CheckFile(path.c_str()))
        return true;

    return false;
}
