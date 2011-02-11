#ifndef IOS_USBSTORAGE_H
#define IOS_USBSTORAGE_H
#include "syscalls.h"

void ums_init(void);
s32 ums_read_sectors(u32 sector, u32 numSectors, void *buffer);
void ums_close(void);
void *ums_alloc(int size);
void ums_free(void *ptr);
#endif
