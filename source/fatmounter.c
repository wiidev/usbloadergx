#include <fat.h>
#include <string.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>

#include "usbloader/sdhc.h"
#include "usbloader/usbstorage.h"

//these are the only stable and speed is good
#define CACHE 32
#define SECTORS 64

#define MOUNT_NONE 0
#define MOUNT_SD   1
#define MOUNT_SDHC 2

extern DISC_INTERFACE __io_sdhc;
extern sec_t _FAT_startSector;

int   fat_sd_mount = MOUNT_NONE;
sec_t fat_sd_sec = 0; // u32

int   fat_usb_mount = 0;
sec_t fat_usb_sec = 0;

int   fat_wbfs_mount = 0;
sec_t fat_wbfs_sec = 0;

int USBDevice_Init() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");
    //right now mounts first FAT-partition

	//try first mount with cIOS
    if (!fatMount("USB", &__io_wiiums, 0, CACHE, SECTORS)) {
        //try now mount with libogc
		if (!fatMount("USB", &__io_usbstorage, 0, CACHE, SECTORS)) {
			return -1;
		}
    }
    
	fat_usb_mount = 1;
	fat_usb_sec = _FAT_startSector;
	return 0;
}

void USBDevice_deInit() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");

	fat_usb_mount = 0;
	fat_usb_sec = 0;
}

int WBFSDevice_Init(u32 sector) {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("WBFS:/");
    //right now mounts first FAT-partition

	//try first mount with cIOS
    if (!fatMount("WBFS", &__io_wiiums, 0, CACHE, SECTORS)) {
        //try now mount with libogc
		if (!fatMount("WBFS", &__io_usbstorage, 0, CACHE, SECTORS)) {
			return -1;
		}
    }
    
	fat_wbfs_mount = 1;
	fat_wbfs_sec = _FAT_startSector;
	if (sector && fat_wbfs_sec != sector) {
		// This is an error situation...actually, but is ignored in Config loader also
		// Should ask Oggzee about it...
	}
	return 0;
}

void WBFSDevice_deInit() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("WBFS:/");

	fat_wbfs_mount = 0;
	fat_wbfs_sec = 0;
}

int isInserted(const char *path) {
    if (!strncmp(path, "USB:", 4))
        return 1;

    return __io_sdhc.isInserted() || __io_wiisd.isInserted();
}

int SDCard_Init() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("SD:/");
    //right now mounts first FAT-partition
	if (fatMount("SD", &__io_wiisd, 0, CACHE, SECTORS)) {
		fat_sd_mount = MOUNT_SD;
		fat_sd_sec = _FAT_startSector;
		return 1;
	}
	else if (fatMount("SD", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE)) {
		fat_sd_mount = MOUNT_SDHC;
		fat_sd_sec = _FAT_startSector;
		return 1;
	}
    return -1;
}

void SDCard_deInit() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("SD:/");
	
	fat_sd_mount = MOUNT_NONE;
	fat_sd_sec = 0;
}
