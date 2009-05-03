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
#include "geckomenu.h"
#include "apploader.h"
#include "patchcode.h"
#include "fwrite_patch.h"

extern void patchhook(u32 address, u32 len);
extern void patchhook2(u32 address, u32 len);
extern void patchhook3(u32 address, u32 len);
extern void patchhook4(u32 address, u32 len);
extern void multidolpatchone(u32 address, u32 len);
extern void multidolpatchtwo(u32 address, u32 len);

extern void regionfreejap(u32 address, u32 len);
extern void regionfreeusa(u32 address, u32 len);
extern void regionfreepal(u32 address, u32 len);

extern void removehealthcheck(u32 address, u32 len);

extern void copyflagcheck1(u32 address, u32 len);
extern void copyflagcheck2(u32 address, u32 len);
extern void copyflagcheck3(u32 address, u32 len);
extern void copyflagcheck4(u32 address, u32 len);
extern void copyflagcheck5(u32 address, u32 len);

extern void patchupdatecheck(u32 address, u32 len);

extern void movedvdhooks(u32 address, u32 len);


extern void patchhookdol(u32 address, u32 len);
extern void langvipatch(u32 address, u32 len, u8 langbyte);
extern void vipatch(u32 address, u32 len);
extern u32 regionfreeselect;

static const u32 viwiihooks[4] = {
	0x7CE33B78,0x38870034,0x38A70038,0x38C7004C 
};

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

static const u32 fwritepatch[8] = {
	0x9421FFD0,0x7C0802A6,0x90010034,0xBF210014,0x7C9B2378,0x7CDC3378,0x7C7A1B78,0x7CB92B78  // bushing fwrite
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

static const u32 langpatch[3] = {
	0x7C600775, 0x40820010, 0x38000000
};

static const u32 vipatchcode[3] = {
	0x4182000C,0x4180001C,0x48000018
};

static const u32 wpadlibogc[5] = {
//	0x38A00140, 0x7C095878, 0x7D600078, 0x901F0010,0x913F0014
//	0x7FA00124, 0x8001001C, 0x83810008, 0x83A1000C,0x7C0803A6
	0x90A402E0,0x806502E4,0x908502E4,0x2C030000,0x906402E4
};

void dogamehooks(void *addr, u32 len)
{
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		
		switch(hooktype)
		{
		
			case 0:
					
			break;
			
			case 1:
				if(memcmp(addr_start, viwiihooks, sizeof(viwiihooks))==0){
				//	printf("\n\n\n");
				//	printf("found at address %x\n", addr_start);
				//	sleep(2);
					patchhook((u32)addr_start, len);
					patched = 1;
					hooktype = 1;
				}
			break;

/*
			case 2:
				if(memcmp(addr_start, kpadhooks, sizeof(kpadhooks))==0){
					patchhook((u32)addr_start, len);
					patched = 1;
				}

				if(memcmp(addr_start, kpadoldhooks, sizeof(kpadoldhooks))==0){
					patchhook((u32)addr_start, len);
					patched = 1;
				}
			break;
		
			case 3:
				if(memcmp(addr_start, joypadhooks, sizeof(joypadhooks))==0){
					patchhook((u32)addr_start, len);
					patched = 1;
				}
			break;

			case 4:
				if(memcmp(addr_start, recoveryhooks, sizeof(recoveryhooks))==0){
						patchhook3((u32)addr_start, len);
					}
			break;
*/
			case 2:

				if(memcmp(addr_start, viwiihooks, sizeof(viwiihooks))==0){
						patchhook2((u32)addr_start, len);
				}
				
			break;

/*
			case 6:
				// jap region free	
				if(memcmp(addr_start, regionfreehooks, sizeof(regionfreehooks))==0){
						regionfreejap((u32)addr_start, len);
					}

				// usa region free
				if(memcmp(addr_start, regionfreehooks, sizeof(regionfreehooks))==0){
					regionfreeusa((u32)addr_start, len);
				}

				// pal region free
				if(memcmp(addr_start, regionfreehooks, sizeof(regionfreehooks))==0){
					regionfreepal((u32)addr_start, len);
				}

				// skip disc update
				if(memcmp(addr_start, updatecheckhook, sizeof(updatecheckhook))==0){
					patchupdatecheck((u32)addr_start, len);
				}
			break;


			case 7:
				if(memcmp(addr_start, healthcheckhook, sizeof(healthcheckhook))==0){
					removehealthcheck((u32)addr_start, len);
				}
			break;

			// no copy flags
			case 8:
					// Remove the actual flag so can copy back
				if(memcmp(addr_start, nocopyflag5, sizeof(nocopyflag5))==0){
					copyflagcheck5((u32)addr_start, len);
				}
			
			
				if(memcmp(addr_start, nocopyflag1, sizeof(nocopyflag1))==0){
					copyflagcheck1((u32)addr_start, len);
				}

				if(memcmp(addr_start, nocopyflag2, sizeof(nocopyflag2))==0){
					copyflagcheck2((u32)addr_start, len);
				}
				
				// no VC and GH3 save
				if(memcmp(addr_start, nocopyflag3, sizeof(nocopyflag2))==0){
					copyflagcheck3((u32)addr_start, len);
				}
				// no VC and GH3 save display remove
				if(memcmp(addr_start, nocopyflag4, sizeof(nocopyflag4))==0){
					copyflagcheck4((u32)addr_start, len);
				}
				
			break;

			case 9:
				if(memcmp(addr_start, movedvdpatch, sizeof(movedvdpatch))==0){
					movedvdhooks((u32)addr_start, len);
				}
			break;
*/
			// multidol
			case 3:

				if(memcmp(addr_start, multidolpatch1, sizeof(multidolpatch1))==0){
						multidolpatchone((u32)addr_start, len);
				}
				if(memcmp(addr_start, multidolpatch2, sizeof(multidolpatch2))==0){
						multidolpatchtwo((u32)addr_start, len);
				}
                        break;
                }
		addr_start += 4;
        }
}

// Not used yet, for patching DOL once loaded into memory and befor execution
void patchdol(void *addr, u32 len)
{
	
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		if(memcmp(addr_start, wpadlibogc, sizeof(wpadlibogc))==0) {
	//		printf("\n\n\n");
	//		printf("found at address %x\n", addr_start);
		//	sleep(10);
	//		patchhookdol((u32)addr_start, len);
			patched = 1;
			break;
		}
		addr_start += 4;
	}
}

void langpatcher(void *addr, u32 len)
{
	
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		
		if(memcmp(addr_start, langpatch, sizeof(langpatch))==0) {
			if(configbytes[0] != 0xCD){
				langvipatch((u32)addr_start, len, configbytes[0]);
			}	
		}
		addr_start += 4;
	}
}

void patchdebug(void *addr, u32 len)
{
	
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		
		if(memcmp(addr_start, fwritepatch, sizeof(fwritepatch))==0) {

			memcpy(addr_start,fwrite_patch_bin,fwrite_patch_bin_len);
			// apply patch	
		}
		addr_start += 4;
	}
}

void vidolpatcher(void *addr, u32 len)
{
	
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		if(memcmp(addr_start, vipatchcode, sizeof(vipatchcode))==0) {
			vipatch((u32)addr_start, len);
		}
		addr_start += 4;
	}
}


