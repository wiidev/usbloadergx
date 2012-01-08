#ifndef GAMETDB_TITLES_H_
#define GAMETDB_TITLES_H_

#include <string>
#include <vector>
#include <gctypes.h>
#include "usbloader/disc.h"

typedef struct _GameTitle
{
	char GameID[7];
	std::string Title;
	int ParentalRating;
	int PlayersCount;
	char FromWiiTDB;

} GameTitle;

class CGameTitles
{
	public:
		//! Set a game title from GameTDB
		void SetGameTitle(const char * id, const char * title);
		//! Overload
		void SetGameTitle(const u8 * id, const char * title) { SetGameTitle((const char *) id, title); };

		//! Get a game title
		const char * GetTitle(const char * id) const;
		//! Overload
		const char * GetTitle(const u8 * id) const { return GetTitle((const char *) id); };
		//! Overload
		const char * GetTitle(const struct discHdr *header) const;

		//! Get game parental rating
		int GetParentalRating(const char * id) const;
		//! Get possible number of players for this game
		int GetPlayersCount(const char * id) const;
		//! Load Game Titles from GameTDB
		void LoadTitlesFromGameTDB(const char * path, bool removeUnused = true);
		//! Set default game titles
		void SetDefault();
		//! Free memory and remove all titles - Same as SetDefault()
		void Clear() { SetDefault(); }
		//! Cache titles functions
		u32 ReadCachedTitles(const char * path);
		void WriteCachedTitles(const char * path);
	protected:
		void GetMissingTitles(std::vector<std::string> &MissingTitles, bool removeUnused);

		std::vector<GameTitle> TitleList;
};

extern CGameTitles GameTitles;

#endif
