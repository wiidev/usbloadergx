/****************************************************************************
 * DiscBrowser
 * USB Loader GX 2009
 *
 * DiscBrowser.h
 ***************************************************************************/
#include <unistd.h>
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "menu/menus.h"
#include "usbloader/disc.h"
#include "usbloader/fstfile.h"
#include "usbloader/wdvd.h"
#include "usbloader/wbfs.h"
#include "patches/dvd_broadway.h"
#include "libs/libwbfs/libwbfs.h"
#include "libs/libwbfs/wiidisc.h"
#include "main.h"
#include "sys.h"
#include "settings/GameTitles.h"
#include "themes/CTheme.h"
#include "memory/memory.h"
#include "gecko.h"

/********************************************************************************
 *Disk Browser
 *********************************************************************************/
int DiscBrowse(const char * GameID, char * alternatedname, int alternatedname_size)
{
    gprintf("\nDiscBrowser() started");
    bool exit = false;
    int ret = -1, choice;

    HaltGui();

    gprintf("WBFS_OpenDisc\n");
    wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) GameID);
    if (!disc)
    {
        ResumeGui();
        WindowPrompt(tr( "ERROR:" ), tr( "Could not open Disc" ), tr( "OK" ));
        return ret;
    }
    gprintf("wd_open_disc\n");
    wiidisc_t *wdisc = wd_open_disc((int(*)(void *, u32, u32, void *)) wbfs_disc_read, disc);
    if (!wdisc)
    {
        ResumeGui();
        WindowPrompt(tr( "ERROR:" ), tr( "Could not open Disc" ), tr( "OK" ));
        return ret;
    }

    gprintf("wd_get_fst\n");
    FST_ENTRY * fstbuffer = (FST_ENTRY *) wd_get_fst(wdisc, ONLY_GAME_PARTITION);
    if (!fstbuffer)
    {
        ResumeGui();
        WindowPrompt(tr( "ERROR:" ), tr( "Not enough free memory." ), tr( "OK" ));
        return -1;
    }

    gprintf("wd_close_disc\n");
    wd_close_disc(wdisc);
    gprintf("WBFS_CloseDisc\n");
    WBFS_CloseDisc(disc);

    gprintf("options\n");
    OptionList options;

    for (u32 i = 0, position = 0; i < fstbuffer[0].filelen; i++)
    {
        //don't add files that aren't .dol to the list
        const char * filename = fstfiles(fstbuffer, i);
        const char * fileext = NULL;

        if(filename)
            fileext = strrchr(filename, '.');

        if (fileext && strcasecmp(fileext, ".dol") == 0)
        {
            options.SetName(position, "%s %03i", tr("Offset"), i);
            options.SetValue(position, filename);
            position++;
        }
    }

    free(fstbuffer);

    gprintf("\n%i alt dols found", options.GetLength()+1);
    if (options.GetLength() <= 0)
    {
        WindowPrompt(tr( "ERROR" ), tr( "No DOL file found on disc." ), tr( "OK" ));
        return ret;
    }

    GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText titleTxt(GameTitles.GetTitle(GameID), 28, ( GXColor ) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(12, 40);
    titleTxt.SetMaxWidth(356, SCROLL_HORIZONTAL);

    GuiImage settingsbackground(&settingsbg);
    GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
    settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    settingsbackgroundbtn.SetPosition(0, 0);
    settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText cancelBtnTxt(tr( "Back" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows text color"));
    cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
    GuiImage cancelBtnImg(&btnOutline);
    if (Settings.wsprompt == ON)
    {
        cancelBtnTxt.SetWidescreen(Settings.widescreen);
        cancelBtnImg.SetWidescreen(Settings.widescreen);
    }
    GuiButton cancelBtn(&cancelBtnImg, &cancelBtnImg, 2, 3, 180, 400, &trigA, btnSoundOver, btnSoundClick2, 1);
    cancelBtn.SetScale(0.9);
    cancelBtn.SetLabel(&cancelBtnTxt);
    cancelBtn.SetTrigger(&trigB);

    GuiCustomOptionBrowser optionBrowser3(396, 280, &options, "bg_options_settings.png");
    optionBrowser3.SetPosition(0, 90);
    optionBrowser3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&settingsbackgroundbtn);
    w.Append(&titleTxt);
    w.Append(&cancelBtn);
    w.Append(&optionBrowser3);

    mainWindow->Append(&w);

    ResumeGui();
    while (!exit)
    {
        usleep(100);

        if (shutdown)
            Sys_Shutdown();
        if (reset)
            Sys_Reboot();

        ret = optionBrowser3.GetClickedOption();

        if (ret >= 0)
        {
            choice = WindowPrompt(options.GetValue(ret), tr( "Load this DOL as alternate DOL?" ), tr( "OK" ), tr( "Cancel" ));
            if (choice)
            {
                snprintf(alternatedname, alternatedname_size, options.GetValue(ret));
                const char * offset = options.GetName(ret);
                if(offset)
                    ret = atoi(offset+strlen("Offset ")); //doloffset
                else
                    ret = -1; // weird problem
                exit = true;
            }
        }

        if (cancelBtn.GetState() == STATE_CLICKED)
        {
            exit = true;
        }
    }

    HaltGui();
    mainWindow->Remove(&w);
    ResumeGui();

    return ret;
}

/********************************************************************************
 * Mount a DVD, get the type and ID.
 *********************************************************************************/
u8 DiscMount(struct discHdr * header)
{
    gprintf("\nDiscMount() ");
    u8 * g_diskID = (u8 *) 0x80000000;
    int ret;
    HaltGui();

    u8 tmpBuff[0x60];
    memcpy(tmpBuff, g_diskID, 0x60); // Make a backup of the first 96 bytes at 0x80000000

    Disc_SetUSB(NULL);
    ret = WDVD_Reset();
    if(ret < 0)
        return 0;

    ret = WDVD_ReadDiskId(g_diskID);
    if(ret < 0)
        return 0;

    ret = WDVD_UnencryptedRead(g_diskID, 0x60, 0x00);
    if(ret < 0)
        return 0;

    memcpy(header, g_diskID, 0x60);
    memcpy(g_diskID, tmpBuff, 0x60); // Put the backup back, or games won't load

    ResumeGui();

    return (header->magic == 0x5D1C9EA3) ? 1 : 2; // Don't check gamecube magic (0xC2339F3D)
}
