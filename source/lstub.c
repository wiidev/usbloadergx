//functions for manipulating the HBC stub by giantpune

#include <string.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>


#include "lstub.h"
#include "stub_bin.h"

#define TITLE_1(x)		((u8)((x) >> 8))
#define TITLE_2(x)		((u8)((x) >> 16))
#define TITLE_3(x)		((u8)((x) >> 24))
#define TITLE_4(x)		((u8)((x) >> 32))
#define TITLE_5(x)		((u8)((x) >> 40))
#define TITLE_6(x)		((u8)((x) >> 48))
#define TITLE_7(x)		((u8)((x) >> 56))
#define TITLE_ID(x,y)	(((u64)(x) << 32) | (y))

s32 Set_Stub(u64 reqID) {
    u32 tmdsize;
    u64 tid = 0;
    u64 *list;
    u32 titlecount;
    s32 ret;
    u32 i;

    ret = ES_GetNumTitles(&titlecount);
    if (ret < 0)
        return WII_EINTERNAL;

    list = memalign(32, titlecount * sizeof(u64) + 32);

    ret = ES_GetTitles(list, titlecount);
    if (ret < 0) {
        free(list);
        return WII_EINTERNAL;
    }

    for (i=0; i<titlecount; i++) {
        if (list[i]==reqID) {
            tid = list[i];
            break;
        }
    }
    free(list);

    if (!tid)
        return -69;//id to stub is not installed

    if (ES_GetStoredTMDSize(tid, &tmdsize) < 0)
        return WII_EINSTALL;

    char *stub = (char *)0x800024C6;

    stub[0] = TITLE_7(reqID);
    stub[1] = TITLE_6(reqID);
    stub[8] = TITLE_5(reqID);
    stub[9] = TITLE_4(reqID);
    stub[4] = TITLE_3(reqID);
    stub[5] = TITLE_2(reqID);
    stub[12] = TITLE_1(reqID);
    stub[13] = ((u8)(reqID));

    return 1;

}

s32 Set_Stub_Split(u32 type, const char* reqID) {
    char tmp[4];
    u32 lower;
    sprintf(tmp,"%c%c%c%c",reqID[0],reqID[1],reqID[2],reqID[3]);
    memcpy(&lower, tmp, 4);
    u64 reqID64 = TITLE_ID(type, lower);
    return Set_Stub(reqID64);

}

void loadStub() {
    char *stubLoc = (char *)0x80001800;
    memcpy(stubLoc, stub_bin, stub_bin_size);
}

u64 getStubDest() {
    if (!hbcStubAvailable())
        return 0;


    char *stub = (char *)0x800024C6;
    char ret[9];
    u64 retu =0;

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

u8 hbcStubAvailable() {
    char * sig = (char *)0x80001804;
    return (
               sig[0] == 'S' &&
               sig[1] == 'T' &&
               sig[2] == 'U' &&
               sig[3] == 'B' &&
               sig[4] == 'H' &&
               sig[5] == 'A' &&
               sig[6] == 'X' &&
               sig[7] == 'X') ? 1 : 0;
}

