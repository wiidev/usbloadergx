/*
 * gct.h
 * Class to handle Ocarina TXT Cheatfiles
 * nIxx
 */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include "gct.h"

#define ERRORRANGE "Error: CheatNr out of range"

GCTCheats::GCTCheats(void) {
    iCntCheats = 0;
}

GCTCheats::~GCTCheats(void) {

    string sGameID ="";
    string sGameTitle = "";
    /*string sCheatName[MAXCHEATS];
    string sCheats[MAXCHEATS];
    string sCheatComment[MAXCHEATS];*/
}

int GCTCheats::getCnt() {
    return iCntCheats;
}

string GCTCheats::getGameName(void) {
    return sGameTitle;
}

string GCTCheats::getGameID(void) {
    return sGameID;
}

string GCTCheats::getCheat(int nr) {
    if (nr <= (iCntCheats-1)) {
        return sCheats[nr];
    } else {
        return ERRORRANGE;
    }
}

string GCTCheats::getCheatName(int nr) {
    if (nr <= (iCntCheats-1)) {
        return sCheatName[nr];
    } else {
        return ERRORRANGE;
    }
}

string GCTCheats::getCheatComment(int nr) {
    if (nr <= (iCntCheats-1)) {
        return sCheatComment[nr];
    } else {
        return ERRORRANGE;
    }
}

int GCTCheats::createGCT(int nr,const char * filename) {

	if (nr == 0)
		return 0;

    ofstream filestr;
    filestr.open(filename);

    if (filestr.fail())
        return 0;

    //Header and Footer
    char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
    char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    string buf = getCheat(nr);
    filestr.write(header,sizeof(header));

    int x = 0;
    long int li;
    int len = buf.size();

    while (x < len) {
        string temp = buf.substr(x,2);
        li = strtol(temp.c_str(),NULL,16);
        temp = li;
        filestr.write(temp.c_str(),1);
        x +=2;
    }
    filestr.write(footer,sizeof(footer));

    filestr.close();
    return 1;
}

int GCTCheats::createGCT(const char * chtbuffer,const char * filename) {

    ofstream filestr;
    filestr.open(filename);

    if (filestr.fail())
        return 0;

    //Header and Footer
    char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
    char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    string buf = chtbuffer;
    filestr.write(header,sizeof(header));

    int x = 0;
    long int li;
    int len = buf.size();

    while (x < len) {
        string temp = buf.substr(x,2);
        li = strtol(temp.c_str(),NULL,16);
        temp = li;
        filestr.write(temp.c_str(),1);
        x +=2;
    }

    filestr.write(footer,sizeof(footer));

    filestr.close();

    return 1;
}

int GCTCheats::createGCT(int nr[],int cnt,const char * filename) {

	if (cnt == 0)
		return 0;

    ofstream filestr;
    filestr.open(filename);

    if (filestr.fail())
        return 0;

    //Header and Footer
    char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
    char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    filestr.write(header,sizeof(header));

    int c = 0;
    while (c != cnt) {
        int actnr = nr[c];
        string buf = getCheat(actnr);
        long int li;
        int len = buf.size();
        int x = 0;

        while (x < len) {
            string temp = buf.substr(x,2);
            li = strtol(temp.c_str(),NULL,16);
            temp = li;
            filestr.write(temp.c_str(),1);
            x +=2;
        }
        c++;
    }

    filestr.write(footer,sizeof(footer));
    filestr.close();
    return 1;
}

int GCTCheats::openTxtfile(const char * filename) {
    ifstream filestr;
    int i = 0;
    string str;
    filestr.open(filename);

    if (filestr.fail())
        return 0;

    filestr.seekg(0,ios_base::end);
    int size = filestr.tellg();
    if (size <= 0) return -1;
    filestr.seekg(0,ios_base::beg);

    getline(filestr,sGameID);
	if (sGameID[sGameID.length() - 1] == '\r')
		sGameID.erase(sGameID.length() - 1);
	
    getline(filestr,sGameTitle);
	if (sGameTitle[sGameTitle.length() - 1] == '\r')
		sGameTitle.erase(sGameTitle.length() - 1);
				
    getline(filestr,sCheatName[i]);  // skip first line if file uses CRLF
	if (!sGameTitle[sGameTitle.length() - 1] == '\r')
	   filestr.seekg(0,ios_base::beg);

    while (!filestr.eof()) {
        getline(filestr,sCheatName[i]); // '\n' delimiter by default
		if (sCheatName[i][sCheatName[i].length() - 1] == '\r')
			sCheatName[i].erase(sCheatName[i].length() - 1);

        string cheatdata;
        bool emptyline = false;

        do {
			getline(filestr,str);
			if (str[str.length() - 1] == '\r')
				str.erase(str.length() - 1);
				 
            if (str == "" || str[0] == '\r' || str[0] == '\n') {
                emptyline = true;
                break;
            }

            if (IsCode(str)) {
				// remove any garbage (comment) after code
				while (str.size() > 17) {
					str.erase(str.length() - 1);
				}
			    cheatdata.append(str);
                size_t found=cheatdata.find(' ');
                cheatdata.replace(found,1,"");
			} else {
                //printf("%i",str.size());
                sCheatComment[i] = str;
            }
			if (filestr.eof()) break;
		   
        } while (!emptyline);

        sCheats[i] = cheatdata;
		i++;
		if (i == MAXCHEATS) break;
    }
    iCntCheats = i;
    filestr.close();
    return 1;
}

bool GCTCheats::IsCode(const std::string& str) {
	if (str[8] == ' ' && str.size() >= 17) {
	// accept strings longer than 17 in case there is a comment on the same line as the code
		char part1[9];
		char part2[9];
		snprintf(part1,sizeof(part1),"%c%c%c%c%c%c%c%c",str[0],str[1],str[2],str[3],str[4],str[5],str[6],str[7]);
		snprintf(part1,sizeof(part2),"%c%c%c%c%c%c%c%c",str[9],str[10],str[11],str[12],str[13],str[14],str[15],str[16]);
		if ((strtok(part1,"0123456789ABCDEFabcdef") == NULL) && (strtok(part2,"0123456789ABCDEFabcdef") == NULL)) {
			return true;
		}
	}
	return false;
}
