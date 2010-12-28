#include <string.h>
#include <unistd.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>
#include <locale.h>
#include <fat.h>
#include <ntfs.h>
#include <ext2.h>

#include "usbloader/usbstorage2.h"
#include "usbloader/sdhc.h"
#include "usbloader/wbfs.h"
#include "fatmounter.h"
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

extern sec_t _FAT_startSector;

extern s32 wbfsDev;

int fat_sd_mount = MOUNT_NONE;
sec_t fat_sd_sec = 0; // u32

int fat_usb_mount = 0;
sec_t fat_usb_sec = 0;

int USBDevice_Init()
{
    //closing all open Files write back the cache and then shutdown em!
    USBDevice_deInit();
    //right now mounts first FAT-partition

    bool started = false;
    int retries = 10;

    // wait 0.5 sec for the USB to spin up...stupid slow ass HDD
    do
    {
        started = (__io_usbstorage2.startup() && __io_usbstorage2.isInserted());
        usleep(50000);
        --retries;
    }
    while(!started && retries > 0);

    if(!started)
        return -1;

    if (fatMount("USB", &__io_usbstorage2, 0, CACHE, SECTORS))
    {
        fat_usb_sec = _FAT_startSector;
        return (fat_usb_mount = 1);
    }

    return -1;
}

int USBDevice_Init_Loop()
{
    time_t starttime = time(0);
    time_t timenow = starttime;
    bool StatusPrinted = false;
    bool started = false;

    do
    {
        started = (__io_usbstorage2.startup() && __io_usbstorage2.isInserted());

        if(!started)
        {
            if(timenow != time(0))
            {
                timenow = time(0);
                if(!StatusPrinted)
                {
                    printf("\tWaiting for slow HDD...");
                    StatusPrinted = true;
                }
                printf("%i ", (int) (timenow-starttime));
            }
            usleep(100000);
        }
    }
    while(!started && timenow-starttime < 30);

    if(StatusPrinted)
        printf("\n");

    return USBDevice_Init();
}

void USBDevice_deInit()
{
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");
    //only shutdown libogc usb and not the cios one
    __io_usbstorage2.shutdown();

    fat_usb_mount = 0;
    fat_usb_sec = 0;
}

int isInserted(const char *path)
{
    if (!strncmp(path, "USB:", 4)) return 1;

    return __io_sdhc.isInserted() || __io_wiisd.isInserted();
}

int SDCard_Init()
{
    //closing all open Files write back the cache and then shutdown em!
    SDCard_deInit();

    //right now mounts first FAT-partition
    if (fatMount("SD", &__io_wiisd, 0, CACHE, SECTORS))
    {
        fat_sd_mount = MOUNT_SD;
        fat_sd_sec = _FAT_startSector;
        return 1;
    }

    __io_wiisd.shutdown();

    if (fatMount("SD", &__io_sdhc, 0, CACHE, SECTORS))
    {
        fat_sd_mount = MOUNT_SDHC;
        fat_sd_sec = _FAT_startSector;
        return 1;
    }

    return -1;
}

void SDCard_deInit()
{
    fatUnmount("SD:/");
    __io_wiisd.shutdown();
    __io_sdhc.shutdown();

    fat_sd_mount = MOUNT_NONE;
    fat_sd_sec = 0;
}
