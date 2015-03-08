/****************************************************************************
 * Copyright (C) 2009 nIxx
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gct.h"

#define ERRORRANGE "Error: CheatNr out of range"

//Header and Footer
static const char GCT_Header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde };
static const char GCT_Footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

GCTCheats::GCTCheats(void)
{
}

GCTCheats::~GCTCheats(void)
{
}

void GCTCheats::Clear(void)
{
	cheatList.clear();
	sGameID.clear();
	sGameTitle.clear();

}

string GCTCheats::getGameName(void)
{
	return sGameTitle;
}

string GCTCheats::getGameID(void)
{
	return sGameID;
}

vector<unsigned int> GCTCheats::getCheat(int nr)
{
	if((unsigned int)nr >= cheatList.size())
		return vector<unsigned int>();

	return cheatList[nr].sCheats;
}

string GCTCheats::getCheatName(int nr)
{
	if((unsigned int)nr >= cheatList.size())
		return ERRORRANGE;

	return cheatList[nr].sCheatName;
}

string GCTCheats::getCheatComment(int nr)
{
	if((unsigned int)nr >= cheatList.size())
		return ERRORRANGE;

	return cheatList[nr].sCheatComment;
}

int GCTCheats::createGCT(const vector<int> &vCheats, const char * filename)
{
	if (vCheats.size() == 0 || !filename)
		return 0;

	FILE *pFile = fopen(filename, "wb");

	if (!pFile)
		return 0;

	fwrite(GCT_Header, sizeof(GCT_Header), 1, pFile);

	int cnt = vCheats.size();
	int c = 0;
	while (c < cnt)
	{
		if((unsigned int)vCheats[c] >= cheatList.size())
			continue;

		vector<unsigned int> &cheatBuf = cheatList[vCheats[c]].sCheats;
		if(cheatBuf.size() > 0)
			fwrite((char*)&cheatBuf[0], cheatBuf.size() * sizeof(unsigned int), 1, pFile);

		c++;
	}

	fwrite(GCT_Footer, sizeof(GCT_Footer), 1, pFile);
	fclose(pFile);
	return 1;
}

static inline void RemoveLineEnds(char *str)
{
	const char *strPtr = str;
	while(*strPtr != 0)
	{
		if(*strPtr == '\n' || *strPtr == '\r')
		{
			strPtr++;
			continue;
		}

		*str = *strPtr;
		str++;
		strPtr++;
	}
	*str = 0;
}

int GCTCheats::openTxtfile(const char * filename)
{
	//! clear already loaded things
	Clear();

	FILE *pFile = fopen(filename, "rb");
	if (!pFile)
		return 0;

	fseek(pFile, 0, SEEK_END);
	int size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	if (size <= 0) {
		fclose(pFile);
		return -1;
	}

	const int max_line_size = 4096;
	char *line = new (std::nothrow) char[max_line_size];
	if(!line) return -1;

	fgets(line, max_line_size, pFile);
	RemoveLineEnds(line);
	sGameID = line;
	fgets(line, max_line_size, pFile);
	RemoveLineEnds(line);
	sGameTitle = line;

	while (fgets(line, max_line_size, pFile))
	{
		RemoveLineEnds(line);

		if(*line == 0)
			continue;

		// first line is the cheat name
		CheatEntry cheatEntry;
		cheatEntry.sCheatName = line;

		while (fgets(line, max_line_size, pFile))
		{
			RemoveLineEnds(line);

			if(*line == 0)  // empty line means start of new cheat
				break;

			if (IsCode(line))
			{
				// remove any garbage (comment) after code
				line[8] = 0;
				line[17] = 0;

				cheatEntry.sCheats.push_back(strtoul(&line[0], 0, 16));
				cheatEntry.sCheats.push_back(strtoul(&line[9], 0, 16));
			}
			else
			{
				cheatEntry.sCheatComment = line;
			}
		}

		if(!cheatEntry.sCheats.empty())
			cheatList.push_back(cheatEntry);
	}
	fclose(pFile);
	delete [] line;
	return 1;
}

bool GCTCheats::IsCode(const char *str)
{
	if (strlen(str) >= 17 && str[8] == ' ')
	{
		// accept strings longer than 17 in case there is a comment on the same line as the code
		char part1[9];
		char part2[9];
		snprintf(part1, sizeof(part1), "%c%c%c%c%c%c%c%c", str[0], str[1], str[2], str[3], str[4], str[5], str[6],
				str[7]);
		snprintf(part2, sizeof(part2), "%c%c%c%c%c%c%c%c", str[9], str[10], str[11], str[12], str[13], str[14],
				str[15], str[16]);
		if ((strtok(part1, "0123456789ABCDEFabcdef") == NULL) && (strtok(part2, "0123456789ABCDEFabcdef") == NULL))
		{
			return true;
		}
	}
	return false;
}

bool GCTCheats::IsCheatIncluded(int iCheat, const unsigned char *gctBuf, unsigned int gctSize)
{
	if(!gctBuf || (unsigned int)iCheat >= cheatList.size())
		return false;

	vector<unsigned int> &Cheat = cheatList[iCheat].sCheats;
	int len = Cheat.size() * sizeof(unsigned int);

	for(unsigned int i = sizeof(GCT_Header); i + len <= gctSize - sizeof(GCT_Footer); i += 4)
	{
		if(memcmp(&Cheat[0], &gctBuf[i], len) == 0)
			return true;
	}
	return false;
}
