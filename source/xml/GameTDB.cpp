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

#define NAME_OFFSET_DB  "wiitdb_offsets.bin"
#define MAXREADSIZE	 1024*1024   // Cache size only for parsing the offsets: 1MB

typedef struct _ReplaceStruct
{
	const char * orig;
	char replace;
	short size;
} ReplaceStruct;

//! More replacements can be added if needed
static const ReplaceStruct Replacements[] =
{
	{ "&gt;", '>', 4 },
	{ "&lt;", '<', 4 },
	{ "&quot;", '\"', 6 },
	{ "&apos;", '\'', 6 },
	{ "&amp;", '&', 5 },
	{ NULL, '\0', 0 }
};

GameTDB::GameTDB()
	: file(0), LangCode("EN"), GameNodeCache(0)
{
}

GameTDB::GameTDB(const char * filepath)
	: file(0), LangCode("EN"), GameNodeCache(0)
{
	OpenFile(filepath);
}

GameTDB::~GameTDB()
{
	CloseFile();
}

bool GameTDB::OpenFile(const char * filepath)
{
	if(!filepath)
		return false;

	file = fopen(filepath, "rb");
	if(file)
	{
		int pos;
		string OffsetsPath = filepath;
		if((pos = OffsetsPath.find_last_of('/')) != (int) string::npos)
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
	vector<GameOffsets>().swap(OffsetMap);

	if(GameNodeCache)
		delete [] GameNodeCache;
	GameNodeCache = NULL;

	if(file)
		fclose(file);
	file = NULL;
}

bool GameTDB::LoadGameOffsets(const char * path)
{
	if(!path)
		return false;

	string OffsetDBPath = path;
	if(strlen(path) > 0 && path[strlen(path)-1] != '/')
		OffsetDBPath += '/';
	OffsetDBPath += NAME_OFFSET_DB;

	FILE * fp = fopen(OffsetDBPath.c_str(), "rb");
	if(!fp)
	{
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	unsigned long long ExistingVersion = GetGameTDBVersion();
	unsigned long long Version = 0;
	unsigned int NodeCount = 0;

	fread(&Version, 1, sizeof(Version), fp);

	if(ExistingVersion != Version)
	{
		fclose(fp);
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	fread(&NodeCount, 1, sizeof(NodeCount), fp);

	if(NodeCount == 0)
	{
		fclose(fp);
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	OffsetMap.resize(NodeCount);

	if((int) fread(&OffsetMap[0], 1, NodeCount*sizeof(GameOffsets), fp) < 0)
	{
		fclose(fp);
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());

		return result;
	}

	fclose(fp);

	return true;
}

bool GameTDB::SaveGameOffsets(const char * path)
{
	if(OffsetMap.size() == 0 || !path)
		return false;

	FILE * fp = fopen(path, "wb");
	if(!fp)
		return false;

	unsigned long long ExistingVersion = GetGameTDBVersion();
	unsigned int NodeCount = OffsetMap.size();

	if(fwrite(&ExistingVersion, 1, sizeof(ExistingVersion), fp) != sizeof(ExistingVersion))
	{
		fclose(fp);
		return false;
	}

	if(fwrite(&NodeCount, 1, sizeof(NodeCount), fp) != sizeof(NodeCount))
	{
		fclose(fp);
		return false;
	}

	if(fwrite(&OffsetMap[0], 1, NodeCount*sizeof(GameOffsets), fp) != NodeCount*sizeof(GameOffsets))
	{
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}

unsigned long long GameTDB::GetGameTDBVersion()
{
	if(!file)
		return 0;

	char TmpText[1024];

	if(GetData(TmpText, 0, sizeof(TmpText)) < 0)
		return 0;

	char * VersionText = GetNodeText(TmpText, "<WiiTDB version=\"", "/>");
	if(!VersionText)
		return 0;

	return strtoull(VersionText, NULL, 10);
}

int GameTDB::GetData(char * data, int offset, int size)
{
	if(!file || !data)
		return -1;

	fseek(file, offset, SEEK_SET);

	return fread(data, 1, size, file);
}

char * GameTDB::LoadGameNode(const char * id)
{
	unsigned int read = 0;

	GameOffsets * offset = this->GetGameOffset(id);
	if(!offset)
		return NULL;

	char * data = new (std::nothrow) char[offset->nodesize+1];
	if(!data)
		return NULL;

	if((read = GetData(data, offset->gamenode, offset->nodesize)) != offset->nodesize)
	{
		delete [] data;
		return NULL;
	}

	data[read] = '\0';

	return data;
}

char * GameTDB::GetGameNode(const char * id)
{
	char * data = NULL;

	if(GameNodeCache != 0 && strncmp(id, GameIDCache, strlen(GameIDCache)) == 0)
	{
		data = new (std::nothrow) char[strlen(GameNodeCache)+1];
		if(data)
			strcpy(data, GameNodeCache);
	}
	else
	{
		if(GameNodeCache)
			delete [] GameNodeCache;

		GameNodeCache = LoadGameNode(id);

		if(GameNodeCache)
		{
			snprintf(GameIDCache, sizeof(GameIDCache), id);
			data = new (std::nothrow) char[strlen(GameNodeCache)+1];
			if(data)
				strcpy(data, GameNodeCache);
		}
	}

	return data;
}

GameOffsets * GameTDB::GetGameOffset(const char * gameID)
{
	for(unsigned int i = 0; i < OffsetMap.size(); ++i)
	{
		if(strncmp(gameID, OffsetMap[i].gameID, strlen(OffsetMap[i].gameID)) == 0)
			return &OffsetMap[i];
	}

	return 0;
}

static inline char * CleanText(char * in_text)
{
	if(!in_text)
		return NULL;

	const char * ptr = in_text;
	char * text = in_text;

	while(*ptr != '\0')
	{
		for(int i = 0; Replacements[i].orig != 0; ++i)
		{
			if(strncmp(ptr, Replacements[i].orig, Replacements[i].size) == 0)
			{
				ptr += Replacements[i].size;
				*text = Replacements[i].replace;
				++text;
				i = 0;
				continue;
			}
		}

		if(*ptr == '\r')
		{
			++ptr;
			continue;
		}

		*text = *ptr;
		++ptr;
		++text;
	}

	*text = '\0';

	return in_text;
}

char * GameTDB::GetNodeText(char * data, const char * nodestart, const char * nodeend)
{
	if(!data || !nodestart || !nodeend)
		return NULL;

	char * position = strstr(data, nodestart);
	if(!position)
		return NULL;

	position += strlen(nodestart);

	char * end = strstr(position, nodeend);
	if(!end)
		return NULL;

	*end = '\0';

	return CleanText(position);
}

char * GameTDB::SeekLang(char * text, const char * langcode)
{
	if(!text || !langcode) return NULL;

	char * ptr = text;
	while((ptr = strstr(ptr, "<locale lang=")) != NULL)
	{
		ptr += strlen("<locale lang=\"");

		if(strncmp(ptr, langcode, strlen(langcode)) == 0)
		{
			//! Cut off all the other languages
			char * end = strstr(ptr, "</locale>");
			if(!end)
				return NULL;

			end += strlen("</locale>");
			*end = '\0';

			return ptr;
		}
	}

	return NULL;
}

bool GameTDB::ParseFile()
{
	OffsetMap.clear();

	if(!file)
		return false;

	char * Line = new (std::nothrow) char[MAXREADSIZE+1];
	if(!Line)
		return false;

	bool readnew = false;
	int i, currentPos = 0;
	int read = 0;
	const char * gameNode = NULL;
	const char * idNode = NULL;
	const char * gameEndNode = NULL;
	const char * genreNode = NULL;
	const char * descriptNode = NULL;

	while((read = GetData(Line, currentPos, MAXREADSIZE)) > 0)
	{
		gameNode = Line;
		readnew = false;

		//! Ensure the null termination at the end
		Line[read] = '\0';

		//! Try to find genre translation map
		if(!genreNode && (genreNode = strstr(gameNode, "<genres>")) != NULL)
		{
			const char *genreNodeEnd = strstr(genreNode, "</genres>");
			if(genreNodeEnd)
			{
				int size = OffsetMap.size();
				OffsetMap.resize(size+1);
				strcpy(OffsetMap[size].gameID, "gnrmap");
				OffsetMap[size].gamenode = currentPos+(genreNode-Line);
				OffsetMap[size].nodesize = (genreNodeEnd-genreNode);
			}
		}

		//! Try to find description translation map
		if(!descriptNode && (descriptNode = strstr(gameNode, "<descriptors>")) != NULL)
		{
			const char *descriptNodeEnd = strstr(descriptNode, "</descriptors>");
			if(descriptNodeEnd)
			{
				int size = OffsetMap.size();
				OffsetMap.resize(size+1);
				strcpy(OffsetMap[size].gameID, "dscmap");
				OffsetMap[size].gamenode = currentPos+(descriptNode-Line);
				OffsetMap[size].nodesize = (descriptNodeEnd-descriptNode);
			}
		}

		while((gameNode = strstr(gameNode, "<game name=\"")) != NULL)
		{
			idNode = strstr(gameNode, "<id>");
			gameEndNode = strstr(gameNode, "</game>");
			if(!idNode || !gameEndNode)
			{
				//! We are in the middle of the game node, reread complete node and more
				currentPos += (gameNode-Line);
				fseek(file, currentPos, SEEK_SET);
				readnew = true;
				break;
			}

			idNode += strlen("<id>");
			gameEndNode += strlen("</game>");

			int size = OffsetMap.size();
			OffsetMap.resize(size+1);

			for(i = 0; i < 7 && *idNode != '<'; ++i, ++idNode)
				OffsetMap[size].gameID[i] = *idNode;
			OffsetMap[size].gameID[i] = '\0';
			OffsetMap[size].gamenode = currentPos+(gameNode-Line);
			OffsetMap[size].nodesize = (gameEndNode-gameNode);
			gameNode = gameEndNode;
		}

		if(readnew)
			continue;

		currentPos += read;
	}

	delete [] Line;

	return true;
}

bool GameTDB::GetTitle(const char * id, string & title)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * language = SeekLang(data, LangCode.c_str());
	if(!language)
	{
		language = SeekLang(data, "EN");
		if(!language)
		{
			delete [] data;
			return false;
		}
	}

	char * the_title = GetNodeText(language, "<title>", "</title>");
	if(!the_title)
	{
		delete [] data;
		return false;
	}

	title = the_title;

	delete [] data;

	return true;
}

bool GameTDB::GetSynopsis(const char * id, string & synopsis)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * language = SeekLang(data, LangCode.c_str());
	if(!language)
	{
		language = SeekLang(data, "EN");
		if(!language)
		{
			delete [] data;
			return false;
		}
	}

	char * the_synopsis = GetNodeText(language, "<synopsis>", "</synopsis>");
	if(!the_synopsis)
	{
		delete [] data;
		return false;
	}

	synopsis = the_synopsis;

	delete [] data;

	return true;
}

bool GameTDB::GetRegion(const char * id, string & region)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * the_region = GetNodeText(data, "<region>", "</region>");
	if(!the_region)
	{
		delete [] data;
		return false;
	}

	region = the_region;

	delete [] data;

	return true;
}

bool GameTDB::GetDeveloper(const char * id, string & dev)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * the_dev = GetNodeText(data, "<developer>", "</developer>");
	if(!the_dev)
	{
		delete [] data;
		return false;
	}

	dev = the_dev;

	delete [] data;

	return true;
}

