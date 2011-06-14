/*
 Load game information from XML - Lustar
 - Mini-XML by Michael Sweet
 - MiniZip adapted by Tantric
 */

#include <malloc.h>
#include <zip/unzip.h>
#include "settings/CSettings.h"
#include "settings/CGameSettings.h"
#include "settings/GameTitles.h"
#include "xml/xml.h"

extern char game_partition[6];

/* config */
static char xmlcfg_filename[100] = "wiitdb";
static int xmlmaxsize = 1572864;

static char langlist[11][22] = { { "Console Default" }, { "Japanese" }, { "English" }, { "German" }, { "French" }, {
        "Spanish" }, { "Italian" }, { "Dutch" }, { "S. Chinese" }, { "T. Chinese" }, { "Korean" } };

static char langcodes[11][5] = { { "" }, { "JA" }, { "EN" }, { "DE" }, { "FR" }, { "ES" }, { "IT" }, { "NL" },
        { "ZHCN" }, // People's Republic of China
        { "ZHTW" }, // Taiwan
        { "KO" } };

static char element_text[5000];
static mxml_node_t *nodetree = NULL;
static mxml_node_t *nodedata = NULL;
static mxml_node_t *nodeid = NULL;
static mxml_node_t *nodeidtmp = NULL;
static mxml_node_t *nodefound = NULL;
static mxml_index_t *nodeindex = NULL;
static mxml_index_t *nodeindextmp = NULL;
int xmlloadtime = 0;
char * get_nodetext(mxml_node_t *node, char *buffer, int buflen);
bool xml_loaded = false;

/* load renamed titles from proper names and game info XML, needs to be after cfg_load_games */
bool OpenXMLDatabase(char* xmlfilepath, char* argdblang, bool argJPtoEN, bool openfile, bool loadtitles, bool keepopen)
{
    if (!xml_loaded)
    {
        bool opensuccess = false;
        char pathname[200];
        snprintf(pathname, sizeof(pathname), "%s", xmlfilepath);
        if (xmlfilepath[strlen(xmlfilepath) - 1] != '/') snprintf(pathname, sizeof(pathname), "%s/", pathname);
        snprintf(pathname, sizeof(pathname), "%s%s_%s.zip", pathname, xmlcfg_filename, game_partition);
        if (openfile) opensuccess = OpenXMLFile(pathname);
        if (!opensuccess)
        {
            snprintf(pathname, sizeof(pathname), "%s", xmlfilepath);
            if (xmlfilepath[strlen(xmlfilepath) - 1] != '/') snprintf(pathname, sizeof(pathname), "%s/", pathname);
            snprintf(pathname, sizeof(pathname), "%swiitdb.zip", pathname);
            if (openfile) opensuccess = OpenXMLFile(pathname);
        }
        if (!opensuccess && openfile)
        {
            CloseXMLDatabase();
            return false;
        }
        if (loadtitles) LoadTitlesFromXML(argdblang, argJPtoEN);
        if (!keepopen) CloseXMLDatabase();
    }
    else
    {
        if (loadtitles) LoadTitlesFromXML(argdblang, argJPtoEN);
        if (!keepopen) CloseXMLDatabase();
    }
    return true;
}

void CloseXMLDatabase()
{
    /* free memory */
    if (xml_loaded)
    {
        mxmlDelete(nodedata);
        mxmlDelete(nodetree);
        xml_loaded = false;
    }
}

mxml_node_t *GetTextFromNode(mxml_node_t *currentnode, mxml_node_t *topnode, const char *nodename, const char *attributename,
        char *value, int descend, char *dest, int destsize)
{
    *element_text = 0;
    nodefound = mxmlFindElement(currentnode, topnode, nodename, attributename, value, descend);
    if (nodefound != NULL)
    {
        if (attributename != NULL)
        {
            strlcpy(dest, mxmlElementGetAttr(nodefound, attributename), destsize);
        }
        else
        {
            get_nodetext(nodefound, element_text, sizeof(element_text));
            strlcpy(dest, element_text, destsize);
        }
    }
    else
    {
        strcpy(dest, "");
    }

    return nodefound;
}

