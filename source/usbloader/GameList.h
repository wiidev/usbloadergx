#ifndef GAME_LIST_H_
#define GAME_LIST_H_

#include <vector>
#include "Controls/DeviceHandler.hpp"
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
		void SortList();
		void clear();
		bool operator!() const { return (FullGameList.size() == 0); }
		//! Gamelist scrolling operators
		int operator+=(int i) { return (selectedGame = (selectedGame+i) % FilteredList.size()); }
		int operator-=(int i) { return (selectedGame = (selectedGame-i+FilteredList.size()) % FilteredList.size()); }
		int operator++() { return (selectedGame = (selectedGame+1) % FilteredList.size()); }
		int operator--() { return (selectedGame = (selectedGame-1+FilteredList.size()) % FilteredList.size()); }
		int operator++(int i) { return operator++(); }
		int operator--(int i) { return operator--(); }
		struct discHdr * GetCurrentSelected() const { return operator[](selectedGame); }
		int GetPartitionNumber(const u8 *gameid) const;
		int GetGameFS(const u8 *gameID) const { return DeviceHandler::Instance()->GetFilesystemType(USB1+GetPartitionNumber(gameID)); }
		void RemovePartition(int part_num);
		std::vector<struct discHdr *> &GetFilteredList(void) { return FilteredList; }
		std::vector<struct discHdr> &GetFullGameList(void) { return FullGameList; }
	protected:
		int InternalReadList(int part);
		void InternalFilterList(std::vector<struct discHdr> &FullList);
		void InternalLoadUnfiltered(std::vector<struct discHdr> &FullList);
		static bool NameSortCallback(const struct discHdr *a, const struct discHdr *b);
		static bool PlaycountSortCallback(const struct discHdr *a, const struct discHdr *b);
		static bool RankingSortCallback(const struct discHdr *a, const struct discHdr *b);
		static bool PlayersSortCallback(const struct discHdr *a, const struct discHdr *b);

		wString GameFilter;
		int selectedGame;
		std::vector<struct discHdr *> FilteredList;
		std::vector<struct discHdr> FullGameList;
		std::vector<int> GamePartitionList;
};

extern GameList gameList;

#endif
