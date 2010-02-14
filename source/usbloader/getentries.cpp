#include <string.h>

#include "settings/cfg.h"
#include "usbloader/wbfs.h"
#include "main.h"
#include <wctype.h>
#include "getentries.h"
#include "settings/newtitles.h"

#include "../prompts/TitleBrowser.h"
#include  "prompts/PromptWindows.h"

#include "wad/wad.h"
#include "xml/xml.h"
#include "../wad/title.h"
#include <algorithm>
#include <vector>
#include <wchar.h>

#include "gecko.h"

#include "listfiles.h"
#define typei 0x00010001


struct discHdr * gameList=NULL;
s32 gameSelected=0, gameStart=0;
u32		 gameCnt=0;
wchar_t *gameFilter=NULL;
wchar_t *gameFilterNextList=NULL;
wchar_t *gameFilterPrev=NULL;

struct discHdr * fullGameList =	NULL;
s32 			 fullGameCnt  =	-1;

extern u8 mountMethod;

/****************************************************************************
 * wcsdup_new based on new wchar_t [...]
 ***************************************************************************/
static wchar_t *wcsdup_new(const wchar_t *src)
{
	int len = wcslen(src)+1;
	wchar_t *dst = new wchar_t[len];
	if(dst) wcscpy(dst, src);
	return dst;
}
static inline int wcsnicmp(const wchar_t *s1, const wchar_t *s2, int len)
{
	if (len <= 0)
		return (0);
	do
    {
		int r = towupper(*s1) - towupper(*s2++);
		if (r) return r;
		if (*s1++ == 0)
			break;
    } while (--len != 0);
	
	return (0);
} 


/****************************************************************************
 * EntryCmp
 ***************************************************************************/
s32 __Menu_EntryCmp(const void *a, const void *b)
{
    struct discHdr *hdr1 = (struct discHdr *)a;
    struct discHdr *hdr2 = (struct discHdr *)b;

    /* Compare strings */
    return stricmp(get_title(hdr1), get_title(hdr2));
}

s32 __Menu_EntryCmpCount(const void *a, const void *b) {
    s32 ret;

    struct discHdr *hdr1 = (struct discHdr *)a;

    struct discHdr *hdr2 = (struct discHdr *)b;

    /* Compare Play Count */
    u16 count1 = 0;
    u16 count2 = 0;
    struct Game_NUM* game_num1 = CFG_get_game_num(hdr1->id);
    struct Game_NUM* game_num2 = CFG_get_game_num(hdr2->id);

    if (game_num1) count1 = game_num1->count;
    if (game_num2) count2 = game_num2->count;

    ret = (s32) (count2-count1);
    if (ret == 0) return stricmp(get_title(hdr1), get_title(hdr2));

    return ret;
}

s32 __Menu_EntryCmpFavorite(const void *a, const void *b) {
    s32 ret;

    struct discHdr *hdr1 = (struct discHdr *)a;

    struct discHdr *hdr2 = (struct discHdr *)b;

    /* Compare Favorite (rank) */
    u16 fav1 = 0;
    u16 fav2 = 0;
    struct Game_NUM* game_num1 = CFG_get_game_num(hdr1->id);
    struct Game_NUM* game_num2 = CFG_get_game_num(hdr2->id);

    if (game_num1) fav1 = game_num1->favorite;
    if (game_num2) fav2 = game_num2->favorite;

    ret = (s32) (fav2-fav1);
    if (ret == 0) return stricmp(get_title(hdr1), get_title(hdr2));

    return ret;
}

void ResetGamelist()
{
	if (fullGameList != NULL)
	{
		fullGameCnt = -1;
		free(fullGameList);
		fullGameList = NULL;
	}
}