bool GameTDB::GetPublisher(const char * id, string & pub)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * the_pub = GetNodeText(data, "<publisher>", "</publisher>");
	if(!the_pub)
	{
		delete [] data;
		return false;
	}

	pub = the_pub;

	delete [] data;

	return true;
}

unsigned int GameTDB::GetPublishDate(const char * id)
{
	if(!id)
		return 0;

	char * data = GetGameNode(id);
	if(!data)
		return 0;

	char * year_string = GetNodeText(data, "<date year=\"", "/>");
	if(!year_string)
	{
		delete [] data;
		return 0;
	}

	unsigned int year, day, month;

	year = atoi(year_string);

	char * month_string = strstr(year_string, "month=\"");
	if(!month_string)
	{
		delete [] data;
		return 0;
	}

	month_string += strlen("month=\"");

	month = atoi(month_string);

	char * day_string = strstr(month_string, "day=\"");
	if(!day_string)
	{
		delete [] data;
		return 0;
	}

	day_string += strlen("day=\"");

	day = atoi(day_string);

	delete [] data;

	return ((year & 0xFFFF) << 16 | (month & 0xFF) << 8 | (day & 0xFF));
}

bool GameTDB::GetGenreList(const char * id, vector<string> & genre)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * the_genre = GetNodeText(data, "<genre>", "</genre>");
	if(!the_genre)
	{
		delete [] data;
		return false;
	}

	unsigned int genre_num = 0;
	const char * ptr = the_genre;

	while(*ptr != '\0')
	{
		if(genre_num >= genre.size())
			genre.resize(genre_num+1);

		if(*ptr == ',' || *ptr == '/' || *ptr == ';')
		{
			ptr++;
			while(*ptr == ' ') ptr++;

			while(genre[genre_num].size() > 0 && genre[genre_num][genre[genre_num].size()-1] == ' ')
				genre[genre_num].erase(genre[genre_num].size()-1);

			genre_num++;
			continue;
		}

		if(genre[genre_num].size() == 0)
			genre[genre_num].push_back(toupper((int)*ptr));
		else
			genre[genre_num].push_back(*ptr);

		++ptr;
	}

	while(genre.size() > genre_num && genre[genre_num].size() > 0 && genre[genre_num][genre[genre_num].size()-1] == ' ')
		genre[genre_num].erase(genre[genre_num].size()-1);

	delete [] data;

	if(strcmp(LangCode.c_str(), "EN") != 0)
		TranslateGenres(genre);

	return true;
}

