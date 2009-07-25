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
extern "C"
{
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

// Load the database from SD card
s32 loadDatabase();

// Free the database on exit
void freeDatabase();

// Get the number of entries in the database
s32 getDatabaseCount();

// Get the name of a title
s32 getTitle_Name(char *name, u64 id, char *tid);

// Get the name of a title from the database located on the SD card
s32 getNameDB(char *name, char* id);

// Get the name of a title from its banner.bin in NAND
s32 getNameBN(char *name, u64 id);

// Get the name of a title from its 00000000.app in NAND
s32 getName00(char *name, u64 id);

// Get string representation of lower title id
char *titleText(u32 kind, u32 title);

// Converts a 16 bit Wii string to a printable 8 bit string
s32 __convertWiiString(char *str, u8 *data, u32 cnt);

// Get the number of titles on the Wii of a given type
s32 getTitles_TypeCount(u32 type, u32 *count);

// Get the list of titles of this type
s32 getTitles_Type(u32 type, u32 *titles, u32 count);

#ifdef __cplusplus
}
#endif

#endif
