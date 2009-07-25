#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogcsys.h>

#include "utils.h"
#include "../settings/cfg.h"
#include "fatmounter.h"

#define MAX_TITLES 256


s32 Title_GetList(u64 **outbuf, u32 *outlen)
{
	u64 *titles = NULL;

	u32 len, nb_titles;
	s32 ret;

	/* Get number of titles */
	ret = ES_GetNumTitles(&nb_titles);
	if (ret < 0)
		return ret;

	/* Calculate buffer lenght */
	len = round_up(sizeof(u64) * nb_titles, 32);

	/* Allocate memory */
	titles = memalign(32, len);
	if (!titles)
		return -1;

	/* Get titles */
	ret = ES_GetTitles(titles, nb_titles);
	if (ret < 0)
		goto err;

	/* Set values */
	*outbuf = titles;
	*outlen = nb_titles;

	return 0;

err:
	/* Free memory */
	if (titles)
		free(titles);

	return ret;
}

s32 Title_GetTicketViews(u64 tid, tikview **outbuf, u32 *outlen)
{
	tikview *views = NULL;

	u32 nb_views;
	s32 ret;

	/* Get number of ticket views */
	ret = ES_GetNumTicketViews(tid, &nb_views);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	views = (tikview *)memalign(32, sizeof(tikview) * nb_views);
	if (!views)
		return -1;

	/* Get ticket views */
	ret = ES_GetTicketViews(tid, views, nb_views);
	if (ret < 0)
		goto err;

	/* Set values */
	*outbuf = views;
	*outlen = nb_views;

	return 0;

err:
	/* Free memory */
	if (views)
		free(views);

	return ret;
}

s32 Title_GetTMD(u64 tid, signed_blob **outbuf, u32 *outlen)
{
	void *p_tmd = NULL;

	u32 len;
	s32 ret;

	/* Get TMD size */
	ret = ES_GetStoredTMDSize(tid, &len);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	p_tmd = memalign(32, round_up(len, 32));
	if (!p_tmd)
		return -1;

	/* Read TMD */
	ret = ES_GetStoredTMD(tid, p_tmd, len);
	if (ret < 0)
		goto err;

	/* Set values */
	*outbuf = p_tmd;
	*outlen = len;

	return 0;

err:
	/* Free memory */
	if (p_tmd)
		free(p_tmd);

	return ret;
}

s32 Title_GetVersion(u64 tid, u16 *outbuf)
{
	signed_blob *p_tmd = NULL;
	tmd      *tmd_data = NULL;

	u32 len;
	s32 ret;

	/* Get title TMD */
	ret = Title_GetTMD(tid, &p_tmd, &len);
	if (ret < 0)
		return ret;

	/* Retrieve TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	/* Set values */
	*outbuf = tmd_data->title_version;

	/* Free memory */
	free(p_tmd);

	return 0;
}

s32 Title_GetSysVersion(u64 tid, u64 *outbuf)
{
	signed_blob *p_tmd = NULL;
	tmd      *tmd_data = NULL;

	u32 len;
	s32 ret;

	/* Get title TMD */
	ret = Title_GetTMD(tid, &p_tmd, &len);
	if (ret < 0)
		return ret;

	/* Retrieve TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	/* Set values */
	*outbuf = tmd_data->sys_version;

	/* Free memory */
	free(p_tmd);

	return 0;
}

s32 Title_GetSize(u64 tid, u32 *outbuf)
{
	signed_blob *p_tmd = NULL;
	tmd      *tmd_data = NULL;

	u32 cnt, len, size = 0;
	s32 ret;

	/* Get title TMD */
	ret = Title_GetTMD(tid, &p_tmd, &len);
	if (ret < 0)
		return ret;

	/* Retrieve TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	/* Calculate title size */
	for (cnt = 0; cnt < tmd_data->num_contents; cnt++) {
		tmd_content *content = &tmd_data->contents[cnt];

		/* Add content size */
		size += content->size;
	}

	/* Set values */
	*outbuf = size;

	/* Free memory */
	free(p_tmd);

	return 0;
}

