#ifndef WIITDB_TITLES_H_
#define WIITDB_TITLES_H_

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

} GameTitle;

class CGameTitles
{
    public:
        //! Set a game title from wiitdb
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
        //! Load Game Titles from WiiTDB
        void LoadTitlesFromWiiTDB(const char * path, bool forceCacheReload = false);
        //! Set default game titles
        void SetDefault();
        //! Free memory and remove all titles - Same as SetDefault()
        void Clear() { SetDefault(); }
    protected:
        u32 ReadCachedTitles(const char * path);
        void WriteCachedTitles(const char * path);
        void RemoveUnusedCache(std::vector<std::string> &MissingTitles);

        std::vector<GameTitle> TitleList;
};

extern CGameTitles GameTitles;

#endif
