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
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include "GameTDB.hpp"

#define NAME_OFFSET_DB "wiitdb_offsets.bin"
#define MAXREADSIZE 1024 * 1024 // Cache size only for parsing the offsets: 1MB

GameTDB::GameTDB()
	: file(0), LangCode("EN")
{
}

GameTDB::GameTDB(const char *filepath)
	: file(0), LangCode("EN")
{
	OpenFile(filepath);
}

GameTDB::~GameTDB()
{
	CloseFile();
}

bool GameTDB::OpenFile(const char *filepath)
{
	if (!filepath)
		return false;

	file = fopen(filepath, "rb");
	if (file)
	{
		int pos;
		std::string OffsetsPath(filepath);
		if ((pos = OffsetsPath.find_last_of('/')) != (int)std::string::npos)
			OffsetsPath[pos] = '\0';
		else
			OffsetsPath.clear(); //! Relative path

		LoadGameOffsets(OffsetsPath.c_str());
	}

	return (file != NULL);
}

void GameTDB::CloseFile()
{
	OffsetMap.clear();
	std::vector<GameOffsets>().swap(OffsetMap);

	if (file)
		fclose(file);
	file = NULL;
}

bool GameTDB::LoadGameOffsets(const char *path)
{
	if (!path)
		return false;

	std::string OffsetDBPath(path);
	if (OffsetDBPath.back() != '/')
		OffsetDBPath += '/';
	OffsetDBPath += NAME_OFFSET_DB;

	FILE *fp = fopen(OffsetDBPath.c_str(), "rb");
	if (!fp)
	{
		bool result = ParseFile();
		if (result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	unsigned long long ExistingVersion = GetGameTDBVersion();
	unsigned long long Version = 0;
	unsigned int NodeCount = 0;

	fread(&Version, 1, sizeof(Version), fp);

	if (ExistingVersion != Version)
	{
		fclose(fp);
		bool result = ParseFile();
		if (result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	fread(&NodeCount, 1, sizeof(NodeCount), fp);

	if (NodeCount == 0)
	{
		fclose(fp);
		bool result = ParseFile();
		if (result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	OffsetMap.resize(NodeCount);

	if ((int)fread(&OffsetMap[0], 1, NodeCount * sizeof(GameOffsets), fp) < 0)
	{
		fclose(fp);
		bool result = ParseFile();
		if (result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	fclose(fp);

	return true;
}

bool GameTDB::SaveGameOffsets(const char *path)
{
	if (OffsetMap.size() == 0 || !path)
		return false;

	FILE *fp = fopen(path, "wb");
	if (!fp)
		return false;

	unsigned long long ExistingVersion = GetGameTDBVersion();
	unsigned int NodeCount = OffsetMap.size();

	if (fwrite(&ExistingVersion, 1, sizeof(ExistingVersion), fp) != sizeof(ExistingVersion))
	{
		fclose(fp);
		return false;
	}

	if (fwrite(&NodeCount, 1, sizeof(NodeCount), fp) != sizeof(NodeCount))
	{
		fclose(fp);
		return false;
	}

	if (fwrite(&OffsetMap[0], 1, NodeCount * sizeof(GameOffsets), fp) != NodeCount * sizeof(GameOffsets))
	{
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}

unsigned long long GameTDB::GetGameTDBVersion()
{
	if (!file)
		return 0;

	char TmpText[1024];

	if (GetData(TmpText, 0, sizeof(TmpText)) < 0)
		return 0;

	char *versionNode = NULL;
	//! Try to find WiiTDB version node
	if (!versionNode && (versionNode = strstr(TmpText, "<WiiTDB")) != NULL)
	{
		char *versionNodeEnd = strstr(versionNode, "/>");
		if (versionNodeEnd)
		{
			versionNodeEnd += strlen("/>");
			*versionNodeEnd = '\0';
		}
	}

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result result = xmlDoc.load_string(versionNode);
	if (!result)
		return 0;

	return xmlDoc.child("WiiTDB").attribute("version").as_llong();
}

int GameTDB::GetData(char *data, int offset, int size)
{
	if (!file || !data)
		return -1;

	fseek(file, offset, SEEK_SET);

	return fread(data, 1, size, file);
}

char *GameTDB::LoadGameNode(const char *id)
{
	unsigned int read = 0;

	GameOffsets *offset = this->GetGameOffset(id);
	if (!offset)
		return NULL;

	char *data = new (std::nothrow) char[offset->nodesize + 1];
	if (!data)
		return NULL;

	if ((read = GetData(data, offset->gamenode, offset->nodesize)) != offset->nodesize)
	{
		delete[] data;
		return NULL;
	}

	data[read] = '\0';

	return data;
}

GameOffsets *GameTDB::GetGameOffset(const char *gameID)
{
	for (unsigned int i = 0; i < OffsetMap.size(); ++i)
	{
		if (strncmp(gameID, OffsetMap[i].gameID, 6) == 0)
			return &OffsetMap[i];
	}

	return 0;
}

bool GameTDB::ParseFile()
{
	OffsetMap.clear();

	if (!file)
		return false;

	char *Line = new (std::nothrow) char[MAXREADSIZE + 1];
	if (!Line)
		return false;

	bool readnew = false;
	int i, currentPos = 0;
	int read = 0;
	const char *gameNode = NULL;
	const char *idNode = NULL;
	const char *gameEndNode = NULL;
	const char *genreNode = NULL;
	const char *descriptNode = NULL;

	while ((read = GetData(Line, currentPos, MAXREADSIZE)) > 0)
	{
		gameNode = Line;
		readnew = false;

		//! Ensure the null termination at the end
		Line[read] = '\0';

		//! Try to find genre translation map
		if (!genreNode && (genreNode = strstr(gameNode, "<genres>")) != NULL)
		{
			const char *genreNodeEnd = strstr(genreNode, "</genres>");
			if (genreNodeEnd)
			{
				genreNodeEnd += strlen("</genres>");
				int size = OffsetMap.size();
				OffsetMap.resize(size + 1);
				strcpy(OffsetMap[size].gameID, "gnrmap");
				OffsetMap[size].gamenode = currentPos + (genreNode - Line);
				OffsetMap[size].nodesize = (genreNodeEnd - genreNode);
			}
		}

		//! Try to find description translation map
		if (!descriptNode && (descriptNode = strstr(gameNode, "<descriptors>")) != NULL)
		{
			const char *descriptNodeEnd = strstr(descriptNode, "</descriptors>");
			if (descriptNodeEnd)
			{
				descriptNodeEnd += strlen("</descriptors>");
				int size = OffsetMap.size();
				OffsetMap.resize(size + 1);
				strcpy(OffsetMap[size].gameID, "dscmap");
				OffsetMap[size].gamenode = currentPos + (descriptNode - Line);
				OffsetMap[size].nodesize = (descriptNodeEnd - descriptNode);
			}
		}

		while ((gameNode = strstr(gameNode, "<game name=\"")) != NULL)
		{
			idNode = strstr(gameNode, "<id>");
			gameEndNode = strstr(gameNode, "</game>");
			if (!idNode || !gameEndNode)
			{
				//! We are in the middle of the game node, reread complete node and more
				currentPos += (gameNode - Line);
				fseek(file, currentPos, SEEK_SET);
				readnew = true;
				break;
			}

			idNode += strlen("<id>");
			gameEndNode += strlen("</game>");

			int size = OffsetMap.size();
			OffsetMap.resize(size + 1);

			for (i = 0; i < 7 && *idNode != '<'; ++i, ++idNode)
				OffsetMap[size].gameID[i] = *idNode;
			OffsetMap[size].gameID[i] = '\0';
			OffsetMap[size].gamenode = currentPos + (gameNode - Line);
			OffsetMap[size].nodesize = (gameEndNode - gameNode);
			gameNode = gameEndNode;
		}

		if (readnew)
			continue;

		currentPos += read;
	}

	delete[] Line;

	return true;
}

const char *GameTDB::RatingToString(int rating)
{
	switch (rating)
	{
	case 0:
		return "CERO";
	case 1:
		return "ESRB";
	case 2:
		return "PEGI";
	default:
		break;
	}

	return NULL;
}

int GameTDB::StringToRating(const char *rate_string)
{
	if (strcasecmp(rate_string, "CERO") == 0)
		return 0;

	if (strcasecmp(rate_string, "ESRB") == 0)
		return 1;

	if (strcasecmp(rate_string, "PEGI") == 0)
		return 2;

	return -1;
}

int GameTDB::ConvertRating(const char *value, const char *from, const char *to)
{
	if (strcasecmp(from, to) == 0)
	{
		int ret = atoi(value);
		if (ret < 7)
			return 0;
		else if (ret < 12)
			return 1;
		else if (ret < 16)
			return 2;
		else if (ret < 18)
			return 3;
		else
			return 4;
	}

	int type = -1;
	int desttype = -1;

	type = StringToRating(from);
	desttype = StringToRating(to);
	if (type == -1 || desttype == -1)
		return -1;

	/* rating conversion table */
	/* the list is ordered to pick the most likely value first: */
	/* EC and AO are less likely to be used so they are moved down to only be picked up when converting ESRB to PEGI or CERO */
	/* the conversion can never be perfect because ratings can differ between regions for the same game */
	const int table_size = 12;
	char ratingtable[table_size][3][5] =
		{
			{{"A"},   {"E"},    {"3"}},
			{{"A"},   {"E"},    {"4"}},
			{{"A"},   {"E"},    {"6"}},
			{{"A"},   {"E"},    {"7"}},
			{{"A"},   {"EC"},   {"3"}},
			{{"A"},   {"E10+"}, {"7"}},
			{{"B"},   {"T"},    {"12"}},
			{{"D"},   {"M"},    {"18"}},
			{{"D"},   {"M"},    {"16"}},
			{{"C"},   {"T"},    {"16"}},
			{{"C"},   {"T"},    {"15"}},
			{{"Z"},   {"AO"},   {"18"}},
		};

	for (int i = 0; i < table_size; i++)
	{
		if (strcasecmp(ratingtable[i][type], value) == 0)
		{
			int res = atoi(ratingtable[i][desttype]);
			if (res < 7)
				return 0;
			else if (res < 12)
				return 1;
			else if (res < 16)
				return 2;
			else if (res < 18)
				return 3;
			else
				return 4;
		}
	}

	return -1;
}

bool GameTDB::GetGameXMLInfo(const char *id, GameXMLInfo *gameInfo)
{
	if (!id || !gameInfo)
		return false;

	GameIDCache[0] = '\0';

	for (int i = 0; i < 6 && id[i] != 0; ++i)
		gameInfo->GameID.push_back(id[i]);

	GetTitle(id, gameInfo->Title);
	GetSynopsis(id, gameInfo->Synopsis);
	GetRegion(id, gameInfo->Region);
	GetDeveloper(id, gameInfo->Developer);
	GetPublisher(id, gameInfo->Publisher);
	gameInfo->PublishDate = GetPublishDate(id);
	GetGenreList(id, gameInfo->GenreList);
	gameInfo->RatingType = GetRating(id);
	GetRatingValue(id, gameInfo->RatingValue);
//	GetRatingDescriptorList(id, gameInfo->RatingDescriptorList); // We don't use it yet
	gameInfo->WifiPlayers = GetWifiPlayers(id);
	GetWifiFeatureList(id, gameInfo->WifiFeatureList);
	gameInfo->Players = GetPlayers(id);
	GetAccessoryList(id, gameInfo->AccessoryList);
	gameInfo->CaseColor = GetCaseColor(id);

	return true;
}

bool GameTDB::ParseGameNode(const char *id)
{
	if (strncasecmp(GameIDCache, id, 6) == 0)
		return true;

	const char *data = LoadGameNode(id);
	if (!data)
		return false;

	pugi::xml_parse_result result = xmlDoc.load_string(data);
	if (!result)
		return false;

	snprintf(GameIDCache, sizeof(GameIDCache), id);
	return true;
}

bool GameTDB::GetTitle(const char *id, std::string &title)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;
	
	title = xmlDoc.child("game").find_child_by_attribute("locale", "lang", LangCode.c_str()).child_value("title");
	if (title.empty())
		title = xmlDoc.child("game").find_child_by_attribute("locale", "lang", "EN").child_value("title");

	if (title.empty())
		return false;

	return true;
}

bool GameTDB::GetSynopsis(const char *id, std::string &synopsis)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	synopsis = xmlDoc.child("game").find_child_by_attribute("locale", "lang", LangCode.c_str()).child_value("synopsis");
	if (synopsis.empty())
		synopsis = xmlDoc.child("game").find_child_by_attribute("locale", "lang", "EN").child_value("synopsis");

	if (synopsis.empty())
		return false;

	return true;
}

bool GameTDB::GetRegion(const char *id, std::string &region)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	region = xmlDoc.child("game").child_value("region");

	if (region.empty())
		return false;

	return true;
}

bool GameTDB::GetLanguages(const char *id, std::string &languages)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	languages = xmlDoc.child("game").child_value("languages");

	if (languages.empty())
		return false;

	return true;
}

bool GameTDB::GetDeveloper(const char *id, std::string &dev)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	dev = xmlDoc.child("game").child_value("developer");

	if (dev.empty())
		return false;

	return true;
}

bool GameTDB::GetPublisher(const char *id, std::string &pub)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	pub = xmlDoc.child("game").child_value("publisher");

	if (pub.empty())
		return false;

	return true;
}

unsigned int GameTDB::GetPublishDate(const char *id)
{
	if (!id)
		return 0;

	if (!ParseGameNode(id))
		return 0;

	return ((xmlDoc.child("game").child("date").attribute("year").as_int() & 0xFFFF) << 16 |
			(xmlDoc.child("game").child("date").attribute("month").as_int() & 0xFF) << 8 |
			(xmlDoc.child("game").child("date").attribute("day").as_int() & 0xFF));
}

bool GameTDB::GetGenreList(const char *id, std::vector<std::string> &genre)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	std::string the_genre = xmlDoc.child("game").child_value("genre");
	const char *delims = ",/;";

	size_t beg, pos = 0;
	while ((beg = the_genre.find_first_not_of(delims, pos)) != std::string::npos)
	{
		pos = the_genre.find_first_of(delims, beg + 1);
		std::string cat = the_genre.substr(beg, pos - beg);
		cat[0] = toupper(int(cat[0]));
		genre.push_back(cat);
	}

	if (genre.empty())
		return false;

	if (strcmp(LangCode.c_str(), "EN") != 0)
		TranslateGenres(genre);

	return true;
}

void GameTDB::TranslateGenres(std::vector<std::string> &GenreList)
{
	char *data = LoadGameNode("gnrmap");
	if (!data)
		return;

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result result = xmlDoc.load_string(data);
	if (!result)
		return;

	pugi::xml_node genres = xmlDoc.child("genres");
	const char *tag;

	if (genres.child("genre"))
		tag = "genre";
	else
		tag = "maingenre";

	for (unsigned int n = 0; n < GenreList.size(); ++n)
	{
		std::string genre = GenreList[n];
		std::string trans;
		for (pugi::xml_node maingenre : genres.children(tag))
		{
			if (strcasecmp(genre.c_str(), maingenre.attribute("name").value()) == 0)
				trans = maingenre.find_child_by_attribute("loc", "lang", LangCode.c_str()).text().as_string();
			else
				for (pugi::xml_node subgenre : maingenre.children("subgenre"))
				{
					if (strcasecmp(genre.c_str(), subgenre.attribute("name").value()) == 0)
						trans = subgenre.find_child_by_attribute("loc", "lang", LangCode.c_str()).text().as_string();
				}
		}

		if (!trans.empty())
		{
			trans[0] = toupper((int)trans[0]);
			GenreList[n] = trans;
		}
	}

	return;
}

int GameTDB::GetRating(const char *id)
{
	int rating = -1;

	if (!ParseGameNode(id))
		return rating;

	const char *rating_text = xmlDoc.child("game").child("rating").attribute("type").value();
	if (!rating_text)
		return rating;

	if (strncmp(rating_text, "CERO", 4) == 0)
		rating = 0;

	else if (strncmp(rating_text, "ESRB", 4) == 0)
		rating = 1;

	else if (strncmp(rating_text, "PEGI", 4) == 0)
		rating = 2;

	return rating;
}

bool GameTDB::GetRatingValue(const char *id, std::string &rating_value)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	rating_value = xmlDoc.child("game").child("rating").attribute("value").value();

	if (rating_value.empty())
		return false;

	return true;
}

int GameTDB::GetRatingDescriptorList(const char *id, std::vector<std::string> &desc_list)
{
	if (!id)
		return -1;

	if (!ParseGameNode(id))
		return -1;

	desc_list.clear();

	for (pugi::xml_node descriptor : xmlDoc.child("game").child("rating").children("descriptor"))
		desc_list.push_back(descriptor.text().as_string());

//	if(strcmp(LangCode.c_str(), "EN") != 0)
//		TranslateDescriptors(desc_list);

	return desc_list.size();
}

/*
void GameTDB::TranslateDescriptors(std::vector<std::string> &DescList)
{
	char *data = LoadGameNode("dscmap");
	if (!data)
		return;

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result result = xmlDoc.load_string(data);
	if (!result)
		return;

	pugi::xml_node descriptors = xmlDoc.child("descriptors");
	const char *tag;

	if (descriptors.child("descriptor"))
		tag = "descriptor";
	else
		tag = "descr";

	for (unsigned int n = 0; n < DescList.size(); ++n)
	{
		std::string desc = DescList[n];
		std::string trans;
		for (pugi::xml_node descr : descriptors.children(tag))
		{
			if (strcasecmp(desc.c_str(), descr.attribute("name").value()) == 0)
				trans = descr.find_child_by_attribute("loc", "lang", LangCode.c_str()).text().as_string();
		}

		if (!trans.empty())
		{
			DescList[n] = trans;
		}
	}

	return;
}
*/

int GameTDB::GetWifiPlayers(const char *id)
{
	if (!id)
		return -1;

	if (!ParseGameNode(id))
		return -1;

	return xmlDoc.child("game").child("wi-fi").attribute("players").as_int();
	;
}

int GameTDB::GetWifiFeatureList(const char *id, std::vector<std::string> &feat_list)
{
	if (!id)
		return -1;

	if (!ParseGameNode(id))
		return -1;

	feat_list.clear();

	for (pugi::xml_node feature : xmlDoc.child("game").child("wi-fi").children("feature"))
	{
		std::string entry(feature.text().as_string());
		// These don't need to be listed as we show icons
		if (entry.compare("nintendods") == 0 || entry.compare("online") == 0)
			continue;
		feat_list.push_back(entry);
	}

	return feat_list.size();
}

int GameTDB::GetPlayers(const char *id)
{
	if (!id)
		return -1;

	if (!ParseGameNode(id))
		return -1;

	return xmlDoc.child("game").child("input").attribute("players").as_int();
}

int GameTDB::GetAccessoryList(const char *id, std::vector<Accessory> &acc_list)
{
	if (!id)
		return -1;

	if (!ParseGameNode(id))
		return -1;

	acc_list.clear();

	for (pugi::xml_node control : xmlDoc.child("game").child("input").children("control"))
	{
		Accessory accr;
		accr.Name = control.attribute("type").value();
		accr.Required = control.attribute("required").as_bool();
		acc_list.push_back(accr);
	}

	return acc_list.size();
}

int GameTDB::GetCaseColor(const char *id)
{
	if (!id)
		return -1;

	if (!ParseGameNode(id))
		return -1;

	return xmlDoc.child("game").child("case").attribute("color").as_int();
}

bool GameTDB::GetGameType(const char *id, std::string &GameType)
{
	if (!id)
		return false;

	if (!ParseGameNode(id))
		return false;

	GameType = xmlDoc.child("game").child_value("type");

	if (GameType.empty())
		return false;

	return true;
}
