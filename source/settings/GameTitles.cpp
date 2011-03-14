#include <string.h>
#include "GameTitles.h"
#include "CSettings.h"
#include "usbloader/GameList.h"
#include "xml/xml.h"
#include "xml/WiiTDB.hpp"

CGameTitles GameTitles;

void CGameTitles::SetGameTitle(const char * id, const char * title)
{
    if(!id || !title)
        return;

    for(u32 i = 0; i < TitleList.size(); ++i)
    {
        if(strncasecmp(id, TitleList[i].GameID, 6) == 0)
        {
            TitleList[i].Title = title;
            return;
        }
    }

    GameTitle newTitle;
    newTitle.Title = title;
    snprintf(newTitle.GameID, sizeof(newTitle.GameID), id);

    TitleList.push_back(newTitle);
}

const char * CGameTitles::GetTitle(const char * id) const
{
    if(!id)
        return NULL;

    for(u32 i = 0; i < TitleList.size(); ++i)
    {
        if(strncasecmp(id, TitleList[i].GameID, 6) == 0)
            return TitleList[i].Title.c_str();
    }

    return NULL;
}

const char * CGameTitles::GetTitle(const struct discHdr *header) const
{
    if(!header)
        return NULL;

    for(u32 i = 0; i < TitleList.size(); ++i)
    {
        if(strncasecmp((const char *) header->id, TitleList[i].GameID, 6) == 0)
            return TitleList[i].Title.c_str();
    }

    return header->title;
}

int CGameTitles::GetParentalRating(const char * id) const
{
    if(!id)
        return -1;

    for(u32 i = 0; i < TitleList.size(); ++i)
    {
        if(strncasecmp(id, TitleList[i].GameID, 6) == 0)
            return TitleList[i].ParentalRating;
    }

    return -1;
}


int CGameTitles::GetPlayersCount(const char * id) const
{
    if(!id)
        return 1;

    for(u32 i = 0; i < TitleList.size(); ++i)
    {
        if(strncasecmp(id, TitleList[i].GameID, 6) == 0)
            return TitleList[i].PlayersCount;
    }

    return 1;
}

void CGameTitles::SetDefault()
{
    TitleList.clear();
    //! Free vector memory
    std::vector<GameTitle>().swap(TitleList);
}

typedef struct _CacheTitle
{
    char GameID[7];
    char Title[100];
    int ParentalRating;
    int PlayersCount;

} ATTRIBUTE_PACKED CacheTitle;

u32 CGameTitles::ReadCachedTitles(const char * path)
{
    //! Load cached least so that the titles are preloaded before reading list
    FILE * f = fopen(path, "rb");
    if(!f) return 0;

    char LangCode[11];
    memset(LangCode, 0, sizeof(LangCode));

    fread(LangCode, 1, 10, f);

    //! Check if cache has correct language code
    if(strcmp(LangCode, Settings.db_language) != 0)
    {
        fclose(f);
        return 0;
    }

    u32 count = 0;
    fread(&count, 1, 4, f);

    std::vector<CacheTitle> CachedList(count);
    TitleList.resize(count);

    fread(&CachedList[0], 1, count*sizeof(CacheTitle), f);
    fclose(f);

    for(u32 i = 0; i < count; ++i)
    {
        strcpy(TitleList[i].GameID, CachedList[i].GameID);
        TitleList[i].Title = CachedList[i].Title;
        TitleList[i].ParentalRating = CachedList[i].ParentalRating;
        TitleList[i].PlayersCount = CachedList[i].PlayersCount;
    }

    return count;
}

void CGameTitles::WriteCachedTitles(const char * path)
{
    FILE *f = fopen(path, "wb");
    if(!f)
        return;

    CacheTitle Cache;
    u32 count = TitleList.size();

    fwrite(Settings.db_language, 1, 10, f);
    fwrite(&count, 1, 4, f);

    for(u32 i = 0; i < count; ++i)
    {
        memset(&Cache, 0, sizeof(CacheTitle));

        strcpy(Cache.GameID, TitleList[i].GameID);
        snprintf(Cache.Title, sizeof(Cache.Title), TitleList[i].Title.c_str());
        Cache.ParentalRating = TitleList[i].ParentalRating;
        Cache.PlayersCount = TitleList[i].PlayersCount;

        fwrite(&Cache, 1, sizeof(CacheTitle), f);
    }

    fclose(f);
}

void CGameTitles::RemoveUnusedCache(std::vector<std::string> &MissingTitles)
{
    std::vector<bool> UsedCachedList(TitleList.size(), false);

    for(int i = 0; i < gameList.GameCount(); ++i)
    {
        bool isCached = false;

        for(u32 n = 0; n < TitleList.size(); ++n)
        {
            if(strncmp(TitleList[n].GameID, (const char *) gameList[i]->id, 6) == 0)
            {
                UsedCachedList[n] = true;
                isCached = true;
                break;
            }
        }

        if(!isCached)
        {
            char gameID[7];
            snprintf(gameID, sizeof(gameID), (const char *) gameList[i]->id);
            MissingTitles.push_back(std::string(gameID));
        }
    }

    for(u32 n = 0; n < TitleList.size(); ++n)
    {
        if(!UsedCachedList[n])
            TitleList.erase(TitleList.begin()+n);
    }
}

void CGameTitles::LoadTitlesFromWiiTDB(const char * path, bool forceCacheReload)
{
    this->SetDefault();

    if(!path || !Settings.titlesOverride)
        return;

    std::string Filepath = path;
    if(path[strlen(path)-1] != '/')
        Filepath += '/';

    std::string Cachepath = Filepath;
    Cachepath += "TitlesCache.bin";
    Filepath += "wiitdb.xml";

    //! Read game titles cache database
    if(!forceCacheReload && Settings.CacheTitles)
        ReadCachedTitles(Cachepath.c_str());

    //! Read game list
    gameList.LoadUnfiltered();

    //! Removed unused cache titles and get the still missing ones
    std::vector<std::string> MissingTitles;
    RemoveUnusedCache(MissingTitles);
    if(MissingTitles.size() == 0)
    {
        WriteCachedTitles(Cachepath.c_str());
        return;
    }

    std::string Title;

    WiiTDB XML_DB(Filepath.c_str());
    XML_DB.SetLanguageCode(Settings.db_language);
    int Rating;
    std::string RatValTxt;

    for(u32 i = 0; i < MissingTitles.size(); ++i)
    {
        if(!XML_DB.GetTitle(MissingTitles[i].c_str(), Title))
            continue;

        this->SetGameTitle(MissingTitles[i].c_str(), Title.c_str());

        TitleList[TitleList.size()-1].ParentalRating = -1;
        TitleList[TitleList.size()-1].PlayersCount = 1;

        Rating = XML_DB.GetRating(MissingTitles[i].c_str());
        if(Rating < 0)
            continue;

        if(!XML_DB.GetRatingValue(MissingTitles[i].c_str(), RatValTxt))
            continue;

        TitleList[TitleList.size()-1].ParentalRating = ConvertRating(RatValTxt.c_str(), WiiTDB::RatingToString(Rating), "PEGI");
        int ret = XML_DB.GetPlayers(MissingTitles[i].c_str());
        if(ret > 0)
            TitleList[TitleList.size()-1].PlayersCount = ret;
    }

    if(Settings.CacheTitles)
        WriteCachedTitles(Cachepath.c_str());
}
