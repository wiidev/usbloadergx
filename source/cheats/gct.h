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
#ifndef _GCT_H
#define _GCT_H

#include <string>
#include <vector>

using namespace std;

//!Handles Ocarina TXT Cheatfiles
class GCTCheats
{
	private:
		string sGameID;
		string sGameTitle;
		struct CheatEntry
		{
			string sCheatName;
			string sCheatComment;
			vector<unsigned int> sCheats;
		};
		vector<CheatEntry> cheatList;
	public:
		//!Constructor
		GCTCheats(void);
		//!Destructor
		~GCTCheats(void);
		//!Open txt file with cheats
		//!\param filename name of TXT file
		//!\return error code
		int openTxtfile(const char * filename);
		//!Creates GCT file
		//!\param nr[] array of selected Cheat Numbers
		//!\param cnt size of array
		//!\param filename name of GCT file
		//!\return error code
		int createGCT(const vector<int> &vCheats, const char * filename);
		//!Gets Count cheats
		//!\return Count cheats
		int getCnt() const { return cheatList.size(); }
		//!Gets Game Name
		//!\return Game Name
		string getGameName(void);
		//!Gets GameID
		//!\return GameID
		string getGameID(void);
		//!Gets cheat data
		//!\return cheat data
		vector<unsigned int> getCheat(int nr);
		//!Gets Cheat Name
		//!\return Cheat Name
		string getCheatName(int nr);
		//!Gets Cheat Comment
		//!\return Cheat Comment
		string getCheatComment(int nr);
		//!Clear all loaded cheats
		void Clear(void);
		//!Check if string is a code
		//!\return true/false
		bool IsCode(const char *s);
		//!Check if cheat is included in GCT buffer
		bool IsCheatIncluded(int iCheat, const unsigned char *gctBuf, unsigned int gctSize);
};

#endif  /* _GCT_H */
