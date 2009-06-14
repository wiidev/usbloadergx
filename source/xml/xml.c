/*
Load game information from XML - Lustar
 - Mini-XML ported by Beardface
 - MiniZip adapted by Tantric
*/

#include <malloc.h>
#include <mxml.h>
#include "unzip/unzip.h"
#include "settings/cfg.h"
#include "xml/xml.h"
//#include "cfg.h"
//#include "xml.h"


bool xml_loaded = false;
static bool xmldebug = false;
extern struct SSettings Settings; // for loader GX

static char langlist[11][22] =
{{"Console Default"},
{"Japanese"},
{"English"},
{"German"},
{"French"},
{"Spanish"},
{"Italian"},
{"Dutch"},
{"S. Chinese"},
{"T. Chinese"},
{"Korean"}};

static char langcodes[11][22] =
{{""},
{"JA"},
{"EN"},
{"DE"},
{"FR"},
{"ES"},
{"IT"},
{"NL"},
{"ZH"},
{"ZH"},
{"KO"}};

static char element_text[5000];

static mxml_node_t *nodetree=NULL;
static mxml_node_t *nodedata=NULL;
static mxml_node_t *nodeid=NULL;
static mxml_node_t *nodeidtmp=NULL;
static mxml_node_t *nodefound=NULL;
static mxml_index_t *nodeindex=NULL;
static mxml_index_t *nodeindextmp=NULL;


int xmlloadtime = 0;

/* get_text() taken as is from mini-mxml example mxmldoc.c */
/* get_text() - Get the text for a node. */
static char		*get_text(mxml_node_t *node, char *buffer, int buflen);
static char *				/* O - Text in node */
get_text(mxml_node_t *node,		/* I - Node to get */
         char        *buffer,		/* I - Buffer */
	 int         buflen)		/* I - Size of buffer */
{
  char		*ptr,			/* Pointer into buffer */
		*end;			/* End of buffer */
  int		len;			/* Length of node */
  mxml_node_t	*current;		/* Current node */
  ptr = buffer;
  end = buffer + buflen - 1;
  for (current = node->child; current && ptr < end; current = current->next)
  {
    if (current->type == MXML_TEXT) {
      if (current->value.text.whitespace)
        *ptr++ = ' ';
      len = (int)strlen(current->value.text.string);
      if (len > (int)(end - ptr))
        len = (int)(end - ptr);
      memcpy(ptr, current->value.text.string, len);
      ptr += len;
    } else if (current->type == MXML_OPAQUE) {
      len = (int)strlen(current->value.opaque);
      if (len > (int)(end - ptr))
        len = (int)(end - ptr);
      memcpy(ptr, current->value.opaque, len);
      ptr += len;
    }
  }
  *ptr = '\0';
  return (buffer);
}


void GetTextFromNode(mxml_node_t *currentnode, mxml_node_t *topnode, char *nodename, char *attributename, char *value, int descend, char *dest)
{
	*element_text = 0; // reset text
	
    nodefound = mxmlFindElement(currentnode, topnode, nodename, attributename, value, descend);
	if (nodefound != NULL)	{
		if (attributename != NULL) {
			strcpy(dest,mxmlElementGetAttr(nodefound, attributename));
		} else {
			get_text(nodefound, element_text, sizeof(element_text));
			strcpy(dest,element_text);
		}
	} else {
		strcpy(dest,"");
	}
}	


