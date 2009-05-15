#include <fat.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>
#include <sys/dir.h>

#define CACHE 8


int USBDevice_Init()
{
    //right now only mounts first partition and only under IOS36
    __io_usbstorage.startup();

    if (fatMount("USB", &__io_usbstorage, 0, CACHE)) {
    return 1;
    }

return -1;
}

void USBDevice_deInit()
{
    //First unmount all the devs...
    fatUnmount("USB");
    //...and then shutdown em!
    __io_usbstorage.shutdown();
}

int isSdInserted()
{
    return __io_wiisd.isInserted();
}

int SDCard_Init()
{
    //mount SD if inserted
    __io_wiisd.startup();
    if (!isSdInserted()){
        return -1;
    }
    if (fatMount("SD", &__io_wiisd, 0, CACHE)) {
        return 1;
    }
return -1;
}

void SDCard_deInit()
{
    //First unmount all the devs...
    fatUnmount("SD");
    //...and then shutdown em!
    __io_wiisd.shutdown();
}
