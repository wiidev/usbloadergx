#include <stdio.h>
#include <ogcsys.h>
#include <unistd.h>

#include "usbloader/wdvd.h"
#include "usbloader/usbstorage.h"
#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "mload/mload.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "fatmounter.h"
#include "sys.h"
#include "wpad.h"

/* Constants */
#define CERTS_LEN	0x280

//Wiilight stuff
static vu32 *_wiilight_reg = (u32*)0xCD0000C0;
void wiilight(int enable) {             // Toggle wiilight (thanks Bool for wiilight source)
    u32 val = (*_wiilight_reg&~0x20);
    if(enable && Settings.wiilight) val |= 0x20;
    *_wiilight_reg=val;
}

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
    mload_set_ES_ioctlv_vector(NULL);
	mload_close();
}


void Sys_Reboot(void)
{
	/* Restart console */
	_ExitApp();
	STM_RebootSystem();
}

int Sys_IosReload(int IOS)
{
    s32 ret = -1;

	//shutdown SD and USB before IOS Reload in DiscWait
    SDCard_deInit();
    USBDevice_deInit();

    mload_close();

    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    USBStorage_Deinit();
    WDVD_Close();

    if(IOS == 249 || IOS == 222 || IOS == 223) {
        for(int i = 0; i < 10; i++) {
            ret = IOS_ReloadIOS(IOS);
            if(ret < 0) return ret;
            if(IOS == 222 || IOS == 223) load_ehc_module();
            ret = WBFS_Init(WBFS_DEVICE_USB);
            if(!(ret < 0)) break;
            sleep(1);
            USBStorage_Deinit();
    }
    if(ret>=0) {
			ret = Disc_Init();
			int i = 0;
			if(ret>=0) {
			for(i = 0; i < 4; i++) {
				ret = WBFS_Open2(i);
				if(ret == 0) break;
			}
			}
		} else Sys_BackToLoader();
	}

    PAD_Init();
    Wpad_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
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
