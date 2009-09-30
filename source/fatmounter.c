#include <fat.h>
#include <string.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>

#include "usbloader/usbstorage.h"

//these are the only stable and speed is good
#define CACHE 8
#define SECTORS 64

int USBDevice_Init() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");
    //right now mounts first FAT-partition
    if (fatMount("USB", &__io_wiiums, 0, CACHE, SECTORS)) {
        //try first mount with cIOS
        return 1;
    } else if (fatMount("USB", &__io_usbstorage, 0, CACHE, SECTORS)) {
        //try now mount with libogc
        return 1;
    }
    return -1;
}

void USBDevice_deInit() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("USB:/");
}

int isSdInserted() {
    return __io_wiisd.isInserted();
}

int isInserted(const char *path) {
    if (!strncmp(path, "USB:", 4))
        return 1;

    return __io_wiisd.isInserted();
}
int SDCard_Init() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("SD:/");
    //right now mounts first FAT-partition
    if (fatMount("SD", &__io_wiisd, 0, CACHE, SECTORS))
        return 1;
    return -1;
}

void SDCard_deInit() {
    //closing all open Files write back the cache and then shutdown em!
    fatUnmount("SD:/");
}