int GetFullHeaders(struct discHdr **headers, u32 *count)
{
	if (fullGameList == NULL || fullGameCnt == -1)
	{
		gprintf("Retrieving gamelist from WBFS\n");

		// Retrieve all stuff from WBFS
		u32 cnt;
		
		int ret = WBFS_GetCount(&cnt);
		if (ret < 0)
			return ret;
		
		/* Buffer length */
		u32 len = sizeof(struct discHdr) * cnt;

		/* Allocate memory */
		struct discHdr *buffer = (struct discHdr *)memalign(32, len);
		if (!buffer)
			return -1;

		/* Clear buffer */
		memset(buffer, 0, len);

		/* Get header list */
		ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
		if (ret < 0) {
			if (buffer) free(buffer);
			return ret;
		}
				
		fullGameList = buffer;
		fullGameCnt = cnt;
	}
	else
		gprintf("Retrieving gamelist from cache\n");
	
	*count = fullGameCnt;
	if (headers != NULL)
	{
		*headers = fullGameList;
	}
	
	return 0;
}

/****************************************************************************
 * Get PrevFilter
 ***************************************************************************/
int __Menu_GetPrevFilter(int t, wchar_t* gameFilter, u32 gameFiltered, wchar_t **PgameFilterPrev)
{

	std::vector<wchar_t *> nameList;

	struct discHdr *buffer = NULL; // DO NOT FREE THIS BUFFER, IT'S A REFERENCE
	u32 cnt, len, i;
	
	wchar_t *new_gameFilterPrev = wcsdup_new(gameFilter);

	if (GetFullHeaders(&buffer, &cnt))
	{
		return -1;
	}

	/* Fill nameList */
  	for (i = 0; i < cnt; i++)
	{
		struct discHdr *header = &buffer[i];
		
		/* Filter Favorite */
		if (Settings.fave && t==0)
		{
			struct Game_NUM* game_num = CFG_get_game_num(header->id);
			if (!game_num || game_num->favorite==0)
				continue;
		}
		/* Filter Pparental */
		if (Settings.parentalcontrol && !Settings.godmode && t==0)
			if(get_block(header) >= Settings.parentalcontrol)
				continue;

		/* Other parental control method */
		if (Settings.parentalcontrol == 0 && Settings.godmode == 0 && Settings.parental.enabled == 1)
		{
			// Check game rating in WiiTDB, since the default Wii parental control setting is enabled
			s32 rating = GetRatingForGame((char *) header->id);
		
			if ((rating != -1 && rating > Settings.parental.rating) || 
			    (rating == -1 && get_pegi_block(header) > Settings.parental.rating))
			{
				continue;
			}
		}

		wchar_t *wname = FreeTypeGX::charToWideChar(get_title(header));
		if(wname) nameList.push_back(wname);
	}

	/* Find Prev-Filter */
	len = wcslen(new_gameFilterPrev);
	while(len)
	{
		cnt = 0;
		new_gameFilterPrev[--len] = 0;
		for(std::vector<wchar_t *>::iterator iter = nameList.begin(); iter < nameList.end(); iter++)
		{
			if(!wcsncmp(*iter, new_gameFilterPrev, len))
				cnt++;
		}
		if(cnt > gameFiltered)
			break;
	}
	/* Clean name List */
	for(std::vector<wchar_t *>::iterator iter = nameList.begin(); iter < nameList.end(); iter++)
		delete [] *iter;

	if(PgameFilterPrev)
		*PgameFilterPrev = new_gameFilterPrev;

	return 0;
}
/****************************************************************************
 * Get GameFilter NextList
 ***************************************************************************/
 
int int_cmp(const void *a, const void *b) { return *((u32*)a)-*((u32*)b); }

