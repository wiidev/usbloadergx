#include <unistd.h>
#include "StartUpProcess.h"
#include "GUI/gui.h"
#include "video.h"
#include "audio.h"
#include "input.h"
#include "themes/CTheme.h"
#include "gecko.h"
#include "wpad.h"
#include "Controls/DeviceHandler.hpp"
#include "wad/nandtitle.h"
#include "SystemMenu/SystemMenuResources.h"
#include "system/IosLoader.h"
#include "libs/libruntimeiospatch/runtimeiospatch.h"
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
#include "usbloader/wdvd.h"
#include "utils/tools.h"
#include "sys.h"
#include "version.h"
#include "settings/meta.h"

extern bool isWiiVC; // in sys.cpp

StartUpProcess::StartUpProcess()
{
	
	background = new GuiImage(screenwidth, screenheight, (GXColor){0, 0, 0, 255});

	GXImageData = Resources::GetImageData("gxlogo.png");
	GXImage = new GuiImage(GXImageData);
	GXImage->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	GXImage->SetPosition(screenwidth / 2, screenheight / 2 - 50);

// Please don't release unofficial builds w/o tagging them as such
#if defined(FULLCHANNEL)
	versionTxt->SetTextf("v4.0c Rev. %s (%s)", LOADER_REV, GIT_VER);
#elif defined(GITRELEASE)
	versionTxt->SetTextf("v4.0 Rev. %s (%s)", LOADER_REV, GIT_VER);
#else
	versionTxt->SetTextf("v4.0 Rev. %s (%s) / Unofficial", LOADER_REV, GIT_VER);
#endif

	if (strncmp(Settings.ConfigPath, "sd", 2) == 0)
		cancelTxt = new GuiText("Press B to cancel or A to enable SD card mode", 22, (GXColor){255, 255, 255, 255});
	else
		cancelTxt = new GuiText("Press B to cancel", 22, (GXColor){255, 255, 255, 255});
	cancelTxt->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	cancelTxt->SetPosition(screenwidth / 2, screenheight / 2 + 90);

	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	cancelBtn = new GuiButton(0, 0);
	cancelBtn->SetTrigger(trigB);

	trigA = new GuiTrigger;
	trigA->SetButtonOnlyTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	sdmodeBtn = new GuiButton(0, 0);
	if (strncmp(Settings.ConfigPath, "sd", 2) == 0)
		sdmodeBtn->SetTrigger(trigA);

	drawCancel = false;
}

StartUpProcess::~StartUpProcess()
{
	delete background;
	delete GXImageData;
	delete GXImage;
	delete titleTxt;
	delete messageTxt;
	delete versionTxt;
	delete cancelTxt;
	delete cancelBtn;
	delete sdmodeBtn;
	delete trigB;
	delete trigA;
}

int StartUpProcess::ParseArguments(int argc, char *argv[])
{
	int quickBoot = -1;

	//! The arguments override
	for (int i = 0; i < argc; ++i)
	{
		if (!argv[i])
			continue;

		gprintf("Boot argument %i: %s\n", i + 1, argv[i]);

		char *ptr = strcasestr(argv[i], "-ios=");
		if (ptr)
		{
			if (atoi(ptr + strlen("-ios=")) == 58)
				Settings.LoaderIOS = 58;
			else
				Settings.LoaderIOS = LIMIT(atoi(ptr + strlen("-ios=")), 200, 255);
		}

		ptr = strcasestr(argv[i], "-bootios=");
		if (ptr)
		{
			if (atoi(ptr + strlen("-bootios=")) == 58)
				Settings.BootIOS = 58;
			else
				Settings.BootIOS = LIMIT(atoi(ptr + strlen("-bootios=")), 200, 255);
		}

		ptr = strcasestr(argv[i], "-usbport=");
		if (ptr)
		{
			Settings.USBPort = LIMIT(atoi(ptr + strlen("-usbport=")), 0, 2);
		}

		if (strncmp(Settings.ConfigPath, "sd", 2) == 0)
		{
			ptr = strcasestr(argv[i], "-sdmode=");
			if (ptr)
				Settings.SDMode = LIMIT(atoi(ptr + strlen("-sdmode=")), 0, 1);
		}

		if ((strlen(argv[i]) == 6 || strlen(argv[i]) == 4) && strchr(argv[i], '=') == 0 && strchr(argv[i], '-') == 0)
			quickBoot = i;
	}

	return quickBoot;
}

void StartUpProcess::TextFade(int direction)
{
	if (direction > 0)
	{
		for (int i = 0; i < 255; i += direction)
		{
			messageTxt->SetAlpha(i);
			Draw();
		}
		messageTxt->SetAlpha(255);
		Draw();
	}
	else if (direction < 0)
	{
		for (int i = 255; i > 0; i += direction)
		{
			messageTxt->SetAlpha(i);
			Draw();
		}
		messageTxt->SetAlpha(0);
		Draw();
	}
}

