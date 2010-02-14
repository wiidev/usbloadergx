/****************************************************************************
 * PromptWindows
 * USB Loader GX 2009
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _GETENTRIES_H_
#define _GETENTRIES_H_

#include <wctype.h>

struct discHdr;
extern discHdr *gameList;
extern u32 gameCnt;
extern wchar_t *gameFilter;
extern wchar_t *gameFilterNextList;
extern wchar_t *gameFilterPrev;

//! param t
//! make this 1 if you want the function to ignore the rules
//! (settings and parental control) when making the game list.
//!
//! param Filter
//! set this Parameter to Filter the List
//! if this Parameter=NULL then the filter is not changed
//! if this Parameter="" then no filter is activ
//!
int __Menu_GetEntries(int t=0, const wchar_t* Filter=NULL);
s32 __Menu_EntryCmpCount(const void *a, const void *b);
s32 __Menu_EntryCmp(const void *a, const void *b);
void ResetGamelist();

#endif