int __Menu_GetGameFilter_NextList(discHdr *gameList, u32 gameCnt, wchar_t **PgameFilter, wchar_t **PgameFilterNextList)
{
	u32 filter_len = wcslen(*PgameFilter);
	u32 i, lastChar=0;
	bool autofill = filter_len > 0; // autofill only when gameFilter is not empty
	wchar_t *p;
	u32 *nextList = new u32[gameCnt]; if(nextList==NULL) return -1;
	for(i=0; i<gameCnt; i++)
	{
		u32 nextFilterChar = 0x10000;
		wchar_t *gameName = FreeTypeGX::charToWideChar(get_title(&gameList[i]));
		if(gameName == NULL) goto error;

		if(wcslen(gameName) > filter_len)
		{
			if((filter_len == 0 || !wcsnicmp(*PgameFilter, gameName, filter_len)))
				nextFilterChar = towupper(gameName[filter_len]);
		}
		else if(wcslen(gameName) == filter_len)
			autofill = false; // no autofill when gameNameLen == filterLen
			
		nextList[i] = nextFilterChar;
	}
	qsort(nextList, gameCnt, sizeof(u32), int_cmp);
	
	*PgameFilterNextList = new wchar_t[gameCnt+1];
	if(*PgameFilterNextList == NULL) goto error;
	
	
	p = *PgameFilterNextList;
	lastChar = 0;
	for(i=0; i<gameCnt; i++)
	{
		u32 Char = nextList[i];
		if(lastChar != Char)
		{
			if(Char == 0x10000)
				break;
			*p++ = lastChar = Char;
		}
	}
	*p = 0;
	if(wcslen(*PgameFilterNextList) == 1 && autofill) // only one nextChar and autofill is true
	{
		wchar_t *newFilter = new wchar_t[filter_len + 2];
		if(newFilter == NULL) goto error;
		
		wcscpy(newFilter, *PgameFilter);
		wcscat(newFilter, *PgameFilterNextList);
		delete [] *PgameFilter; *PgameFilter = newFilter;
		delete [] *PgameFilterNextList; *PgameFilterNextList = NULL;
		return __Menu_GetGameFilter_NextList(gameList, gameCnt, PgameFilter, PgameFilterNextList);
	}
	
	return 0;
error:
	if(nextList) delete [] nextList;
	if(*PgameFilterNextList) delete [] *PgameFilterNextList;
	return -1;
}

