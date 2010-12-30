#include <ntfs.h>
#include <locale.h>
#include "wbfs_ntfs.h"
#include "usbloader/usbstorage2.h"

extern int wbfs_part_fs;
sec_t ntfs_wbfs_sec = 0;

s32 Wbfs_Ntfs::Open()
{
    s32 ret = Wbfs_Fat::Open();
    if(ret == 0)
    {
        ntfs_wbfs_sec = lba;
        wbfs_part_fs = PART_FS_NTFS;
    }

    return 0;
}