bool OpenXMLFile(char *filename)
{
	//if (xmldebug) dbg_time1();

	nodeid=NULL;
	nodedata=NULL;
	nodetree=NULL;
	nodeidtmp=NULL;
	nodefound=NULL;
	nodeindex=NULL;
	nodeindextmp=NULL;
	
	char* strresult = strstr(filename,".zip");
    if (strresult == NULL) {
		/* Load XML file */
		FILE *filexml;
		filexml = fopen(filename, "rb");
		if (!filexml)
			return false;
		
		nodetree = mxmlLoadFile(NULL, filexml, MXML_NO_CALLBACK);
		fclose(filexml);

	} else {
		/* load zipped XML file */
		unzFile unzfile = unzOpen(filename);
		if (unzfile == NULL)
			return false;
		unzOpenCurrentFile(unzfile);
		
		unz_file_info zipfileinfo;
		unzGetCurrentFileInfo(unzfile, &zipfileinfo, NULL, 0, NULL, 0, NULL, 0);	
		int zipfilebuffersize = zipfileinfo.uncompressed_size;
		char * zipfilebuffer = malloc(zipfilebuffersize);
		memset(zipfilebuffer, 0, zipfilebuffersize);
		if (zipfilebuffer == NULL) {
			unzCloseCurrentFile(unzfile);
			unzClose(unzfile);
			return false;
		}
		
		unzReadCurrentFile(unzfile, zipfilebuffer, zipfilebuffersize);
		unzCloseCurrentFile(unzfile);
		unzClose(unzfile);
		
		nodetree = mxmlLoadString(NULL, zipfilebuffer, MXML_NO_CALLBACK);
		free(zipfilebuffer);
	}

	if (nodetree == NULL)
		return false;
			
    nodedata = mxmlFindElement(nodetree, nodetree, "datafile", NULL, NULL, MXML_DESCEND);
   	if (nodedata == NULL)
	    return false;
	
    /* create index of <id> elements */
    nodeindex = mxmlIndexNew(nodetree,"id", NULL);	
	if (nodedata == NULL) {
	    return false;
	} else {
		//if (xmldebug)	xmlloadtime = dbg_time2(NULL);
		xml_loaded = true;
		return true;
	}
}


void FreeXMLMemory()
{
    /* free memory */
	if (xml_loaded) {
		mxmlIndexDelete(nodeindex);
		mxmlIndexDelete(nodeindextmp);
		mxmlDelete(nodeid);
		mxmlDelete(nodeidtmp);
		mxmlDelete(nodefound);
		mxmlDelete(nodedata);
		mxmlDelete(nodetree);
		xml_loaded = false;
	}
}	


/* convert language text into ISO 639 two-letter language code */
char *ConvertLangTextToCode(char *languagetxt)
{
	int i;
	for (i=0;i<=10;i++)
	{
		if (!strcasecmp(languagetxt,langlist[i])) // case insensitive comparison
			return langcodes[i];
	}
	return "";
}


char ConvertRatingToIndex(char *ratingtext)
{
	int type = -1;
	if (strcmp(ratingtext,"CERO") == 0)	{ type = 0; }
	if (strcmp(ratingtext,"ESRB") == 0)	{ type = 1; }
	if (strcmp(ratingtext,"PEGI") == 0)	{ type = 2; }
	return type;
}


void ConvertRating(char *ratingvalue, char *fromrating, char *torating, char *destvalue)
{
	if(strcmp(fromrating,torating) == 0) {
		strcpy(destvalue,ratingvalue);
		return;
	}

	strcpy(destvalue,"");
	int type = -1;
	int desttype = -1;

	type = ConvertRatingToIndex(fromrating);
	desttype = ConvertRatingToIndex(torating);
	if (type == -1 || desttype == -1)
		return;
	
	/* rating conversion table */
	/* the list is ordered to pick the most likely value first: */
	/* EC and AO are less likely to be used so they are moved down to only be picked up when converting ESRB to PEGI or CERO */
	/* the conversion can never be perfect because ratings can differ between regions for the same game */
	char ratingtable[12][3][4] =
	{
		{{"A"},{"E"},{"3"}},
		{{"A"},{"E"},{"4"}},
		{{"A"},{"E"},{"6"}},
		{{"A"},{"E"},{"7"}},
		{{"A"},{"EC"},{"3"}},
		{{"A"},{"E10+"},{"7"}},
		{{"B"},{"T"},{"12"}},
		{{"D"},{"M"},{"18"}},
		{{"D"},{"M"},{"16"}},
		{{"C"},{"T"},{"16"}},
		{{"C"},{"T"},{"15"}},
		{{"Z"},{"AO"},{"18"}},
	};
	
	int i;
	for (i=0;i<=11;i++)
	{
		if (strcmp(ratingtable[i][type],ratingvalue) == 0) {
			strcpy(destvalue,ratingtable[i][desttype]);
			return;
		}
	}
}