void GameTDB::TranslateGenres(vector<string> &GenreList)
{
	char * data = GetGameNode("gnrmap");
	if(!data)
		return;

	for(unsigned int i = 0; i < GenreList.size(); ++i)
	{
		for(unsigned int n = 0; n < 2; n++)
		{
			string nodeStart;

			if(n == 0)
				nodeStart = "<genre name=\"";
			else
				nodeStart = "<subgenre name=\"";

			nodeStart += GenreList[i];

			const char *genreNode = strcasestr(data, nodeStart.c_str());
			if(!genreNode)
				continue;

			genreNode += nodeStart.size();

			const char *genreNodeEnd = strstr(genreNode, "</genre>");
			const char *subNodeStart = strstr(genreNode, "<subgenre name=");
			if(!genreNodeEnd || (subNodeStart && subNodeStart < genreNodeEnd))
				genreNodeEnd = subNodeStart;
			if(!genreNodeEnd)
				continue;

			string localStr = "<locale lang=\"";
			localStr += LangCode;
			localStr += "\">";

			const char *langPtr = strcasestr(genreNode, localStr.c_str());
			if(langPtr && langPtr < genreNodeEnd)
			{
				bool firstLetter = true;
				GenreList[i].clear();
				langPtr += localStr.size();

				while(*langPtr == ' ')
					langPtr++;

				while(*langPtr != 0 && !(langPtr[0] == '<' && langPtr[1] == '/'))
				{
					if(firstLetter)
					{
						GenreList[i].push_back(toupper((int)*langPtr));
						firstLetter = false;
					}
					else
						GenreList[i].push_back(*langPtr);
					langPtr++;
				}

				while(GenreList[i].size() > 0 && GenreList[i][GenreList[i].size()-1] == ' ')
					GenreList[i].erase(GenreList[i].size()-1);
			}
			break;
		}
	}

	delete [] data;
}


