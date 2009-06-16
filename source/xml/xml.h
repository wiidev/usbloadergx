
#ifndef _XML_H_
#define _XML_H_


#ifdef __cplusplus
extern "C"
{
#endif



struct gameXMLinfo
{
	char id[8];
	char version[50];
	char region[10];
	char title[100];
	char synopsis[2000];
	char title_EN[100];
	char synopsis_EN[2000];
	char locales[15][50];
	char developer[75];
	char publisher[75];
	char publisherfromid[75];
	char year[10];
	char month[10];
	char day[10];
	char genre[40];
	char genresplit[10][15];
	char ratingtype[6];
	char ratingvalue[6];
	char ratingdescriptors[20][15];
	char ratingvalueCERO[6];
	char ratingvalueESRB[6];
	char ratingvaluePEGI[6];
	char wifiplayers[4];
	char wififeatures[10][15];
	char players[4];
	char accessories[10][15];
	char accessories_required[10][15];
	char iso_crc[10];
	char iso_md5[50];
	char iso_sha1[50];
	int descriptorCnt;
	int accessoryCnt;
	int accessoryReqCnt;
	int wifiCnt;
	
} ;

struct gameXMLinfo gameinfo;
struct gameXMLinfo gameinfo_reset;

bool OpenXMLFile(char* filename);
bool LoadGameInfoFromXML(char* gameid, char* langcode);
void LoadTitlesFromXML(char *langcode, bool forcejptoen);
void GetPublisherFromGameid(char *idtxt, char *dest);
char *ConvertLangTextToCode(char *langtext);
void ConvertRating(char *ratingvalue, char *fromrating, char *torating, char *destvalue);
void PrintGameInfo(bool showfullinfo);
void FreeXMLMemory();

void title_set(char *id, char *title);
char* trimcopy(char *dest, char *src, int size);

#ifdef __cplusplus
}
#endif

#endif