void LoadTitlesFromXML(char *langtxt, bool forcejptoen)
/* langtxt: set to "English","French","German", to force language for all titles, or "" to load title depending on each game's setting */
/* forcejptoen: set to true to load English title instead of Japanese title when game is set to Japanese */
{
	if (nodeindex == NULL || nodedata == NULL)
	    return;
                
	bool forcelang = false;
	if (strcmp(langtxt,""))
		forcelang = true;

	char langcode[100] = "";
	if (forcelang)
		strcpy(langcode,ConvertLangTextToCode(langtxt)); /* convert language text into ISO 639 two-letter language code */
	
    /* reset index before new search */
    nodeid = mxmlIndexReset(nodeindex);
    *element_text = 0;
	char id_text[10];
	char title_text[500] = "";
	char title_text_EN[500] = "";
	/* search index of id elements, load all id/titles text */
    while (nodeid != NULL)
    {
        nodeid = mxmlIndexFind(nodeindex,"id", NULL);
	    if (nodeid != NULL) {	
			strcpy(title_text,"");
			strcpy(title_text_EN,"");
			
			get_text(nodeid, element_text, sizeof(element_text));
			snprintf(id_text, 7, "%s",element_text);
			
			// if language is not forced, use game language setting from config
			if (!forcelang) {
				struct Game_CFG *game_cfg = NULL;
				int opt_lang;
				game_cfg = CFG_get_game_opt((u8*)id_text);
				if (game_cfg) {
					opt_lang = game_cfg->language;
				} else {
					//opt_lang = CFG.language; // for Configurable Loader
					opt_lang = Settings.language; // for Loader GX
				}
                strcpy(langcode,ConvertLangTextToCode(langlist[opt_lang]));
			}
			
			/* if enabled, force English title for all games set to Japanese */
			if (forcejptoen && strcmp(langcode,"JA") == 0)
				strcpy(langcode,"EN");
	
			/* load title from nodes */
			nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", "EN", MXML_NO_DESCEND);
			if (nodefound != NULL){// &&(Settings.titlesOverride==1)){
				GetTextFromNode(nodefound, nodedata, "title", NULL, NULL, MXML_DESCEND, title_text_EN);
			
			}
			nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", langcode, MXML_NO_DESCEND);
			if (nodefound != NULL) {
			
				GetTextFromNode(nodefound, nodedata, "title", NULL, NULL, MXML_DESCEND, title_text);
			}
			/* fall back to English title if prefered language was not found */
			if (strcmp(title_text,"") == 0) {
				strcpy(title_text,title_text_EN);
			}
			
			snprintf(id_text, 5, "%s",id_text);
			//printf("%s %s\n",id_text,title_text);
			title_set(id_text, title_text);
	    }
    }
    //if (xmldebug);
    //xmlloadtime = dbg_time2(NULL);
}


void GetPublisherFromGameid(char *idtxt, char *dest) 
{
	/* guess publisher from company list using last two characters from game id */
	
	nodeindextmp = mxmlIndexNew(nodedata,"company", NULL);
	nodeidtmp = mxmlIndexReset(nodeindextmp);

    *element_text = 0;
	char publishercode[3];
	sprintf(publishercode,"%c%c", idtxt[4],idtxt[5]);

    while (strcmp(element_text,publishercode) != 0)
    {
	    nodeidtmp = mxmlIndexFind(nodeindextmp,"company", NULL);
	    if (nodeidtmp != NULL) {
			strcpy(element_text,mxmlElementGetAttr(nodeidtmp, "code"));
	    } else {
	        break;
	    }
    }
	if (strcmp(element_text,publishercode) == 0) {
		strcpy(dest,mxmlElementGetAttr(nodeidtmp, "name"));
	} else {
		strcpy(dest,"");
	}
}


