#include <string.h>
#include <unistd.h>
#include <algorithm>

#include "GameTitles.h"
#include "CSettings.h"
#include "usbloader/GameList.h"
#include "Channels/channels.h"
#include "xml/GameTDB.hpp"
#include "svnrev.h"
#include "FileOperations/fileops.h"

#define VALID_CACHE_REVISION 1280

CGameTitles GameTitles;

bool dbExists = false;

void CGameTitles::SortTitleList()
{
	std::sort(TitleList.begin(), TitleList.end(), [](const GameTitle &x, const GameTitle &y)
			  { return (strncasecmp(x.GameID, y.GameID, 6) < 0); });
}

void CGameTitles::SetGameTitle(const char *id, const char *title, char TitleType, const std::string region, int ParentalRating, int PlayersCount)
{
	if (!id || !title)
		return;

	for (u32 i = 0; i < TitleList.size(); ++i)
	{
		if (strncasecmp(TitleList[i].GameID, id, 6) == 0)
		{
			TitleList[i].Title = title;
			TitleList[i].Region = region;
			if (ParentalRating != -1)
				TitleList[i].ParentalRating = ParentalRating;
			if (PlayersCount != 1)
				TitleList[i].PlayersCount = PlayersCount;
			TitleList[i].TitleType = TitleType;
			return;
		}
	}

	GameTitle newTitle;
	snprintf(newTitle.GameID, sizeof(newTitle.GameID), id);
	newTitle.Title = title;
	newTitle.Region = region;
	newTitle.ParentalRating = ParentalRating;
	newTitle.PlayersCount = PlayersCount;
	newTitle.TitleType = TitleType;

	TitleList.push_back(newTitle);
}

const char *CGameTitles::GetTitle(const char *id, bool allow_access) const
{
	if (!id)
		return "";

	auto game = std::lower_bound(TitleList.begin(), TitleList.end(), id, [](const GameTitle &gt, const char *gameid)
								 { return (strncasecmp(gt.GameID, gameid, 6) < 0); });
	if (game != TitleList.end())
	{
		if (strncasecmp((const char *)game->GameID, (const char *)id, 6) == 0)
		{
			if ((dbExists && Settings.TitlesType == TITLETYPE_FROMWIITDB) || game->TitleType == TITLETYPE_MANUAL_OVERRIDE || (Settings.TitlesType == TITLETYPE_FORCED_DISC && game->TitleType == TITLETYPE_FORCED_DISC) || allow_access)
				return game->Title.c_str();
		}
	}

	for (int i = 0; i < gameList.size(); ++i)
	{
		if (strncasecmp((const char *)gameList[i]->id, id, 6) == 0)
			return gameList[i]->title;
	}

	return "";
}

const char *CGameTitles::GetTitle(const struct discHdr *header) const
{
	if (!header)
		return "";

	auto game = std::lower_bound(TitleList.begin(), TitleList.end(), (const char *)header->id, [](const GameTitle &gt, const char *gameid)
								 { return (strncasecmp(gt.GameID, gameid, 6) < 0); });
	if (game != TitleList.end())
	{
		if (strncasecmp(game->GameID, (const char *)header->id, 6) == 0)
		{
			if ((dbExists && Settings.TitlesType == TITLETYPE_FROMWIITDB) || game->TitleType == TITLETYPE_MANUAL_OVERRIDE || (Settings.TitlesType == TITLETYPE_FORCED_DISC && game->TitleType == TITLETYPE_FORCED_DISC))
				return game->Title.c_str();
		}
	}

	return header->title;
}

char CGameTitles::GetTitleType(const char *id) const
{
	if (!id)
		return 0;

	for (u32 i = 0; i < TitleList.size(); ++i)
	{
		if (strncasecmp(TitleList[i].GameID, id, 6) == 0)
			return TitleList[i].TitleType;
	}

	return 0;
}

