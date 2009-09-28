#include <string.h>

#include "settings/cfg.h"
#include "usbloader/wbfs.h"
#include "main.h"
#include <wctype.h>
#include "getentries.h"
#include <algorithm>
#include <vector>


struct discHdr * gameList=NULL;
s32 gameSelected=0, gameStart=0;
u32		 gameCnt=0;
wchar_t *gameFilter=NULL;
wchar_t *gameFilterNextList=NULL;
wchar_t *gameFilterPrev=NULL;

/****************************************************************************
 * wcsdup based on new wchar_t [...]
 ***************************************************************************/
wchar_t *wcsdup(const wchar_t *src)
{
	int len = wcslen(src)+1;
	wchar_t *dst = new wchar_t[len];
	if(dst) wcscpy(dst, src);
	return dst;
}
inline int wcsnicmp(const wchar_t *s1, const wchar_t *s2, int len)
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
/****************************************************************************
 * Get PrevFilter
 ***************************************************************************/

int __Menu_GetPrevFilter(int t, wchar_t* gameFilter, u32 gameFiltered, wchar_t **PgameFilterPrev)
{

	std::vector<wchar_t *> nameList;

	struct discHdr *buffer = NULL;
	u32 cnt, len, i;
	s32 ret;
	
	wchar_t *new_gameFilterPrev = wcsdup(gameFilter);


	/* Get list length */
	ret = WBFS_GetCount(&cnt);
	if (ret < 0)
		return ret;

	/* Buffer length */
	len = sizeof(struct discHdr) * cnt;

	/* Allocate memory */
	buffer = (struct discHdr *)memalign(32, len);
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

		wchar_t *wname = FreeTypeGX::charToWideChar(get_title(header));
		if(wname) nameList.push_back(wname);
	}

	/* delete buffer */
	if(buffer) free(buffer);

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

/****************************************************************************
 * Get Gamelist
 ***************************************************************************/
int __Menu_GetGameList(int t, wchar_t* gameFilter, discHdr ** PgameList, u32 *PgameCnt) {
    struct discHdr *buffer = NULL;

    u32 cnt, cnt2=0, len;
    s32 ret;

    /* Get list length */
    ret = WBFS_GetCount(&cnt);
    if (ret < 0)
        return ret;

    /* Buffer length */
    len = sizeof(struct discHdr) * cnt;

    /* Allocate memory */
    buffer = (struct discHdr *)memalign(32, len);
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
    for (u32 i = 0; i < cnt; i++) {
		struct discHdr *header = &buffer[i];

		/* Filters */
		if (Settings.fave && t==0) {
			struct Game_NUM* game_num = CFG_get_game_num(header->id);
			if (!game_num || game_num->favorite==0)
				continue;
		}
		
		if (Settings.parentalcontrol && !Settings.godmode && t==0) {
			if (get_block(header) >= Settings.parentalcontrol)
				continue;
		}
		
		if(gameFilter && *gameFilter && t==0) {
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
	if(cnt > cnt2)
	{
		cnt = cnt2;
		buffer = (struct discHdr *)realloc(buffer, sizeof(struct discHdr) * cnt);
	}
	if (!buffer)
		return -1;
	
	if (Settings.sort==pcount) {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmpCount);
	} else {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);
	}

	/* Set values */
	if(PgameList) *PgameList = buffer; else free(buffer);
	if(PgameCnt) *PgameCnt  = cnt;
 
    return 0;
}

int __Menu_GetEntries(int t, const wchar_t* Filter) {

	u32				 new_gameCnt			= 0;
	struct discHdr	*new_gameList			= NULL;
	wchar_t 		*new_gameFilter			= NULL;
	wchar_t			*new_gameFilterNextList	= NULL;
	wchar_t			*new_gameFilterPrev		= NULL;

	new_gameFilter = wcsdup(Filter ? Filter : (gameFilter ? gameFilter : L"") );
	if(new_gameFilter == NULL) return -1;
	for(;;)
	{
		if(__Menu_GetGameList(t, new_gameFilter, &new_gameList, &new_gameCnt) < 0)
			return -1;
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
