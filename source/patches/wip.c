#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "settings/cfg.h"

u32 doltableoffset[64];
u32 doltablelength[64];
u32 doltableentries;

void wipreset()
{
	doltableentries = 0;
}

void wipregisteroffset(u32 dst, u32 len)
{
	doltableoffset[doltableentries] = dst;
	doltablelength[doltableentries] = len;
	doltableentries++;
}

void patchu8(u32 offset, u8 value)
{
	u32 i = 0;
	u32 tempoffset = 0;

	while ((doltablelength[i] <= offset-tempoffset) && (i+1 < doltableentries))
	{
		tempoffset+=doltablelength[i];
		i++;
	}
	if (offset-tempoffset < doltablelength[i])
	{
		*(u8 *)(offset-tempoffset+doltableoffset[i]) = value;
	}
}

void wipparsebuffer(u8 *buffer, u32 length)
// The buffer needs a 0 at the end to properly terminate the string functions
{
	u32 pos = 0;
	u32 offset;
	char buf[10];
	
	while (pos < length)
	{
		if ( *(char *)(buffer + pos) != '#' && *(char *)(buffer + pos) != ';' && *(char *)(buffer + pos) != 10 && *(char *)(buffer + pos) != 13 && *(char *)(buffer + pos) != 32 && *(char *)(buffer + pos) != 0 )
		{
			memcpy(buf, (char *)(buffer + pos), 8);
			buf[8] = 0;
			offset = strtol(buf,NULL,16);

			pos += (u32)strchr((char *)(buffer + pos), 32)-(u32)(buffer + pos) + 1;
			pos += (u32)strchr((char *)(buffer + pos), 32)-(u32)(buffer + pos) + 1;
			
			while (*(char *)(buffer + pos) != 10 && *(char *)(buffer + pos) != 13 && *(char *)(buffer + pos) != 0)
			{
				memcpy(buf, (char *)(buffer + pos), 2);
				buf[2] = 0;
			
				patchu8(offset, strtol(buf,NULL,16));
				offset++;
				pos +=2;		
			}	
		}
		if (strchr((char *)(buffer + pos), 10) == NULL)
		{
			return;
		} else
		{
			pos += (u32)strchr((char *)(buffer + pos), 10)-(u32)(buffer + pos) + 1;
		}
	}
}

u32 do_wip_code(u8 *gameid)
{
	FILE *fp;
	u32 filesize;
	char filepath[150];
	memset(filepath, 0, 150);
	u8 *wipCode;

	sprintf(filepath, "%s%6s", Settings.WipCodepath, gameid);
	filepath[strlen(Settings.WipCodepath)+6] = '.';
	filepath[strlen(Settings.WipCodepath)+7] = 'w';
	filepath[strlen(Settings.WipCodepath)+8] = 'i';
	filepath[strlen(Settings.WipCodepath)+9] = 'p';

	fp = fopen(filepath, "rb");
	if (!fp) {
		memset(filepath, 0, 150);
		sprintf(filepath, "%s%3s", Settings.WipCodepath, gameid + 1);
		filepath[strlen(Settings.WipCodepath)+3] = '.';
		filepath[strlen(Settings.WipCodepath)+4] = 'w';
		filepath[strlen(Settings.WipCodepath)+5] = 'i';
		filepath[strlen(Settings.WipCodepath)+6] = 'p';
		fp = fopen(filepath, "rb");
		
		if (!fp) {
			return -1;
		}
	}

	if (fp) {
		u32 ret = 0;

		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		
		wipCode = malloc(filesize + 1);
		wipCode[filesize] = 0; // Wip code functions need a 0 termination
		
		fseek(fp, 0, SEEK_SET);
		ret = fread(wipCode, 1, filesize, fp);
		fclose(fp);
		
		if (ret == filesize)
		{
			// Apply wip patch
			wipparsebuffer(wipCode, filesize + 1);
			return 0;
		}
	}
	return -2;
}