int buildTitleList(int t, wchar_t* gameFilter, discHdr ** PgameList, u32 *PgameCnt){

    struct discHdr *buffer = NULL;
	u32 i = 0;
    u32 cnt, cnt2=0, len;
    u32 num_titles;
    u32 titles[100] ATTRIBUTE_ALIGN(32);
    u32 num_sys_titles;
    u32 sys_titles[10] ATTRIBUTE_ALIGN(32);
    s32 ret = -1;
    FILE *f;
    char path[100];

    ISFS_Initialize();

    ret = getTitles_TypeCount(typei, &num_titles);
    if (ret < 0) {
    	return -1; 
    }

    ret = getTitles_Type(typei, titles, num_titles);
    if (ret < 0) {
    	return -2;
    }

    ret = getTitles_TypeCount(0x00010002, &num_sys_titles);
    if (ret < 0) {
    	return -3;
    }

    ret = getTitles_Type(0x00010002, sys_titles, num_sys_titles);
    if (ret < 0) {
    	return -4;
    }

    cnt = (num_titles+num_sys_titles);
    len = sizeof(struct discHdr) * cnt;
	buffer = (struct discHdr *)memalign(32, len);
    if (!buffer)
        return -1;

    memset(buffer, 0, len);
	
	sprintf(path,"%s/config/database.txt",bootDevice);
    f = fopen(path, "r");

    char name[64];
    while (i < (num_titles+num_sys_titles)) {
        //start from the beginning of the file each loop
        if (f)rewind(f);
        char text[15];
        strcpy(name,"");//make sure name is empty
        u8 found=0;
        
		sprintf(text, "%s", titleText(i<num_titles?typei:0x00010002, i<num_titles?titles[i]:sys_titles[i-num_titles]));

        
		char line[200];
        char tmp[50];
        snprintf(tmp,50," ");
        
		//check if the content.bin is on the SD card for that game
		//if there is content.bin,then the game is on the SDmenu and not the wii
		sprintf(line,"SD:/private/wii/title/%s/content.bin",text);
		if (!checkfile(line))
			{
			
				struct discHdr *header = &buffer[i];
				if (f) {
					while (fgets(line, sizeof(line), f)) {
						if (line[0]== text[0]&&
								line[1]== text[1]&&
								line[2]== text[2]) {
							int j=0;
							found=1;
							for (j=0;(line[j+4]!='\0' || j<51);j++)

								tmp[j]=line[j+4];
							snprintf(header->title,sizeof(header->title),"%s",tmp);
							//break;
						}
					}
				}
				if (!found) {
					if (getName00(header->title, TITLE_ID(i<num_titles?typei:0x00010002, i<num_titles?titles[i]:sys_titles[i-num_titles]),CONF_GetLanguage()*2)>=0)
						found=2;

					if (!found) {
						if (getNameBN(header->title, TITLE_ID(i<num_titles?typei:0x00010002, i<num_titles?titles[i]:sys_titles[i-num_titles]))>=0)
							found=3;

						if (!found)
							snprintf(header->title,sizeof(header->title),"%s (%08x)",text,i<num_titles?titles[i]:sys_titles[i-num_titles]);
					}
				}
				//put the 4th and 8th digit of the title type as the last 2 characters here
				//no reason other than it will let us know how we should boot the title later
				header->id[0]=text[0];
				header->id[1]=text[1];
				header->id[2]=text[2];
				header->id[3]=text[3];
				header->id[4]='1';
				header->id[5]=(i<num_titles?'1':'2');
				//header->
				
//not using these filters right now, but i left them in just in case
		// Filters
		/*if (Settings.fave) {
			struct Game_NUM* game_num = CFG_get_game_num(header->id);
			if (!game_num || game_num->favorite==0)
				continue;
		}
		
		if (Settings.parentalcontrol && !Settings.godmode) {
			if (get_block(header) >= Settings.parentalcontrol)
				continue;
		}*/
		
		if(gameFilter && *gameFilter) {
			u32 filter_len = wcslen(gameFilter);
			wchar_t *gameName = FreeTypeGX::charToWideChar(get_title(header));
			if (!gameName || wcsnicmp(gameName, gameFilter, filter_len)) {
				delete [] gameName;
				continue;
			}
		}
		if(i != cnt2)
			buffer[cnt2] = buffer[i];
		cnt2++;
				}
        i++;
    }
	
	if (f)fclose(f);

    Uninstall_FromTitle(TITLE_ID(1, 0));
    ISFS_Deinitialize();
	
	if(cnt > cnt2)
	{
		cnt = cnt2;
		buffer = (struct discHdr *)realloc(buffer, sizeof(struct discHdr) * cnt);
	}
	if (!buffer)
		return -1;
	
	if (Settings.sort==pcount) {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmpCount);
	} else if (Settings.fave) {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmpFavorite);
	} else {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);
	}
	/*PgameList = buffer;
	buffer = NULL;
	PgameCnt  = cnt;*/
	
	if(PgameList) *PgameList = buffer; else free(buffer);
	if(PgameCnt) *PgameCnt  = cnt;
 
    return 0;
 
    return cnt;
}


/****************************************************************************
 * Get Gamelist
 ***************************************************************************/
