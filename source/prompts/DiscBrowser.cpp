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

int autoSelectDol(const char *id, bool force)
{
    gprintf("\nautoSelectDol() started");

    char id4[10];
    sprintf(id4, "%c%c%c%c", id[0], id[1], id[2], id[3]);

    ////// games that can be forced (always need alt dol)

    //Boogie
    if (strcmp(id, "RBOP69") == 0) return 675;//previous value was 657
    if (strcmp(id, "RBOE69") == 0) return 675;//starstremr

    //Fifa 08
    if (strcmp(id, "RF8E69") == 0) return 439;//from isostar
    if (strcmp(id, "RF8P69") == 0) return 463;//from isostar
    if (strcmp(id, "RF8X69") == 0) return 464;//from isostar

    //Madden NFL07
    if (strcmp(id, "RMDP69") == 0) return 39;//from isostar

    //Madden NFL08
    if (strcmp(id, "RNFP69") == 0) return 1079;//from isostar

    //Medal of Honor: Heroes 2
    if (strcmp(id, "RM2X69") == 0) return 601;//dj_skual
    if (strcmp(id, "RM2P69") == 0) return 517;//MZottel
    if (strcmp(id, "RM2E69") == 0) return 492;//Old8oy

    //Mortal Kombat
    if (strcmp(id, "RKMP5D") == 0) return 290;//from isostar
    if (strcmp(id, "RKME5D") == 0) return 290;//starstremr

    //NBA 08
    if (strcmp(id, "RNBX69") == 0) return 964;//from isostar

    //Pangya! Golf with Style
    if (strcmp(id, "RPYP9B") == 0) return 12490;//from isostar

    //Redsteel
    if (strcmp(id, "REDP41") == 0) return 1957;//from isostar
    if (strcmp(id, "REDE41") == 0) return 1957;//starstremr

    //SSX
    if (strcmp(id, "RSXP69") == 0) return 377;//previous value was 337
    if (strcmp(id, "RSXE69") == 0) return 377;//previous value was 337

    //Madden NFL 07
    if (strcmp(id, "RMDE69") == 0) return 39; //from TwEbErIs

    //Madden NFL 08
    if (strcmp(id, "RNFE69") == 0) return 1079; //from TwEbErIs

    //Super Swing Golf
    if (strcmp(id, "RPYE9B") == 0) return 10617; //from TwEbErIs

    //NBA Live 08
    if (strcmp(id, "RNBE69") == 0) return 936; //from TwEbErIs

    ///// games that can't be forced (alt dol is not always needed)
    if (!force)
    {
        //Grand Slam Tennis
        if (strcmp(id, "R5TP69") == 0) return 1493;//from isostar
        if (strcmp(id, "R5TE69") == 0) return 1493;//starstremr

        //Medal of Honor Heroes
        if (strcmp(id, "RMZX69") == 0) return 492;//from isostar
        if (strcmp(id, "RMZP69") == 0) return 492;//from isostar
        if (strcmp(id, "RMZE69") == 0) return 492;//starstremr

        //Tiger Woods 10
        if (strcmp(id, "R9OP69") == 0) return 1991;//from isostar
        if (strcmp(id, "R9OE69") == 0) return 1973;//starstremr

        //Virtual Tennis 2009
        if (strcmp(id, "RVUP8P") == 0) return 16426;//from isostar
        if (strcmp(id, "RVUE8P") == 0) return 16405;//from isostar

        //Wii Sports Resort
        if (strcmp(id, "RZTP01") == 0) return 952;//from isostar
        if (strcmp(id, "RZTE01") == 0) return 674;//from starstremr
    }

    return -1;
}

int autoSelectDolMenu(const char *id, bool force)
{

    //Indiana Jones and the Staff of Kings (Fate of Atlantis)
    if (strcmp(id, "RJ8E64") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Fate of Atlantis", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 8; //from starstremr
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }
    if (strcmp(id, "RJ8P64") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Fate of Atlantis", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 8; //from isostar
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }

    //Metal Slug Anthology (Metal Slug 6)
    if (strcmp(id, "RMLEH4") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metal Slug 6", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 54; //from lustar
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }
    if (strcmp(id, "RMLP7U") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metal Slug 6", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 56; //from isostar
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }

    //Metroid Prime Trilogy
    if (strcmp(id, "R3ME01") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metroid Prime", "Metroid Prime 2", "Metroid Prime 3", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 780;
                break;
            case 2:
                choice = 781;
                break;
            case 3:
                choice = 782;
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }
    if (strcmp(id, "R3MP01") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metroid Prime", "Metroid Prime 2", "Metroid Prime 3", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 782;
                break;
            case 2:
                choice = 783;
                break;
            case 3:
                choice = 784;
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }

    //Rampage: Total Destruction (M1.dol=Rampage, jarvos.dol=Rampage World Tour)
    if (strcmp(id, "RPGP5D") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Rampage", "World Tour", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 369; //from Ramzee
                break;
            case 2:
                choice = 368; //from Ramzee
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }

    //The House Of The Dead 2 & 3 Return (only to play 2)
    if (strcmp(id, "RHDE8P") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "HotD 2", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 149; //from starstremr
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }
    if (strcmp(id, "RHDP8P") == 0)
    {
        int choice = WindowPrompt(tr( "Select a DOL" ), 0, "HotD 2", tr( "Cancel" ));
        switch (choice)
        {
            case 1:
                choice = 149; //from isostar
                break;
            default: // no alt dol
                choice = 0;
                break;
        }
        return choice;
    }

    return -1;
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
