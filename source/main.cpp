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
#include <ogc/libversion.h>
#include <wiiuse/wpad.h>
//#include <debug.h>
extern "C" {
extern void __exception_setreload(int t);
}

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
#include "xml/xml.h"
#include "settings/newtitles.h"
#include "menu/menus.h"

extern bool geckoinit;
extern bool textVideoInit;
extern char headlessID[8];
PartList partitions;

/* Constants */
#define CONSOLE_XCOORD		260
#define CONSOLE_YCOORD		115
#define CONSOLE_WIDTH		340
#define CONSOLE_HEIGHT		218

FreeTypeGX *fontSystem=0;
FreeTypeGX *fontClock=0;

void LoadHeadlessID(const char * ID)
{
    InitTextVideo();
    strncpy(headlessID, ID, sizeof(headlessID));
    InitCheckThread();
    time_t endtime = time(0) + 30;
    time_t curtime;
    printf("\tWaiting for USB-Device:\n");
    while(checkthreadState != 1)
    {
        usleep(100);
        curtime = time(0);
        printf("\t\t%d\n", int(endtime-curtime));
        if(endtime == curtime)
        {
            printf("\n\tDevice could not be loaded.\n\tExiting...\n");
            sleep(5);
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }
    }
    mountMethod = 0;
    checkthreadState = 0;
    ExitCheckThread();
	CloseXMLDatabase();
    NewTitles::DestroyInstance();
	ShutdownAudio();
    StopGX();
	gettextCleanUp();
    menuBootgame(headlessID);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "en.UTF-8");
	geckoinit = InitGecko();

	if (hbcStubAvailable() || geckoinit)
		InitTextVideo();

	__exception_setreload(5);//auto reset code dump nobody gives us codedump info anyways.

	gprintf("\n\n------------------");
	gprintf("\nUSB Loader GX rev%s linked with %s",GetRev(), _V_STRING);
	gprintf("\nmain(%d", argc);
	for (int i=0;i<argc;i++)
		gprintf(", %s",argv[i]?argv[i]:"<NULL>");
    gprintf(")");

    // This part is added, because we need a identify patched ios
	//printf("\n\tReloading into ios 236");
	if (IOS_ReloadIOSsafe(236) < 0)
		IOS_ReloadIOSsafe(36);

	printf("\n\tStarting up");

	MEM2_init(36); // Initialize 36 MB
	MEM2_takeBigOnes(true);

	printf("\n\tInitialize USB (wake up)");
    USBDevice_Init();// seems enough to wake up some HDDs if they are in sleep mode when the loader starts (tested with WD MyPassport Essential 2.5")
    USBDevice_deInit();

    s32 ret;

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

    ret = CheckForCIOS();

	printf("\n\tInitialize sd card");
    SDCard_Init(); // mount SD for loading cfg's
	printf("\n\tInitialize usb device");
    USBDevice_Init(); // and mount USB:/

    if (!bootDevice_found)
    {
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

	LoadAppCIOS();
    printf("\n\tcIOS = %u (Rev %u)",IOS_GetVersion(), IOS_GetRevision());

	//if a ID was passed via args copy it and try to boot it after the partition is mounted
	//its not really a headless mode.  more like hairless.
	if (argc > 1 && argv[1])
	{
		if (strlen(argv[1]) == 6)
            LoadHeadlessID(argv[1]);
	}

    //! Init the rest of the System
    Sys_Init();
    Wpad_Init();
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

	gprintf("\n\tEnd of Main()");
    InitGUIThreads();
    MainMenu(MENU_DISCLIST);

    return 0;
}