bool LoadGameInfoFromXML(char* gameid, char* langtxt)
/* gameid: full game id */
/* langcode: "English","French","German" */
{
	bool exist=false;
	if (nodeindex == NULL || nodedata == NULL)
		return exist;
		
	/* convert language text into ISO 639 two-letter language codes */
	char langcode[100] = "";
	strcpy(langcode,ConvertLangTextToCode(langtxt));

	/* reset all game info */
	gameinfo = gameinfo_reset;

    /* reset index before new search */
    nodeid = mxmlIndexReset(nodeindex);
	*element_text = 0;
		
	/* search for game matching gameid */
    while (1)
    {

        nodeid = mxmlIndexFind(nodeindex,"id", NULL);
	    if (nodeid != NULL) {
			get_text(nodeid, element_text, sizeof(element_text));
			if (!strcmp(element_text,gameid)) {
				exist=true;
				break;
			}
	    } else {
			break;
	    }
    }
		
    if (strcmp(element_text,gameid) == 0) {
		/* text from elements */
		strcpy(gameinfo.id,element_text);
		GetTextFromNode(nodeid, nodedata, "region", NULL, NULL, MXML_NO_DESCEND, gameinfo.region);
		GetTextFromNode(nodeid, nodedata, "version", NULL, NULL, MXML_NO_DESCEND, gameinfo.version);
		GetTextFromNode(nodeid, nodedata, "genre", NULL, NULL, MXML_NO_DESCEND, gameinfo.genre);
		GetTextFromNode(nodeid, nodedata, "developer", NULL, NULL, MXML_NO_DESCEND, gameinfo.developer);
		GetTextFromNode(nodeid, nodedata, "publisher", NULL, NULL, MXML_NO_DESCEND, gameinfo.publisher);
		// try to guess publisher from game id in case it is missing
		if (strcmp(gameinfo.publisher,"") == 0) {
			GetPublisherFromGameid(gameid,gameinfo.publisher);
		}
		GetPublisherFromGameid(gameid,gameinfo.publisherfromid);
		
		/* text from attributes */
		GetTextFromNode(nodeid, nodedata, "date", "year", NULL, MXML_NO_DESCEND, gameinfo.year);
		GetTextFromNode(nodeid, nodedata, "date", "month", NULL,MXML_NO_DESCEND, gameinfo.month);
		GetTextFromNode(nodeid, nodedata, "date", "day", NULL, MXML_NO_DESCEND, gameinfo.day);
		GetTextFromNode(nodeid, nodedata, "rating", "type", NULL, MXML_NO_DESCEND, gameinfo.ratingtype);
		GetTextFromNode(nodeid, nodedata, "rating", "value", NULL, MXML_NO_DESCEND, gameinfo.ratingvalue);
		GetTextFromNode(nodeid, nodedata, "rom", "crc", NULL, MXML_NO_DESCEND, gameinfo.iso_crc);
		GetTextFromNode(nodeid, nodedata, "rom", "md5", NULL, MXML_NO_DESCEND, gameinfo.iso_md5);
		GetTextFromNode(nodeid, nodedata, "rom", "sha1", NULL, MXML_NO_DESCEND, gameinfo.iso_sha1);
				
		/* text from child elements */
		nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", "EN", MXML_NO_DESCEND);
		if (nodefound != NULL) {
			GetTextFromNode(nodefound, nodedata, "title", NULL, NULL, MXML_DESCEND, gameinfo.title_EN);
			GetTextFromNode(nodefound, nodedata, "synopsis", NULL, NULL, MXML_DESCEND, gameinfo.synopsis_EN);
		}
		nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", langcode, MXML_NO_DESCEND);
		if (nodefound != NULL) {
			GetTextFromNode(nodefound, nodedata, "title", NULL, NULL, MXML_DESCEND, gameinfo.title);
			GetTextFromNode(nodefound, nodedata, "synopsis", NULL, NULL, MXML_DESCEND, gameinfo.synopsis);
		}
		// fall back to English title and synopsis if prefered language was not found
		if (strcmp(gameinfo.title,"") == 0) {
			strcpy(gameinfo.title,gameinfo.title_EN);
		}
		if (strcmp(gameinfo.synopsis,"") == 0) {
			strcpy(gameinfo.synopsis,gameinfo.synopsis_EN);
		}
		
		/* list locale lang attributes */
		nodefound = mxmlFindElement(nodeid, nodedata, "locale", "lang", NULL, MXML_NO_DESCEND);
		if (nodefound != NULL) {
			int incr = 0;
			while (nodefound != NULL)
			{
				++incr;
				strcpy(gameinfo.locales[incr],mxmlElementGetAttr(nodefound, "lang"));
				nodefound = mxmlWalkNext(nodefound, nodedata, MXML_NO_DESCEND);
				if (nodefound != NULL) {
					nodefound = mxmlFindElement(nodefound, nodedata, "locale", "lang", NULL, MXML_NO_DESCEND);
				}
			}
		}
		
		/* unbounded child elements */
		GetTextFromNode(nodeid, nodedata, "wi-fi", "players", NULL, MXML_NO_DESCEND, gameinfo.wifiplayers);
		nodefound = mxmlFindElement(nodeid, nodedata, "wi-fi", NULL, NULL, MXML_NO_DESCEND);
		if (nodefound != NULL) {
			gameinfo.wifiCnt = 0;
			nodeindextmp = mxmlIndexNew(nodefound,"feature", NULL);
			nodeidtmp = mxmlIndexReset(nodeindextmp);
			
			while (nodeidtmp != NULL)
			{
				nodeidtmp = mxmlIndexFind(nodeindextmp,"feature", NULL);
				if (nodeidtmp != NULL) {
					++gameinfo.wifiCnt;
					GetTextFromNode(nodeidtmp, nodedata, "feature", NULL, NULL, MXML_DESCEND, gameinfo.wififeatures[gameinfo.wifiCnt]);
				}gameinfo.wififeatures[gameinfo.wifiCnt][0] = toupper(gameinfo.wififeatures[gameinfo.wifiCnt][0]);
			}
		}
		
		nodefound = mxmlFindElement(nodeid, nodedata, "rating", NULL, NULL, MXML_NO_DESCEND);
		if (nodefound != NULL) {
			gameinfo.descriptorCnt=0;
			nodeindextmp = mxmlIndexNew(nodefound,"descriptor", NULL);
			nodeidtmp = mxmlIndexReset(nodeindextmp);
			
			while (nodeidtmp != NULL)
			{
				nodeidtmp = mxmlIndexFind(nodeindextmp,"descriptor", NULL);
				if (nodeidtmp != NULL) {
					++gameinfo.descriptorCnt;
					GetTextFromNode(nodeidtmp, nodedata, "descriptor", NULL, NULL, MXML_DESCEND, gameinfo.ratingdescriptors[gameinfo.descriptorCnt]);
				}
			}
		}
		
		GetTextFromNode(nodeid, nodedata, "input", "players", NULL, MXML_NO_DESCEND, gameinfo.players);
		nodefound = mxmlFindElement(nodeid, nodedata, "input", NULL, NULL, MXML_NO_DESCEND);
		if (nodefound != NULL) {
			gameinfo.accessoryCnt=0;
			gameinfo.accessoryReqCnt=0;
	
			nodeindextmp = mxmlIndexNew(nodefound,"control", NULL);
			nodeidtmp = mxmlIndexReset(nodeindextmp);
			
			while (nodeidtmp != NULL)
			{
				nodeidtmp = mxmlIndexFind(nodeindextmp,"control", NULL);
				if (nodeidtmp != NULL) {
					if (strcmp(mxmlElementGetAttr(nodeidtmp, "required"),"true") == 0)	{
						++gameinfo.accessoryReqCnt;
						strcpy(gameinfo.accessories_required[gameinfo.accessoryReqCnt],mxmlElementGetAttr(nodeidtmp, "type"));
					} else {
						++gameinfo.accessoryCnt;
						strcpy(gameinfo.accessories[gameinfo.accessoryCnt],mxmlElementGetAttr(nodeidtmp, "type"));
					}
				}
			}
		}
		
		/* convert rating value */
		ConvertRating(gameinfo.ratingvalue, gameinfo.ratingtype, "CERO",gameinfo.ratingvalueCERO);
		ConvertRating(gameinfo.ratingvalue, gameinfo.ratingtype, "ESRB",gameinfo.ratingvalueESRB);
		ConvertRating(gameinfo.ratingvalue, gameinfo.ratingtype, "PEGI",gameinfo.ratingvaluePEGI);

		//PrintGameInfo();
		
		exist=true;
    } else {
	    /*game not found */
		exist=false;
	}return exist;
}