const char * GameTDB::RatingToString(int rating)
{
	switch(rating)
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
		if(ret < 7)
			return 0;
		else if(ret < 12)
			return 1;
		else if(ret < 16)
			return 2;
		else if(ret < 18)
			return 3;
		else
			return 4;
	}

	int type = -1;
	int desttype = -1;

	type = StringToRating(from);
	desttype = StringToRating(to);
	if (type == -1 || desttype == -1) return -1;

	/* rating conversion table */
	/* the list is ordered to pick the most likely value first: */
	/* EC and AO are less likely to be used so they are moved down to only be picked up when converting ESRB to PEGI or CERO */
	/* the conversion can never be perfect because ratings can differ between regions for the same game */
	const int table_size = 12;
	char ratingtable[table_size][3][5] =
	{
		{ { "A" }, { "E" }, { "3" } },
		{ { "A" }, { "E" }, { "4" } },
		{ { "A" }, { "E" }, { "6" } },
		{ { "A" }, { "E" }, { "7" } },
		{ { "A" }, { "EC" }, { "3" } },
		{ { "A" }, { "E10+" }, { "7" } },
		{ { "B" }, { "T" }, { "12" } },
		{ { "D" }, { "M" }, { "18" } },
		{ { "D" }, { "M" }, { "16" } },
		{ { "C" }, { "T" }, { "16" } },
		{ { "C" }, { "T" }, { "15" } },
		{ { "Z" }, { "AO" }, { "18" } },
	};

	for (int i = 0; i < table_size; i++)
	{
		if (strcasecmp(ratingtable[i][type], value) == 0)
		{
			int res = atoi(ratingtable[i][desttype]);
			if(res < 7)
				return 0;
			else if(res < 12)
				return 1;
			else if(res < 16)
				return 2;
			else if(res < 18)
				return 3;
			else
				return 4;
		}
	}

	return -1;
}

