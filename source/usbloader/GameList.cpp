/****************************************************************************
 * Copyright (C) 2010
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
 ***************************************************************************/
#include <algorithm>
#include <string>
#include <wctype.h>
#include <malloc.h>
#include "usbloader/wbfs.h"
#include "settings/newtitles.h"
#include "settings/CSettings.h"
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/GameTitles.h"
#include "xml/xml.h"
#include "FreeTypeGX.h"
#include "GameList.h"
#include "memory/memory.h"

GameList gameList;

void GameList::clear()
{
    GameFilter.clear();
    AvailableSearchChars.clear();
    FullGameList.clear();
    GamePartitionList.clear();
    FilteredList.clear();
    //! Clear memory of the vector completely
    std::vector<struct discHdr *>().swap(FilteredList);
    std::vector<struct discHdr>().swap(FullGameList);
}

struct discHdr * GameList::GetDiscHeader(const char * gameID) const
{
    for (u32 i = 0; i < FilteredList.size(); ++i)
    {
        if(strncasecmp(gameID, (const char *) FilteredList[i]->id, 6) == 0)
            return FilteredList[i];
    }

    return NULL;
}

int GameList::GetPartitionNumber(const u8 *gameID) const
{
    for (u32 i = 0; i < FullGameList.size(); ++i)
    {
        if(strncasecmp((const char *) gameID, (const char *) FullGameList[i].id, 6) == 0)
            return GamePartitionList[i];
    }

    return -1;
}

void GameList::RemovePartition(int part)
{
    wString filter(GameFilter);

    for(u32 i = 0; i < GamePartitionList.size(); ++i)
    {
        if(GamePartitionList[i] == part)
        {
            FullGameList.erase(FullGameList.begin()+i);
            GamePartitionList.erase(GamePartitionList.begin()+i);
            --i;
        }
    }

    if(FullGameList.size() > 0)
        FilterList(filter.c_str());
}

int GameList::InternalReadList(int part)
{
    // Retrieve all stuff from WBFS
    u32 cnt = 0;

    int ret = WBFS_GetCount(part, &cnt);
    if (ret < 0) return -1;

    // We are done here if no games are there
    if(cnt == 0)
        return 0;

    /* Buffer length */
    u32 len = sizeof(struct discHdr) * cnt;

    /* Allocate memory */
    struct discHdr *buffer = (struct discHdr *) allocate_memory( len );
    if (!buffer) return -1;

    /* Clear buffer */
    memset(buffer, 0, len);

    /* Get header list */
    ret = WBFS_GetHeaders(part, buffer, cnt, sizeof(struct discHdr));
    if (ret < 0)
    {
        free(buffer);
        return -1;
    }

    u32 oldSize = FullGameList.size();

    FullGameList.resize(oldSize+cnt);
    memcpy(&FullGameList[oldSize], buffer, len);

    free(buffer);

    GamePartitionList.resize(oldSize+cnt);

    for(u32 i = oldSize; i < GamePartitionList.size(); ++i)
        GamePartitionList[i] = part;

    return cnt;
}

int GameList::ReadGameList()
{
    // Clear list
    clear();

    if(!Settings.MultiplePartitions)
    {
        int ret = InternalReadList(Settings.partition);
        if(ret <= 0) return ret;
    }
    else
    {
        PartitionHandle * usbHandle = DeviceHandler::Instance()->GetUSBHandle();
        int cnt = 0;

        for(int part = 0; part < usbHandle->GetPartitionCount(); ++part)
        {
            int ret = InternalReadList(part);
            if(ret > 0) cnt += ret;
        }

        if(!cnt) return cnt;
    }

    return LoadUnfiltered();
}

static bool WCharSortCallback(const wchar_t char1, const wchar_t char2)
{
    if (char2 == 0) return true;
    if (char1 == 0) return false;

    return char2 > char1;
}

