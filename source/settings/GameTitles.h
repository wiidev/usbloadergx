#ifndef GAMETDB_TITLES_H_
#define GAMETDB_TITLES_H_

#include <string>
#include <vector>
#include <gctypes.h>
#include "usbloader/disc.h"
#include "SettingsEnums.h"

typedef struct _GameTitle
{
	char GameID[7];
	std::string Title;
	std::string Region;
	int ParentalRating;
	int PlayersCount;
	char TitleType;

} GameTitle;

typedef struct _CacheTitle
{
	char GameID[7];
	char Title[130]; // long titles e.g. RGOJJ9 & DLSP64
	char Region[7];
	int ParentalRating;
	int PlayersCount;
	char TitleType;

} ATTRIBUTE_PACKED CacheTitle;

class CGameTitles
{
	public:
		//! Sort the title list
		void SortTitleList();
		//! Set a game title from GameTDB
		void SetGameTitle(const char * id, const char * title, char TitleType = TITLETYPE_DEFAULT, std::string region = "NULL", int ParentalRating = -1, int PlayersCount = 1);
		//! Overload
		void SetGameTitle(const u8 * id, const char * title, char TitleType = TITLETYPE_DEFAULT) { SetGameTitle((const char *) id, title, TitleType); };

		//! Get a game title
		const char * GetTitle(const char * id, bool allow_access = false) const;
		//! Overload
		const char * GetTitle(const u8 * id, bool allow_access = false) const { return GetTitle((const char *) id, allow_access); };
		//! Overload
		const char * GetTitle(const struct discHdr *header) const;
		//! Get title type
		char GetTitleType(const char * id) const;
		//! Get game region
		const char * GetRegion(const char * id) const;
		//! Get game parental rating
		int GetParentalRating(const char * id) const;
		//! Get possible number of players for this game
		int GetPlayersCount(const char * id) const;
		//! Load Game Titles from GameTDB
		void LoadTitlesFromGameTDB(const char * path);
		//! Set default game titles
		void SetDefault();
		void Reset();
		//! Free memory and remove all titles - Same as SetDefault()
		void Clear() { SetDefault(); }
		//! Cache titles functions
		u32 ReadCachedTitles(const char * path);
		void WriteCachedTitles(const char * path);
	protected:
		int GetMissingTitles(std::vector<std::string> &MissingTitles, std::vector<struct discHdr *> &headerlist);
		void CleanTitles(std::vector<struct discHdr *> &headerlist);

		std::vector<GameTitle> TitleList;
};

extern CGameTitles GameTitles;

#endif
