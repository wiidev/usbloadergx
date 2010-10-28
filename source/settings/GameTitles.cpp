#include <string.h>
#include "GameTitles.h"

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

const char * CGameTitles::GetTitle(const char * id)
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

const char * CGameTitles::GetTitle(const struct discHdr *header)
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

void CGameTitles::SetDefault()
{
    TitleList.clear();
    //! Free vector memory
    std::vector<GameTitle>().swap(TitleList);
}
