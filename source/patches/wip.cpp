#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gecko.h"

#include "settings/CSettings.h"
#include "memory/mem2.h"
#include "wip.h"

static WIP_Code * CodeList = NULL;
static u32 CodesCount = 0;
static u32 ProcessedLength = 0;
static u32 Counter = 0;

extern "C" void do_wip_code(u8 * dst, u32 len)
{
	if (!CodeList) return;

	if (Counter < 3)
	{
		Counter++;
		return;
	}

	u32 i = 0;
	int n = 0;
	int offset = 0;

	for (i = 0; i < CodesCount; i++)
	{
		for (n = 0; n < 4; n++)
		{
			offset = CodeList[i].offset + n - ProcessedLength;

			if (offset < 0 || offset >= (int) len) continue;

			if (dst[offset] == ((u8 *) &CodeList[i].srcaddress)[n])
			{
				dst[offset] = ((u8 *) &CodeList[i].dstaddress)[n];
				gprintf("WIP: %08X Address Patched.\n", CodeList[i].offset + n);
			}
			else
			{
				gprintf("WIP: %08X Address does not match with WIP entrie.\n", CodeList[i].offset + n);
				gprintf("Destination: %02X | Should be: %02X.\n", dst[offset], ((u8 *) &CodeList[i].srcaddress)[n]);
			}
		}
	}
	ProcessedLength += len;
	Counter++;
}

//! for internal patches only
//! .wip files override internal patches
//! the codelist has to be freed if the set fails
//! if set was successful the codelist will be freed when it's done
extern "C" bool set_wip_list(WIP_Code * list, int size)
{
	if (!CodeList && size > 0)
	{
		CodeList = list;
		CodesCount = size;
		return true;
	}

	return false;
}

extern "C" void wip_reset_counter()
{
	ProcessedLength = 0;
	//alternative dols don't need a skip. only main.dol.
	Counter = 3;
}

extern "C" void free_wip()
{
	if (CodeList)
		MEM2_free(CodeList);
	CodeList = NULL;
	CodesCount = 0;
	Counter = 0;
	ProcessedLength = 0;
}

extern "C" int load_wip_code(u8 *gameid)
{
	char filepath[150];
	char GameID[8];
	memset(GameID, 0, sizeof(GameID));
	memcpy(GameID, gameid, 6);
	snprintf(filepath, sizeof(filepath), "%s%s.wip", Settings.WipCodepath, GameID);

	FILE * fp = fopen(filepath, "rb");
	if (!fp)
	{
		memset(GameID, 0, sizeof(GameID));
		memcpy(GameID, gameid, 4);
		snprintf(filepath, sizeof(filepath), "%s%s.wip", Settings.WipCodepath, GameID);
		fp = fopen(filepath, "rb");
	}
	if (!fp)
	{
		memset(GameID, 0, sizeof(GameID));
		memcpy(GameID, gameid, 3);
		snprintf(filepath, sizeof(filepath), "%s%s.wip", Settings.WipCodepath, GameID);
		fp = fopen(filepath, "rb");
	}

	if (!fp) return -1;

	free_wip();

	char line[255];
	gprintf("\nLoading WIP code from %s.\n", filepath);

	while (fgets(line, sizeof(line), fp))
	{
		if (line[0] == '#') continue;
		if (line[0] == ';') continue;
		if (line[0] == ':') continue;

		if (strlen(line) < 26) continue;

		u32 offset = (u32) strtoul(line, NULL, 16);
		u32 srcaddress = (u32) strtoul(line + 9, NULL, 16);
		u32 dstaddress = (u32) strtoul(line + 18, NULL, 16);

		if (!CodeList) CodeList = (WIP_Code *) MEM2_alloc(sizeof(WIP_Code));

		WIP_Code * tmp = (WIP_Code *) MEM2_realloc(CodeList, (CodesCount + 1) * sizeof(WIP_Code));
		if (!tmp)
		{
			fclose(fp);
			free_wip();
			return -1;
		}

		CodeList = tmp;

		CodeList[CodesCount].offset = offset;
		CodeList[CodesCount].srcaddress = srcaddress;
		CodeList[CodesCount].dstaddress = dstaddress;
		CodesCount++;
	}
	fclose(fp);
	gprintf("\n");

	return 0;
}
