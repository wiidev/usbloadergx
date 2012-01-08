/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/unistd.h>

#include "codehandler.h"
#include "codehandleronly.h"
#include "codehandlerslota.h"
#include "multidol.h"
#include "defaultgameconfig.h"
#include "usbloader/apploader.h"
#include "patchcode.h"
#include "settings/SettingsEnums.h"
#include "FileOperations/fileops.h"
#include "memory/mem2.h"
#include "memory/memory.h"
#include "gecko.h"

static u8 *codelistend = (u8 *) 0x80003000;
static u8 *codelist = (u8 *) 0x800022A8;

static u8 *code_buf = NULL;
static u32 code_size = 0;

static u32 gameconfsize = 0;
static u32 *gameconf = NULL;
static u32 tempgameconfsize = 0;
static u8 *tempgameconf = NULL;

extern void patchhook(u32 address, u32 len);

extern void multidolhook(u32 address);
extern void langvipatch(u32 address, u32 len, u8 langbyte);
extern void vipatch(u32 address, u32 len);

static const u32 multidolpatch1[2] = {
	0x3C03FFB4,0x28004F43
};

static const u32 healthcheckhook[2] = {
	0x41810010,0x881D007D
};

static const u32 updatecheckhook[3] = {
	0x80650050,0x80850054,0xA0A50058
};

static const u32 multidolpatch2[2] = {
	0x3F608000, 0x807B0018
};

static const u32 recoveryhooks[3] = {
	0xA00100AC,0x5400073E,0x2C00000F
};

static const u32 nocopyflag1[3] = {
	0x540007FF, 0x4182001C, 0x80630068
};

static const u32 nocopyflag2[3] = {
	0x540007FF, 0x41820024, 0x387E12E2
};

// this one is for the GH3 and VC saves
//static const u32 nocopyflag3[5] = {
//	0x2C030000, 0x40820010, 0x88010020, 0x28000002, 0x41820234
//};

static const u32 nocopyflag3[5] = {
	0x2C030000, 0x41820200,0x48000058,0x38610100
};
// this removes the display warning for no copy VC and GH3 saves
static const u32 nocopyflag4[4] = {
	0x80010008, 0x2C000000, 0x4182000C, 0x3BE00001
};

static const u32 nocopyflag5[3] = {
	0x801D0024,0x540007FF,0x41820024
};

static const u32 movedvdpatch[3] = {
	0x2C040000, 0x41820120, 0x3C608109
};

static const u32 regionfreehooks[5] = {
	0x7C600774, 0x2C000001, 0x41820030,0x40800010,0x2C000000
};

static const u32 cIOScode[16] = {
	0x7f06c378, 0x7f25cb78, 0x387e02c0, 0x4cc63182
};

static const u32 cIOSblock[16] = {
	0x2C1800F9, 0x40820008, 0x3B000024
};

static const u32 fwritepatch[8] = {
	0x9421FFD0,0x7C0802A6,0x90010034,0xBF210014,0x7C9B2378,0x7CDC3378,0x7C7A1B78,0x7CB92B78  // bushing fwrite
};

static const u32 vipatchcode[3] = {
0x4182000C,0x4180001C,0x48000018
};

static const u32 viwiihooks[4] = {
	0x7CE33B78,0x38870034,0x38A70038,0x38C7004C
};

static const u32 kpadhooks[4] = {
	0x9A3F005E,0x38AE0080,0x389FFFFC,0x7E0903A6
};

static const u32 kpadoldhooks[6] = {
	0x801D0060, 0x901E0060, 0x801D0064, 0x901E0064, 0x801D0068, 0x901E0068
};

static const u32 joypadhooks[4] = {
	0x3AB50001, 0x3A73000C, 0x2C150004, 0x3B18000C
};

static const u32 gxdrawhooks[4] = {
	0x3CA0CC01, 0x38000061, 0x3C804500, 0x98058000
};

static const u32 gxflushhooks[4] = {
	0x90010014, 0x800305FC, 0x2C000000, 0x41820008
};

static const u32 ossleepthreadhooks[4] = {
	0x90A402E0, 0x806502E4, 0x908502E4, 0x2C030000
};

static const u32 axnextframehooks[4] = {
	0x3800000E, 0x7FE3FB78, 0xB0050000, 0x38800080
};

