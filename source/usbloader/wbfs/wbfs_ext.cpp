#include <ext2.h>
#include "usbloader/usbstorage2.h"
#include "wbfs_ext.h"

sec_t ext_wbfs_sec = 0;

s32 Wbfs_Ext::Open()
{
    Close();
    strcpy(wbfs_fs_drive, "EXT:");

    if (Mounted)
        return 0;

    Mounted = ext2Mount("EXT", &__io_usbstorage2, lba, CACHE_SIZE, CACHED_SECTORS, EXT2_FLAG_DEFAULT);
    if (!Mounted)
        return -2;

    ext_wbfs_sec = lba;

    return 0;
}

void Wbfs_Ext::Close()
{
    if (hdd)
    {
        wbfs_close(hdd);
        hdd = NULL;
    }

    /* Unmount device */
    ext2Unmount("EXT:/");

    Mounted = false;
    ext_wbfs_sec = 0;
}
