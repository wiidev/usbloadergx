#include <stdio.h>
#include <ogcsys.h>

#include "sys.h"
#include "wpad.h"
#include "wdvd.h"
#include "usbstorage.h"
#include "disc.h"
#include "wbfs.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "fatmounter.h"

/* Constants */
#define CERTS_LEN	0x280

/* Variables */
static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";
u8 shutdown = 0;
u8 reset = 0;

void __Sys_ResetCallback(void)
{
	/* Reboot console */
	reset = 1;
}

void __Sys_PowerCallback(void)
{
	/* Poweroff console */
	shutdown = 1;
}


void Sys_Init(void)
{
	/* Initialize video subsytem */
	//VIDEO_Init();

	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

static void _ExitApp()
{
	ExitGUIThreads();
	StopGX();
	ShutdownAudio();

    SDCard_deInit();
	USBDevice_deInit();
}


void Sys_Reboot(void)
{
	/* Restart console */
	_ExitApp();
	STM_RebootSystem();
}

int Sys_IosReload(int IOS)
{
    s32 ret;

	//shutdown SD and USB before IOS Reload in DiscWait
    SDCard_deInit();
    USBDevice_deInit();

    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    WDVD_Close();

    USBStorage_Deinit();

    ret = IOS_ReloadIOS(IOS);

    PAD_Init();
    Wpad_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

    if(ret < 0) {
        return ret;
    }
 
    if(IOS == 249 || IOS == 222 || IOS == 223) {
		ret = WBFS_Init(WBFS_DEVICE_USB);
		if(ret>=0)
		{
			ret = Disc_Init();
			if(ret>=0)
				ret = WBFS_Open();
		}
	}
	//reinitialize SD and USB
    SDCard_Init();
    USBDevice_Init();

    return ret;

}



#define ShutdownToDefault	0
#define ShutdownToIdle		1
#define ShutdownToStandby	2

static void _Sys_Shutdown(int SHUTDOWN_MODE)
{
	_ExitApp();
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Shutdown();

	/* Poweroff console */
	if((CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE &&  SHUTDOWN_MODE != ShutdownToStandby) || SHUTDOWN_MODE == ShutdownToIdle) {
		s32 ret;

		/* Set LED mode */
		ret = CONF_GetIdleLedMode();
		if(ret >= 0 && ret <= 2)
			STM_SetLedMode(ret);

		/* Shutdown to idle */
		STM_ShutdownToIdle();
	} else {
		/* Shutdown to standby */
		STM_ShutdownToStandby();
	}
}
void Sys_Shutdown(void)
{
	_Sys_Shutdown(ShutdownToDefault);
}
void Sys_ShutdownToIdel(void)
{
	_Sys_Shutdown(ShutdownToIdle);
}
void Sys_ShutdownToStandby(void)
{
	_Sys_Shutdown(ShutdownToStandby);
}

void Sys_LoadMenu(void)
{
	_ExitApp();
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void Sys_BackToLoader(void)
{
	if (*((u32*) 0x80001800))
	{
		_ExitApp();
		exit(0);
	}
	// Channel Version
	Sys_LoadMenu();
}

s32 Sys_GetCerts(signed_blob **certs, u32 *len)
{
	static signed_blob certificates[CERTS_LEN] ATTRIBUTE_ALIGN(32);

	s32 fd, ret;

	/* Open certificates file */
	fd = IOS_Open(certs_fs, 1);
	if (fd < 0)
		return fd;

	/* Read certificates */
	ret = IOS_Read(fd, certificates, sizeof(certificates));

	/* Close file */
	IOS_Close(fd);

	/* Set values */
	if (ret > 0) {
		*certs = certificates;
		*len   = sizeof(certificates);
	}

	return ret;
}