static const u32 wpadbuttonsdownhooks[4] = {
	0x7D6B4A14, 0x816B0010, 0x7D635B78, 0x4E800020
};

static const u32 wpadbuttonsdown2hooks[4] = {
	0x7D6B4A14, 0x800B0010, 0x7C030378, 0x4E800020
};

static const u32 multidolhooks[4] = {
	0x7C0004AC, 0x4C00012C, 0x7FE903A6, 0x4E800420
};

static const u32 multidolchanhooks[4] = {
	0x4200FFF4, 0x48000004, 0x38800000, 0x4E800020
};

static const u32 langpatch[3] = {
	0x7C600775, 0x40820010, 0x38000000
};

static const u32 oldpatch002[3] = {
	0x2C000000, 0x40820214, 0x3C608000
};

static const u32 newpatch002[3] = {
	0x2C000000, 0x48000214, 0x3C608000
};

static const u32 dczeropatch[4] = {
	0x7C001FEC, 0x38630020, 0x4200FFF8, 0x4E800020
};

//---------------------------------------------------------------------------------
void dogamehooks(u32 hooktype, void *addr, u32 len)
//---------------------------------------------------------------------------------
{
	if(hooktype == 0x00)
		return;

	bool isChannel = (*((char *) 0x80000005) == 0) && (*((char *) 0x80000006) == 0);
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		switch(hooktype)
		{
			default:
			case 0x00:

			break;

			case 0x01:
				if(memcmp(addr_start, viwiihooks, sizeof(viwiihooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;

			case 0x02:

				if(memcmp(addr_start, kpadhooks, sizeof(kpadhooks))==0){
					patchhook((u32)addr_start, len);
				}

				if(memcmp(addr_start, kpadoldhooks, sizeof(kpadoldhooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;

			case 0x03:

				if(memcmp(addr_start, joypadhooks, sizeof(joypadhooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;

			case 0x04:

				if(memcmp(addr_start, gxdrawhooks, sizeof(gxdrawhooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;

			case 0x05:

				if(memcmp(addr_start, gxflushhooks, sizeof(gxflushhooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;

			case 0x06:

				if(memcmp(addr_start, ossleepthreadhooks, sizeof(ossleepthreadhooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;

			case 0x07:

				if(memcmp(addr_start, axnextframehooks, sizeof(axnextframehooks))==0){
					patchhook((u32)addr_start, len);
				}
			break;
			/*
			case 0x08:

				if(memcmp(addr_start, customhook, customhooksize)==0){
					patchhook((u32)addr_start, len);
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0){
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
				}
			break;
			*/
		}

		if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
		{
			multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
		}

		if(isChannel && memcmp(addr_start, multidolchanhooks, sizeof(multidolchanhooks)) == 0)
		{
				*(((u32*)addr_start)+1) = 0x7FE802A6;
				DCFlushRange(((u32*)addr_start)+1, 4);
				ICInvalidateRange(((u32*)addr_start)+1, 4);
				multidolhook((u32)addr_start+sizeof(multidolchanhooks)-4);
		}

		addr_start += 4;
	}
}

//---------------------------------------------------------------------------------
void app_pokevalues()
//---------------------------------------------------------------------------------
{
	u32 i, *codeaddr, *codeaddr2, *addrfound = NULL;

	if (gameconfsize != 0)
	{
		for (i = 0; i < gameconfsize / 4; i++)
		{
			if (*(gameconf + i) == 0)
			{
				if (((u32 *) (*(gameconf + i + 1))) == NULL || *((u32 *) (*(gameconf + i + 1))) == *(gameconf + i + 2))
				{
					*((u32 *) (*(gameconf + i + 3))) = *(gameconf + i + 4);
					DCFlushRange((void *) *(gameconf + i + 3), 4);
				}
				i += 4;
			}
			else
			{
				codeaddr = (u32 *) *(gameconf + i + *(gameconf + i) + 1);
				codeaddr2 = (u32 *) *(gameconf + i + *(gameconf + i) + 2);
				if (codeaddr == 0 && addrfound != NULL)
					codeaddr = addrfound;
				else if (codeaddr == 0 && codeaddr2 != 0)
					codeaddr = (u32 *) ((((u32) codeaddr2) >> 28) << 28);
				else if (codeaddr == 0 && codeaddr2 == 0)
				{
					i += *(gameconf + i) + 4;
					continue;
				}
				if (codeaddr2 == 0) codeaddr2 = codeaddr + *(gameconf + i);
				addrfound = NULL;
				while (codeaddr <= (codeaddr2 - *(gameconf + i)))
				{
					if (memcmp(codeaddr, gameconf + i + 1, (*(gameconf + i)) * 4) == 0)
					{
						*(codeaddr + ((*(gameconf + i + *(gameconf + i) + 3)) / 4)) = *(gameconf + i + *(gameconf + i)
								+ 4);
						if (addrfound == NULL) addrfound = codeaddr;
					}
					codeaddr++;
				}
				i += *(gameconf + i) + 4;
			}
		}
	}
}

//---------------------------------------------------------------------------------
static void app_loadgameconfig()
//---------------------------------------------------------------------------------
{
	if (gameconf == NULL)
	{
		gameconf = (u32*) MEM2_alloc(65536);
		if (gameconf == NULL)
			return;
	}
	const char *discid = (const char *) Disc_ID;
	if(!tempgameconf)
	{
		tempgameconf = (u8 *) defaultgameconfig;
		tempgameconfsize = defaultgameconfig_size;
	}

	u32 ret;
	s32 gameidmatch, maxgameidmatch = -1, maxgameidmatch2 = -1;
	u32 i, numnonascii, parsebufpos;
	u32 codeaddr, codeval, codeaddr2, codeval2, codeoffset;
	u32 temp, tempoffset = 0;
	char parsebuffer[18];

	// Remove non-ASCII characters
	numnonascii = 0;
	for (i = 0; i < tempgameconfsize; i++)
	{
		if (tempgameconf[i] < 9 || tempgameconf[i] > 126)
			numnonascii++;
		else
			tempgameconf[i - numnonascii] = tempgameconf[i];
	}
	tempgameconfsize -= numnonascii;

	*(tempgameconf + tempgameconfsize) = 0;
	//gameconf = (tempgameconf + tempgameconfsize) + (4 - (((u32) (tempgameconf + tempgameconfsize)) % 4));

	for (maxgameidmatch = 0; maxgameidmatch <= 6; maxgameidmatch++)
	{
		i = 0;
		while (i < tempgameconfsize)
		{
			maxgameidmatch2 = -1;
			while (maxgameidmatch != maxgameidmatch2)
			{
				while (i != tempgameconfsize && tempgameconf[i] != ':')
					i++;
				if (i == tempgameconfsize) break;
				while ((tempgameconf[i] != 10 && tempgameconf[i] != 13) && (i != 0))
					i--;
				if (i != 0) i++;
				parsebufpos = 0;
				gameidmatch = 0;
				while (tempgameconf[i] != ':')
				{
					if (tempgameconf[i] == '?')
					{
						parsebuffer[parsebufpos] = discid[parsebufpos];
						parsebufpos++;
						gameidmatch--;
						i++;
					}
					else if (tempgameconf[i] != 0 && tempgameconf[i] != ' ')
						parsebuffer[parsebufpos++] = tempgameconf[i++];
					else if (tempgameconf[i] == ' ')
						break;
					else i++;
					if (parsebufpos == 8) break;
				}
				parsebuffer[parsebufpos] = 0;
				if (strncasecmp("DEFAULT", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 7)
				{
					gameidmatch = 0;
					goto idmatch;
				}
				if (strncasecmp(discid, parsebuffer, strlen(parsebuffer)) == 0)
				{
					gameidmatch += strlen(parsebuffer);
					idmatch: if (gameidmatch > maxgameidmatch2)
					{
						maxgameidmatch2 = gameidmatch;
					}
				}
				while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
					i++;
			}
			while (i != tempgameconfsize && tempgameconf[i] != ':')
			{
				parsebufpos = 0;
				while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
				{
					if (tempgameconf[i] != 0 && tempgameconf[i] != ' ' && tempgameconf[i] != '(' && tempgameconf[i]
							!= ':')
						parsebuffer[parsebufpos++] = tempgameconf[i++];
					else if (tempgameconf[i] == ' ' || tempgameconf[i] == '(' || tempgameconf[i] == ':')
						break;
					else i++;
					if (parsebufpos == 17) break;
				}
				parsebuffer[parsebufpos] = 0;
				//if (!autobootcheck)
				{
					if (strncasecmp("codeliststart", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer)
							== 13)
					{
						sscanf((char *) (tempgameconf + i), " = %x", (unsigned int *) &codelist);
					}
					if (strncasecmp("codelistend", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						sscanf((char *) (tempgameconf + i), " = %x", (unsigned int *) &codelistend);
					}
					if (strncasecmp("poke", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 4)
					{
						ret = sscanf((char *) tempgameconf + i, "( %x , %x", &codeaddr, &codeval);
						if (ret == 2)
						{
							*(gameconf + (gameconfsize / 4)) = 0;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = 0;
							gameconfsize += 8;
							*(gameconf + (gameconfsize / 4)) = codeaddr;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeval;
							gameconfsize += 4;
							DCFlushRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
						}
					}
					if (strncasecmp("pokeifequal", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						ret = sscanf((char *) (tempgameconf + i), "( %x , %x , %x , %x", &codeaddr, &codeval,
								&codeaddr2, &codeval2);
						if (ret == 4)
						{
							*(gameconf + (gameconfsize / 4)) = 0;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeaddr;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeval;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeaddr2;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeval2;
							gameconfsize += 4;
							DCFlushRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
						}
					}
					if (strncasecmp("searchandpoke", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer)
							== 13)
					{
						ret = sscanf((char *) (tempgameconf + i), "( %x%n", &codeval, &tempoffset);
						if (ret == 1)
						{
							gameconfsize += 4;
							temp = 0;
							while (ret == 1)
							{
								*(gameconf + (gameconfsize / 4)) = codeval;
								gameconfsize += 4;
								temp++;
								i += tempoffset;
								ret = sscanf((char *) (tempgameconf + i), " %x%n", &codeval, &tempoffset);
							}
							*(gameconf + (gameconfsize / 4) - temp - 1) = temp;
							ret = sscanf((char *) (tempgameconf + i), " , %x , %x , %x , %x", &codeaddr, &codeaddr2,
									&codeoffset, &codeval2);
							if (ret == 4)
							{
								*(gameconf + (gameconfsize / 4)) = codeaddr;
								gameconfsize += 4;
								*(gameconf + (gameconfsize / 4)) = codeaddr2;
								gameconfsize += 4;
								*(gameconf + (gameconfsize / 4)) = codeoffset;
								gameconfsize += 4;
								*(gameconf + (gameconfsize / 4)) = codeval2;
								gameconfsize += 4;
								DCFlushRange((void *) (gameconf + (gameconfsize / 4) - temp - 5), temp * 4 + 20);
							}
							else gameconfsize -= temp * 4 + 4;
						}

					}
				}
				if (tempgameconf[i] != ':')
				{
					while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
						i++;
					if (i != tempgameconfsize) i++;
				}
			}
			if (i != tempgameconfsize) while ((tempgameconf[i] != 10 && tempgameconf[i] != 13) && (i != 0))
				i--;
		}
	}

	if(tempgameconf != defaultgameconfig)
		free(tempgameconf);

	if (code_size > (u32) codelistend - (u32) codelist)
	{
		gprintf("Ocarina: Too many codes found: filesize %i, maxsize: %i\n", code_size, (u32) codelistend - (u32) codelist);
		MEM2_free(code_buf);
		code_buf = NULL;
		code_size = 0;
	}
}

//---------------------------------------------------------------------------------
void load_handler(u32 hooktype, u32 debugger, u32 pauseAtStart)
//---------------------------------------------------------------------------------
{
	if (hooktype != 0x00)
	{
		if (debugger == 0x01)
			codelist = (u8 *) 0x800028B8;
		codelistend = (u8 *) 0x80003000;
		app_loadgameconfig();

		if (debugger == 0x01)
		{
			 //! Prefer Slot B
			if(usb_isgeckoalive(EXI_CHANNEL_1))
			{
				// slot B
				memset((void*)0x80001800,0,codehandler_size);
				memcpy((void*)0x80001800,codehandler,codehandler_size);
				if (pauseAtStart == 0x01)
					*(u32*)0x80002774 = 1;
				memcpy((void*)0x80001CDE, &codelist, 2);
				memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
				memcpy((void*)0x80001F5A, &codelist, 2);
				memcpy((void*)0x80001F5E, ((u8*) &codelist) + 2, 2);
				DCFlushRange((void*)0x80001800,codehandler_size);
			}
			else
			{
				// Slot A
				memset((void*)0x80001800,0,codehandlerslota_size);
				memcpy((void*)0x80001800,codehandlerslota,codehandlerslota_size);
				if (pauseAtStart == 0x01)
					*(u32*)0x80002774 = 1;
				memcpy((void*)0x80001CDE, &codelist, 2);
				memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
				memcpy((void*)0x80001F5A, &codelist, 2);
				memcpy((void*)0x80001F5E, ((u8*) &codelist) + 2, 2);
				DCFlushRange((void*)0x80001800,codehandlerslota_size);
			}
		}
		else
		{
			memset((void*)0x80001800,0,codehandleronly_size);
			memcpy((void*)0x80001800,codehandleronly,codehandleronly_size);
			memcpy((void*)0x80001906, &codelist, 2);
			memcpy((void*)0x8000190A, ((u8*) &codelist) + 2, 2);
			DCFlushRange((void*)0x80001800,codehandleronly_size);
		}
		// Load multidol handler
		memset((void*)0x80001000,0,multidol_size);
		memcpy((void*)0x80001000,multidol,multidol_size);
		DCFlushRange((void*)0x80001000,multidol_size);
		switch(hooktype)
		{
			default:
				break;
			case 0x01:
				memcpy((void*)0x8000119C,viwiihooks,12);
				memcpy((void*)0x80001198,viwiihooks+3,4);
				break;
			case 0x02:
				memcpy((void*)0x8000119C,kpadhooks,12);
				memcpy((void*)0x80001198,kpadhooks+3,4);
				break;
			case 0x03:
				memcpy((void*)0x8000119C,joypadhooks,12);
				memcpy((void*)0x80001198,joypadhooks+3,4);
				break;
			case 0x04:
				memcpy((void*)0x8000119C,gxdrawhooks,12);
				memcpy((void*)0x80001198,gxdrawhooks+3,4);
				break;
			case 0x05:
				memcpy((void*)0x8000119C,gxflushhooks,12);
				memcpy((void*)0x80001198,gxflushhooks+3,4);
				break;
			case 0x06:
				memcpy((void*)0x8000119C,ossleepthreadhooks,12);
				memcpy((void*)0x80001198,ossleepthreadhooks+3,4);
				break;
			case 0x07:
				memcpy((void*)0x8000119C,axnextframehooks,12);
				memcpy((void*)0x80001198,axnextframehooks+3,4);
				break;
			/*
			case 0x08:
				if (customhooksize == 16)
				{
					memcpy((void*)0x8000119C,customhook,12);
					memcpy((void*)0x80001198,customhook+3,4);
				}
				break;
			*/
			case 0x09:
				memcpy((void*)0x8000119C,wpadbuttonsdownhooks,12);
				memcpy((void*)0x80001198,wpadbuttonsdownhooks+3,4);
				break;
			case 0x0A:
				memcpy((void*)0x8000119C,wpadbuttonsdown2hooks,12);
				memcpy((void*)0x80001198,wpadbuttonsdown2hooks+3,4);
				break;
		}
		DCFlushRange((void*)0x80001198,16);

		memcpy((void *) 0x80001800, (void*) Disc_ID, 6); // For Wiird
		DCFlushRange((void *) 0x80001800, 6);
	}

	//Copy the codes
	if (code_buf && code_size > 0)
	{
		memset(codelist, 0, (u32) codelistend - (u32) codelist);
		memcpy(codelist, code_buf, code_size);
		DCFlushRange(codelist, (u32) codelistend - (u32) codelist);
		free(code_buf);
		code_buf = NULL;
		gprintf("Ocarina codes applied to %p size: %i\n", codelist, (u32) codelistend - (u32) codelist);
	}

	if(hooktype != 0x00)
	{
		//This needs to be done after loading the .dol into memory
		app_pokevalues();
	}
}

static int LoadGameConfig(const char *CheatFilepath)
{
	int filesize = 0;
	tempgameconf = (u8 *) defaultgameconfig;
	tempgameconfsize = defaultgameconfig_size;
	gameconfsize = 0;

	FILE* fp;
	char filepath[200];
	snprintf(filepath, sizeof(filepath), "%s/gameconfig.txt", CheatFilepath);

	fp = fopen(filepath, "rb");

	if (!fp)
	{
		snprintf(filepath, sizeof(filepath), "sd:/gameconfig.txt");
		fp = fopen(filepath, "rb");
		int i;
		for(i = 1; i <= 8; ++i)
		{
			if(fp) break;

			snprintf(filepath, sizeof(filepath), "usb%i:/gameconfig.txt", i);
			fp = fopen(filepath, "rb");
		}
	}

	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	tempgameconf = (u8*) MEM2_alloc(filesize);
	if (tempgameconf == NULL) {
		tempgameconf = (u8 *) defaultgameconfig;
		fclose(fp);
		return -1;
	}

	int ret = fread((void*) tempgameconf, 1, filesize, fp);

	fclose(fp);

	if (ret != filesize)
	{
		MEM2_free(tempgameconf);
		tempgameconf = (u8 *) defaultgameconfig;
		return -1;
	}
	tempgameconfsize = filesize;

	return 0;
}

int ocarina_load_code(const char *CheatFilepath, u8 *gameid)
{
	char filepath[150];
	char id[7];

	memset(id, 0, sizeof(id));
	memcpy(id, gameid, 6);
	snprintf(filepath, sizeof(filepath), "%s%s.gct", CheatFilepath, id);

	gprintf("Ocarina: Searching codes...%s\n", filepath);

	FILE * fp = fopen(filepath, "rb");
	if (!fp)
	{
		gprintf("Ocarina: No codes found");
		printf("\n");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	u32 filesize = ftell(fp);
	rewind(fp);

	code_buf = (u8*) MEM2_alloc(filesize);
	if (!code_buf)
	{
		gprintf("Ocarina: Not enough memory\n");
		fclose(fp);
		return 0;
	}

	code_size = fread(code_buf, 1, filesize, fp);

	fclose(fp);

	if (code_size == 0)
	{
		gprintf("Ocarina: could not read file.\n");
		MEM2_free(code_buf);
		code_buf = NULL;
		code_size = 0;
		return 0;
	}

	gprintf("Ocarina: Codes found.\n");

	LoadGameConfig(CheatFilepath);

	return code_size;
}

void langpatcher(void *addr, u32 len, u8 languageChoice)
{
	u8 ocarinaLangPatchByte = 1;
	switch (languageChoice)
	{
		case JAPANESE:
			ocarinaLangPatchByte = 0x00;
			break;
		case ENGLISH:
			ocarinaLangPatchByte = 0x01;
			break;
		case GERMAN:
			ocarinaLangPatchByte = 0x02;
			break;
		case FRENCH:
			ocarinaLangPatchByte = 0x03;
			break;
		case SPANISH:
			ocarinaLangPatchByte = 0x04;
			break;
		case ITALIAN:
			ocarinaLangPatchByte = 0x05;
			break;
		case DUTCH:
			ocarinaLangPatchByte = 0x06;
			break;
		case S_CHINESE:
			ocarinaLangPatchByte = 0x07;
			break;
		case T_CHINESE:
			ocarinaLangPatchByte = 0x08;
			break;
		case KOREAN:
			ocarinaLangPatchByte = 0x09;
			break;
		default:
			return;
	}

	u8 * addr_start = addr;
	u8 * addr_end = addr + len;

	while (addr_start < addr_end)
	{

		if (memcmp(addr_start, langpatch, sizeof(langpatch)) == 0)
		{
			langvipatch((u32) addr_start, len, ocarinaLangPatchByte);
		}
		addr_start += 4;
	}
}

void vidolpatcher(void *addr, u32 len)
{

	void *addr_start = addr;
	void *addr_end = addr + len;

	while (addr_start < addr_end)
	{
		if (memcmp(addr_start, vipatchcode, sizeof(vipatchcode)) == 0)
		{
			vipatch((u32) addr_start, len);
		}
		addr_start += 4;
	}
}

