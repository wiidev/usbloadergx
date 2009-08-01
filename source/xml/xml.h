
#ifndef _XML_H_
#define _XML_H_

#include <mxml.h>


#ifdef __cplusplus
extern "C" {
#endif

// open database, close database, load info for a game
    bool OpenXMLDatabase(char* xmlfilepath, char* argdblang, bool argJPtoEN, bool openfile, bool loadtitles, bool keepopen);
    void CloseXMLDatabase();
    bool LoadGameInfoFromXML(char* gameid, char* langcode);

#define XML_ELEMMAX 15

    struct gameXMLinfo {
        char    id[7];
        char    version[50];
        char    region[7];
        char    title[200];
        char    synopsis[3000];
        char    title_EN[200];
        char    synopsis_EN[3000];
        char    locales[XML_ELEMMAX+1][3];
        int     localeCnt;
        char    developer[75];
        char    publisher[75];
        char    publisherfromid[75];
        char    year[5];
        char    month[3];
        char    day[3];
        char    genre[75];
        char    genresplit[XML_ELEMMAX+1][20];
        int     genreCnt;
        char    ratingtype[5];
        char    ratingvalue[5];
        char    ratingdescriptors[XML_ELEMMAX+1][40];
        int     descriptorCnt;
        char    ratingvalueCERO[5];
        char    ratingvalueESRB[5];
        char    ratingvaluePEGI[5];
        char    wifiplayers[4];
        char    wififeatures[XML_ELEMMAX+1][20];
        int     wifiCnt;
        char    players[4];
        char    accessories[XML_ELEMMAX+1][20];
        int     accessoryCnt;
        char    accessoriesReq[XML_ELEMMAX+1][20];
        int     accessoryReqCnt;
        char    iso_crc[9];
        char    iso_md5[33];
        char    iso_sha1[41];
    } ;

    bool OpenXMLFile(char* filename);
    void LoadTitlesFromXML(char *langcode, bool forcejptoen);
    void GetPublisherFromGameid(char *idtxt, char *dest, int destsize);
    char *ConvertLangTextToCode(char *langtext);
    void ConvertRating(char *ratingvalue, char *fromrating, char *torating, char *destvalue, int destsize);
    void PrintGameInfo(bool showfullinfo);
    char *MemInfo();
    void GetTextFromNode(mxml_node_t *currentnode, mxml_node_t *topnode, char *nodename,
                         char *attributename, char *value, int descend, char *dest, int destsize);
#ifdef __cplusplus
}
#endif

#endif