void StartUpProcess::SetTextf(const char *format, ...)
{
	char *tmp = NULL;
	va_list va;
	va_start(va, format);
	if ((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
		TextFade(-40);
		gprintf(tmp);
		messageTxt->SetText(tmp);
		TextFade(40);
	}
	va_end(va);

	if (tmp)
		free(tmp);
}

bool StartUpProcess::USBSpinUp()
{
	drawCancel = true;
	Timer countDown;
	bool started0 = false;
	bool started1 = false;

	const DISC_INTERFACE *handle0 = NULL;
	const DISC_INTERFACE *handle1 = NULL;
	if (Settings.USBPort == 0 || Settings.USBPort == 2)
		handle0 = DeviceHandler::GetUSB0Interface();
	if (Settings.USBPort == 1 || Settings.USBPort == 2)
		handle1 = DeviceHandler::GetUSB1Interface();

	// wait 20 sec for the USB to spin up...stupid slow ass HDD
	do
	{
		if (handle0)
			started0 = (handle0->startup() && handle0->isInserted());

		if (handle1)
			started1 = (handle1->startup() && handle1->isInserted());

		if ((!handle0 || started0) && (!handle1 || started1))
			break;

		UpdatePads();
		for (int i = 0; i < 4; ++i)
		{
			cancelBtn->Update(&userInput[i]);
			sdmodeBtn->Update(&userInput[i]);
		}

		if (cancelBtn->GetState() == STATE_CLICKED)
			break;

		if (sdmodeBtn->GetState() == STATE_CLICKED)
		{
			Settings.SDMode = ON;
			break;
		}

		messageTxt->SetTextf("Waiting for USB devices: %i sec left\n", 20 - (int)countDown.elapsed());
		Draw();
		usleep(50000);
	} while (countDown.elapsed() < 20.f);

	drawCancel = false;

	return (started0 || started1);
}

int StartUpProcess::Run(int argc, char *argv[])
{
	bool isBadBoot = false;
	// A normal launch should always have the first arg be the path
	char *ptr = strrchr(argv[0], '/');
	if (ptr && (argv[0][2] == ':' || argv[0][3] == ':'))
	{
		*ptr = 0;
		// HBC doesn't specify the USB port
		if (strncmp(argv[0], "usb", 3) == 0)
		{
			snprintf(Settings.BootDevice, sizeof(Settings.BootDevice), "usb1:");
			snprintf(Settings.ConfigPath, sizeof(Settings.ConfigPath), "usb1:%s/", argv[0] + 4);
		}
		else if (strncmp(argv[0], "sd", 2) == 0)
			snprintf(Settings.ConfigPath, sizeof(Settings.ConfigPath), "%s/", argv[0]);
		gprintf("Loader path: %s\n", Settings.ConfigPath);
	}
	// Priiloader breaks updates and passes outdated meta.xml info
	else if (strncmp(argv[0], "/title/00000001/", 16) == 0)
		isBadBoot = true;

	int quickGameBoot = ParseArguments(argc, argv);

	StartUpProcess Process;

	int ret = Process.Execute(quickGameBoot != -1, isBadBoot);

	if (quickGameBoot != -1)
		return QuickGameBoot(argv[quickGameBoot]);

	return ret;
}

void StartUpProcess::LoadIOS(u8 ios, bool boot)
{
	SetTextf("Reloading to IOS%d%s\n", ios, boot ? " requested in meta.xml" : "");
	if (IosLoader::LoadAppCios(ios) < 0)
	{
		SetTextf("Failed to load an IOS. USB Loader GX requires a cIOS or IOS58 with AHB access. Exiting...\n");
		sleep(5);
		Sys_BackToLoader();
	}
	SetTextf("Reloaded to IOS%d r%d\n", Settings.LoaderIOS, IOS_GetRevision());
}

int StartUpProcess::Execute(bool quickGameBoot, bool isBadBoot)
{
	if (isBadBoot)
	{
		SetTextf("Install the UNEO channel booter instead\n");
		sleep(5);
		*(vu32 *)0x8132FFFB = 0x4461636F;
		*(vu32 *)0x817FEFF0 = 0x4461636F;
		DCFlushRange((void *)0x8132FFFB, 4);
		DCFlushRange((void *)0x817FEFF0, 4);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}

	Settings.EntryIOS = IOS_GetVersion();
	isWiiVC = IsWiiVCActive();

	// Disable AHBPROT
	IosPatch_AHBPROT(false);

	// Patch permissions for vWii
	IosPatch_RUNTIME(!isWiiVC, false, false, isWiiVC, false);

	// Reset the region
	ResetRegion();

	// Get NAND titles
	NandTitles.Get();

	// Store dx2 cIOS info
	IosLoader::GetD2XInfo();

	gprintf("Current IOS: %d - have AHB access: %s\n", Settings.EntryIOS, AHBPROT_DISABLED ? "yes" : "no");

	// Reload to a cIOS if running as a Wii U vWii VC inject
	if (isWiiVC)
	{
		Settings.SDMode = ON;
		LoadIOS(Settings.LoaderIOS, false);
	}
	// Reload to a cIOS if we're using both USB ports
	else if (Settings.USBPort == 2 && !Settings.SDMode)
		LoadIOS(Settings.LoaderIOS, false);

	// Reload to a cIOS if required (old forwarder?) or requested
	else if (!AHBPROT_DISABLED || (Settings.EntryIOS != Settings.BootIOS))
		LoadIOS(Settings.BootIOS, true);

	// Setup the pads
	SetupPads();

	// Do not mount USB if not needed. USB is not available with Wii U WiiVC injected channel
	bool USBSuccess = false;
	if (!isWiiVC && !Settings.SDMode)
	{
		SetTextf("Initializing USB devices\n");
		if (USBSpinUp())
		{
			DeviceHandler::Instance()->MountAllUSB(false);
			USBSuccess = true;
			gprintf("Completed initialization of USB devices\n");
		}
	}

	// Mount the SD card
	SetTextf("Initializing SD card\n");
	DeviceHandler::Instance()->MountSD();

	SetTextf("Loading config files\n");
	gprintf("\tLoading config...%s\n", Settings.Load() ? "done" : "failed");
	gprintf("\tLoading language...%s\n", Settings.LoadLanguage(Settings.language_path, CONSOLE_DEFAULT) ? "done" : "failed");
	gprintf("\tLoading game settings...%s\n", GameSettings.Load(Settings.ConfigPath) ? "done" : "failed");
	gprintf("\tLoading game statistics...%s\n", GameStatistics.Load(Settings.ConfigPath) ? "done" : "failed");
	gprintf("\tLoading game categories...%s\n", GameCategories.Load(Settings.ConfigPath) ? "done" : "failed");
	gprintf("\tLoading cached titles...%s\n", GameTitles.ReadCachedTitles(Settings.titlestxt_path) ? "done" : "failed (using default)");

	// Some settings need to be enabled to boot directly into games
	gprintf("Quick game boot: %s\n", quickGameBoot ? "yes" : "no");
	if (quickGameBoot)
	{
		Settings.LoaderMode = MODE_WIIGAMES | MODE_GCGAMES | MODE_EMUCHANNELS;
		Settings.GameDisplayType = DISP_CUSTOM;
		Settings.CacheTitles = OFF;
		Settings.AutobootDiscs = OFF;
		Settings.skipSaving = true;
	}

	// Reload to users settings if different than current IOS, and if not using an injected WiiU WiiVC IOS255 (fw.img)
	if (Settings.LoaderIOS != IOS_GetVersion() && !isWiiVC)
	{
		// Shutdown pads, but wait for up to 2 seconds so that Wiimotes reconnect correctly
		for (int i = 0; i < 20; i++)
		{
			if (WPAD_GetStatus() == WPAD_STATE_ENABLED)
				break;
			usleep(100000);
		}
		WPAD_Shutdown();

		// Unmount devices
		DeviceHandler::DestroyInstance();
		USBStorage2_Deinit();

		// Now load the cIOS that was set in the settings menu
		if (IosLoader::LoadAppCios(Settings.LoaderIOS) > -1)
		{
			SetTextf("Reloaded to IOS%d r%d\n", Settings.LoaderIOS, IOS_GetRevision());
			// Re-Mount devices
			SetTextf("Reinitializing devices\n");
		}
		gprintf("Current IOS: %d - have AHB access: %s\n", IOS_GetVersion(), AHBPROT_DISABLED ? "yes" : "no");

		// Start pads again
		SetupPads();

		DeviceHandler::Instance()->MountSD();
		if (!Settings.SDMode && USBSuccess)
		{
			if (USBSpinUp())
				DeviceHandler::Instance()->MountAllUSB(false);
		}
	}

	if (!isWiiVC)
		editMetaArguments();

	if (!IosLoader::IsHermesIOS() && !IosLoader::IsD2X() && !Settings.SDMode)
	{
		Settings.USBPort = 0;
	}
	else if (Settings.USBPort == 1 && (USBStorage2_GetPort() != Settings.USBPort) && !Settings.SDMode && !isWiiVC)
	{
		SetTextf("Changing USB port to %i\n", Settings.USBPort);
		DeviceHandler::Instance()->UnMountAllUSB();
		DeviceHandler::Instance()->MountAllUSB();
	}
	else if (Settings.USBPort == 2 && !Settings.SDMode && !isWiiVC)
	{
		SetTextf("Mounting USB port to 1\n");
		DeviceHandler::Instance()->MountUSBPort1();
	}

	// Enable isfs permission if using Hermes v4 without AHB, or WiiU WiiVC (IOS255 fw.img)
	if (IOS_GetVersion() < 200 || (IosLoader::IsHermesIOS() && IOS_GetRevision() == 4) || isWiiVC)
	{
		SetTextf("Patching IOS%d\n", IOS_GetVersion());
		if (IosPatch_RUNTIME(!isWiiVC, false, false, isWiiVC, false) == ERROR_PATCH)
			gprintf("Patching IOS%d failed!\n", IOS_GetVersion());
		else
			NandTitles.Get(); // get NAND channel's titles

		gprintf("Current IOS: %d - have AHB access: %s\n", IOS_GetVersion(), AHBPROT_DISABLED ? "yes" : "no");
	}

	// Initialize again
	ISFS_Initialize();

	// Check MIOS version
	SetTextf("Checking installed MIOS\n");
	IosLoader::GetMIOSInfo();

	if (Settings.AutobootDiscs == ON)
	{
		Timer countDown;
		bool skipDiscAutoboot = false;
		s32 delay = 0;
		u32 DiscDriveCover = 0;

		Disc_Init();
		WDVD_GetCoverStatus(&DiscDriveCover);
		if (DiscDriveCover & 0x02)
		{
			drawCancel = true;
			gprintf("Disc found in drive\n");
			cancelTxt->SetText("Press B to cancel");
			do
			{
				UpdatePads();
				for (int i = 0; i < 4; ++i)
					cancelBtn->Update(&userInput[i]);
				if (cancelBtn->GetState() == STATE_CLICKED)
				{
					skipDiscAutoboot = true;
					break;
				}

				delay = Settings.AutobootDiscsDelay - (int)countDown.elapsed();
				messageTxt->SetTextf("Booting from disc in %d second%s\n", delay, delay > 1 ? "s" : "");
				Draw();
				usleep(50000);
			} while (countDown.elapsed() < (float)Settings.AutobootDiscsDelay);

			drawCancel = false;
			if (skipDiscAutoboot == false)
			{
				messageTxt->SetTextf("Booting from disc\n");
				Draw();
				return AutobootDisc();
			}
		}
		else
		{
			gprintf("No disc found in drive\n");
			WDVD_Close();
		}
	}
	return FinalizeExecute();
}

int StartUpProcess::FinalizeExecute()
{
	SetTextf("Loading resources\n");
	// Do not allow banner grid mode without AHBPROT
	// this function does nothing if it was already initiated before
	if (!SystemMenuResources::Instance()->IsLoaded() && !SystemMenuResources::Instance()->Init() && Settings.gameDisplay == BANNERGRID_MODE)
	{
		Settings.gameDisplay = LIST_MODE;
		Settings.GameWindowMode = GAMEWINDOW_DISC;
	}

	LoadNewTheme();
	gprintf("\tLoading font...%s\n", Theme::LoadFont(Settings.ConfigPath) ? "done" : "failed (using default)");
	gprintf("\tLoading theme...%s\n", Theme::Load(Settings.theme) ? "done" : "failed (using default)");

	//! Init the rest of the system
	Sys_Init();
	InitAudio();
	setlocale(LC_CTYPE, "en_US.UTF-8");
	setlocale(LC_MESSAGES, "en_US.UTF-8");
	AdjustOverscan(Settings.AdjustOverscanX, Settings.AdjustOverscanY);

	return 0;
}

void StartUpProcess::Draw()
{
	background->Draw();
	GXImage->Draw();
	titleTxt->Draw();
	messageTxt->Draw();
	versionTxt->Draw();
	if (drawCancel)
		cancelTxt->Draw();
	Menu_Render();
}

int StartUpProcess::QuickGameBoot(const char *gameID)
{
	MountGamePartition(false);

	struct discHdr *header = NULL;
	for (int i = 0; i < gameList.size(); ++i)
	{
		if (strncasecmp((char *)gameList[i]->id, gameID, 6) == 0)
			header = gameList[i];
	}

	if (!header)
		return -1;

	GameStatistics.SetPlayCount(header->id, GameStatistics.GetPlayCount(header->id) + 1);
	GameStatistics.Save();

	return GameBooter::BootGame(header);
}

int StartUpProcess::AutobootDisc()
{
	struct discHdr *header = new struct discHdr;
	if (Disc_Mount(header) < 0)
	{
		delete header;
		header = NULL;
		SetTextf("Error mounting disc\n");
		sleep(3);
		return FinalizeExecute();
	}
	else
	{
		GameStatistics.SetPlayCount(header->id, GameStatistics.GetPlayCount(header->id) + 1);
		GameStatistics.Save();
		return GameBooter::BootGame(header);
	}
}
