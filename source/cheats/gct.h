/*
 * gct.h
 * Class to handle Ocarina TXT Cheatfiles
 * 
 */

#ifndef _GCT_H
#define _GCT_H

#include <sstream>

#define MAXCHEATS 300

using namespace std;

//!Handles Ocarina TXT Cheatfiles
class GCTCheats {
private:
    string sGameID;
    string sGameTitle;
    string sCheatName[MAXCHEATS];
    string sCheats[MAXCHEATS];
    string sCheatComment[MAXCHEATS];
    int iCntCheats;

public:
    //!Constructor
    GCTCheats(void);
    //!Destructor
    ~GCTCheats(void);
    //!Open txt file with cheats
    //!\param filename name of TXT file
    //!\return error code
    int openTxtfile(const char * filename);
    //!Creates GCT file for one cheat
    //!\param nr selected Cheat Numbers
    //!\param filename name of GCT file
    //!\return error code
    int createGCT(int nr,const char * filename);
    //!Creates GCT file from a buffer
    //!\param chtbuffer buffer that holds the cheat data
    //!\param filename name of GCT file
    //!\return error code
    int createGCT(const char * chtbuffer,const char * filename);
    //!Creates GCT file
    //!\param nr[] array of selected Cheat Numbers
    //!\param cnt size of array
    //!\param filename name of GCT file
    //!\return error code
    int createGCT(int nr[],int cnt,const char * filename);
    //!Gets Count cheats
    //!\return Count cheats
    int getCnt();
    //!Gets Game Name
    //!\return Game Name
    string getGameName(void);
    //!Gets GameID
    //!\return GameID
    string getGameID(void);
    //!Gets cheat data
    //!\return cheat data
    string getCheat(int nr);
    //!Gets Cheat Name
    //!\return Cheat Name
    string getCheatName(int nr);
    //!Gets Cheat Comment
    //!\return Cheat Comment
    string getCheatComment(int nr);
	//!Check if string is a code
    //!\return true/false
	bool IsCode(const std::string& s);
};

#endif  /* _GCT_H */
