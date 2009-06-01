#include <fat.h>
#include <sys/dir.h>
#include <sys/iosupport.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>

//these are the only stable and speed is good
#define CACHE 32
#define SECTORS 128


int USBDevice_Init()
{
    return 0;
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("USB:/");
	//right now only mounts first partition
	if (fatMount("USB", &__io_usbstorage, 0, CACHE, SECTORS)) {
		return 1;
	}
	return -1;
}

void USBDevice_deInit()
{
    return;
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("USB:/");
}

int isSdInserted()
{
    return __io_wiisd.isInserted();
}

int SDCard_Init()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("SD:/");
	//right now only mounts first partition
	if (fatMount("SD", &__io_wiisd, 0, CACHE, SECTORS))
		return 1;
	return -1;
}

void SDCard_deInit()
{
	//closing all open Files write back the cache and then shutdown em!
	fatUnmount("SD:/");
}
