/****************************************************************************
 * USB Loader GX Team
 * banner.h
 *
 * Dump opening.bnr thanks to Wiipower
 ***************************************************************************/

#ifndef BANNER_H
#define BANNER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    u8 filetype;
    char name_offset[3];
    u32 fileoffset;
    u32 filelen;
} __attribute__((packed)) FST_ENTRY;

s32 dump_banner(const char * discid,const char * dest);

#ifdef __cplusplus
}
#endif

#endif