int GameTDB::GetRating(const char * id)
{
	int rating = -1;

	if(!id)
		return rating;

	char * data = GetGameNode(id);
	if(!data)
		return rating;

	char * rating_text = GetNodeText(data, "<rating type=\"", "/>");
	if(!rating_text)
	{
		delete [] data;
		return rating;
	}

	if(strncmp(rating_text, "CERO", 4) == 0)
		rating = 0;

	else if(strncmp(rating_text, "ESRB", 4) == 0)
		rating = 1;

	else if(strncmp(rating_text, "PEGI", 4) == 0)
		rating = 2;

	delete [] data;

	return rating;
}

bool GameTDB::GetRatingValue(const char * id, string & rating_value)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * rating_text = GetNodeText(data, "<rating type=\"", "/>");
	if(!rating_text)
	{
		delete [] data;
		return false;
	}

	char * value_text = GetNodeText(rating_text, "value=\"", "\"");
	if(!value_text)
	{
		delete [] data;
		return false;
	}

	rating_value = value_text;

	delete [] data;

	return true;
}

int GameTDB::GetRatingDescriptorList(const char * id, vector<string> & desc_list)
{
	if(!id)
		return -1;

	char * data = GetGameNode(id);
	if(!data)
		return -1;

	char * descriptor_text = GetNodeText(data, "<descriptor>", "</rating>");
	if(!descriptor_text)
	{
		delete [] data;
		return -1;
	}

	unsigned int list_num = 0;
	desc_list.clear();

	while(*descriptor_text != '\0')
	{
		if(strncmp(descriptor_text, "</descriptor>", strlen("</descriptor>")) == 0)
		{
			descriptor_text = strstr(descriptor_text, "<descriptor>");
			if(!descriptor_text)
				break;

			descriptor_text += strlen("<descriptor>");
			list_num++;
		}

		if(list_num >= desc_list.size())
			desc_list.resize(list_num+1);

		desc_list[list_num].push_back(*descriptor_text);
		++descriptor_text;
	}

	delete [] data;

	if(strcmp(LangCode.c_str(), "EN") != 0)
		TranslateDescriptors(desc_list);

	return desc_list.size();
}

void GameTDB::TranslateDescriptors(vector<string> &DescList)
{
	char * data = GetGameNode("dscmap");
	if(!data)
		return;

	for(unsigned int i = 0; i < DescList.size(); ++i)
	{
		string nodeStart = "<descriptor name=\"";
		nodeStart += DescList[i];

		const char *genreNode = strcasestr(data, nodeStart.c_str());
		if(!genreNode)
			continue;

		genreNode += nodeStart.size();

		const char *genreNodeEnd = strstr(genreNode, "</descriptor>");
		if(!genreNodeEnd)
			continue;

		string localStr = "<locale lang=\"";
		localStr += LangCode;
		localStr += "\">";

		const char *langPtr = strcasestr(genreNode, localStr.c_str());
		if(langPtr && langPtr < genreNodeEnd)
		{
			bool firstLetter = true;
			DescList[i].clear();
			langPtr += localStr.size();

			while(*langPtr == ' ')
				langPtr++;

			while(*langPtr != 0 && !(langPtr[0] == '<' && langPtr[1] == '/'))
			{
				if(firstLetter)
				{
					DescList[i].push_back(toupper((int)*langPtr));
					firstLetter = false;
				}
				else
					DescList[i].push_back(*langPtr);
				langPtr++;
			}

			while(DescList[i].size() > 0 && DescList[i][DescList[i].size()-1] == ' ')
				DescList[i].erase(DescList[i].size()-1);
		}
	}

	delete [] data;
}

int GameTDB::GetWifiPlayers(const char * id)
{
	int players = -1;

	if(!id)
		return players;

	char * data = GetGameNode(id);
	if(!data)
		return players;

	char * PlayersNode = GetNodeText(data, "<wi-fi players=\"", "\">");
	if(!PlayersNode)
	{
		delete [] data;
		return players;
	}

	players = atoi(PlayersNode);

	delete [] data;

	return players;
}

