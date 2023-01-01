/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#ifndef GAMETDB_HPP_
#define GAMETDB_HPP_

#include <vector>
#include <string>
#include "pugixml.hpp"

typedef struct _Accessory
{
	std::string Name;
	bool Required;
} Accessory;

typedef struct _GameXMLInfo
{
	std::string GameID;
	std::string Region;
	std::string Title;
	std::string Synopsis;
	std::string Developer;
	std::string Publisher;
	unsigned int PublishDate;
	std::vector<std::string> GenreList;
	int RatingType;
	std::string RatingValue;
	std::vector<std::string> RatingDescriptorList;
	int WifiPlayers;
	std::vector<std::string> WifiFeatureList;
	int Players;
	std::vector<Accessory> AccessoryList;
	long CaseColor;

} GameXMLInfo;

typedef struct _GameOffsets
{
	char gameID[7];
	unsigned int gamenode;
	unsigned int nodesize;
} __attribute__((__packed__)) GameOffsets;

class GameTDB
{
public:
	//! Constructor
	GameTDB();
	//! Constructor
	//! If filepath is passed the xml file is opened and the node offsets are loaded
	GameTDB(const char *filepath);
	//! Destructor
	~GameTDB();
	//! If filepath is passed the xml file is opened and the node offsets are loaded
	bool OpenFile(const char *filepath);
	//! Closes the GameTDB xml file
	void CloseFile();
	//! Set the language code which should be use to find the appropriate language
	//! If the language code is not found, the language code defaults to EN
	void SetLanguageCode(const char *code) { if(code) LangCode = code; };
	//! Get the current set language code
	const char *GetLanguageCode() { return LangCode.c_str(); };
	//! Get the title of a specific game id in the language defined in LangCode
	bool GetTitle(const char *id, std::string &title);
	//! Get the synopsis of a specific game id in the language defined in LangCode
	bool GetSynopsis(const char *id, std::string &synopsis);
	//! Get the region of a game for a specific game id
	bool GetRegion(const char *id, std::string &region);
	//! Get the languages of a game for a specific game id
	bool GetLanguages(const char *id, std::string &languages);
	//! Get the developer of a game for a specific game id
	bool GetDeveloper(const char *id, std::string &dev);
	//! Get the publisher of a game for a specific game id
	bool GetPublisher(const char *id, std::string &pub);
	//! Get the publish date of a game for a specific game id
	//! First 1 byte is the day, than 1 byte month and last 2 bytes is the year
	//! year = (return >> 16), month = (return >> 8) & 0xFF, day = return & 0xFF
	unsigned int GetPublishDate(const char *id);
	//! Get the genre list of a game for a specific game id
	bool GetGenreList(const char *id, std::vector<std::string> &genre);
	//! Get the rating type for a specific game id
	//! The rating type can be converted to a std::string with GameTDB::RatingTostd::string(rating)
	int GetRating(const char *id);
	//! Get the rating value for a specific game id
	bool GetRatingValue(const char *id, std::string &rating_value);
	//! Get the rating descriptor list inside a std::vector for a specific game id
	//! Returns the amount of descriptors found or -1 if failed
	int GetRatingDescriptorList(const char *id, std::vector<std::string> &desc_list);
	//! Get the wifi player count for a specific game id
	//! Returns the amount of wifi players or -1 if failed
	int GetWifiPlayers(const char *id);
	//! Get the wifi feature list inside a std::vector for a specific game id
	//! Returns the amount of wifi features found or -1 if failed
	int GetWifiFeatureList(const char *id, std::vector<std::string> &feat_list);
	//! Get the player count for a specific game id
	//! Returns the amount of players or -1 if failed
	int GetPlayers(const char *id);
	//! Returns the amount of accessories found or -1 if failed
	//! Get the accessory (inputs) list inside a std::vector for a specific game id
	int GetAccessoryList(const char *id, std::vector<Accessory> &acc_list);
	//! Get the box (case) color for a specific game id
	//! Returns the color in RGB (first 3 bytes)
	int GetCaseColor(const char *id);
	//! Get the complete game info in the GameXMLInfo struct
	bool GetGameXMLInfo(const char *id, GameXMLInfo *gameInfo);
	//! Get the type of the game. If blank the game is a Wii game.
	bool GetGameType(const char *id, std::string &GameType);
	//! Translate genre list to configure language code
	void TranslateGenres(std::vector<std::string> &GenreList);
	//! Translate descriptors list to configure language code
	//! void TranslateDescriptors(std::vector<std::string> &DescList);
	//! Convert a specific game rating to a std::string
	static const char *RatingToString(int rating);
	//! Convert a rating std::string to a rating number
	static int StringToRating(const char *rate_string);
	//! Convert a rating to another rating
	static int ConvertRating(const char *value, const char *from, const char *to);
	//! Get the version of the gametdb xml database
	unsigned long long GetGameTDBVersion();
	//! Get the entry count in the xml database
	inline size_t GetEntryCount() { return OffsetMap.size(); };

private:
	bool ParseFile();
	bool LoadGameOffsets(const char *path);
	bool SaveGameOffsets(const char *path);
	bool ParseGameNode(const char *id);
	inline int GetData(char *data, int offset, int size);
	inline char *LoadGameNode(const char *id);
	inline GameOffsets *GetGameOffset(const char *id);

	std::vector<GameOffsets> OffsetMap;
	FILE *file;
	std::string LangCode;
	char GameIDCache[7];
	pugi::xml_document xmlDoc;
};

#endif
