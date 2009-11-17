/****************************************************************************
 * USB Loader GX Team
 *
 * Main loadup of the application
 *
 * libwiigui
 * Tantric 2009
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <ogcsys.h>
#include <unistd.h>
#include <locale.h>
#include <wiiuse/wpad.h>

#include "libwiigui/gui.h"
#include "usbloader/wbfs.h"
#include "settings/cfg.h"
#include "language/gettext.h"
#include "mload/mload.h"
#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "listfiles.h"
#include "main.h"
#include "fatmounter.h"
#include "sys.h"
#include "wpad.h"
#include "fat.h"
#include "gecko.h"
#include "svnrev.h"
#include "usbloader/partition_usbloader.h"
#include "usbloader/usbstorage.h"

extern bool geckoinit;

/* Constants */
#define CONSOLE_XCOORD		260
#define CONSOLE_YCOORD		115
#define CONSOLE_WIDTH		340
#define CONSOLE_HEIGHT		218

FreeTypeGX *fontSystem=0;
FreeTypeGX *fontClock=0;
PartList partitions;

static void BootUpProblems()
{
    s32 ret2;

    // load main font from file, or default to built-in font
    fontSystem = new FreeTypeGX();
    fontSystem->loadFont(NULL, font_ttf, font_ttf_size, 0);
    fontSystem->setCompatibilityMode(FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE);

    GuiImageData bootimageData(gxlogo_png);
    GuiImage bootimage(&bootimageData);
    GuiText boottext(NULL, 20, (GXColor) {255, 255, 255, 255});
    boottext.SetPosition(200, 240-1.2*bootimage.GetHeight()/2+250);
    bootimage.SetPosition(320-1.2*bootimage.GetWidth()/2, 240-1.2*bootimage.GetHeight()/2);
    bootimage.SetScale(1.2);

    GuiImageData usbimageData(usbport_png);
    GuiImage usbimage(&usbimageData);
    usbimage.SetPosition(400,300);
    usbimage.SetScale(.7);
	usbimage.SetAlpha(200);

    time_t curtime;
	time_t endtime = time(0) + 30;
	do {
		ret2 = IOS_ReloadIOS(249);
		if (ret2 < 0) {
			ret2 = IOS_ReloadIOS(222);
			SDCard_Init(); 
			load_ehc_module();
			SDCard_deInit();
			if(ret2 <0) {
				boottext.SetText("ERROR: cIOS could not be loaded!");
				bootimage.Draw();
				boottext.Draw();
				Menu_Render();
				sleep(5);
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			}
		}

		ret2 = WBFS_Init(WBFS_DEVICE_USB);
		if (ret2 >= 0) {
			boottext.SetText("Loading...");
			bootimage.Draw();
			boottext.Draw();
			Menu_Render();
			break;
		}
		curtime = time(0);
		boottext.SetTextf("Waiting for your slow USB Device: %i secs...", int(endtime-curtime));
		while(curtime == time(0)) {
			boottext.Draw();
			bootimage.Draw();
			if (endtime-curtime<15)usbimage.Draw();
			Menu_Render();
		}
    } while((endtime-time(0)) > 0);

    if(ret2 < 0) {
        boottext.SetText("ERROR: USB device could not be loaded!");
        usbimage.Draw();
        bootimage.Draw();
        boottext.Draw();
        Menu_Render();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    ///delete font to load up custom if set
    if(fontSystem) {
        delete fontSystem;
        fontSystem = NULL;
    }
}

unsigned int *xfb = NULL;

void InitTextVideo () {
    VIDEO_Init();
	GXRModeObj *vmode = VIDEO_GetPreferredMode(NULL); // get default video mode

    // widescreen fix
    VIDEO_Configure (vmode);

    // Allocate the video buffers
    xfb = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));

    // A console is always useful while debugging
    console_init (xfb, 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);

    // Clear framebuffers etc.
    VIDEO_ClearFrameBuffer (vmode, xfb, COLOR_BLACK);
    VIDEO_SetNextFramebuffer (xfb);

    VIDEO_SetBlack (FALSE);
    VIDEO_Flush ();
    VIDEO_WaitVSync ();
    if (vmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync ();
}