bool OpenXMLFile(char *filename)
{
    if (xml_loaded) return false;

    nodedata = NULL;
    nodetree = NULL;
    nodeid = NULL;
    nodeidtmp = NULL;
    nodefound = NULL;

    char* strresult = strstr(filename, ".zip");
    if (strresult == NULL)
    {
        /* Load XML file */
        FILE *filexml;
        filexml = fopen(filename, "rb");
        if (!filexml) return false;

        nodetree = mxmlLoadFile(NULL, filexml, MXML_OPAQUE_CALLBACK);
        fclose(filexml);

    }
    else
    {
        /* load zipped XML file */
        unzFile unzfile = unzOpen(filename);
        if (unzfile == NULL) return false;
        unzOpenCurrentFile(unzfile);

        unz_file_info zipfileinfo;
        unzGetCurrentFileInfo(unzfile, &zipfileinfo, NULL, 0, NULL, 0, NULL, 0);
        int zipfilebuffersize = zipfileinfo.uncompressed_size;
        if (zipfilebuffersize >= xmlmaxsize)
        {
            unzCloseCurrentFile(unzfile);
            unzClose(unzfile);
            return false;
        }

        char * zipfilebuffer = (char *) malloc(zipfilebuffersize);
        memset(zipfilebuffer, 0, zipfilebuffersize);
        if (zipfilebuffer == NULL)
        {
            unzCloseCurrentFile(unzfile);
            unzClose(unzfile);
            return false;
        }

        unzReadCurrentFile(unzfile, zipfilebuffer, zipfilebuffersize);
        unzCloseCurrentFile(unzfile);
        unzClose(unzfile);

        nodetree = mxmlLoadString(NULL, zipfilebuffer, MXML_OPAQUE_CALLBACK);
        free(zipfilebuffer);
    }

    if (nodetree == NULL) return false;

    nodedata = mxmlFindElement(nodetree, nodetree, "datafile", NULL, NULL, MXML_DESCEND);
    if (nodedata == NULL)
    {
        return false;
    }
    else
    {
        //if (xmldebug) xmlloadtime = dbg_time2(NULL);
        xml_loaded = true;
        return true;
    }
}

char *GetLangSettingFromGame(char *gameid)
{
    int langcode;
    GameCFG *game_cfg = GameSettings.GetGameCFG((u8*) gameid);
    if (game_cfg)
    {
        langcode = game_cfg->language;
    }
    else
    {
        //langcode = CFG.language; // for Configurable Loader
        langcode = Settings.language; // for Loader GX
    }
    char *langtxt = langlist[langcode];
    return langtxt;
}

/* convert language text into ISO 639 two-letter language code (+ZHTW/ZHCN) */
const char *ConvertLangTextToCode(char *languagetxt)
{
    // do not convert if languagetext seems to be a language code (can be 2 or 4 letters)
    if (strlen(languagetxt) <= 4) return languagetxt;
    int i;
    for (i = 0; i <= 10; i++)
    {
        if (!strcasecmp(languagetxt, langlist[i])) // case insensitive comparison
        return langcodes[i];
    }
    return "";
}

char ConvertRatingToIndex(char *ratingtext)
{
    int type = -1;
    if (!strcmp(ratingtext, "CERO"))
    {
        type = 0;
    }
    if (!strcmp(ratingtext, "ESRB"))
    {
        type = 1;
    }
    if (!strcmp(ratingtext, "PEGI"))
    {
        type = 2;
    }
    return type;
}