int GameList::FilterList(const wchar_t * gameFilter)
{
    if (FullGameList.size() == 0) ReadGameList();
    if (gameFilter) GameFilter.assign(gameFilter);

    FilteredList.clear();
    AvailableSearchChars.clear();

    for (u32 i = 0; i < FullGameList.size(); ++i)
    {
        struct discHdr *header = &FullGameList[i];

        /* Register game */
        NewTitles::Instance()->CheckGame(header->id);

        /* Filters */
        if (Settings.GameSort & SORT_FAVORITE)
        {
            GameStatus * GameStats = GameStatistics.GetGameStatus(header->id);
            if (Settings.marknewtitles)
            {
                bool isNew = NewTitles::Instance()->IsNew(header->id);
                if (!isNew && (!GameStats || GameStats->FavoriteRank == 0)) continue;
            }
            else
            {
                if (!GameStats || GameStats->FavoriteRank == 0) continue;
            }
        }

        //ignore uLoader cfg "iso".  i was told it is "__CFG_"  but not confirmed
        if (strncasecmp((char*) header->id, "__CFG_", 6) == 0)
            continue;

        GameCFG * GameConfig = GameSettings.GetGameCFG(header);

        /* Rating based parental control method */
        if (Settings.parentalcontrol != PARENTAL_LVL_ADULT && !Settings.godmode)
        {
            if (GameConfig && GameConfig->parentalcontrol > Settings.parentalcontrol)
                continue;

            // Check game rating in WiiTDB, since the default Wii parental control setting is enabled
            int rating = GameTitles.GetParentalRating((char *) header->id);
            if (rating > Settings.parentalcontrol)
                continue;
        }

        //! Per game lock method
        if(!Settings.godmode && GameConfig && GameConfig->Locked)
            continue;

        wchar_t *gameName = charToWideChar(GameTitles.GetTitle(header));
        if (gameName && *GameFilter.c_str())
        {
            if (wcsnicmp(gameName, GameFilter.c_str(), GameFilter.size()) != 0)
            {
                delete [] gameName;
                continue;
            }
        }

        if (gameName)
        {
            if (wcslen(gameName) > GameFilter.size() &&
                AvailableSearchChars.find(towupper(gameName[GameFilter.size()])) == std::string::npos &&
                AvailableSearchChars.find(towlower(gameName[GameFilter.size()])) == std::string::npos)
            {
                AvailableSearchChars.push_back(gameName[GameFilter.size()]);
            }

            delete [] gameName;
        }

        FilteredList.push_back(header);
    }

    NewTitles::Instance()->Save();

    AvailableSearchChars.push_back(L'\0');

    if (FilteredList.size() < 2)
        AvailableSearchChars.clear();

    SortList();

    return FilteredList.size();
}

int GameList::LoadUnfiltered()
{
    if (FullGameList.size() == 0) ReadGameList();

    GameFilter.clear();
    AvailableSearchChars.clear();
    FilteredList.clear();

    for (u32 i = 0; i < FullGameList.size(); ++i)
    {
        struct discHdr *header = &FullGameList[i];

        /* Register game */
        NewTitles::Instance()->CheckGame(header->id);

        wchar_t *gameName = charToWideChar(GameTitles.GetTitle(header));
        if (gameName)
        {
            if (wcslen(gameName) > GameFilter.size() &&
                AvailableSearchChars.find(towupper(gameName[GameFilter.size()])) == std::string::npos &&
                AvailableSearchChars.find(towlower(gameName[GameFilter.size()])) == std::string::npos)
            {
                AvailableSearchChars.push_back(gameName[GameFilter.size()]);
            }

            delete [] gameName;
        }

        FilteredList.push_back(header);
    }

    NewTitles::Instance()->Save();

    AvailableSearchChars.push_back(L'\0');

    if (FilteredList.size() < 2)
        AvailableSearchChars.clear();

    SortList();

    return FilteredList.size();
}

void GameList::SortList()
{
    if (FilteredList.size() < 2) return;

    if (Settings.GameSort & SORT_PLAYCOUNT)
    {
        std::sort(FilteredList.begin(), FilteredList.end(), PlaycountSortCallback);
    }
    else if(Settings.GameSort & SORT_RANKING)
    {
        std::sort(FilteredList.begin(), FilteredList.end(), RankingSortCallback);
    }
    else if(Settings.GameSort & SORT_PLAYERS)
    {
        std::sort(FilteredList.begin(), FilteredList.end(), PlayersSortCallback);
    }
    else
    {
        std::sort(FilteredList.begin(), FilteredList.end(), NameSortCallback);
    }

    if (AvailableSearchChars.size() > 1)
        std::sort(AvailableSearchChars.begin(), AvailableSearchChars.end(), WCharSortCallback);

}

bool GameList::NameSortCallback(const struct discHdr *a, const struct discHdr *b)
{
    return (strcasecmp(GameTitles.GetTitle((struct discHdr *) a), GameTitles.GetTitle((struct discHdr *) b)) < 0);
}

bool GameList::PlaycountSortCallback(const struct discHdr *a, const struct discHdr *b)
{
    int count1 = GameStatistics.GetPlayCount(a->id);
    int count2 = GameStatistics.GetPlayCount(b->id);

    if (count1 == count2) return NameSortCallback(a, b);

    return (count1 > count2);
}

bool GameList::RankingSortCallback(const struct discHdr *a, const struct discHdr *b)
{
    int fav1 = GameStatistics.GetFavoriteRank(a->id);
	int fav2 = GameStatistics.GetFavoriteRank(b->id);

    if (fav1 == fav2) return NameSortCallback(a, b);

    return (fav1 > fav2);
}

bool GameList::PlayersSortCallback(const struct discHdr *a, const struct discHdr *b)
{
    int count1 = GameTitles.GetPlayersCount((const char *) a->id);
	int count2 = GameTitles.GetPlayersCount((const char *) b->id);

    if (count1 == count2) return NameSortCallback(a, b);

    return (count1 > count2);
}
