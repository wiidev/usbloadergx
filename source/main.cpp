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

#include <debug.h>
extern "C" { //not sure if this is in the libogc that the buildbot is using so it isnt used yet
extern void __exception_setreload(int t);
}

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <di/di.h>
#include <sys/iosupport.h>

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
#include "wad/title.h"
#include "usbloader/partition_usbloader.h"
#include "usbloader/usbstorage.h"
#include "memory/mem2.h"
#include "lstub.h"

extern bool geckoinit;
extern bool textVideoInit;
extern char headlessID[8];

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
		ret2 = IOS_ReloadIOSsafe(249);
		if (ret2 < 0) {
			ret2 = IOS_ReloadIOSsafe(222);
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

	if (textVideoInit)return;
	
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
		
	//send console output to the gecko
	if (geckoinit)CON_EnableGecko(1, true);
	textVideoInit = true;
	
}

int
main(int argc, char *argv[]) {
	if (hbcStubAvailable()) {
		InitTextVideo();
	}
	
//	DEBUG_Init(GDBSTUB_DEVICE_USB, 1);
//_break();
	
//	__exception_setreload(5);//auto reset is code dump nobody gives us codedump info anyways.
	setlocale(LC_ALL, "en.UTF-8");
	geckoinit = InitGecko();
	if (geckoinit)InitTextVideo();
	gprintf("\x1b[2J");
	gprintf("------------------");
	gprintf("\nUSB Loader GX rev%s",GetRev());
	gprintf("\nmain(%d", argc);
	for (int i=0;i<argc;i++)
		gprintf(", %s",argv[i]?argv[i]:"<NULL>");
	gprintf(")");
	
	// This part is added, because we need a identify patched ios
	printf("\n\tReloading into ios 236");
	if (IOS_ReloadIOSsafe(236) < 0) {
		printf("\n\tIOS 236 not found, reloading into 36");
		IOS_ReloadIOSsafe(36);
	}
	
	printf("\n\tStarting up");
	
	MEM2_init(36); // Initialize 36 MB
	MEM2_takeBigOnes(true);

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
	
	printf("\n\tInitializing controllers");
	
    /** PAD_Init has to be before InitVideo don't move that **/
    PAD_Init(); // initialize PAD/WPAD
	
	printf("\n\tInitialize USB (wake up)");
	
    USBDevice_Init();// seems enough to wake up some HDDs if they are in sleep mode when the loader starts (tested with WD MyPassport Essential 2.5")
	 
	gprintf("\n\tChecking for stub IOS");
	ios222rev = getIOSrev(0x00000001000000dell);
	ios249rev = getIOSrev(0x00000001000000f9ll);
	
	//if we don't like either of the cIOS then scram
	if (!(ios222rev==4 || (ios249rev>=9 && ios249rev<65280)))
	{
		InitTextVideo();
		printf("\x1b[2J");
		if ((ios222rev < 0 && ios222rev != WII_EINSTALL) || (ios249rev < 0 && ios249rev != WII_EINSTALL)) {
			printf("\n\n\n\tWARNING!");
			printf("\n\tUSB Loader GX needs unstubbed cIOS 222 v4 or 249 v9+");
			printf("\n\n\tWe cannot determine the versions on your system,\n\tsince you have no patched ios 36 or 236 installed.");
			printf("\n\tTherefor, if loading of USB Loader GX fails, you\n\tprobably have installed the 4.2 update,");
			printf("\n\tand you should go figure out how to get some cios action going on\n\tin your Wii.");
			printf("\n\n\tThis message will show every time.");
			sleep(5);
		} else {
			printf("\n\n\n\tERROR!");
			printf("\n\tUSB Loader GX needs unstubbed cIOS 222 v4 or 249 v9+");
			printf("\n\n\tI found \n\t\t222 = %d%s",ios222rev,ios222rev==65280?" (Stubbed by 4.2 update)":"");
			printf("\n\t\t249 = %d%s",ios249rev,ios249rev==65280?" (Stubbed by 4.2 update)":"");
			printf("\n\n\tGo figure out how to get some cIOS action going on\n\tin your Wii and come back and see me.");
			
			sleep(15);
			printf("\n\n\tBye");
			
			USBDevice_deInit();
			exit(0);
		}
	}

	printf("\n\tReloading ios 249...");
    ret = IOS_ReloadIOSsafe(249);
	
	printf("%d", ret);
	
    if (ret < 0) {
		printf("\n\tIOS 249 failed, reloading ios 222...");
        ret = IOS_ReloadIOSsafe(222);
		printf("%d", ret);
		
		if (ret < 0) {
			printf("\n\tIOS 222 failed, reloading ios 250...");
			ret = IOS_ReloadIOSsafe(250);
			printf("%d", ret);
		
			if(ret < 0) {
				printf("\n\tIOS 250 failed, reloading ios 223...");
				ret = IOS_ReloadIOSsafe(223);
				printf("%d", ret);
				
				if (ret < 0) {
					printf("\n\tERROR: cIOS could not be loaded!\n");
					sleep(5);
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
				}
			}
		}
		printf("\n\tInitialize sd card");
		SDCard_Init(); 
		printf("\n\tLoad ehc module");
		load_ehc_module();
		printf("\n\tdeinit sd card");
		SDCard_deInit();
    }
	
	printf("\n\tInit wbfs...");
    ret = WBFS_Init(WBFS_DEVICE_USB);
	printf("%d", ret);

    if (ret < 0) {
		printf("\n\tYou have issues with a slow disc, or a difficult disc\n\tReloading 222...");
        ret = IOS_ReloadIOSsafe(222);
		printf("%d", ret);
        if(ret < 0) {
			printf("\n\tSleeping for 4 seconds");
//			sleep(4);

            InitVideo(); // Initialise video
            Menu_Render();
            BootUpProblems();
            startupproblem = true;
            ret = 1;
        }
		printf("\n\tInitialize sd card");
		SDCard_Init(); 
		printf("\n\tLoad ehc module");
		load_ehc_module();
		printf("\n\tdeinit sd card");
		SDCard_deInit();

		printf("\n\tInitialize wbfs...");
        ret = WBFS_Init(WBFS_DEVICE_USB);
		printf("%d", ret);

        if(ret < 0) {
			printf("\n\tSleeping for 4 seconds");
//			sleep(4);
            InitVideo(); // Initialise video
            Menu_Render();
            BootUpProblems();
            startupproblem = true;
            ret = 1;
        }
    }

	printf("\n\tInitialize sd card");
    SDCard_Init(); // mount SD for loading cfg's
	printf("\n\tInitialize usb device");
    USBDevice_Init(); // and mount USB:/
	gprintf("\n\tSD and USB Init OK");

    if (!bootDevice_found) {
		printf("\n\tSearch for configuration file");

        //try USB
        //left in all the dol and elf files in this check in case this is the first time running the app and they dont have the config
        if (checkfile((char*) "USB:/config/GXglobal.cfg") || (checkfile((char*) "USB:/apps/usbloader_gx/boot.elf"))
                || checkfile((char*) "USB:/apps/usbloadergx/boot.dol") || (checkfile((char*) "USB:/apps/usbloadergx/boot.elf"))
                || checkfile((char*) "USB:/apps/usbloader_gx/boot.dol"))
            strcpy(bootDevice, "USB:");

		printf("\n\tConfiguration file is on %s", bootDevice);
    }

    gettextCleanUp();
	printf("\n\tLoading configuration...");
    CFG_Load();
	printf("done");
//	gprintf("\n\tbootDevice = %s",bootDevice);

    /* Load Custom IOS */	
    if ((Settings.cios == ios222 && IOS_GetVersion() != 222) ||
        (Settings.cios == ios223 && IOS_GetVersion() != 223)) {
		printf("\n\tReloading IOS to config setting (%d)...", ios222 ? 222 : 223);
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
		USBStorage_Deinit();
        ret = IOS_ReloadIOSsafe(ios222 ? 222 : 223);
		printf("%d", ret);
		SDCard_Init();
        load_ehc_module();
        if (ret < 0) {
			SDCard_deInit();
            Settings.cios = ios249;
            ret = IOS_ReloadIOSsafe(249);
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
		WBFS_Init(WBFS_DEVICE_USB);
    } else if ((Settings.cios == ios249 && IOS_GetVersion() != 249) ||
				(Settings.cios == ios250 && IOS_GetVersion() != 250)) {

		printf("\n\tReloading IOS to config setting (%d)...", ios249 ? 249 : 250);
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
		USBStorage_Deinit();
        ret = IOS_ReloadIOSsafe(ios249 ? 249 : 250);
		printf("%d", ret);
        if (ret < 0) {
            Settings.cios = ios222;
            ret = IOS_ReloadIOSsafe(222);
			SDCard_Init();
            load_ehc_module();
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
		WBFS_Init(WBFS_DEVICE_USB);
	}

//	Partition_GetList(&partitions);

    if (ret < 0) {
        printf("\nERROR: cIOS could not be loaded!");
        sleep(5);
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }
	//gprintf("\n\tcIOS = %u (Rev %u)",IOS_GetVersion(), IOS_GetRevision());//don't need gprintf if sending console shit to gecko, too
	printf("\n\tcIOS = %u (Rev %u)",IOS_GetVersion(), IOS_GetRevision());

//	printf("Sleeping for 5 seconds\n");
//	sleep(5);

	//if a ID was passed via args copy it and try to boot it after the partition is mounted
	//its not really a headless mode.  more like hairless.
	if (argc > 1 && argv[1])
	{
		strncpy(headlessID, argv[1], sizeof(headlessID));
	}

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

