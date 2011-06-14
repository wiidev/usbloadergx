#ifndef _XML_H_
#define _XML_H_

#include <mxml.h>

// open database, close database, load info for a game
bool OpenXMLDatabase(char* xmlfilepath, char* argdblang, bool argJPtoEN, bool openfile, bool loadtitles, bool keepopen);
void CloseXMLDatabase();

#define XML_ELEMMAX 15

bool OpenXMLFile(char* filename);
void LoadTitlesFromXML(char *langcode, bool forcejptoen);
void GetPublisherFromGameid(char *idtxt, char *dest, int destsize);
const char *ConvertLangTextToCode(char *langtext);
int ConvertRating(const char *ratingvalue, const char *fromrating, const char *torating);
char *MemInfo();
mxml_node_t *GetTextFromNode(mxml_node_t *currentnode, mxml_node_t *topnode, const char *nodename, const char *attributename,
        char *value, int descend, char *dest, int destsize);
char * get_nodetext(mxml_node_t *node, char *buffer, int buflen);

#endif