s32 Title_GetIOSVersions(u8 **outbuf, u32 *outlen)
{
	u8  *buffer = NULL;
	u64 *list   = NULL;

	u32 count, cnt, idx;
	s32 ret;

	/* Get title list */
	ret = Title_GetList(&list, &count);
	if (ret < 0)
		return ret;

	/* Count IOS */
	for (cnt = idx = 0; idx < count; idx++) {
		u32 tidh = (list[idx] >> 32);
		u32 tidl = (list[idx] &  0xFFFFFFFF);

		/* Title is IOS */
		if ((tidh == 0x1) && (tidl >= 3) && (tidl <= 255))
			cnt++;
	}

	/* Allocate memory */
	buffer = (u8 *)memalign(32, cnt);
	if (!buffer) {
		ret = -1;
		goto out;
	}

	/* Copy IOS */
	for (cnt = idx = 0; idx < count; idx++) {
		u32 tidh = (list[idx] >> 32);
		u32 tidl = (list[idx] &  0xFFFFFFFF);

		/* Title is IOS */
		if ((tidh == 0x1) && (tidl >= 3) && (tidl <= 255))
			buffer[cnt++] = (u8)(tidl & 0xFF);
	}

	/* Set values */
	*outbuf = buffer;
	*outlen = cnt;

	goto out;

out:
	/* Free memory */
	if (list)
		free(list);

	return ret;
}


/*-------------------------------------------------------------
 taken from anytitledeleter
 name.c -- functions for determining the name of a title
 
 Copyright (C) 2009 MrClick
 
-------------------------------------------------------------*/
// Max number of entries in the database
#define MAX_DB 		1024

// Max name length
#define MAX_LINE	80

// Contains all title ids (e.g.: "HAC")
static char **__db_i;
// Contains all title names (e.g.: "Mii Channel")
static char **__db;
// Contains the number of entries in the database
static u32 __db_cnt = 0;

s32 loadDatabase(){
	FILE *fdb;
	
	char dbfile[100];
	snprintf(dbfile,sizeof(dbfile),"SD:/database.txt");
	// Init SD card access, open database file and check if it worked
	//fatInitDefault();
	SDCard_Init();
	fdb = fopen(dbfile, "r");
	if (fdb == NULL)
		return -1;
	
	// Allocate memory for the database
	__db_i = calloc(MAX_DB, sizeof(char*));
	__db = calloc(MAX_DB, sizeof(char*));
	
	// Define the line buffer. Each line in the db file will be stored here first
	char line[MAX_LINE];
	line[sizeof(line)] = 0;
	
	// Generic char buffer and counter variable
	char byte;
	u32 i = 0;
	
	// Read each character from the file
	do {
		byte = fgetc(fdb);
		// In case a line was longer than MAX_LINE
		if (i == -1){
			// Read bytes till a new line is hit
			if (byte == 0x0A)
				i = 0;
		// In case were still good with the line length
		} else {
			// Add the new byte to the line buffer
			line[i] = byte;
			i++;
			// When a new line is hit or MAX_LINE is reached
			if (byte == 0x0A || i == sizeof(line) - 1) {
				// Terminate finished line to create a string
				line[i] = 0;
				// When the line is not a comment or not to short
				if (line[0] != '#' && i > 5){
					
					// Allocate and copy title id to database
					__db_i[__db_cnt] = calloc(4, sizeof(char*));
					memcpy(__db_i[__db_cnt], line, 3);
					__db_i[__db_cnt][3] = 0;
					// Allocate and copy title name to database
					__db[__db_cnt] = calloc(i - 4, sizeof(char*));
					memcpy(__db[__db_cnt], line + 4, i - 4);
					__db[__db_cnt][i - 5] = 0;
					
					// Check that the next line does not overflow the database
					__db_cnt++;
					if (__db_cnt == MAX_DB)
						break;
				}
				// Set routine to ignore all bytes in the line when MAX_LINE is reached
				if (byte == 0x0A) i = 0; else i = -1;
			}
		}	
	} while (!feof(fdb));	
	
	// Close database file; we are done with it
	fclose(fdb);
	
	return 0;
}


void freeDatabase(){
	u32 i = 0;
	for(; i < __db_cnt; i++){
		free(__db_i[i]);
		free(__db[i]);
	}
	free(__db_i);
	free(__db);
}


s32 getDatabaseCount(){
	return __db_cnt;
}

s32 __convertWiiString(char *str, u8 *data, u32 cnt){
	u32 i = 0;
	for(; i < cnt; data += 2){
		u16 *chr = (u16*)data;
		if (*chr == 0)
			break;
		// ignores all but ASCII characters
		else if (*chr >= 0x20 && *chr <= 0x7E)
			str[i] = *chr;
		else
			str[i] = '.';
		i++;
	}
	str[i] = 0;
		
	return 0;
}

