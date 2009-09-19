/*-------------------------------------------------------------
 from any title deleter and wad manager 1.4
title.h --

Copyright (C) 2008 tona and/or waninkoko
-------------------------------------------------------------*/
#include <gccore.h>
#include <ogcsys.h>
#include <string.h>
#include <stdio.h>
#include <fat.h>
#include <malloc.h>

// Turn upper and lower into a full title ID
#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
// Get upper or lower half of a title ID
#define TITLE_UPPER(x)		((u32)((x) >> 32))
// Turn upper and lower into a full title ID
#define TITLE_LOWER(x)		((u32)(x))

#define MAX_TITLES 100

#ifndef _TITLE_H_
#define _TITLE_H_

#ifdef __cplusplus
extern "C" {
#endif
    /* Constants */
#define BLOCK_SIZE	1024

    /* Prototypes */
    s32 Title_GetList(u64 **, u32 *);
    s32 Title_GetTicketViews(u64, tikview **, u32 *);
    s32 Title_GetTMD(u64, signed_blob **, u32 *);
    s32 Title_GetVersion(u64, u16 *);
    s32 Title_GetSysVersion(u64, u64 *);
    s32 Title_GetSize(u64, u32 *);
    s32 Title_GetIOSVersions(u8 **, u32 *);

// Get the name of a title from its banner.bin in NAND
    s32 getNameBN(char *name, u64 id);

// Get the name of a title from its 00000000.app in NAND
    s32 getName00(char *name, u64 id, int lang = 2);

// Get string representation of lower title id
    char *titleText(u32 kind, u32 title);

// Converts a 16 bit Wii string to a printable 8 bit string
    s32 __convertWiiString(char *str, u8 *data, u32 cnt);

// Get the number of titles on the Wii of a given type
    s32 getTitles_TypeCount(u32 type, u32 *count);

// Get the list of titles of this type
    s32 getTitles_Type(u32 type, u32 *titles, u32 count);

//returns a name for a title
    char *__getTitleName(u64 titleid, int language);

    s32 Uninstall_FromTitle(const u64 tid);
	
//check for a game save present on nand based on game ID
int CheckForSave(const char *gameID);

#ifdef __cplusplus
}
#endif

#endif
