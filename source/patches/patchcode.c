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

#include "usbloader/apploader.h"
#include "patchcode.h"
#include "../gecko.h"

//#include "sd.h"
//#include "fwrite_patch.h"
//#include "fwrite_patch_slota.h"
//#include "main.h"

extern void patchhook(u32 address, u32 len);
extern void patchhook2(u32 address, u32 len);
extern void patchhook3(u32 address, u32 len);

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

const u32 viwiihooks[4] = {
	0x7CE33B78,0x38870034,0x38A70038,0x38C7004C
};

const u32 kpadhooks[4] = {
	0x9A3F005E,0x38AE0080,0x389FFFFC,0x7E0903A6
};

const u32 kpadoldhooks[6] = {
	0x801D0060, 0x901E0060, 0x801D0064, 0x901E0064, 0x801D0068, 0x901E0068
};

const u32 joypadhooks[4] = {
	0x3AB50001, 0x3A73000C, 0x2C150004, 0x3B18000C
};

const u32 gxdrawhooks[4] = {
	0x3CA0CC01, 0x38000061, 0x3C804500, 0x98058000
};

const u32 gxflushhooks[4] = {
	0x90010014, 0x800305FC, 0x2C000000, 0x41820008
};

const u32 ossleepthreadhooks[4] = {
	0x90A402E0, 0x806502E4, 0x908502E4, 0x2C030000
};

const u32 axnextframehooks[4] = {
	0x3800000E, 0x7FE3FB78, 0xB0050000, 0x38800080
};

const u32 wpadbuttonsdownhooks[4] = {
	0x7D6B4A14, 0x816B0010, 0x7D635B78, 0x4E800020
};

const u32 wpadbuttonsdown2hooks[4] = {
	0x7D6B4A14, 0x800B0010, 0x7C030378, 0x4E800020
};

const u32 multidolhooks[4] = {
	0x7C0004AC, 0x4C00012C, 0x7FE903A6, 0x4E800420
};

const u32 multidolchanhooks[4] = {
	0x4200FFF4, 0x48000004, 0x38800000, 0x4E800020
};

const u32 langpatch[3] = {
	0x7C600775, 0x40820010, 0x38000000
};

static const u32 oldpatch002[3] = {
	0x2C000000, 0x40820214, 0x3C608000
};

static const u32 newpatch002[3] = {
	0x2C000000, 0x48000214, 0x3C608000
};