s32 getNameDB(char* name, char* id){
	// Return fixed values for special entries
	if (strncmp(id, "IOS", 3) == 0){
		sprintf(name, "Operating System %s", id);
		return 0;
	}
	if (strncmp(id, "MIOS", 3) == 0){
		sprintf(name, "Gamecube Compatibility Layer");
		return 0;
	}
	if (strncmp(id, "SYSMENU", 3) == 0){
		sprintf(name, "System Menu");
		return 0;
	}
	if (strncmp(id, "BC", 2) == 0){
		sprintf(name, "BC");
		return 0;
	}
	
	// Create an ? just in case the function aborts prematurely
	sprintf(name, "?");
	
	u32 i;
	u8 db_found = 0;
	// Compare each id in the database to the title id
	for (i = 0; i < __db_cnt; i++)
		if (strncmp(id, __db_i[i], 3) == 0){
			db_found = 1;
			break;
		}
	
	if (db_found == 0)
		// Return -1 if no mathcing entry was found
		return -1;
	else {
		// Get name from database once a matching id was found	
		sprintf(name, __db[i]);
		return 0;
	}
}


s32 getNameBN(char* name, u64 id){
	// Terminate the name string just in case the function exits prematurely
	name[0] = 0;

	// Create a string containing the absolute filename
	char file[256] __attribute__ ((aligned (32)));
	sprintf(file, "/title/%08x/%08x/data/banner.bin", (u32)(id >> 32), (u32)id);
	
	// Bring the Wii into the title's userspace
	if (ES_SetUID(id) < 0){
		// Should that fail repeat after setting permissions to system menu mode
//		Identify_SysMenu();
//		if (ES_SetUID(id) < 0)
//			return -1;
	}
	
	// Try to open file
	s32 fh = ISFS_Open(file, ISFS_OPEN_READ);
	
	// If a title does not have a banner.bin bail out
	if (fh == -106)
		return -2;
	
	// If it fails try to open again after identifying as SU
	if (fh == -102){
//		Identify_SU();
//		fh = ISFS_Open(file, ISFS_OPEN_READ);
	}
	// If the file won't open 
	else if (fh < 0)
		return fh;

	// Seek to 0x20 where the name is stored
	ISFS_Seek(fh, 0x20, 0);

	// Read a chunk of 256 bytes from the banner.bin
	u8 *data = memalign(32, 0x100);
	if (ISFS_Read(fh, data, 0x100) < 0){
		ISFS_Close(fh);
		free(data);
		return -3;
	}
	
	
	// Prepare the strings that will contain the name of the title
	char name1[0x41] __attribute__ ((aligned (32)));
	char name2[0x41] __attribute__ ((aligned (32)));
	name1[0x40] = 0;
	name2[0x40] = 0;

	__convertWiiString(name1, data + 0x00, 0x40);
	__convertWiiString(name2, data + 0x40, 0x40);
	free(data);
	
	// Assemble name
	sprintf(name, "%s", name1);
	if (strlen(name2) > 1)
		sprintf(name, "%s (%s)", name, name2);

	// Close the banner.bin
	ISFS_Close(fh);

	// Job well done
	return 1;
}


s32 getName00(char* name, u64 id){
	// Create a string containing the absolute filename
	char file[256] __attribute__ ((aligned (32)));
	sprintf(file, "/title/%08x/%08x/content/00000000.app", (u32)(id >> 32), (u32)id);
	
	s32 fh = ISFS_Open(file, ISFS_OPEN_READ);
	
	
	
	// If the title does not have 00000000.app bail out
	if (fh == -106)
		return fh;
	
	// In case there is some problem with the permission
	if (fh == -102){
		// Identify as super user
//		Identify_SU();
//		fh = ISFS_Open(file, ISFS_OPEN_READ);
	}
	else if (fh < 0)
		return fh;
	
	// Jump to start of the name entries
	ISFS_Seek(fh, 0x9C, 0);

	// Read a chunk of 0x22 * 0x2B bytes from 00000000.app
	u8 *data = memalign(32, 2048);
	s32 r = ISFS_Read(fh, data, 0x22 * 0x2B);
	//printf("%s %d\n", file, r);wait_anyKey();
	if (r < 0){
		ISFS_Close(fh);
		free(data);
		return -4;
	}

	// Take the entries apart
	char str[0x22][0x2B];
	u8 i = 0;
	// Convert the entries to ASCII strings
	for(; i < 0x22; i++)
		__convertWiiString(str[i], data + (i * 0x2A), 0x2A);
	
	// Clean up
	ISFS_Close(fh);
	free(data);
	
	// Assemble name
	// Only the English name is returned
	// There are 6 other language names in the str array
	sprintf(name, "%s", str[2]);
	if (strlen(str[3]) > 1)
		sprintf(name, "%s (%s)", name, str[3]);
	
	// Job well done
	return 2;
}


