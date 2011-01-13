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

    //! Just in case a 0 termination is missing
    int n;
    for(n = 0; n < 6; ++n)
        newTitle.GameID[n] = id[n];

    newTitle.GameID[n] = '\0';

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

void CGameTitles::SetDefault()
{
    TitleList.clear();
    //! Free vector memory
    std::vector<GameTitle>().swap(TitleList);
}

void CGameTitles::LoadTitlesFromWiiTDB(const char * path)
{
    this->SetDefault();

    if(!path || !Settings.titlesOverride)
        return;

    gameList.LoadUnfiltered();

    std::string Title;
    std::string Filepath = path;
    if(path[strlen(path)-1] != '/')
        Filepath += '/';
    Filepath += "wiitdb.xml";

    WiiTDB XML_DB(Filepath.c_str());
    XML_DB.SetLanguageCode(Settings.db_language);
    int Rating;
    std::string RatValTxt;

    for(int i = 0; i < gameList.GameCount(); ++i)
    {
        if(!XML_DB.GetTitle((const char *) gameList[i]->id, Title))
            continue;

        this->SetGameTitle(gameList[i]->id, Title.c_str());

        TitleList[TitleList.size()-1].ParentalRating = -1;

        Rating = XML_DB.GetRating((const char *) gameList[i]->id);
        if(Rating < 0)
            continue;

        if(!XML_DB.GetRatingValue((const char *) gameList[i]->id, RatValTxt))
            continue;

        TitleList[TitleList.size()-1].ParentalRating = ConvertRating(RatValTxt.c_str(), WiiTDB::RatingToString(Rating), "PEGI");
    }
}
