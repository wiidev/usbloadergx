#include <unistd.h>
#include "StartUpProcess.h"
#include "libwiigui/gui.h"
#include "video.h"
#include "audio.h"
#include "input.h"
#include "themes/CTheme.h"
#include "gecko.h"
#include "Controls/DeviceHandler.hpp"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "settings/CSettings.h"
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "usbloader/usbstorage2.h"
#include "sys.h"

StartUpProcess::StartUpProcess()
{
    //! Load default font for the next text outputs
    Theme::LoadFont("");

    background = new GuiImage(screenwidth, screenheight, (GXColor) {0, 0, 0, 255});

    GXImageData = Resources::GetImageData("gxlogo.png");
    GXImage = new GuiImage(GXImageData);
    GXImage->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
    GXImage->SetPosition(screenwidth/2, screenheight/2-50);

    titleTxt = new GuiText("Loading...", 24, (GXColor) {255, 255, 255, 255});
    titleTxt->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
    titleTxt->SetPosition(screenwidth/2, screenheight/2+30);

    messageTxt = new GuiText(" ", 22, (GXColor) {255, 255, 255, 255});
    messageTxt->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
    messageTxt->SetPosition(screenwidth/2, screenheight/2+60);
}

StartUpProcess::~StartUpProcess()
{
    delete background;
    delete GXImageData;
    delete GXImage;
    delete titleTxt;
    delete messageTxt;
}

void StartUpProcess::TextFade(int direction)
{
    if(direction > 0)
    {
        for(int i = 0; i < 255; i += direction)
        {
            messageTxt->SetAlpha(i);
            Draw();
        }
        messageTxt->SetAlpha(255);
        Draw();
    }
    else if(direction < 0)
    {
        for(int i = 255; i > 0; i += direction)
        {
            messageTxt->SetAlpha(i);
            Draw();
        }
        messageTxt->SetAlpha(0);
        Draw();
    }
}

void StartUpProcess::SetTextf(const char * format, ...)
{
	char * tmp = NULL;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
    {
        TextFade(-40);
        gprintf(tmp);
        messageTxt->SetText(tmp);
        TextFade(40);
	}
	va_end(va);

	if(tmp)
        free(tmp);
}

bool StartUpProcess::USBSpinUp()
{
    bool started = false;
    int retries = 400;
    // wait 10 sec for the USB to spin up...stupid slow ass HDD
    do
    {
        started = (__io_usbstorage2.startup() && __io_usbstorage2.isInserted());
        usleep(50000);

        if(retries < 400 && retries % 20 == 0)
        {
            messageTxt->SetTextf("Waiting for slow HDD: %i sec left\n", retries/20);
            Draw();
        }
    }
    while(!started && --retries > 0);

    return started;
}

bool StartUpProcess::Run()
{
    StartUpProcess Process;

    return Process.Execute();
}

bool StartUpProcess::Execute()
{
    //! Now we startup the GUI so no need for console prints. Output only to gecko.
    USBGeckoOutput();

    // Let's try loading some cIOS
    if (IosLoader::LoadAppCios() < 0)
    {
        titleTxt->SetText("WARNING!");
        messageTxt->SetMaxWidth(400, WRAP);
        messageTxt->SetText("USB Loader GX needs unstubbed cIOS 222 v4+ or 249 v9+. \
                             We cannot determine the versions on your system, since you have no patched ios 36 or 236 installed. \
                             Therefor, if loading of USB Loader GX fails, you probably have installed the 4.2 update, \
                             and you should go figure out how to get some cios action going on\n\tin your Wii. \
                             ERROR: No cIOS could be loaded. Exiting....");
        sleep(10);
        Sys_BackToLoader();
    }

    SetTextf("Initialize sd card\n");
    DeviceHandler::Instance()->MountSD();

    SetTextf("Initialize usb device\n");
    USBSpinUp();
    DeviceHandler::Instance()->MountAllUSB(false);

    SetTextf("Loading config files");
    gprintf("\tLoading config...%s\n", Settings.Load() ? "done" : "failed");
    gprintf("\tLoading language...%s\n", Settings.LoadLanguage(Settings.language_path, CONSOLE_DEFAULT) ? "done" : "failed");
    gprintf("\tLoading game settings...%s\n", GameSettings.Load(Settings.ConfigPath) ? "done" : "failed");
    gprintf("\tLoading game statistics...%s\n", GameStatistics.Load(Settings.ConfigPath) ? "done" : "failed");

    if(Settings.cios != IOS_GetVersion())
    {
        SetTextf("Loading cIOS %i\n", Settings.cios);

        DeviceHandler::Instance()->UnMountAll();

        // Loading now the cios setup in the settings
        IosLoader::LoadAppCios();

        SetTextf("Loaded cIOS %i R%i\n", IOS_GetVersion(), IOS_GetRevision());

        DeviceHandler::Instance()->MountAll();
    }
    else
        SetTextf("Loaded cIOS %i R%i\n", IOS_GetVersion(), IOS_GetRevision());

    gprintf("\tLoading font...%s\n", Theme::LoadFont(Settings.theme_path) ? "done" : "failed (using default)");
    gprintf("\tLoading theme...%s\n", Theme::Load(Settings.theme) ? "done" : "failed (using default)");

    //! Init the rest of the System
    Sys_Init();
    SetupPads();
    InitAudio();
    setlocale(LC_CTYPE, "C-UTF-8");
    setlocale(LC_MESSAGES, "C-UTF-8");

    return true;
}

void StartUpProcess::Draw()
{
    background->Draw();
    GXImage->Draw();
    titleTxt->Draw();
    messageTxt->Draw();
    Menu_Render();
}
