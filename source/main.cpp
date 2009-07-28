/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * demo.cpp
 * Basic template/demonstration of libwiigui capabilities. For a
 * full-featured app using many more extensions, check out Snes9x GX.
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "usbloader/wbfs.h"
#include "usbloader/video2.h"
#include "settings/cfg.h"
#include "language/gettext.h"
#include "mload/mload.h"
#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "main.h"
#include "fatmounter.h"
#include "sys.h"
#include "wpad.h"
#include "fat.h"

/* Constants */
#define CONSOLE_XCOORD		260
#define CONSOLE_YCOORD		115
#define CONSOLE_WIDTH		340
#define CONSOLE_HEIGHT		218

FreeTypeGX *fontSystem=0;
FreeTypeGX *fontClock=0;

int
main(int argc, char *argv[])
{
	s32 ret2;

	bool bootDevice_found=false;
	if(argc >= 1)
	{
		if(!strncasecmp(argv[0], "usb:/", 5))
		{
			strcpy(bootDevice, "USB:");
			bootDevice_found = true;
		}
		else if(!strncasecmp(argv[0], "sd:/", 4))
			bootDevice_found = true;
	}
	
	if(!bootDevice_found)
	{
		//try USB
		struct stat st;
        if((stat("USB:/apps/usbloader_gx/boot.dol", &st) == 0) || (stat("USB:/apps/usbloader_gx/boot.elf", &st) == 0))
			strcpy(bootDevice, "USB:");
	}

    ret2 = IOS_ReloadIOS(249);
	if(ret2 < 0) {
		ret2 = IOS_ReloadIOS(222);
		load_ehc_module();
	}
	
	SDCard_Init(); // mount SD for loading cfg's
	USBDevice_Init(); // and mount USB:/



	gettextCleanUp();
	CFG_Load();

    /* Load Custom IOS */
    if(Settings.cios == ios222 && IOS_GetVersion() != 222) {
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
        ret2 = IOS_ReloadIOS(222);
        load_ehc_module();
        if (ret2 < 0) {
            Settings.cios = ios249;
            ret2 = IOS_ReloadIOS(249);
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
	} else if(Settings.cios == ios249 && IOS_GetVersion() != 249) {
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
	    ret2 = IOS_ReloadIOS(249);
	    if(ret2 < 0) {
            Settings.cios = ios222;
            ret2 = IOS_ReloadIOS(222);
            load_ehc_module();
	    }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
	}

	if (ret2 < 0) {
		printf("ERROR: cIOS could not be loaded!");
		sleep(5);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}

	Sys_Init();

	/** PAD_Init has to be before InitVideo don't move that **/
    PAD_Init(); // initialize PAD/WPAD
	Wpad_Init();

	InitVideo(); // Initialise video
	InitAudio(); // Initialize audio
	
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	// load main font from file, or default to built-in font
	fontSystem = new FreeTypeGX();
	char *fontPath = NULL;
	asprintf(&fontPath, "%sfont.ttf", CFG.theme_path);
	fontSystem->loadFont(fontPath, font_ttf, font_ttf_size, 0);
	fontSystem->setCompatibilityMode(FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE);
	free(fontPath);

	fontClock = new FreeTypeGX();
	fontClock->loadFont(NULL, clock_ttf, clock_ttf_size, 0);
	fontClock->setCompatibilityMode(FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE);

	InitGUIThreads();
	MainMenu(MENU_CHECK);
	return 0;
}

