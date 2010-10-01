#ifndef _WBFS_RW_H
#define _WBFS_RW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "libs/libwbfs/libwbfs.h"

    extern u32 sector_size;
    extern rw_sector_callback_t readCallback;
    extern rw_sector_callback_t writeCallback;

    s32 __ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);
    s32 __ReadUSB(void *fp, u32 lba, u32 count, void *iobuf);
    s32 __WriteUSB(void *fp, u32 lba, u32 count, void *iobuf);
    s32 __ReadSDHC(void *fp, u32 lba, u32 count, void *iobuf);
    s32 __WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf);

#ifdef __cplusplus
}
#endif

#endif //_WBFS_RW_H
