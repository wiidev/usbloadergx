#include <ext2.h>
#include "usbloader/usbstorage2.h"
#include "wbfs_ext.h"

extern int wbfs_part_fs;
sec_t ext_wbfs_sec = 0;

s32 Wbfs_Ext::Open()
{
    s32 ret = Wbfs_Fat::Open();
    if(ret == 0)
    {
        ext_wbfs_sec = lba;
        wbfs_part_fs = PART_FS_EXT;
    }

    return 0;
}
