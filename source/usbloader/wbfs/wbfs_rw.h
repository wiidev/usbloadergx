#ifndef _WBFS_RW_H
#define _WBFS_RW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <gccore.h>
#include "libs/libwbfs/libwbfs.h"

	typedef struct _WBFS_PartInfo
	{
		u32 wbfs_sector_size;
		u32 hdd_sector_size;
		u32 partition_lba;
		u32 partition_num_sec;
		const DISC_INTERFACE * handle;
	} WBFS_PartInfo;

	extern rw_sector_callback_t readCallback;
	extern rw_sector_callback_t writeCallback;

	s32 __ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);
	s32 __ReadUSB(void *fp, u32 lba, u32 count, void *iobuf);
	s32 __WriteUSB(void *fp, u32 lba, u32 count, void *iobuf);
	s32 __ReadSDHC(void *fp, u32 lba, u32 count, void *iobuf);
	s32 __WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf);
	s32 __ReadDVDPlain(void *iobuf, u32 len, u64 offset);

#ifdef __cplusplus
}
#endif

#endif //_WBFS_RW_H
