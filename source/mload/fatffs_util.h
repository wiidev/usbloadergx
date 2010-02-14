#ifndef _FAT_UTIL_
#define _FAT_UTIL_

#include <gctypes.h>

#include "usbloader/disc.h"

s32 FAT_DeleteDir(const char *dirpath);

s32 FFS_to_FAT_Copy(const char *ffsdirpath, const char *fatdirpath);

void create_FAT_FFS_Directory(struct discHdr *header);

int test_FAT_game(char * directory);

char *get_FAT_directory1(void);

char *get_FAT_directory2(void);

char *get_FFS_directory1(void);

char *get_FFS_directory2(void);

#endif