int GameTDB::GetWifiFeatureList(const char * id, vector<string> & feat_list)
{
	if(!id)
		return -1;

	char * data = GetGameNode(id);
	if(!data)
		return -1;

	char * feature_text = GetNodeText(data, "<feature>", "</wi-fi>");
	if(!feature_text)
	{
		delete [] data;
		return -1;
	}

	unsigned int list_num = 0;
	feat_list.clear();

	while(*feature_text != '\0')
	{
		if(strncmp(feature_text, "</feature>", strlen("</feature>")) == 0)
		{
			feature_text = strstr(feature_text, "<feature>");
			if(!feature_text)
				break;

			feature_text += strlen("<feature>");
			list_num++;
		}

		if(list_num >= feat_list.size())
			feat_list.resize(list_num+1);


		if(feat_list[list_num].size() == 0)
			feat_list[list_num].push_back(toupper((int)*feature_text));
		else
			feat_list[list_num].push_back(*feature_text);

		++feature_text;
	}

	delete [] data;

	return feat_list.size();
}

int GameTDB::GetPlayers(const char * id)
{
	int players = -1;

	if(!id)
		return players;

	char * data = GetGameNode(id);
	if(!data)
		return players;

	char * PlayersNode = GetNodeText(data, "<input players=\"", "\">");
	if(!PlayersNode)
	{
		delete [] data;
		return players;
	}

	players = atoi(PlayersNode);

	delete [] data;

	return players;
}

int GameTDB::GetAccessoirList(const char * id, vector<Accessoir> & acc_list)
{
	if(!id)
		return -1;

	char * data = GetGameNode(id);
	if(!data)
		return -1;

	char * ControlsNode = GetNodeText(data, "<control type=\"", "</input>");
	if(!ControlsNode)
	{
		delete [] data;
		return -1;
	}

	unsigned int list_num = 0;
	acc_list.clear();

	while(ControlsNode && *ControlsNode != '\0')
	{
		if(list_num >= acc_list.size())
			acc_list.resize(list_num+1);

		for(const char * ptr = ControlsNode; *ptr != '"' && *ptr != '\0'; ptr++)
			acc_list[list_num].Name.push_back(*ptr);

		acc_list[list_num].Required = false;

		char * requiredField = strstr(ControlsNode, "required=\"");
		if(requiredField && strncmp(requiredField + strlen("required=\""), "true", 4) == 0)
		{
			acc_list[list_num].Required = true;
		}

		ControlsNode = strstr(ControlsNode, "<control type=\"");
		if(ControlsNode)
			ControlsNode += strlen("<control type=\"");

		list_num++;
	}

	delete [] data;

	return acc_list.size();
}

int GameTDB::GetCaseColor(const char * id)
{
	int color = -1;

	if(!id)
		return color;

	char * data = GetGameNode(id);
	if(!data)
		return color;

	char * ColorNode = GetNodeText(data, "<case color=\"", "\"/>");
	if(!ColorNode)
	{
		delete [] data;
		return color;
	}

	color = strtoul(ColorNode, NULL, 16);

	delete [] data;

	return color;
}

bool GameTDB::GetGameType(const char * id, string &GameType)
{
	if(!id)
		return false;

	char * data = GetGameNode(id);
	if(!data)
		return false;

	char * TypeNode = GetNodeText(data, "<type>", "</type>");
	if(!TypeNode)
	{
		delete [] data;
		return false;
	}

	GameType = TypeNode;

	delete [] data;

	return true;
}

bool GameTDB::GetGameXMLInfo(const char * id, GameXMLInfo * gameInfo)
{
	if(!id || !gameInfo)
		return false;

	for(int i = 0; i < 6 && id[i] != 0; ++i)
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
	//GetRatingDescriptorList(id, gameInfo->RatingDescriptorList); We don't use it yet
	gameInfo->WifiPlayers = GetWifiPlayers(id);
	GetWifiFeatureList(id, gameInfo->WifiFeatureList);
	gameInfo->Players = GetPlayers(id);
	GetAccessoirList(id, gameInfo->AccessoirList);
	gameInfo->CaseColor = GetCaseColor(id);

	return true;
}
