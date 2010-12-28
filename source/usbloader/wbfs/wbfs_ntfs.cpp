#include <ntfs.h>
#include <locale.h>
#include "wbfs_ntfs.h"
#include "usbloader/usbstorage2.h"

sec_t ntfs_wbfs_sec = 0;

s32 Wbfs_Ntfs::Open()
{
    Close();

    strcpy(wbfs_fs_drive, "NTFS:");

    if (Mounted)
        return 0;

    Mounted = ntfsMount("NTFS", &__io_usbstorage2, lba, CACHE_SIZE, CACHED_SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
    if (!Mounted)
        return -2;

    // ntfsInit() resets locals
    // which breaks unicode in console
    // so we change it back to C-UTF-8
    setlocale(LC_CTYPE, "C-UTF-8");
    setlocale(LC_MESSAGES, "C-UTF-8");

    ntfs_wbfs_sec = lba;

    return 0;
}

void Wbfs_Ntfs::Close()
{
    if (hdd)
    {
        wbfs_close(hdd);
        hdd = NULL;
    }

    ntfsUnmount("NTFS:/", true);

    Mounted = false;
    ntfs_wbfs_sec = 0;
}
