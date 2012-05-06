//functions for manipulating the HBC stub by giantpune

#include <string.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>

#include "lstub.h"
#include "gecko.h"

#include "wad/nandtitle.h"

extern const u8 stub_bin[];
extern const u32 stub_bin_size;

static char* determineStubTIDLocation()
{
	u32 *stubID = (u32*) 0x80001818;

	//HBC stub 1.0.6 and lower, and stub.bin
	if (stubID[0] == 0x480004c1 && stubID[1] == 0x480004f5)
		return (char *) 0x800024C6;

	//HBC stub changed @ version 1.0.7.  this file was last updated for HBC 1.0.8
	else if (stubID[0] == 0x48000859 && stubID[1] == 0x4800088d) return (char *) 0x8000286A;

	//hexdump( stubID, 0x20 );
	return NULL;

}

s32 Set_Stub(u64 reqID)
{
	if (NandTitles.IndexOf(reqID) < 0) return WII_EINSTALL;

	char *stub = determineStubTIDLocation();
	if (!stub) return -68;

	stub[0] = TITLE_7( reqID );
	stub[1] = TITLE_6( reqID );
	stub[8] = TITLE_5( reqID );
	stub[9] = TITLE_4( reqID );
	stub[4] = TITLE_3( reqID );
	stub[5] = TITLE_2( reqID );
	stub[12] = TITLE_1( reqID );
	stub[13] = ((u8) (reqID));

	DCFlushRange(stub, 0x10);

	return 1;

}

s32 Set_Stub_Split(u32 type, const char* reqID)
{
	const u32 lower = ((u32)reqID[0] << 24) |
					  ((u32)reqID[1] << 16) |
					  ((u32)reqID[2] << 8) |
					  ((u32)reqID[3]);

	u64 reqID64 = TITLE_ID( type, lower );
	return Set_Stub(reqID64);

}

void loadStub()
{
	char *stubLoc = (char *) 0x80001800;
	memcpy(stubLoc, stub_bin, stub_bin_size);
	DCFlushRange(stubLoc, stub_bin_size);
}

u64 getStubDest()
{
	if (!hbcStubAvailable()) return 0;

	char ret[8];
	u64 retu = 0;

	char *stub = determineStubTIDLocation();
	if (!stub) return 0;

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
	char * sig = (char *) 0x80001804;
	return (strncmp(sig, "STUBHAXX", 8) == 0);
}

