#include <unistd.h>
#include "StartUpProcess.h"
#include "GUI/gui.h"
#include "video.h"
#include "audio.h"
#include "input.h"
#include "themes/CTheme.h"
#include "gecko.h"
#include "Controls/DeviceHandler.hpp"
#include "wad/nandtitle.h"
#include "SystemMenu/SystemMenuResources.h"
#include "system/IosLoader.h"
#include "utils/timer.h"
#include "settings/CSettings.h"
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/CGameCategories.hpp"
#include "settings/GameTitles.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/MountGamePartition.h"
#include "usbloader/GameBooter.hpp"
#include "usbloader/GameList.h"
#include "utils/tools.h"
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

	cancelTxt = new GuiText("Press B to cancel", 18, (GXColor) {255, 255, 255, 255});
	cancelTxt->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	cancelTxt->SetPosition(screenwidth/2, screenheight/2+90);

	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	cancelBtn = new GuiButton(0, 0);
	cancelBtn->SetTrigger(trigB);

	drawCancel = false;
}

StartUpProcess::~StartUpProcess()
{
	delete background;
	delete GXImageData;
	delete GXImage;
	delete titleTxt;
	delete messageTxt;
	delete cancelTxt;
	delete cancelBtn;
	delete trigB;
}

int StartUpProcess::ParseArguments(int argc, char *argv[])
{
	int quickBoot = -1;

	//! The arguments override
	for(int i = 0; i < argc; ++i)
	{
		if(!argv[i]) continue;

		gprintf("Boot argument %i: %s\n", i+1, argv[i]);

		char *ptr = strcasestr(argv[i], "-ios=");
		if(ptr)
			Settings.cios = LIMIT(atoi(ptr+strlen("-ios=")), 200, 255);

		ptr = strcasestr(argv[i], "-usbport=");
		if(ptr)
		{
			Settings.USBPort = LIMIT(atoi(ptr+strlen("-usbport=")), 0, 2);
		}

		if(strlen(argv[i]) == 6 && strchr(argv[i], '=') == 0 && strchr(argv[i], '-') == 0)
			quickBoot = i;
	}

	return quickBoot;
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
	drawCancel = true;
	bool started = false;
	const DISC_INTERFACE * handle = Settings.USBPort == 1 ? DeviceHandler::GetUSB1Interface() : DeviceHandler::GetUSB0Interface();
	Timer countDown;
	// wait 20 sec for the USB to spin up...stupid slow ass HDD
	do
	{
		started = (handle->startup() && handle->isInserted());
		if(started)
			break;


		UpdatePads();
		for(int i = 0; i < 4; ++i)
			cancelBtn->Update(&userInput[i]);

		if(cancelBtn->GetState() == STATE_CLICKED)
			break;

		messageTxt->SetTextf("Waiting for HDD: %i sec left\n", 20-(int)countDown.elapsed());
		Draw();
		usleep(50000);
	}
	while(countDown.elapsed() < 20.f);

	drawCancel = false;

	return started;
}

int StartUpProcess::Run(int argc, char *argv[])
{
	int quickGameBoot = ParseArguments(argc, argv);

	StartUpProcess Process;

	int ret = Process.Execute();

	if(quickGameBoot != -1)
		return QuickGameBoot(argv[quickGameBoot]);

	return ret;
}

int StartUpProcess::Execute()
{
	Settings.EntryIOS = IOS_GetVersion();

	// Initialize to read the system menu resources
	ISFS_Initialize();

	SetTextf("Loading system menu resources\n");
	SystemMenuResources::Instance()->Init();

	// Check MIOS version
	SetTextf("Checking installed MIOS... ");
	IosLoader::GetMIOSInfo();

	// Deinitialize ISFS
	ISFS_Deinitialize();

	// Reload app cios
	SetTextf("Loading application cIOS\n");

	if(IosLoader::LoadAppCios() < 0)
	{
		SetTextf("Failed loading any cIOS. USB Loader GX requires at least cIOS 222 or 245-250 to work properly.\n");
		sleep(5);
		// We can allow now operation without cIOS in channel mode with AHPPROT
		// Sys_BackToLoader();
	}

	SetupPads();

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

		// Unmount devices
		DeviceHandler::DestroyInstance();
		USBStorage2_Deinit();

		// Shut down pads
		WPAD_Shutdown();

		// Loading now the cios setup in the settings
		IosLoader::LoadAppCios();

		SetTextf("Reloading into cIOS %i R%i\n", IOS_GetVersion(), IOS_GetRevision());

		// Re-Mount devices
		DeviceHandler::Instance()->MountSD();
		USBSpinUp();
		DeviceHandler::Instance()->MountAllUSB(false);

		// Start pads again
		SetupPads();
	}

	if(!IosLoader::IsHermesIOS() && !IosLoader::IsD2X())
	{
		Settings.USBPort = 0;
	}
	else if(Settings.USBPort == 1 && USBStorage2_GetPort() != Settings.USBPort)
	{
		SetTextf("Changing USB Port to %i\n", Settings.USBPort);
		DeviceHandler::Instance()->UnMountAllUSB();
		DeviceHandler::Instance()->MountAllUSB();
	}
	else if(Settings.USBPort == 2)
	{
		SetTextf("Mounting USB Port to 1\n");
		DeviceHandler::Instance()->MountUSBPort1();
	}

	// We only initialize once for the whole session
	ISFS_Initialize();

	SetTextf("Loading resources\n");
	// Do not allow banner grid mode without AHBPROT
	// this function does nothing if it was already initiated before
	if(   !SystemMenuResources::Instance()->IsLoaded() && !SystemMenuResources::Instance()->Init()
		&& Settings.gameDisplay == BANNERGRID_MODE)
	{
		Settings.gameDisplay = LIST_MODE;
		Settings.GameWindowMode = GAMEWINDOW_DISC;
	}

	gprintf("\tLoading game categories...%s\n", GameCategories.Load(Settings.ConfigPath) ? "done" : "failed");
	gprintf("\tLoading font...%s\n", Theme::LoadFont(Settings.ConfigPath) ? "done" : "failed (using default)");
	gprintf("\tLoading theme...%s\n", Theme::Load(Settings.theme) ? "done" : "failed (using default)");
	if(Settings.CacheTitles)
		gprintf("\tLoading cached titles...%s\n", GameTitles.ReadCachedTitles(Settings.titlestxt_path) ? "done" : "failed (using default)");

	//! Init the rest of the System
	Sys_Init();
	InitAudio();
	setlocale(LC_CTYPE, "C-UTF-8");
	setlocale(LC_MESSAGES, "C-UTF-8");
	AdjustOverscan(Settings.AdjustOverscanX, Settings.AdjustOverscanY);

	return 0;
}

void StartUpProcess::Draw()
{
	background->Draw();
	GXImage->Draw();
	titleTxt->Draw();
	messageTxt->Draw();
	if(drawCancel)
		cancelTxt->Draw();
	Menu_Render();
}

int StartUpProcess::QuickGameBoot(const char * gameID)
{
	MountGamePartition(false);

	struct discHdr *header = NULL;
	for(int i = 0; i < gameList.size(); ++i)
	{
		if(strncasecmp((char *) gameList[i]->id, gameID, 6) == 0)
			header = gameList[i];
	}

	if(!header)
		return -1;

	GameStatistics.SetPlayCount(header->id, GameStatistics.GetPlayCount(header->id)+1);
	GameStatistics.Save();

	return GameBooter::BootGame(header);
}