const char *CGameTitles::GetRegion(const char *id) const
{
	if (!id || Settings.TitlesType != TITLETYPE_FROMWIITDB)
		return "NULL";

	for (u32 i = 0; i < TitleList.size(); ++i)
	{
		if (strncasecmp(TitleList[i].GameID, id, 6) == 0)
			return TitleList[i].Region.c_str();
	}

	return "NULL";
}

int CGameTitles::GetParentalRating(const char *id) const
{
	if (!id)
		return -1;

	for (u32 i = 0; i < TitleList.size(); ++i)
	{
		if (strncasecmp(TitleList[i].GameID, id, 6) == 0)
			return TitleList[i].ParentalRating;
	}

	return -1;
}

int CGameTitles::GetPlayersCount(const char *id) const
{
	if (!id)
		return 1;

	for (u32 i = 0; i < TitleList.size(); ++i)
	{
		if (strncasecmp(TitleList[i].GameID, id, 6) == 0)
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

void CGameTitles::Reset()
{
	if (TitleList.empty())
		return;

	for (u32 i = 0; i < TitleList.size(); ++i)
	{
		if (TitleList[i].TitleType < TITLETYPE_MANUAL_OVERRIDE)
			TitleList[i].TitleType = TITLETYPE_DEFAULT;
	}
}

u32 CGameTitles::ReadCachedTitles(const char *path)
{
	std::string dbpath(Settings.titlestxt_path);
	if (dbpath.back() != '/')
		dbpath += '/';
	dbpath += "wiitdb.xml";
	dbExists = CheckFile(dbpath.c_str());

	std::string Cachepath(path);
	if (Cachepath.back() != '/')
		Cachepath += '/';
	Cachepath += "TitlesCache.bin";

	//! Load cached last so that the titles are preloaded before reading list
	FILE *f = fopen(Cachepath.c_str(), "rb");
	if (!f)
		return 0;

	u32 revision = 0;

	fread(&revision, 1, 4, f);

	if (revision < VALID_CACHE_REVISION)
	{
		fclose(f);
		return 0;
	}

	char LangCode[11];
	memset(LangCode, 0, sizeof(LangCode));

	fread(LangCode, 1, 10, f);

	//! Check if cache has correct language code
	if (strcmp(LangCode, Settings.db_language) != 0)
	{
		fclose(f);
		return 0;
	}

	u32 count = 0;
	fread(&count, 1, 4, f);

	std::vector<CacheTitle> CachedList(count);
	TitleList.resize(count);

	fread(&CachedList[0], 1, count * sizeof(CacheTitle), f);
	fclose(f);

	for (u32 i = 0; i < count; ++i)
	{
		strcpy(TitleList[i].GameID, CachedList[i].GameID);
		TitleList[i].Title = CachedList[i].Title;
		TitleList[i].Region = CachedList[i].Region;
		TitleList[i].ParentalRating = CachedList[i].ParentalRating;
		TitleList[i].PlayersCount = CachedList[i].PlayersCount;
		TitleList[i].TitleType = CachedList[i].TitleType;
	}

	return count;
}

void CGameTitles::WriteCachedTitles(const char *path)
{
	std::string Cachepath(path);
	if (Cachepath.back() != '/')
		Cachepath += '/';
	Cachepath += "TitlesCache.bin";

	FILE *f = fopen(Cachepath.c_str(), "wb");
	if (!f)
		return;

	CacheTitle Cache;
	u32 count = TitleList.size();
	u32 revision = atoi(GetRev());
	fwrite(&revision, 1, 4, f);
	fwrite(Settings.db_language, 1, 10, f);
	fwrite(&count, 1, 4, f);
	SortTitleList();

	for (u32 i = 0; i < count; ++i)
	{
		memset(&Cache, 0, sizeof(CacheTitle));
		snprintf(Cache.GameID, sizeof(Cache.GameID), "%s", TitleList[i].GameID);
		snprintf(Cache.Title, sizeof(Cache.Title), "%s", TitleList[i].Title.c_str());
		snprintf(Cache.Region, sizeof(Cache.Region), "%s", TitleList[i].Region.c_str());
		Cache.ParentalRating = TitleList[i].ParentalRating;
		Cache.PlayersCount = TitleList[i].PlayersCount;
		Cache.TitleType = TitleList[i].TitleType;

		fwrite(&Cache, 1, sizeof(CacheTitle), f);
	}

	fclose(f);
}

int CGameTitles::GetMissingTitles(std::vector<std::string> &MissingTitles, std::vector<struct discHdr *> &headerlist)
{
	for (u32 i = 0; i < headerlist.size(); ++i)
	{
		bool found = false;
		for (u32 n = 0; n < TitleList.size(); ++n)
		{
			if (strncasecmp(TitleList[n].GameID, (const char *)headerlist[i]->id, 6) == 0)
			{
				if (TitleList[n].TitleType >= TITLETYPE_FROMWIITDB)
					found = true;
				break;
			}
		}
		if (!found /*&& !headerlist[i]->tid*/)
			MissingTitles.push_back((std::string)(const char *)headerlist[i]->id);
	}
	return MissingTitles.size();
}

void CGameTitles::CleanTitles(std::vector<struct discHdr *> &headerlist)
{
	if (TitleList.empty())
		return;

	for (u32 n = 0; n < TitleList.size(); ++n)
	{
		bool isCached = false;
		for (u32 i = 0; i < headerlist.size(); ++i)
		{
			if (strncasecmp(TitleList[n].GameID, (const char *)headerlist[i]->id, 6) == 0)
			{
				isCached = true;
				break;
			}
		}
		if (!isCached)
		{
			TitleList.erase(TitleList.begin() + n);
			n--;
		}
	}
	return;
}

void CGameTitles::LoadTitlesFromGameTDB(const char *path)
{
	if (!path || Settings.TitlesType != TITLETYPE_FROMWIITDB)
		return;

	std::string Filepath(path);
	if (Filepath.back() != '/')
		Filepath += '/';
	Filepath += "wiitdb.xml";

	std::vector<struct discHdr *> headerlist;
	if (!gameList.GetGameListHeaders(headerlist, MODE_ALL))
		return;

	//! Removed unused cache titles and get the still missing ones
	CleanTitles(headerlist);
	std::vector<std::string> MissingTitles;
	if (!GetMissingTitles(MissingTitles, headerlist))
		return;

	GameTDB XML_DB;
	if (!XML_DB.OpenFile(Filepath.c_str()))
		return;
	dbExists = true;

	XML_DB.SetLanguageCode(Settings.db_language);

	for (u32 i = 0; i < MissingTitles.size(); ++i)
	{
		std::string Title;
		std::string Region;
		std::string RatValTxt;
		int ParentalRating = -1;
		int PlayersCount = 1;

		if (!XML_DB.GetTitle(MissingTitles[i].c_str(), Title))
			continue;

		// This change allows the game to be sorted correctly
		if (Title.compare("Ōkami") == 0)
			Title.assign("Okami");

		int Rating = XML_DB.GetRating(MissingTitles[i].c_str());
		if (Rating >= 0)
		{
			if (XML_DB.GetRatingValue(MissingTitles[i].c_str(), RatValTxt))
				ParentalRating = GameTDB::ConvertRating(RatValTxt.c_str(), GameTDB::RatingToString(Rating), "PEGI");
		}

		int pc = XML_DB.GetPlayers(MissingTitles[i].c_str());
		if (pc > 0)
			PlayersCount = pc;

		if (XML_DB.GetRegion(MissingTitles[i].c_str(), Region))
			SetGameTitle(MissingTitles[i].c_str(), Title.c_str(), TITLETYPE_FROMWIITDB, Region, ParentalRating, PlayersCount);
		else
			SetGameTitle(MissingTitles[i].c_str(), Title.c_str(), TITLETYPE_FROMWIITDB, "NULL", ParentalRating, PlayersCount);
	}
	XML_DB.CloseFile();
	SortTitleList();
}
