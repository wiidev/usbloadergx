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
        void LoadTitlesFromWiiTDB(const char * path);
        //! Set default game titles
        void SetDefault();
    protected:
        std::vector<GameTitle> TitleList;
};

extern CGameTitles GameTitles;

#endif
