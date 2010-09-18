#ifndef GAME_LIST_H_
#define GAME_LIST_H_

#include <vector>
#include "wstring.hpp"
#include "usbloader/disc.h"

class GameList
{
    public:
        GameList();
        int ReadGameList();
        int size() { return FilteredList.size(); };
        int GameCount() { return FullGameList.size(); };
        int FilterList( const wchar_t * gameFilter = NULL );
        int LoadUnfiltered();
        struct discHdr * at( int i );
        struct discHdr * operator[]( int i ) { return at( i ); };
        const wchar_t * GetCurrentFilter() { return GameFilter.c_str(); };
        const wchar_t * GetAvailableSearchChars() { return AvailableSearchChars.c_str(); };
        void SortList();
        void clear();
    protected:
        static bool NameSortCallback( const struct discHdr *a, const struct discHdr *b );
        static bool PlaycountSortCallback( const struct discHdr *a, const struct discHdr *b );
        static bool FavoriteSortCallback( const struct discHdr *a, const struct discHdr *b );

        wString AvailableSearchChars;
        wString GameFilter;
        std::vector<struct discHdr *> FilteredList;
        std::vector<struct discHdr> FullGameList;
};

extern GameList gameList;

#endif
