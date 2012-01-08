#include <string.h>
#include "GameTitles.h"
#include "CSettings.h"
#include "usbloader/GameList.h"
#include "Channels/channels.h"
#include "xml/GameTDB.hpp"
#include "svnrev.h"
#include "gecko.h"

#define VALID_CACHE_REVISION	1148

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
	snprintf(newTitle.GameID, sizeof(newTitle.GameID), id);
	newTitle.Title = title;
	newTitle.ParentalRating = -1;
	newTitle.PlayersCount = 1;
	newTitle.FromWiiTDB = 0;

	TitleList.push_back(newTitle);
}

const char * CGameTitles::GetTitle(const char * id) const
{
	if(!id)
		return "";

	for(u32 i = 0; i < TitleList.size(); ++i)
	{
		if(strncasecmp(id, TitleList[i].GameID, 6) == 0)
			return TitleList[i].Title.c_str();
	}

	for(int i = 0; i < gameList.size(); ++i)
	{
		if(strncasecmp(id, (char *) gameList[i]->id, 6) == 0)
			return gameList[i]->title;

	}

	return "";
}

const char * CGameTitles::GetTitle(const struct discHdr *header) const
{
	if(!header)
		return "";

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
	char FromWiiTDB;
	int ParentalRating;
	int PlayersCount;

} ATTRIBUTE_PACKED CacheTitle;

u32 CGameTitles::ReadCachedTitles(const char * path)
{
	std::string Cachepath = path;
	if(path[strlen(path)-1] != '/')
		Cachepath += '/';
	Cachepath += "TitlesCache.bin";

	//! Load cached least so that the titles are preloaded before reading list
	FILE * f = fopen(Cachepath.c_str(), "rb");
	if(!f) return 0;

	u32 revision = 0;

	fread(&revision, 1, 4, f);

	if(revision < VALID_CACHE_REVISION)
	{
		fclose(f);
		return 0;
	}

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
		TitleList[i].FromWiiTDB = CachedList[i].FromWiiTDB;
	}

	return count;
}

void CGameTitles::WriteCachedTitles(const char * path)
{
	std::string Cachepath = path;
	if(path[strlen(path)-1] != '/')
		Cachepath += '/';
	Cachepath += "TitlesCache.bin";

	FILE *f = fopen(Cachepath.c_str(), "wb");
	if(!f)
		return;

	CacheTitle Cache;
	u32 count = TitleList.size();
	u32 revision = atoi(GetRev());
	fwrite(&revision, 1, 4, f);
	fwrite(Settings.db_language, 1, 10, f);
	fwrite(&count, 1, 4, f);

	for(u32 i = 0; i < count; ++i)
	{
		memset(&Cache, 0, sizeof(CacheTitle));

		strncpy(Cache.GameID, TitleList[i].GameID, sizeof(Cache.GameID)-1);
		strncpy(Cache.Title, TitleList[i].Title.c_str(), sizeof(Cache.Title)-1);
		Cache.ParentalRating = TitleList[i].ParentalRating;
		Cache.PlayersCount = TitleList[i].PlayersCount;
		Cache.FromWiiTDB = TitleList[i].FromWiiTDB;

		fwrite(&Cache, 1, sizeof(CacheTitle), f);
	}

	fclose(f);
}

void CGameTitles::GetMissingTitles(std::vector<std::string> &MissingTitles, bool removeUnused)
{
	std::vector<struct discHdr *> &FullList = gameList.GetFilteredList();
	std::vector<bool> UsedCachedList(TitleList.size(), false);

	for(u32 i = 0; i < FullList.size(); ++i)
	{
		bool isCached = false;

		for(u32 n = 0; n < TitleList.size(); ++n)
		{
			if(strncasecmp(TitleList[n].GameID, (const char *) FullList[i]->id, 6) == 0)
			{
				UsedCachedList[n] = true;
				//! If the title is not from WiiTDB, try to reload it
				isCached = TitleList[n].FromWiiTDB;
				break;
			}
		}

		if(!isCached)
		{
			char gameID[7];
			snprintf(gameID, sizeof(gameID), (const char *) FullList[i]->id);
			MissingTitles.push_back(std::string(gameID));
		}
	}

	if(!removeUnused)
		return;

	for(u32 n = 0; n < TitleList.size(); ++n)
	{
		if(!UsedCachedList[n])
		{
			TitleList.erase(TitleList.begin()+n);
			n--;
		}
	}
}

void CGameTitles::LoadTitlesFromGameTDB(const char * path, bool removeUnused)
{
	if(!path || !Settings.titlesOverride)
		return;

	std::string Filepath = path;
	if(path[strlen(path)-1] != '/')
		Filepath += '/';

	Filepath += "wiitdb.xml";

	//! Read game list
	gameList.LoadUnfiltered();

	//! Removed unused cache titles and get the still missing ones
	std::vector<std::string> MissingTitles;
	GetMissingTitles(MissingTitles, removeUnused);

	if(MissingTitles.size() == 0)
		return;

	std::string Title;

	GameTDB XML_DB(Filepath.c_str());
	XML_DB.SetLanguageCode(Settings.db_language);
	int Rating;
	std::string RatValTxt;

	for(u32 i = 0; i < MissingTitles.size(); ++i)
	{
		if(!XML_DB.GetTitle(MissingTitles[i].c_str(), Title))
			continue;

		this->SetGameTitle(MissingTitles[i].c_str(), Title.c_str());
		//! Title is loaded from WiiTDB, remember that it's good
		TitleList[TitleList.size()-1].FromWiiTDB = 1;

		Rating = XML_DB.GetRating(MissingTitles[i].c_str());
		if(Rating < 0)
			continue;

		if(!XML_DB.GetRatingValue(MissingTitles[i].c_str(), RatValTxt))
			continue;

		TitleList[TitleList.size()-1].ParentalRating = GameTDB::ConvertRating(RatValTxt.c_str(), GameTDB::RatingToString(Rating), "PEGI");
		int ret = XML_DB.GetPlayers(MissingTitles[i].c_str());
		if(ret > 0)
			TitleList[TitleList.size()-1].PlayersCount = ret;
	}
}
