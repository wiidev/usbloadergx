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

/* Constants */
#define CONSOLE_XCOORD		260
#define CONSOLE_YCOORD		115
#define CONSOLE_WIDTH		340
#define CONSOLE_HEIGHT		218

FreeTypeGX *fontSystem=0;
FreeTypeGX *fontClock=0;

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

    u8 i = 30;
    while (i > 0) {

            ret2 = IOS_ReloadIOS(249);
            if (ret2 < 0) {
                ret2 = IOS_ReloadIOS(222);
                load_ehc_module();
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

            boottext.SetTextf("Waiting for your slow USB Device: %i secs...", i);
            boottext.Draw();
            bootimage.Draw();
            Menu_Render();

            sleep(1);
            i--;
    }

    if(ret2 < 0) {
        boottext.SetText("ERROR: USB device could not be loaded!");
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


int
main(int argc, char *argv[]) {

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

    /** PAD_Init has to be before InitVideo don't move that **/
    PAD_Init(); // initialize PAD/WPAD

    ret = IOS_ReloadIOS(249);

    if (ret < 0) {
        ret = IOS_ReloadIOS(222);
        load_ehc_module();
        if(ret <0) {
            printf("\n\tERROR: cIOS could not be loaded!\n");
            sleep(5);
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }
    }

    ret = WBFS_Init(WBFS_DEVICE_USB);

    if (ret < 0) {
        ret = IOS_ReloadIOS(222);
        load_ehc_module();

        if(ret < 0) {
            InitVideo(); // Initialise video
            Menu_Render();
            BootUpProblems();
            startupproblem = true;
            ret = 1;
        }

        ret = WBFS_Init(WBFS_DEVICE_USB);

        if(ret < 0) {
            InitVideo(); // Initialise video
            Menu_Render();
            BootUpProblems();
            startupproblem = true;
            ret = 1;
        }
    }

    SDCard_Init(); // mount SD for loading cfg's
    USBDevice_Init(); // and mount USB:/

    if (!bootDevice_found) {
        //try USB
        //left in all the dol and elf files in this check in case this is the first time running the app and they dont have the config
        if (checkfile((char*) "USB:/config/GXglobal.cfg") || (checkfile((char*) "USB:/apps/usbloader_gx/boot.elf"))
                || checkfile((char*) "USB:/apps/usbloadergx/boot.dol") || (checkfile((char*) "USB:/apps/usbloadergx/boot.elf"))
                || checkfile((char*) "USB:/apps/usbloader_gx/boot.dol"))
            strcpy(bootDevice, "USB:");
    }

    gettextCleanUp();
    CFG_Load();

    /* Load Custom IOS */
    if (Settings.cios == ios222 && IOS_GetVersion() != 222) {
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
        ret = IOS_ReloadIOS(222);
        load_ehc_module();
        if (ret < 0) {
            Settings.cios = ios249;
            ret = IOS_ReloadIOS(249);
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
    } else if (Settings.cios == ios249 && IOS_GetVersion() != 249) {
        SDCard_deInit();// unmount SD for reloading IOS
        USBDevice_deInit();// unmount USB for reloading IOS
        ret = IOS_ReloadIOS(249);
        if (ret < 0) {
            Settings.cios = ios222;
            ret = IOS_ReloadIOS(222);
            load_ehc_module();
        }
        SDCard_Init(); // now mount SD:/
        USBDevice_Init(); // and mount USB:/
    }

    if (ret < 0) {
        printf("ERROR: cIOS could not be loaded!");
        sleep(5);
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
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

