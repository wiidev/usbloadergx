#ifndef GAME_LIST_H_
#define GAME_LIST_H_

#include <vector>
#include "wstring.hpp"
#include "usbloader/disc.h"

class GameList
{
    public:
        GameList() : selectedGame(0) { };
        int ReadGameList();
        int size() const { return FilteredList.size(); }
        int GameCount() const { return FullGameList.size(); }
        int FilterList(const wchar_t * gameFilter = NULL);
        int LoadUnfiltered();
        struct discHdr * at(int i) const { return operator[](i); }
        struct discHdr * operator[](int i) const { if (i < 0 || i >= (int) FilteredList.size()) return NULL; return FilteredList[i]; }
        struct discHdr * GetDiscHeader(const char * gameID) const;
        const wchar_t * GetCurrentFilter() const { return GameFilter.c_str(); }
        const wchar_t * GetAvailableSearchChars() const { return AvailableSearchChars.c_str(); }
        void SortList();
        void clear();
        bool operator!() const { return (FullGameList.size() == 0); }
        //! Gamelist scrolling operators
        int operator+=(int i) { return (selectedGame = (selectedGame+i) % FilteredList.size()); }
        int operator-=(int i) { return (selectedGame = (selectedGame-i+FilteredList.size()) % FilteredList.size()); }
        int operator++() { return (selectedGame = (++selectedGame) % FilteredList.size()); }
        int operator--() { return (selectedGame = ((--selectedGame)+FilteredList.size()) % FilteredList.size()); }
        int operator++(int i) { return operator++(); }
        int operator--(int i) { return operator--(); }
        struct discHdr * GetCurrentSelected() const { return operator[](selectedGame); }

    protected:
        static bool NameSortCallback(const struct discHdr *a, const struct discHdr *b);
        static bool PlaycountSortCallback(const struct discHdr *a, const struct discHdr *b);
        static bool RankingSortCallback(const struct discHdr *a, const struct discHdr *b);

        wString AvailableSearchChars;
        wString GameFilter;
        int selectedGame;
        std::vector<struct discHdr *> FilteredList;
        std::vector<struct discHdr> FullGameList;
};

extern GameList gameList;

#endif
