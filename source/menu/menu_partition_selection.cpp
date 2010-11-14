#include <unistd.h>

#include "menus.h"
#include "fatmounter.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/utils.h"
#include "usbloader/wbfs.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "themes/CTheme.h"

extern PartList partitions;

/****************************************************************************
 * SelectPartitionMenu
 ***************************************************************************/
int SelectPartitionMenu()
{
    bool ExitSelect = false;
    OptionList options;

    u32 cnt, counter = 0;
    int choice = -1;
    int ret = -1;

    //create the partitionlist
    for (cnt = 0; cnt < (u32) partitions.num; cnt++)
    {
        partitionEntry *entry = &partitions.pentry[cnt];

        /* Calculate size in gigabytes */
        f32 size = entry->size * (partitions.sector_size / GB_SIZE);

        if (size)
        {
            options.SetName(counter, "%s %d:", tr( "Partition" ), cnt + 1);
            options.SetValue(counter, "%.2fGB", size);
        }
        else
        {
            options.SetName(counter, "%s %d:", tr( "Partition" ), cnt + 1);
            options.SetValue(counter, tr( "Can't be formatted" ));
        }
        counter++;
    }

    GuiImageData btnpwroff(Resources::GetFile("wiimote_poweroff.png"), Resources::GetFileSize("wiimote_poweroff.png"));
    GuiImageData btnpwroffOver(Resources::GetFile("wiimote_poweroff_over.png"), Resources::GetFileSize("wiimote_poweroff_over.png"));
    GuiImageData btnhome(Resources::GetFile("menu_button.png"), Resources::GetFileSize("menu_button.png"));
    GuiImageData btnhomeOver(Resources::GetFile("menu_button_over.png"), Resources::GetFileSize("menu_button_over.png"));
    GuiImageData battery(Resources::GetFile("battery.png"), Resources::GetFileSize("battery.png"));
    GuiImageData batteryBar(Resources::GetFile("battery_bar.png"), Resources::GetFileSize("battery_bar.png"));
    GuiImageData batteryRed(Resources::GetFile("battery_red.png"), Resources::GetFileSize("battery_red.png"));
    GuiImageData batteryBarRed(Resources::GetFile("battery_bar_red.png"), Resources::GetFileSize("battery_bar_red.png"));

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    GuiImage poweroffBtnImg(&btnpwroff);
    GuiImage poweroffBtnImgOver(&btnpwroffOver);
    poweroffBtnImg.SetWidescreen(Settings.widescreen);
    poweroffBtnImgOver.SetWidescreen(Settings.widescreen);
    GuiButton poweroffBtn(&poweroffBtnImg, &poweroffBtnImgOver, 0, 3, Theme.power_x, Theme.power_y, &trigA,
            btnSoundOver, btnSoundClick2, 1);
    GuiImage exitBtnImg(&btnhome);
    GuiImage exitBtnImgOver(&btnhomeOver);
    exitBtnImg.SetWidescreen(Settings.widescreen);
    exitBtnImgOver.SetWidescreen(Settings.widescreen);
    GuiButton exitBtn(&exitBtnImg, &exitBtnImgOver, 0, 3, Theme.home_x, Theme.home_y, &trigA, btnSoundOver, btnSoundClick2,
            1);
    exitBtn.SetTrigger(&trigHome);

    GuiCustomOptionBrowser optionBrowser(396, 280, &options, "bg_options_settings.png");
    optionBrowser.SetPosition(0, 40);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&poweroffBtn);
    w.Append(&exitBtn);

    mainWindow->Append(&w);
    mainWindow->Append(&optionBrowser);

    ResumeGui();

    while (!ExitSelect)
    {
        VIDEO_WaitVSync();

        if (shutdown)
            Sys_Shutdown();
        if (reset)
            Sys_Reboot();

        ret = optionBrowser.GetClickedOption();

        if (ret >= 0)
        {
            partitionEntry *entry = &partitions.pentry[ret];
            if (entry->size)
            {
                choice = ret;
                ExitSelect = true;
            }
        }

        if (poweroffBtn.GetState() == STATE_CLICKED)
        {
            choice = WindowPrompt(tr( "Shutdown System" ), tr( "Are you sure?" ), tr( "Yes" ), tr( "No" ));
            if (choice == 1)
                Sys_Shutdown();

        }
        else if (exitBtn.GetState() == STATE_CLICKED)
        {
            choice = WindowPrompt(tr( "Return to Wii Menu" ), tr( "Are you sure?" ), tr( "Yes" ), tr( "No" ));
            if (choice == 1)
                Sys_LoadMenu();
        }
    }

    HaltGui();

    mainWindow->Remove(&optionBrowser);
    mainWindow->Remove(&w);
    ResumeGui();

    return choice;
}