//---------------------------------------------------------------------------------
bool dogamehooks(void *addr, u32 len)
//---------------------------------------------------------------------------------
{
	//TODO for oggzee: when using Ocarina check if a hook as patched

	hooktype = 1; // TODO for oggzee: Create an option for hooktype
	/*
0 No Hook
1 VBI
2 KPAD read
3 Joypad Hook
4 GXDraw Hook
5 GXFlush Hook
6 OSSleepThread Hook
7 AXNextFrame Hook
	*/

	void *addr_start = addr;
	void *addr_end = addr+len;
	bool hookpatched = false;

	while(addr_start < addr_end)
	{
		switch(hooktype)
		{

			case 0x00:
				hookpatched = true;
			break;

			case 0x01:
				if(memcmp(addr_start, viwiihooks, sizeof(viwiihooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x02:

				if(memcmp(addr_start, kpadhooks, sizeof(kpadhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}

				if(memcmp(addr_start, kpadoldhooks, sizeof(kpadoldhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x03:

				if(memcmp(addr_start, joypadhooks, sizeof(joypadhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x04:

				if(memcmp(addr_start, gxdrawhooks, sizeof(gxdrawhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x05:

				if(memcmp(addr_start, gxflushhooks, sizeof(gxflushhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x06:

				if(memcmp(addr_start, ossleepthreadhooks, sizeof(ossleepthreadhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x07:

				if(memcmp(addr_start, axnextframehooks, sizeof(axnextframehooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;

			case 0x08:

				//if(memcmp(addr_start, customhook, customhooksize)==0)
				//{
				//	patchhook((u32)addr_start, len);
				//	hookpatched = true;
				//}
				if(memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
				{
					multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
					hookpatched = true;
				}
			break;
		}
		addr_start += 4;
	}
	return hookpatched;
}

// Not used yet, for patching DOL once loaded into memory and befor execution
/*
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
*/
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
/*
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
*/
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

//giantpune's magic super patch to return to channels
/*bool PatchReturnTo(void *Address, int Size, u32 id) {
    u8 SearchPattern[ 12 ] = 	{ 0x38, 0x80, 0x00, 0x02, 0x38, 0x60, 0x00, 0x01, 0x38, 0xa0, 0x00, 0x00 };

    int found = 0;
    int patched = 0;
    int offset = 0;
    u32 ad[ 3 ] = { 0, 0, 0 };

    void *Addr = Address;
    void *Addr_end = Address+Size;

    //find __OSLaunchMenu() and remember some addresses in it
    while (Addr <= Addr_end - 12 ) {
	if ( memcmp( Addr, SearchPattern, 12 )==0 ) {
	    ad[ found++ ] = Addr;
	}
	else if ( ad[ 0 ] && memcmp( Addr, SearchPattern, 8 )==0 ) //after the first match is found, only search the first 8 bytes for the other 2
	{
	    if( !ad[ 1 ] ) ad[ found++ ] = Addr;
	    else if( !ad[ 2 ] ) ad[ found++ ] = Addr;
	    if( found >= 3 )break;
	}
	Addr += 4;
	offset += 4;
    }

    //if the function is found and if it is not too far into the main.dol
    if( found == 3 && ( offset < 0x1000001 ) )
    {
	u32 nop = 0x60000000;

	//the magic that writes the TID to the registers
	u8 jump[ 20 ] = { 0x3C, 0x60, 0x00, 0x01, 0x60, 0x63, 0x00, 0x01,
			  0x3C, 0x80, 0x4A, 0x4F, 0x60, 0x84, 0x44, 0x49,
			  0x4E, 0x80, 0x00, 0x20 };
	//patch the thing to use the new TID
	jump[ 10 ] = (u8)( id>>24 );
	jump[ 11 ] = (u8)( id>>16 );
	jump[ 14 ] = (u8)( id>>8 );
	jump[ 15 ] = (u8)id;

	u32 jumpTo = Address - sizeof( jump );
	void* addr = jumpTo;

	//write new stuff to memory right before the main.dol
	memcpy( addr, jump, sizeof( jump ) );

	//ES_GetTicketViews()
	u32 newval = ( jumpTo - ad[ 0 ] );
	newval &= 0x03FFFFFC;
	newval |= 0x48000001;
	addr = ad[ 0 ];
	//gprintf("addr: %p\n", addr );
	memcpy( addr, &newval, sizeof( u32) );
	memcpy( addr + 4, &nop, sizeof( u32) );
	//gprintf("%p patched to %08x\n", addr, newval );

	//ES_GetTicketViews() again
	newval = ( jumpTo - ad[ 1 ] );
	newval &= 0x03FFFFFC;
	newval |= 0x48000001;
	addr = ad[ 1 ];
	//gprintf("addr: %p\n", addr );
	memcpy( addr, &newval, sizeof( u32) );
	memcpy( addr + 4, &nop, sizeof( u32) );
	//gprintf("%p patched to %08x\n", addr, newval );

	//ES_LaunchTitle()
	newval = ( jumpTo - ad[ 2 ] );
	newval &= 0x03FFFFFC;
	newval |= 0x48000001;
	addr = ad[ 2 ];
	//gprintf("addr: %p\n", addr );
	memcpy( addr, &newval, sizeof( u32) );
	memcpy( addr + 4, &nop, sizeof( u32) );
	//gprintf("%p patched to %08x\n", addr, newval );

	patched = 1;
    }
    return patched;
}
*/
bool PatchReturnTo(void *Address, int Size, u32 id) {
    u8 SearchPattern[ 12 ] = 	{ 0x38, 0x80, 0x00, 0x02, 0x38, 0x60, 0x00, 0x01, 0x38, 0xa0, 0x00, 0x00 };
    u8 SearchPattern2[ 12 ] = 	{ 0x4D, 0x65, 0x74, 0x72, 0x6F, 0x77, 0x65, 0x72, 0x6B, 0x73, 0x20, 0x54 };

    int found = 0;
    int patched = 0;
    u32 ad[ 4 ] = { 0, 0, 0, 0 };

    void *Addr = Address;
    void *Addr_end = Address+Size;

    while (Addr <= Addr_end - 12 ) {
	//find a safe place or the patch to hang out
	if ( ! ad[ 3 ] && memcmp( Addr, SearchPattern2, 12 )==0 ) {
	    ad[ 3 ] = Addr + 0x30;
	    gprintf("found a safe place @ %08x\n", ad[ 3 ]);
	    //hexdump( Addr, 0x50 );
	}
	//find __OSLaunchMenu() and remember some addresses in it
	else if ( memcmp( Addr, SearchPattern, 12 )==0 ) {
	    ad[ found++ ] = Addr;
	}
	else if ( ad[ 0 ] && memcmp( Addr, SearchPattern, 8 )==0 ) //after the first match is found, only search the first 8 bytes for the other 2
	{
	    if( !ad[ 1 ] ) ad[ found++ ] = Addr;
	    else if( !ad[ 2 ] ) ad[ found++ ] = Addr;
	    if( found >= 3 )break;
	}
	Addr += 4;
    }

    //if the function is found and if it is not too far into the main.dol
    if( found == 3 && ( ad[ 2 ] - ad[ 3 ] < 0x1000001 ) && ad[ 3 ] )
    {
	gprintf("patch __OSLaunchMenu( 0x00010001, 0x%08x )\n", id);
	u32 nop = 0x60000000;

	//the magic that writes the TID to the registers
	u8 jump[ 20 ] = { 0x3C, 0x60, 0x00, 0x01, 0x60, 0x63, 0x00, 0x01,
			  0x3C, 0x80, 0x4A, 0x4F, 0x60, 0x84, 0x44, 0x49,
			  0x4E, 0x80, 0x00, 0x20 };
	//patch the thing to use the new TID
	jump[ 10 ] = (u8)( id>>24 );
	jump[ 11 ] = (u8)( id>>16 );
	jump[ 14 ] = (u8)( id>>8 );
	jump[ 15 ] = (u8)id;

	//u32 jumpTo = Address - sizeof( jump );
	void* addr = ad[ 3 ];

	//write new stuff to memory right before the main.dol
	memcpy( addr, jump, sizeof( jump ) );

	//ES_GetTicketViews()
	u32 newval = ( ad[ 3 ] - ad[ 0 ] );
	newval &= 0x03FFFFFC;
	newval |= 0x48000001;
	addr = ad[ 0 ];
	memcpy( addr, &newval, sizeof( u32 ) );
	memcpy( addr + 4, &nop, sizeof( u32 ) );
	gprintf("%p -> %08x\n", addr, newval );

	//ES_GetTicketViews() again
	newval = ( ad[ 3 ] - ad[ 1 ] );
	newval &= 0x03FFFFFC;
	newval |= 0x48000001;
	addr = ad[ 1 ];
	memcpy( addr, &newval, sizeof( u32 ) );
	memcpy( addr + 4, &nop, sizeof( u32 ) );
	gprintf("%p -> %08x\n", addr, newval );

	//ES_LaunchTitle()
	newval = ( ad[ 3 ] - ad[ 2 ] );
	newval &= 0x03FFFFFC;
	newval |= 0x48000001;
	addr = ad[ 2 ];
	memcpy( addr, &newval, sizeof( u32 ) );
	memcpy( addr + 4, &nop, sizeof( u32 ) );
	gprintf("%p -> %08x\n", addr, newval );

	patched = 1;
    }
    else
    {
	gprintf("not patched\n");
	gprintf("found %d addresses\n", found);
	int i;
	for( i = 0; i< 4; i++)
	    gprintf("ad[ %d ]: %08x\n", i, ad[ i ] );
	gprintf("offset : %08x\n", ad[ 2 ] - ad[ 3 ] );

    }
    return patched;
}