void PrintGameInfo(bool showfullinfo)
{
	if (showfullinfo) {
	
		//Con_Clear();

		//printf("id: %s version: %s region: %s",gameinfo.id, gameinfo.version, gameinfo.region);
		printf("title: %s\n",gameinfo.title);
		int i;
		printf("locales:");
		for (i=1;strcmp(gameinfo.locales[i],"") != 0;i++)
		{
			printf(" %s",gameinfo.locales[i]);
		}
		printf("\n");
		printf("developer: %s\n",gameinfo.developer);
		printf("publisher: %s\n",gameinfo.publisher);
		printf("publisher from ID: %s\n",gameinfo.publisherfromid);
		printf("year:%s month:%s day:%s\n",gameinfo.year,gameinfo.month,gameinfo.day);
		printf("genre: %s\n",gameinfo.genre);
		printf("rating: %s %s (CERO: %s ESRB: %s PEGI: %s)\n",gameinfo.ratingtype, gameinfo.ratingvalue,
                gameinfo.ratingvalueCERO,gameinfo.ratingvalueESRB,gameinfo.ratingvaluePEGI);
		printf("content descriptor:");
		for (i=1;strcmp(gameinfo.wififeatures[i],"") != 0;i++)
		{
			printf(" %s",gameinfo.ratingdescriptors[i]);
		}
		printf("\n");
		printf("players: %s wi-fi: %s\n",gameinfo.players,gameinfo.wifiplayers);
		printf("wi-fi features:");
		for (i=1;strcmp(gameinfo.wififeatures[i],"") != 0;i++)
			{
				printf(" %s",gameinfo.wififeatures[i]);
			}
		printf("\n");
		printf("accessory required:");
		for (i=1;strcmp(gameinfo.accessories_required[i],"") != 0;i++)
			{
				printf(" %s",gameinfo.accessories_required[i]);
			}
		printf("\n");
		printf("accessory:");
		for (i=1;strcmp(gameinfo.accessories[i],"") != 0;i++)
			{
				printf(" %s",gameinfo.accessories[i]);
			}
		printf("\n");
		//printf("iso_crc: %s iso_md5: %s\n",gameinfo.iso_crc,gameinfo.iso_md5);
		//printf("iso_sha1: %s\n",gameinfo.iso_sha1);
		printf("synopsis: %s\n",gameinfo.synopsis);
		
	} else {
	
		char linebuf[1000] = "";
		
		if (xmldebug) {
			//char xmltime[100];
			//sprintf(xmltime,"%d",xmlloadtime);
			//printf("xml load time: %s\n",xmltime);
            /*
			printf("xml forcelang: %s\n",CFG.db_lang);
			printf("xml url: %s\n",CFG.db_url);
			printf("xml file: %s\n",CFG.db_filename);
			char xmljptoen[100];
			sprintf(xmljptoen,"%d",CFG.db_JPtoEN);
			printf("xml JPtoEN: %s\n",xmljptoen);
			*/
			
			// guidebug
			struct mallinfo mymallinfo = mallinfo();
			char memtotal[100];
			char memused[100];
			char memnotinuse[100];
			char memcanbefreed[100];
			sprintf(memtotal,"%d",mymallinfo.arena/1024);
			sprintf(memused,"%d",mymallinfo.uordblks/1024);
			sprintf(memnotinuse,"%d",mymallinfo.fordblks/1024);
			sprintf(memcanbefreed,"%d",mymallinfo.keepcost/1024);
			printf("allocated:%sKB used:%sKB notused:%sKB canbefreed:%s", memtotal, memused, memnotinuse, memcanbefreed);
		}

		//printf("%s: ",gameidfull);
		//printf("%s\n",gameinfo.title);
		if (strcmp(gameinfo.year,"") != 0)
			snprintf(linebuf, sizeof(linebuf), "%s ", gameinfo.year);
		if (strcmp(gameinfo.publisher,"") != 0)
			snprintf(linebuf, sizeof(linebuf), "%s%s", linebuf, gameinfo.publisher);
		if (strcmp(gameinfo.developer,"") != 0 && strcmp(gameinfo.developer,gameinfo.publisher) != 0)
			snprintf(linebuf, sizeof(linebuf), "%s / %s", linebuf, gameinfo.developer);
		if (strlen(linebuf) >= 100) {
			char buffer[200] = "";
			strncpy(buffer, linebuf,  100);
			strncat(buffer, "...", 3);
			snprintf(linebuf, sizeof(linebuf), "%s", buffer);
		}
		printf("%s\n",linebuf);
		strcpy(linebuf,"");
		
		if (strcmp(gameinfo.ratingvalue,"") != 0) {
			snprintf(linebuf, sizeof(linebuf), "rated %s", gameinfo.ratingvalue);
			if (strcmp(gameinfo.ratingtype,"PEGI") == 0)
				snprintf(linebuf, sizeof(linebuf), "%s+ ", linebuf);
			snprintf(linebuf, sizeof(linebuf), "%s ", linebuf);
		}
		if (strcmp(gameinfo.players,"") != 0) {
			snprintf(linebuf, sizeof(linebuf), "%sfor %s player", linebuf, gameinfo.players);
			if (atoi(gameinfo.players) > 1)
				snprintf(linebuf, sizeof(linebuf), "%ss", linebuf);
			if (atoi(gameinfo.wifiplayers) > 1)
				snprintf(linebuf, sizeof(linebuf), "%s (%s online)", linebuf, gameinfo.wifiplayers);
		}
		printf("%s\n",linebuf);
		strcpy(linebuf,"");
	}
}