int
main(int argc, char *argv[]) {
//	InitTextVideo();

	setlocale(LC_ALL, "en.UTF-8");
	geckoinit = InitGecko();
	gprintf("\x1b[2J");
	gprintf("------------------");
	gprintf("\nUSB Loader GX rev%s",GetRev());
	gprintf("\nmain(int argc, char *argv[])");
	
	printf("Starting up\n");

    s32 ret;
    bool startupproblem = false;

    bool bootDevice_found=false;
    if (argc >= 1) {
        if (!strncasecmp(argv[0], "usb:/", 5)) {
            strcpy(bootDevice, "USB:");
            bootDevice_found = true;
        } else if (!strncasecmp(argv[0], "sd:/", 4))
            bootDevice_found = true;
    }
	
	printf("Initializing controllers\n");
	
    /** PAD_Init has to be before InitVideo don't move that **/
    PAD_Init(); // initialize PAD/WPAD
	
	printf("Initialize USB (wake up)\n");
	
    USBDevice_Init();// seems enough to wake up some HDDs if they are in sleep mode when the loader starts (tested with WD MyPassport Essential 2.5")
	 
	printf("Reloading ios 249...");

    ret = IOS_ReloadIOS(249);
	
	printf("%d\n", ret);
	
    if (ret < 0) {
		printf("IOS 249 failed, reloading ios 222...");
        ret = IOS_ReloadIOS(222);
		printf("%d\n", ret);
        if(ret < 0) {
            printf("\n\tERROR: cIOS could not be loaded!\n");
            sleep(5);
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }
		printf("Initialize sd card\n");
		SDCard_Init(); 
		printf("Load ehc module\n");
		load_ehc_module();
		printf("deinit sd card\n");
		SDCard_deInit();
    }
	
	printf("Init wbfs...");
    ret = WBFS_Init(WBFS_DEVICE_USB);
	printf("%d\n", ret);

    if (ret < 0) {
		printf("You have issues with a slow disc, or a difficult disc\nReloading 222...");
        ret = IOS_ReloadIOS(222);
		printf("%d\n", ret);
        if(ret < 0) {
			printf("Sleeping for 4 seconds\n");
//			sleep(4);

            InitVideo(); // Initialise video
            Menu_Render();
            BootUpProblems();
            startupproblem = true;
            ret = 1;
        }
		printf("Initialize sd card\n");
		SDCard_Init(); 
		printf("Load ehc module\n");
		load_ehc_module();
		printf("deinit sd card\n");
		SDCard_deInit();

		printf("Initialize wbfs...");
        ret = WBFS_Init(WBFS_DEVICE_USB);
		printf("%d\n", ret);

        if(ret < 0) {
			printf("Sleeping for 4 seconds\n");
//			sleep(4);
            InitVideo(); // Initialise video
            Menu_Render();
            BootUpProblems();
            startupproblem = true;
            ret = 1;
        }
    }

	printf("Initialize sd card\n");
    SDCard_Init(); // mount SD for loading cfg's
	printf("Initialize usb device\n");
    USBDevice_Init(); // and mount USB:/
	gprintf("\n\tSD and USB Init OK");

    if (!bootDevice_found) {
		printf("Search for configuration file\n");

        //try USB
        //left in all the dol and elf files in this check in case this is the first time running the app and they dont have the config
        if (checkfile((char*) "USB:/config/GXglobal.cfg") || (checkfile((char*) "USB:/apps/usbloader_gx/boot.elf"))
                || checkfile((char*) "USB:/apps/usbloadergx/boot.dol") || (checkfile((char*) "USB:/apps/usbloadergx/boot.elf"))
                || checkfile((char*) "USB:/apps/usbloader_gx/boot.dol"))
            strcpy(bootDevice, "USB:");

		printf("Configuration file is on %s\n", bootDevice);
    }

    gettextCleanUp();
	printf("Loading configuration...");
    CFG_Load();
	printf("done\n");
	gprintf("\n\tbootDevice = %s",bootDevice);

    /* Load Custom IOS */	
    if (Settings.cios == ios222 && IOS_GetVersion() != 222) {
		printf("Reloading IOS to config setting (222)...");
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
		USBStorage_Deinit();
        ret = IOS_ReloadIOS(222);
		printf("%d\n", ret);
		SDCard_Init();
        load_ehc_module();
        if (ret < 0) {
			SDCard_deInit();
            Settings.cios = ios249;
            ret = IOS_ReloadIOS(249);
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
		WBFS_Init(WBFS_DEVICE_USB);
    } else if (Settings.cios == ios249 && IOS_GetVersion() != 249) {
		printf("Reloading IOS to config setting (249)...");
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
		USBStorage_Deinit();
        ret = IOS_ReloadIOS(249);
		printf("%d\n", ret);
        if (ret < 0) {
            Settings.cios = ios222;
            ret = IOS_ReloadIOS(222);
			SDCard_Init();
            load_ehc_module();
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
		WBFS_Init(WBFS_DEVICE_USB);
	}

//	Partition_GetList(&partitions);

    if (ret < 0) {
        printf("ERROR: cIOS could not be loaded!");
        sleep(5);
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }
	gprintf("\n\tcIOS = %u (Rev %u)",IOS_GetVersion(), IOS_GetRevision());
	printf("cIOS = %u (Rev %u)\n",IOS_GetVersion(), IOS_GetRevision());

//	printf("Sleeping for 5 seconds\n");
//	sleep(5);

    //! Init the rest of the System
    Sys_Init();
    Wpad_Init();
    if(!startupproblem)
        InitVideo();
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

