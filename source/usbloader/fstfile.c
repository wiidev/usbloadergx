#include <gccore.h>

#include "fstfile.h"


char *fstfiles(FST_ENTRY *fst, u32 index) {
    u32 count = fst[0].filelen;
    u32 stringoffset;
    if (index < count) {
        stringoffset = *(u32 *)&(fst[index]) % (256*256*256);
        return (char *)((u32)fst + count*12 + stringoffset);
    } else {
        return NULL;
    }
}

char *fstfilename(u32 index) {
    FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
    u32 count = fst[0].filelen;
    u32 stringoffset;
    if (index < count) {
        stringoffset = *(u32 *)&(fst[index]) % (256*256*256);
        return (char *)(*(u32 *)0x80000038 + count*12 + stringoffset);
    } else {
        return NULL;
    }
}

u32 fstfileoffset(u32 index) {
    FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
    u32 count = fst[0].filelen;
    if (index < count) {
        return fst[index].fileoffset;
    } else {
        return 0;
    }
}
