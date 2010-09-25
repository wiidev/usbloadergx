#include <string.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>
#include <locale.h>

#include "usbloader/usbstorage2.h"
#include "usbloader/sdhc.h"
#include "usbloader/wbfs.h"
#include "libntfs/ntfs.h"
#include "libfat/fat.h"
#include "gecko.h"

//these are the only stable and speed is good
#define CACHE 32
#define SECTORS 64
#define SECTORS_SD 32

#define MOUNT_NONE 0
#define MOUNT_SD   1
#define MOUNT_SDHC 2

#define DEBUG_FAT

/* Disc interfaces */
extern const DISC_INTERFACE __io_sdhc;

void _FAT_mem_init();
extern sec_t _FAT_startSector;

extern s32 wbfsDev;

int fat_sd_mount = MOUNT_NONE;
sec_t fat_sd_sec = 0; // u32

int fat_usb_mount = 0;
sec_t fat_usb_sec = 0;

int fat_wbfs_mount = 0;
sec_t fat_wbfs_sec = 0;

int fs_ntfs_mount = 0;
sec_t fs_ntfs_sec = 0;

int USBDevice_Init()
{
#ifdef DEBUG_FAT
    gprintf("USBDevice_Init()");
#endif
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");
    //right now mounts first FAT-partition

    //try first mount with cIOS
    //    if (!fatMount("USB", &__io_wiiums, 0, CACHE, SECTORS)) {
    //      //try now mount with libogc
    if (!fatMount("USB", &__io_usbstorage2, 0, CACHE, SECTORS))
    {
#ifdef DEBUG_FAT
        gprintf(":-1\n");
#endif
        return -1;
    }
    //  }

    fat_usb_mount = 1;
    fat_usb_sec = _FAT_startSector;
#ifdef DEBUG_FAT
    gprintf(":0\n");
#endif
    return 0;
}

void USBDevice_deInit()
{
#ifdef DEBUG_FAT
    gprintf("USBDevice_deInit(): ");
#endif
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");

    fat_usb_mount = 0;
    fat_usb_sec = 0;
#ifdef DEBUG_FAT
    gprintf("ok\n");
#endif
}

int WBFSDevice_Init(u32 sector)
{
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("WBFS:/");
    //right now mounts first FAT-partition

    //try first mount with cIOS
    //    if (!fatMount("WBFS", &__io_wiiums, 0, CACHE, SECTORS)) {
    //try now mount with libogc
    if (!fatMount("WBFS", &__io_usbstorage2, 0, CACHE, SECTORS))
    {
        return -1;
    }
    //  }

    fat_wbfs_mount = 1;
    fat_wbfs_sec = _FAT_startSector;
    if (sector && fat_wbfs_sec != sector)
    {
        // This is an error situation...actually, but is ignored in Config loader also
        // Should ask Oggzee about it...
    }
    return 0;
}

void WBFSDevice_deInit()
{
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("WBFS:/");

    fat_wbfs_mount = 0;
    fat_wbfs_sec = 0;
}

int isInserted(const char *path)
{
    if (!strncmp(path, "USB:", 4)) return 1;

    return __io_sdhc.isInserted() || __io_wiisd.isInserted();
}

static u8 sdIsInited = 0;
int SDCard_Init()
{
#ifdef DEBUG_FAT
    gprintf("SDCard_Init()");
#endif
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("SD:/");
    //right now mounts first FAT-partition
    if (fatMount("SD", &__io_wiisd, 0, CACHE, SECTORS))
    {
        fat_sd_mount = MOUNT_SD;
        fat_sd_sec = _FAT_startSector;
        sdIsInited = 1;
#ifdef DEBUG_FAT
        gprintf(":1\n");
#endif
        return 1;
    }
    else if (fatMount("SD", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE))
    {
        fat_sd_mount = MOUNT_SDHC;
        fat_sd_sec = _FAT_startSector;
        sdIsInited = 1;
#ifdef DEBUG_FAT
        gprintf(":1\n");
#endif
        return 1;
    }
#ifdef DEBUG_FAT
    gprintf(":-1\n");
#endif
    return -1;
}

void SDCard_deInit()
{
#ifdef DEBUG_FAT
    gprintf("SDCard_deInit( %d ): ", sdIsInited);
#endif
    //closing all open Files write back the cache and then shutdown em!
    if (sdIsInited)
    {
        fatUnmount("SD:/");

        fat_sd_mount = MOUNT_NONE;
        fat_sd_sec = 0;
        sdIsInited = 0;
    }
#ifdef DEBUG_FAT
    gprintf("ok\n");
#endif
}

void ntfsInit();

s32 MountNTFS(u32 sector)
{
    s32 ret;

    if (fs_ntfs_mount) return 0;

    ntfsInit(); // Call ntfs init here, to prevent locale resets

    // ntfsInit resets locale settings
    // which breaks unicode in console
    // so we change it back to C-UTF-8
    setlocale(LC_CTYPE, "C-UTF-8");
    setlocale(LC_MESSAGES, "C-UTF-8");

    if (wbfsDev == WBFS_DEVICE_USB)
    {
        /* Initialize WBFS interface */
        //      if (!__io_wiiums.startup()) {
        ret = __io_usbstorage2.startup();
        if (!ret)
        {
            return -1;
        }
        //      }
        /* Mount device */
        //      if (!ntfsMount("NTFS", &__io_wiiums, sector, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER)) {
        ret = ntfsMount("NTFS", &__io_usbstorage2, sector, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
        if (!ret)
        {
            return -2;
        }
        //      }
    }
    else if (wbfsDev == WBFS_DEVICE_SDHC)
    {
        if (sdhc_mode_sd == 0)
        {
            ret = ntfsMount("NTFS", &__io_sdhc, 0, CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
        }
        else
        {
            ret = ntfsMount("NTFS", &__io_sdhc, 0, CACHE, SECTORS_SD, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER);
        }
        if (!ret)
        {
            return -5;
        }
    }

    fs_ntfs_mount = 1;
    fs_ntfs_sec = sector; //_FAT_startSector;

    return 0;
}

s32 UnmountNTFS(void)
{
    /* Unmount device */
    ntfsUnmount("NTFS:/", true);

    fs_ntfs_mount = 0;
    fs_ntfs_sec = 0;

    return 0;
}
