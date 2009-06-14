
#ifndef _XML_H_
#define _XML_H_


#ifdef __cplusplus
extern "C"
{
#endif



struct gameXMLinfo
{
	char id[10];
	char version[500];
	char region[10];
	char title[500];
	char synopsis[2000];
	char title_EN[500];
	char synopsis_EN[2000];
	char locales[100][500];
	char developer[500];
	char publisher[500];
	char publisherfromid[500];
	char year[10];
	char month[10];
	char day[10];
	char genre[500];
	char genresplit[100][500];
	char ratingtype[10];
	char ratingvalue[10];
	char ratingdescriptors[100][500];
	char ratingvalueCERO[10];
	char ratingvalueESRB[10];
	char ratingvaluePEGI[10];
	char wifiplayers[10];
	char wififeatures[100][500];
	char players[10];
	char accessories[100][500];
	char accessories_required[100][500];
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

