//functions for manipulating the HBC stub by giantpune
//updated by blackb0x August 2021

#include <string.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>

#include "lstub.h"
#include "gecko.h"
#include "wad/nandtitle.h"

extern const u8 stub_bin[];
extern const u32 stub_bin_size;

static char *determineStubTIDLocation()
{
	u32 *stubID = (u32 *)0x80001818;

	// HBC stub 1.1.4
	if (stubID[0] == 0x548406B0 && stubID[1] == 0x7C800124)
		return (char *)0x80002662;
	// HBC stub changed @ version 1.0.7.  this file was last updated for HBC 1.0.8
	else if (stubID[0] == 0x48000859 && stubID[1] == 0x4800088d)
		return (char *)0x8000286A;
	// HBC stub 1.0.6 and lower, and stub.bin
	else if (stubID[0] == 0x480004c1 && stubID[1] == 0x480004f5)
		return (char *)0x800024C6;

	return NULL;
}

s32 Set_Stub(u64 reqID)
{
	if (!hbcStubAvailable())
		return 0;
	if (NandTitles.IndexOf(reqID) < 0)
		return WII_EINSTALL;

	char *stub = determineStubTIDLocation();
	if (!stub)
		return 0;

	stub[0] = TITLE_7(reqID);
	stub[1] = TITLE_6(reqID);
	stub[8] = TITLE_5(reqID);
	stub[9] = TITLE_4(reqID);
	stub[4] = TITLE_3(reqID);
	stub[5] = TITLE_2(reqID);
	stub[12] = TITLE_1(reqID);
	stub[13] = ((u8)(reqID));

	DCFlushRange(stub, 0x10);
	return 1;
}

void loadStub()
{
	char *stubLoc = (char *)0x80001800;
	memcpy(stubLoc, stub_bin, stub_bin_size);
	DCFlushRange(stubLoc, stub_bin_size);
}

u64 getStubDest()
{
	if (!hbcStubAvailable())
		return 0;

	char ret[8];
	u64 retu = 0;

	char *stub = determineStubTIDLocation();
	if (!stub)
		return 0;

	ret[0] = stub[0];
	ret[1] = stub[1];
	ret[2] = stub[8];
	ret[3] = stub[9];
	ret[4] = stub[4];
	ret[5] = stub[5];
	ret[6] = stub[12];
	ret[7] = stub[13];

	memcpy(&retu, ret, 8);
	return retu;
}

u8 hbcStubAvailable()
{
	char *sig = (char *)0x80001804;
	return (strncmp(sig, "STUBHAXX", 8) == 0);
}

u64 returnTo(u64 currentStub)
{
	// UNEO
	if (NandTitles.IndexOf(0x00010001554E454FULL) > 0)
		return 0x00010001554E454FULL;
	// ULNR
	else if (NandTitles.IndexOf(0x00010001554C4E52ULL) > 0)
		return 0x00010001554C4E52ULL;
	// IDCL
	else if (NandTitles.IndexOf(0x000100014944434CULL) > 0)
		return 0x000100014944434CULL;
	// OHBC
	else if (NandTitles.IndexOf(0x000100014F484243ULL) > 0)
		return 0x000100014F484243ULL;
	// LULZ
	else if (NandTitles.IndexOf(0x000100014C554C5AULL) > 0)
		return 0x000100014C554C5AULL;
	// 1.0.7
	else if (NandTitles.IndexOf(0x00010001AF1BF516ULL) > 0)
		return 0x00010001AF1BF516ULL;
	// JODI
	else if (NandTitles.IndexOf(0x000100014A4F4449ULL) > 0)
		return 0x000100014A4F4449ULL;
	// HAXX
	else if (NandTitles.IndexOf(0x0001000148415858ULL) > 0)
		return 0x0001000148415858ULL;
	// System menu
	else if (!currentStub)
		return 0x100000002ULL;
	// Current stub
	return currentStub;
}