int __Menu_GetGameList(int t, wchar_t* gameFilter, discHdr ** PgameList, u32 *PgameCnt) {
    struct discHdr *buffer = NULL;
	struct discHdr *output = NULL;

    u32 cnt, cnt2=0;

	if (GetFullHeaders(&buffer, &cnt))
	{
		return -1;
	}

	int len = cnt * sizeof(struct discHdr);
	output = (struct discHdr *) memalign(32, len);
	memset(output, 0, len);
	
    for (u32 i = 0; i < cnt; i++) {
		struct discHdr *header = &buffer[i];

		/* Register game */
		NewTitles::Instance()->CheckGame(header->id);

		/* Filters */
		if (Settings.fave && t==0) {
			struct Game_NUM* game_num = CFG_get_game_num(header->id);
			if (!game_num || game_num->favorite==0)
				continue;
		}

		//ignore uLoader cfg "iso".  i was told it is "__CFG_"  but not confirmed
		if (header->id[0]=='_'&&header->id[1]=='_'&&
			header->id[2]=='C'&&header->id[3]=='F'&&
			header->id[4]=='G'&&header->id[5]=='_')
			continue;
		
		if (Settings.parentalcontrol && !Settings.godmode && t==0) {
			if (get_block(header) >= Settings.parentalcontrol)
				continue;
		}

		/* Other parental control method */
		if (Settings.parentalcontrol == 0 && Settings.godmode == 0 && Settings.parental.enabled == 1 && t==0)
		{
			// Check game rating in WiiTDB, since the default Wii parental control setting is enabled
			s32 rating = GetRatingForGame((char *) header->id);
			if ((rating != -1 && rating > Settings.parental.rating) || 
			    (rating == -1 && get_pegi_block(header) > Settings.parental.rating))
			{
				continue;
			}
		}
		
		if(gameFilter && *gameFilter && t==0) {
			u32 filter_len = wcslen(gameFilter);
			wchar_t *gameName = FreeTypeGX::charToWideChar(get_title(header));
			if (!gameName || wcsnicmp(gameName, gameFilter, filter_len)) {
				delete [] gameName;
				continue;
			}
		}
		output[cnt2] = buffer[i];
		cnt2++;
	}
	NewTitles::Instance()->Save();

	if(cnt > cnt2)
	{
		cnt = cnt2;
		output = (struct discHdr *)realloc(output, sizeof(struct discHdr) * cnt);
	}
	if (!output)
		return -1;
	
	gprintf("After retrieval, gamecount: %d\n", cnt);
			
	if (Settings.sort==pcount) {
		qsort(output, cnt, sizeof(struct discHdr), __Menu_EntryCmpCount);
	} else if (Settings.fave) {
		qsort(output, cnt, sizeof(struct discHdr), __Menu_EntryCmpFavorite);
	} else {
		qsort(output, cnt, sizeof(struct discHdr), __Menu_EntryCmp);
	}

	/* Set values */
	if(PgameList) *PgameList = output; else free(output);
	if(PgameCnt) *PgameCnt  = cnt;

    return 0;
}

int __Menu_GetEntries(int t, const wchar_t* Filter) {

	/*if (mountMethod==3)
	{	
		return buildTitleList();
	}*/
	
	
	u32				 new_gameCnt			= 0;
	struct discHdr	*new_gameList			= NULL;
	wchar_t 		*new_gameFilter			= NULL;
	wchar_t			*new_gameFilterNextList	= NULL;
	wchar_t			*new_gameFilterPrev		= NULL;

	new_gameFilter = wcsdup_new(Filter ? Filter : (gameFilter ? gameFilter : L"") );
	if(new_gameFilter == NULL) return -1;
	
	for(;;)
	{
		if (mountMethod==3)
		{if(buildTitleList(t, new_gameFilter, &new_gameList, &new_gameCnt) < 0)
			return -1;}
			
		else 
		{if(__Menu_GetGameList(t, new_gameFilter, &new_gameList, &new_gameCnt) < 0)
			return -1;}
			
			
		if(new_gameCnt > 0 || new_gameFilter[0] == 0)
			break;
		new_gameFilter[wcslen(new_gameFilter)-1] = 0;
	}

	/* init GameFilterNextList */
	if(__Menu_GetGameFilter_NextList(new_gameList, new_gameCnt, &new_gameFilter, &new_gameFilterNextList) < 0)
		goto error;
	
	/* init GameFilterPrev */
	if(__Menu_GetPrevFilter(t, new_gameFilter, new_gameCnt, &new_gameFilterPrev) < 0)
		goto error;
	
	/* Set values */
	if(gameList) 			free(gameList);
	if(gameFilter)			delete [] gameFilter;
	if(gameFilterNextList)	delete [] gameFilterNextList;
	if(gameFilterPrev)		delete [] gameFilterPrev;

	gameList			= new_gameList;
	gameCnt				= new_gameCnt;
	gameFilter			= new_gameFilter;
	gameFilterNextList	= new_gameFilterNextList;
	gameFilterPrev		= new_gameFilterPrev;

	/* Reset variables */
	gameSelected = gameStart = 0;
	return 0;
error: // clean up
	if(new_gameList)			free(new_gameList);
	if(new_gameFilter)			delete [] new_gameFilter;
	if(new_gameFilterNextList)	delete [] new_gameFilterNextList;
	if(new_gameFilterPrev)		delete [] new_gameFilterPrev;
	return -1;
}