int ConvertRating(const char *ratingvalue, const char *fromrating, const char *torating)
{
    if (!strcmp(fromrating, torating))
    {
        int ret = atoi(ratingvalue);
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

    type = ConvertRatingToIndex((char *) fromrating);
    desttype = ConvertRatingToIndex((char *) torating);
    if (type == -1 || desttype == -1) return -1;

    /* rating conversion table */
    /* the list is ordered to pick the most likely value first: */
    /* EC and AO are less likely to be used so they are moved down to only be picked up when converting ESRB to PEGI or CERO */
    /* the conversion can never be perfect because ratings can differ between regions for the same game */
    char ratingtable[12][3][5] =
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

    int i;
    for (i = 0; i <= 11; i++)
    {
        if (!strcmp(ratingtable[i][type], ratingvalue))
        {
            int ret = atoi(ratingtable[i][desttype]);
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
    }

    return -1;
}

void LoadTitlesFromXML(char *langtxt, bool forcejptoen)
/* langtxt: set to "English","French","German", to force language for all titles, or "" to load title depending on each game's setting */
/* forcejptoen: set to true to load English title instead of Japanese title when game is set to Japanese */
{
    if (nodedata == NULL) return;

    bool forcelang = false;
    if (strcmp(langtxt, "")) forcelang = true;

    char langcode[10] = "";
    if (forcelang) strcpy(langcode, ConvertLangTextToCode(langtxt)); /* convert language text into ISO 639 two-letter language code */

    /* create index of <id> elements */
    nodeindex = mxmlIndexNew(nodedata, "id", NULL);
    nodeid = mxmlIndexReset(nodeindex);
    *element_text = 0;
    char id_text[10];
    char title_text[200] = "";
    char title_text_EN[200] = "";

    /* search index of id elements, load all id/titles text */
    while (nodeid != NULL)
    {
        nodeid = mxmlIndexFind(nodeindex, "id", NULL);
        if (nodeid != NULL)
        {
            strcpy(title_text, "");
            strcpy(title_text_EN, "");

            get_nodetext(nodeid, element_text, sizeof(element_text));
            snprintf(id_text, 7, "%s", element_text);

            // if language is not forced, use game language setting from config
            if (!forcelang)
            {
                langtxt = GetLangSettingFromGame(id_text);
                strcpy(langcode, ConvertLangTextToCode(langtxt));
            }

            /* if enabled, force English title for all games set to Japanese */
            if (forcejptoen && (strcmp(langcode, "JA")) == 0) strcpy(langcode, "EN");

            /* load title from nodes */
            nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", "EN", MXML_NO_DESCEND);
            if (nodefound != NULL)
            {
                GetTextFromNode(nodefound, nodedata, "title", NULL, NULL, MXML_DESCEND, title_text_EN,
                        sizeof(title_text_EN));
            }
            nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", langcode, MXML_NO_DESCEND);
            if (nodefound != NULL)
            {
                GetTextFromNode(nodefound, nodedata, "title", NULL, NULL, MXML_DESCEND, title_text, sizeof(title_text));
            }

            /* fall back to English title if prefered language was not found */
            if (!strcmp(title_text, ""))
            {
                strcpy(title_text, title_text_EN);
            }

            snprintf(id_text, 7, "%s", id_text);
            GameTitles.SetGameTitle(id_text, title_text);
        }
    }

    // free memory
    mxmlIndexDelete(nodeindex);

    //if (xmldebug) xmlloadtime = dbg_time2(NULL);
}

void GetPublisherFromGameid(char *idtxt, char *dest, int destsize)
{
    /* guess publisher from company list using last two characters from game id */
    nodeindextmp = mxmlIndexNew(nodedata, "company", NULL);
    nodeidtmp = mxmlIndexReset(nodeindextmp);

    *element_text = 0;
    char publishercode[3];
    sprintf(publishercode, "%c%c", idtxt[4], idtxt[5]);

    while (strcmp(element_text, publishercode) != 0)
    {
        nodeidtmp = mxmlIndexFind(nodeindextmp, "company", NULL);
        if (nodeidtmp != NULL)
        {
            strlcpy(element_text, mxmlElementGetAttr(nodeidtmp, "code"), sizeof(element_text));
        }
        else
        {
            break;
        }
    }
    if (!strcmp(element_text, publishercode))
    {
        strlcpy(dest, mxmlElementGetAttr(nodeidtmp, "name"), destsize);
    }
    else
    {
        strcpy(dest, "");
    }

    // free memory
    mxmlIndexDelete(nodeindextmp);
}

char *MemInfo()
{
    char linebuf[300] = "";
    char memtotal[20];
    char memused[20];
    char memnotinuse[20];
    char memcanbefreed[20];
    struct mallinfo mymallinfo = mallinfo();
    sprintf(memtotal, "%d", mymallinfo.arena / 1024);
    sprintf(memused, "%d", mymallinfo.uordblks / 1024);
    sprintf(memnotinuse, "%d", mymallinfo.fordblks / 1024);
    sprintf(memcanbefreed, "%d", mymallinfo.keepcost / 1024);
    snprintf(linebuf, sizeof(linebuf), "all:%sKB used:%sKB notused:%sKB canfree:%sKB", memtotal, memused, memnotinuse,
            memcanbefreed);
    char *minfo[300];
    *minfo = linebuf;
    return *minfo;
}

/*-------------------------------------------------------------------------------------*/
/* get_nodetext() - Get the text for a node, taken from mini-mxml example mxmldoc.c */
char * get_nodetext(mxml_node_t *node, char *buffer, int buflen) /* O - Text in node, I - Node to get, I - Buffer, I - Size of buffer */
{
    char *ptr, *end; /* Pointer into buffer, End of buffer */
    int len; /* Length of node */
    mxml_node_t *current; /* Current node */
    ptr = buffer;
    end = buffer + buflen - 1;
    for (current = node->child; current && ptr < end; current = current->next)
    {
        if (current->type == MXML_TEXT)
        {
            if (current->value.text.whitespace) *ptr++ = ' ';
            len = (int) strlen(current->value.text.string);
            if (len > (int) (end - ptr)) len = (int) (end - ptr);
            memcpy(ptr, current->value.text.string, len);
            ptr += len;
        }
        else if (current->type == MXML_OPAQUE)
        {
            len = (int) strlen(current->value.opaque);
            if (len > (int) (end - ptr)) len = (int) (end - ptr);
            memcpy(ptr, current->value.opaque, len);
            ptr += len;
        }
    }
    *ptr = '\0';
    return (buffer);
}