s32 printContent(u64 tid){
	char dir[256] __attribute__ ((aligned (32)));
	sprintf(dir, "/title/%08x/%08x/content", (u32)(tid >> 32), (u32)tid);

	u32 num = 64;
	
	static char list[8000] __attribute__((aligned(32)));

	ISFS_ReadDir(dir, list, &num);
	
	char *ptr = list;
	u8 br = 0;
	for (; strlen(ptr) > 0; ptr += strlen(ptr) + 1){
		printf("     %-12.12s", ptr);
		 br++; if (br == 4) { br = 0; printf("\n"); }
	}
	if (br != 0)
		printf("\n");
	
	return num;
}


s32 getTitle_Name(char* name, u64 id, char *tid){
	char buf[256] __attribute__ ((aligned (32)));
				
	s32 r = -1;
	// Determine the title's name database/banner/00000000.app
	r = getNameDB(buf, tid);
				if (r < 0)
					r = getNameBN(buf, id);
					if (r < 0)
						r = getName00(buf, id);

	switch (r){
		// In case a name was found in the database
		case 0:		sprintf(name, "%s", buf);
					break;
		// In case a name was found in the banner.bin
		case 1:		sprintf(name, "*%s*", buf);
					break;
		// In case a name was found in the 00000000.app
		case 2:		sprintf(name, "+%s+", buf);
					break;
		// In case no proper name was found return a ?	
		default: 	sprintf(name, "Unknown Title");
					break;
	}

	return 0;
}

char *titleText(u32 kind, u32 title){
	static char text[10];
	
	if (kind == 1){
		// If we're dealing with System Titles, use custom names
		switch (title){
			case 1:
				strcpy(text, "BOOT2");
			break;
			case 2:
				strcpy(text, "SYSMENU");
			break;
			case 0x100:
				strcpy(text, "BC");
			break;
			case 0x101:
				strcpy(text, "MIOS");
			break;
			default:
				sprintf(text, "IOS%u", title);
			break;
		}
	} else {
		// Otherwise, just convert the title to ASCII
		int i =32, j = 0;
		do {
			u8 temp;
			i -= 8;
			temp = (title >> i) & 0x000000FF;
			if (temp < 32 || temp > 126)
				text[j] = '.';
			else
				text[j] = temp;
			j++;
		} while (i > 0);
		text[4] = 0;
	}
	return text;
}


/*-------------------------------------------------------------
 from any title deleter
titles.c -- functions for grabbing all titles of a certain type
 
Copyright (C) 2008 tona
-------------------------------------------------------------*/

u32 __titles_init = 0;
u32 __num_titles;
static u64 __title_list[MAX_TITLES] ATTRIBUTE_ALIGN(32);

s32 __getTitles() {
	s32 ret;
	ret = ES_GetNumTitles(&__num_titles);
	if (ret <0)
		return ret;
	if (__num_titles > MAX_TITLES)
		return -1;
	ret = ES_GetTitles(__title_list, __num_titles);
	if (ret <0)
		return ret;
	__titles_init = 1;
	return 0;
}

s32 getTitles_TypeCount(u32 type, u32 *count) {
	s32 ret = 0;
	u32 type_count;
	if (!__titles_init)
		ret = __getTitles();
	if (ret <0)
			return ret;
	int i;
	type_count = 0;
	for (i=0; i < __num_titles; i++) {
		u32 upper;
		upper = __title_list[i] >> 32;
		if(upper == type)
			type_count++;
	}
	*count = type_count;
	return ret;
}
	
s32 getTitles_Type(u32 type, u32 *titles, u32 count) {
	s32 ret = 0;
	u32 type_count;
	if (!__titles_init)
		ret = __getTitles();
	if (ret <0)
			return ret;
	int i;
	type_count = 0;
	for (i=0; type_count < count && i < __num_titles; i++) {
		u32 upper, lower;
		upper = __title_list[i] >> 32;
		lower = __title_list[i] & 0xFFFFFFFF;
		if(upper == type) {
			titles[type_count]=lower;
			type_count++;
		}
	}
	if (type_count < count)
		return -2;
	__titles_init = 0;
	return 0;
}


