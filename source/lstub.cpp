//functions for manipulating the HBC stub by giantpune
//updated by blackb0x April 2022

#include <string.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>

#include "lstub.h"
#include "wad/nandtitle.h"

extern const u8 stub_bin[];
extern const u32 stub_bin_size;

s32 Set_Stub(u64 reqID)
{
	if (!hbcStubAvailable())
		return 0;
	if (NandTitles.IndexOf(reqID) < 0)
		return WII_EINSTALL;

	char *stub = (char *)0x80002662;

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

u8 hbcStubAvailable()
{
	char *sig = (char *)0x80001804;
	return (strncmp(sig, "STUBHAXX", 8) == 0);
}

u64 returnTo(bool onlyHBC)
{
	if (!onlyHBC)
	{
		// UNEO
		if (NandTitles.IndexOf(0x00010001554E454FULL) >= 0)
			return 0x00010001554E454FULL;
		// ULNR
		if (NandTitles.IndexOf(0x00010001554C4E52ULL) >= 0)
			return 0x00010001554C4E52ULL;
		// IDCL
		if (NandTitles.IndexOf(0x000100014944434CULL) >= 0)
			return 0x000100014944434CULL;
	}
	// OHBC
	if (NandTitles.IndexOf(0x000100014F484243ULL) >= 0)
		return 0x000100014F484243ULL;
	// LULZ
	if (NandTitles.IndexOf(0x000100014C554C5AULL) >= 0)
		return 0x000100014C554C5AULL;
	// 1.0.7
	if (NandTitles.IndexOf(0x00010001AF1BF516ULL) >= 0)
		return 0x00010001AF1BF516ULL;
	// JODI
	if (NandTitles.IndexOf(0x000100014A4F4449ULL) >= 0)
		return 0x000100014A4F4449ULL;
	// HAXX
	if (NandTitles.IndexOf(0x0001000148415858ULL) >= 0)
		return 0x0001000148415858ULL;
	// System menu
	return 0x100000002ULL;
}
