/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CSettings.h"
#include "CGameSettings.h"
#include "CGameStatistics.h"
#include "Controls/DeviceHandler.hpp"
#include "language/gettext.h"
#include "themes/CTheme.h"
#include "FileOperations/fileops.h"
#include "utils/encrypt.h"
#include "svnrev.h"

#define VALID_CONFIG_REV	1031

CSettings Settings;

CSettings::CSettings()
{
	CONF_Init();
	strcpy(BootDevice, "sd:");
	snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/", BootDevice);
	this->SetDefault();
	FirstTimeRun = true;
}

CSettings::~CSettings()
{
}

void CSettings::SetDefault()
{
	snprintf(covers_path, sizeof(covers_path), "%simages/", ConfigPath);
	snprintf(covers2d_path, sizeof(covers2d_path), "%simages/2D/", ConfigPath);
	snprintf(coversFull_path, sizeof(coversFull_path), "%simages/full/", ConfigPath);
	snprintf(disc_path, sizeof(disc_path), "%simages/disc/", ConfigPath);
	snprintf(titlestxt_path, sizeof(titlestxt_path), "%s", ConfigPath);
	snprintf(languagefiles_path, sizeof(languagefiles_path), "%slanguage/", ConfigPath);
	snprintf(update_path, sizeof(update_path), "%s/apps/usbloader_gx/", BootDevice);
	snprintf(BNRCachePath, sizeof(BNRCachePath), "%s/apps/usbloader_gx/cache_bnr/", BootDevice);
	snprintf(homebrewapps_path, sizeof(homebrewapps_path), "%s/apps/", BootDevice);
	snprintf(Cheatcodespath, sizeof(Cheatcodespath), "%s/codes/", BootDevice);
	snprintf(TxtCheatcodespath, sizeof(TxtCheatcodespath), "%s/txtcodes/", BootDevice);
	snprintf(BcaCodepath, sizeof(BcaCodepath), "%s/bca/", BootDevice);
	snprintf(WipCodepath, sizeof(WipCodepath), "%s/wip/", BootDevice);
	snprintf(WDMpath, sizeof(WDMpath), "%s/wdm/", BootDevice);
	snprintf(WiinnertagPath, sizeof(WiinnertagPath), "%s", ConfigPath);
	snprintf(theme_path, sizeof(theme_path), "%stheme/", ConfigPath);
	snprintf(dolpath, sizeof(dolpath), "%s/", BootDevice);
	snprintf(NandEmuPath, sizeof(NandEmuPath), "%s/nand/", BootDevice);
	snprintf(DEVOLoaderPath, sizeof(DEVOLoaderPath), "%s/apps/gc_devo/", BootDevice);
	snprintf(NINLoaderPath, sizeof(NINLoaderPath), "%s/apps/nintendont/", BootDevice);
	strlcpy(NandEmuChanPath, NandEmuPath, sizeof(NandEmuChanPath));
	strlcpy(GameCubePath, "usb1:/games/", sizeof(GameCubePath));
	strlcpy(GameCubeSDPath, "sd:/games/", sizeof(GameCubeSDPath));
	strlcpy(CustomBannersURL, "http://copy.com/vRN3HgFVyk9u7YuB/Public/", sizeof(CustomBannersURL));
	theme[0] = 0;
	language_path[0] = 0;
	ogg_path[0] = 0;
	unlockCode[0] = 0;
	db_language[0] = 0;
	returnTo[0] = 0;

	NTSC = (CONF_GetVideo() == CONF_VIDEO_NTSC);
	PAL50 = (CONF_GetVideo() == CONF_VIDEO_PAL) && (CONF_GetEuRGB60() == 0);
	widescreen = (CONF_GetAspectRatio() == CONF_ASPECT_16_9);

	godmode = 1;
	videomode = VIDEO_MODE_DISCDEFAULT;
	videopatch = OFF;
	videoPatchDol = OFF;
	language = CONSOLE_DEFAULT;
	ocarina = OFF;
	hddinfo = CLOCK_HR12;
	sinfo = ON;
	rumble = ON;
	GameSort = SORT_ABC;
	volume = 80;
	sfxvolume = 80;
	gamesoundvolume = 80;
	tooltips = ON;
	gamesound = ON;
	parentalcontrol = PARENTAL_LVL_ADULT;
	LoaderIOS = BUILD_IOS;
	cios = BUILD_IOS;
	gridRows = 3;
	partition = 0;
	discart = DISCARTS_ORIGINALS_CUSTOMS;
	coversfull = COVERSFULL_HQ;
	xflip = XFLIP_NO;
	quickboot = OFF;
	wiilight = WIILIGHT_ON;
	autonetwork = OFF;
	patchcountrystrings = OFF;
	titlesOverride = ON;
	ForceDiscTitles = OFF;
	screensaver = SCREENSAVER_10_MIN;
	musicloopmode = ON;
	marknewtitles = ON;
	ShowFreeSpace = ON;
	PlaylogUpdate = OFF;
	ParentalBlocks = BLOCK_ALL;
	InstallToDir = INSTALL_TO_NAME_GAMEID;
	GameSplit = GAMESPLIT_4GB;
	InstallPartitions = ONLY_GAME_PARTITION;
	HomeMenu = HOME_MENU_DEFAULT;
	MultiplePartitions = OFF;
	BlockIOSReload = AUTO;
	USBPort = 0;
	USBAutoMount = ON;
	CacheTitles = ON;
	WSFactor = 0.8f; //actually should be 0.75 for real widescreen
	FontScaleFactor = 0.8f; //it's a work around to not have to change ALL fonts now
	ClockFontScaleFactor = 1.0f; // Scale of 1 to prevent misaligned clock.
	EnabledCategories.resize(1);
	EnabledCategories[0] = 0;
	RequiredCategories.resize(0);
	ForbiddenCategories.resize(0);
	Wiinnertag = OFF;
	SelectedGame = 0;
	GameListOffset = 0;
	sneekVideoPatch = OFF;
	NandEmuMode = OFF;
	NandEmuChanMode = 2;
	UseSystemFont = ON;
	Hooktype = 0;
	WiirdDebugger = OFF;
	WiirdDebuggerPause = OFF;
	ShowPlayCount = ON;
	RememberUnlock = ON;
	LoaderMode = MODE_WIIGAMES | MODE_GCGAMES;
	SearchMode = SEARCH_BEGINNING;
	GameAspectRatio = ASPECT_SYSTEM_DEFAULT;
	PointerSpeed = 0.18f;
	UseChanLauncher = OFF;
	AdjustOverscanX = 0;
	AdjustOverscanY = 0;
	TooltipDelay = 1500; // ms
	GameWindowMode = GAMEWINDOW_BANNER;
	CacheBNRFiles = ON;
	BannerAnimStart = BANNER_START_ON_ZOOM;
	BannerGridSpeed = 25.6f; // pixel/frames
	BannerZoomDuration = 30; // frames
	BannerProjectionOffsetX = (!widescreen || PAL50) ? 0.0f : 2.0f;
	BannerProjectionOffsetY = PAL50 ? -1.0f : (NTSC ? 0.0f : -4.0f);
	BannerProjectionWidth = (Settings.widescreen ? (Settings.PAL50 ? 616 : 620.0f) : 608.0f);
	BannerProjectionHeight = (Settings.PAL50 ? 448.0f : (NTSC ? 470.0f : 464.0f));
	GCBannerScale = 1.5f;
	GameCubeMode = GC_MODE_MIOS;
	GameCubeSource = AUTO;
	DMLVideo = DML_VIDEO_AUTO;
	DMLProgPatch = OFF;
	DMLNMM = OFF;
	DMLActivityLED = OFF;
	DMLPADHOOK = OFF;
	DMLNoDisc2 = OFF;
	DMLWidescreen = OFF;
	DMLScreenshot = OFF;
	DMLJPNPatch = OFF;
	DMLDebug = OFF;
	NINDeflicker = OFF;
	NINMCEmulation = ON;
	NINMCSize = 2;
	NINAutoboot = ON;
	NINSettings = AUTO;
	NINUSBHID = OFF;
	NINMaxPads = 1;
	NINNativeSI = OFF;
	NINWiiUWide = widescreen;
	NINOSReport = OFF;
	NINLED = OFF;
	NINLog = OFF;
	DEVOMCEmulation = OFF;
	DEVOWidescreen = OFF;
	DEVOActivityLED = ON;
	DEVOFZeroAX = OFF;
	DEVOTimerFix = OFF;
	DEVODButtons = OFF;
	DEVOCropOverscan = OFF;
	DEVODiscDelay = OFF;
	GCInstallCompressed = OFF;
	GCInstallAligned = OFF;
	PrivateServer = OFF;
}

bool CSettings::Load()
{
	FindConfig();
	//! Reset default path variables to the right device
	SetDefault();

	char filepath[300];
	snprintf(filepath, sizeof(filepath), "%sGXGlobal.cfg", ConfigPath);

	FILE * file = fopen(filepath, "r");
	if (!file) return false;

	if(!ValidVersion(file))
	{
		fclose(file);
		return false;
	}

	char line[1024];

	while (fgets(line, sizeof(line), file))
	{
		if (line[0] == '#') continue;

		this->ParseLine(line);
	}
	fclose(file);

	// A valid config file exists on the loader
	// meaning it is not the first run of the loader.
	FirstTimeRun = false;

	return true;
}

bool CSettings::ValidVersion(FILE * file)
{
	if(!file) return false;

	char line[255];
	int revision = 0;

	while (fgets(line, sizeof(line), file))
	{
		const char * ptr = strcasestr(line, "USB Loader GX R");
		if(ptr)
		{
			ptr += strlen("USB Loader GX R");
			revision = atoi(ptr);
			break;
		}
	}

	rewind(file);

	return revision >= VALID_CONFIG_REV;
}

bool CSettings::Reset()
{
	this->SetDefault();

	if (this->Save()) return true;

	return false;
}

bool CSettings::Save()
{
	if (!FindConfig()) return false;

	char filedest[300];
	snprintf(filedest, sizeof(filedest), "%sGXGlobal.cfg", ConfigPath);

	if(!CreateSubfolder(ConfigPath))	return false;

	FILE * file = fopen(filedest, "w");
	if (!file) return false;

	fprintf(file, "# USB Loader GX R%s - Main settings file\n", GetRev());
	fprintf(file, "# Note: This file is automatically generated\n");
	fprintf(file, "godmode = %d\n", godmode);
	fprintf(file, "videomode = %d\n", videomode);
	fprintf(file, "videopatch = %d\n", videopatch);
	fprintf(file, "videoPatchDol = %d\n", videoPatchDol);
	fprintf(file, "language = %d\n", language);
	fprintf(file, "ocarina = %d\n", ocarina);
	fprintf(file, "hddinfo = %d\n", hddinfo);
	fprintf(file, "sinfo = %d\n", sinfo);
	fprintf(file, "rumble = %d\n", rumble);
	fprintf(file, "volume = %d\n", volume);
	fprintf(file, "sfxvolume = %d\n", sfxvolume);
	fprintf(file, "gamesoundvolume = %d\n", gamesoundvolume);
	fprintf(file, "tooltips = %d\n", tooltips);
	fprintf(file, "RememberUnlock = %d\n", RememberUnlock);
	char EncryptedTxt[50];
	EncryptString(unlockCode, EncryptedTxt);
	fprintf(file, "password = %s\n", EncryptedTxt);
	fprintf(file, "GameSort = %d\n", GameSort);
	fprintf(file, "LoaderIOS = %d\n", LoaderIOS);
	fprintf(file, "cios = %d\n", cios);
	fprintf(file, "keyset = %d\n", keyset);
	fprintf(file, "xflip = %d\n", xflip);
	fprintf(file, "gridRows = %d\n", gridRows);
	fprintf(file, "quickboot = %d\n", quickboot);
	fprintf(file, "wsprompt = %d\n", wsprompt);
	fprintf(file, "parentalcontrol = %d\n", parentalcontrol);
	fprintf(file, "covers_path = %s\n", covers_path);
	fprintf(file, "covers2d_path = %s\n", covers2d_path);
	fprintf(file, "coversFull_path = %s\n", coversFull_path);
	fprintf(file, "theme_path = %s\n", theme_path);
	fprintf(file, "theme = %s\n", theme);
	fprintf(file, "disc_path = %s\n", disc_path);
	fprintf(file, "language_path = %s\n", language_path);
	fprintf(file, "languagefiles_path = %s\n", languagefiles_path);
	fprintf(file, "TxtCheatcodespath = %s\n", TxtCheatcodespath);
	fprintf(file, "titlestxt_path = %s\n", titlestxt_path);
	fprintf(file, "gamesound = %d\n", gamesound);
	fprintf(file, "dolpath = %s\n", dolpath);
	fprintf(file, "ogg_path = %s\n", ogg_path);
	fprintf(file, "wiilight = %d\n", wiilight);
	fprintf(file, "gameDisplay = %d\n", gameDisplay);
	fprintf(file, "update_path = %s\n", update_path);
	fprintf(file, "homebrewapps_path = %s\n", homebrewapps_path);
	fprintf(file, "BNRCachePath = %s\n", BNRCachePath);
	fprintf(file, "Cheatcodespath = %s\n", Cheatcodespath);
	fprintf(file, "BcaCodepath = %s\n", BcaCodepath);
	fprintf(file, "WipCodepath = %s\n", WipCodepath);
	fprintf(file, "WDMpath = %s\n", WDMpath);
	fprintf(file, "titlesOverride = %d\n", titlesOverride);
	fprintf(file, "ForceDiscTitles = %d\n", ForceDiscTitles);
	fprintf(file, "patchcountrystrings = %d\n", patchcountrystrings);
	fprintf(file, "screensaver = %d\n", screensaver);
	fprintf(file, "musicloopmode = %d\n", musicloopmode);
	fprintf(file, "autonetwork = %d\n", autonetwork);
	fprintf(file, "discart = %d\n", discart);
	fprintf(file, "coversfull = %d\n", coversfull);
	fprintf(file, "partition = %d\n", partition);
	fprintf(file, "marknewtitles = %d\n", marknewtitles);
	fprintf(file, "ShowFreeSpace = %d\n", ShowFreeSpace);
	fprintf(file, "InstallToDir = %d\n", InstallToDir);
	fprintf(file, "GameSplit = %d\n", GameSplit);
	fprintf(file, "InstallPartitions = %08X\n", InstallPartitions);
	fprintf(file, "PlaylogUpdate = %d\n", PlaylogUpdate);
	fprintf(file, "ParentalBlocks = %08X\n", ParentalBlocks);
	fprintf(file, "returnTo = %s\n", returnTo);
	fprintf(file, "HomeMenu = %d\n", HomeMenu);
	fprintf(file, "MultiplePartitions = %d\n", MultiplePartitions);
	fprintf(file, "USBPort = %d\n", USBPort);
	fprintf(file, "USBAutoMount = %d\n", USBAutoMount);
	fprintf(file, "CacheTitles = %d\n", CacheTitles);
	fprintf(file, "BlockIOSReload = %d\n", BlockIOSReload);
	fprintf(file, "WSFactor = %0.3f\n", WSFactor);
	fprintf(file, "FontScaleFactor = %0.3f\n", FontScaleFactor);
	fprintf(file, "ClockFontScaleFactor = %0.3f\n", ClockFontScaleFactor);
	fprintf(file, "EnabledCategories = ");
	for(u32 i = 0; i < EnabledCategories.size(); ++i)
	{
		fprintf(file, "%i", EnabledCategories[i]);
		if(i+1 < EnabledCategories.size())
			fprintf(file, ",");
	}
	fprintf(file, "\n");
	fprintf(file, "RequiredCategories = ");
	for(u32 i = 0; i < RequiredCategories.size(); ++i)
	{
		fprintf(file, "%i", RequiredCategories[i]);
		if(i+1 < RequiredCategories.size())
			fprintf(file, ",");
	}
	fprintf(file, "\n");
	fprintf(file, "ForbiddenCategories = ");
	for(u32 i = 0; i < ForbiddenCategories.size(); ++i)
	{
		fprintf(file, "%i", ForbiddenCategories[i]);
		if(i+1 < ForbiddenCategories.size())
			fprintf(file, ",");
	}
	fprintf(file, "\n");
	fprintf(file, "Wiinnertag = %d\n", Wiinnertag);
	fprintf(file, "WiinnertagPath = %s\n", WiinnertagPath);
	fprintf(file, "SelectedGame = %d\n", SelectedGame);
	fprintf(file, "GameListOffset = %d\n", GameListOffset);
	fprintf(file, "sneekVideoPatch = %d\n", sneekVideoPatch);
	fprintf(file, "NandEmuMode = %d\n", NandEmuMode);
	fprintf(file, "NandEmuChanMode = %d\n", NandEmuChanMode);
	fprintf(file, "NandEmuPath = %s\n", NandEmuPath);
	fprintf(file, "NandEmuChanPath = %s\n", NandEmuChanPath);
	fprintf(file, "UseSystemFont = %d\n", UseSystemFont);
	fprintf(file, "Hooktype = %d\n", Hooktype);
	fprintf(file, "WiirdDebugger = %d\n", WiirdDebugger);
	fprintf(file, "WiirdDebuggerPause = %d\n", WiirdDebuggerPause);
	fprintf(file, "ShowPlayCount = %d\n", ShowPlayCount);
	fprintf(file, "LoaderMode = %d\n", LoaderMode);
	fprintf(file, "SearchMode = %d\n", SearchMode);
	fprintf(file, "GameAspectRatio = %d\n", GameAspectRatio);
	fprintf(file, "PointerSpeed = %g\n", PointerSpeed);
	fprintf(file, "UseChanLauncher = %d\n", UseChanLauncher);
	fprintf(file, "AdjustOverscanX = %d\n", AdjustOverscanX);
	fprintf(file, "AdjustOverscanY = %d\n", AdjustOverscanY);
	fprintf(file, "TooltipDelay = %d\n", TooltipDelay);
	fprintf(file, "GameWindowMode = %d\n", GameWindowMode);
	fprintf(file, "CacheBNRFiles = %d\n", CacheBNRFiles);
	fprintf(file, "BannerAnimStart = %d\n", BannerAnimStart);
	fprintf(file, "BannerGridSpeed = %g\n", BannerGridSpeed);
	fprintf(file, "BannerZoomDuration = %d\n", BannerZoomDuration);
	fprintf(file, "BannerProjectionOffsetX = %g\n", BannerProjectionOffsetX);
	fprintf(file, "BannerProjectionOffsetY = %g\n", BannerProjectionOffsetY);
	fprintf(file, "BannerProjectionWidth = %g\n", BannerProjectionWidth);
	fprintf(file, "BannerProjectionHeight = %g\n", BannerProjectionHeight);
	fprintf(file, "GCBannerScale = %g\n", GCBannerScale);
	fprintf(file, "GameCubePath = %s\n", GameCubePath);
	fprintf(file, "GameCubeSDPath = %s\n", GameCubeSDPath);
	fprintf(file, "GameCubeMode = %d\n", GameCubeMode);
	fprintf(file, "GameCubeSource = %d\n", GameCubeSource);
	fprintf(file, "DMLVideo = %d\n", DMLVideo);
	fprintf(file, "DMLProgPatch = %d\n", DMLProgPatch);
	fprintf(file, "DMLNMM = %d\n", DMLNMM);
	fprintf(file, "DMLActivityLED = %d\n", DMLActivityLED);
	fprintf(file, "DMLPADHOOK = %d\n", DMLPADHOOK);
	fprintf(file, "DMLNoDisc2 = %d\n", DMLNoDisc2);
	fprintf(file, "DMLWidescreen = %d\n", DMLWidescreen);
	fprintf(file, "DMLScreenshot = %d\n", DMLScreenshot);
	fprintf(file, "DMLJPNPatch = %d\n", DMLJPNPatch);
	fprintf(file, "DMLDebug = %d\n", DMLDebug);
	fprintf(file, "NINDeflicker = %d\n", NINDeflicker);
	fprintf(file, "NINMCEmulation = %d\n", NINMCEmulation);
	fprintf(file, "NINMCSize = %d\n", NINMCSize);
	fprintf(file, "NINAutoboot = %d\n", NINAutoboot);
	fprintf(file, "NINSettings = %d\n", NINSettings);
	fprintf(file, "NINUSBHID = %d\n", NINUSBHID);
	fprintf(file, "NINMaxPads = %d\n", NINMaxPads);
	fprintf(file, "NINNativeSI = %d\n", NINNativeSI);
	fprintf(file, "NINWiiUWide = %d\n", NINWiiUWide);
	fprintf(file, "NINOSReport = %d\n", NINOSReport);
	fprintf(file, "NINLED = %d\n", NINLED);
	fprintf(file, "NINLog = %d\n", NINLog);
	fprintf(file, "DEVOMCEmulation = %d\n", DEVOMCEmulation);
	fprintf(file, "DEVOWidescreen = %d\n", DEVOWidescreen);
	fprintf(file, "DEVOActivityLED = %d\n", DEVOActivityLED);
	fprintf(file, "DEVOFZeroAX = %d\n", DEVOFZeroAX);
	fprintf(file, "DEVOTimerFix = %d\n", DEVOTimerFix);
	fprintf(file, "DEVODButtons = %d\n", DEVODButtons);
	fprintf(file, "DEVOCropOverscan = %d\n", DEVOCropOverscan);
	fprintf(file, "DEVODiscDelay = %d\n", DEVODiscDelay);
	fprintf(file, "DEVOLoaderPath = %s\n", DEVOLoaderPath);
	fprintf(file, "NINLoaderPath = %s\n", NINLoaderPath);
	fprintf(file, "GCInstallCompressed = %d\n", GCInstallCompressed);
	fprintf(file, "GCInstallAligned = %d\n", GCInstallAligned);
	fprintf(file, "PrivateServer = %d\n", PrivateServer);
	fprintf(file, "CustomBannersURL = %s\n", CustomBannersURL);
	fclose(file);

	return true;
}

bool CSettings::SetSetting(char *name, char *value)
{
	if (strcmp(name, "godmode") == 0)
	{
		godmode = atoi(value);
		return true;
	}
	else if (strcmp(name, "videomode") == 0)
	{
		videomode = atoi(value);
		return true;
	}
	else if (strcmp(name, "videopatch") == 0)
	{
		videopatch = atoi(value);
		return true;
	}
	else if (strcmp(name, "videoPatchDol") == 0)
	{
		videoPatchDol = atoi(value);
		return true;
	}
	else if (strcmp(name, "language") == 0)
	{
		language = atoi(value);
		return true;
	}
	else if (strcmp(name, "ocarina") == 0)
	{
		ocarina = atoi(value);
		return true;
	}
	else if (strcmp(name, "hddinfo") == 0)
	{
		hddinfo = atoi(value);
		return true;
	}
	else if (strcmp(name, "sinfo") == 0)
	{
		sinfo = atoi(value);
		return true;
	}
	else if (strcmp(name, "rumble") == 0)
	{
		rumble = atoi(value);
		return true;
	}
	else if (strcmp(name, "volume") == 0)
	{
		volume = atoi(value);
		return true;
	}
	else if (strcmp(name, "sfxvolume") == 0)
	{
		sfxvolume = atoi(value);
		return true;
	}
	else if (strcmp(name, "gamesoundvolume") == 0)
	{
		gamesoundvolume = atoi(value);
		return true;
	}
	else if (strcmp(name, "tooltips") == 0)
	{
		tooltips = atoi(value);
		return true;
	}
	else if (strcmp(name, "RememberUnlock") == 0)
	{
		RememberUnlock = atoi(value);
		return true;
	}
	else if (strcmp(name, "password") == 0)
	{
		char EncryptedTxt[50];
		strlcpy(EncryptedTxt, value, sizeof(EncryptedTxt));
		DecryptString(EncryptedTxt, unlockCode);

		if(!RememberUnlock && strlen(unlockCode) > 0 && strcmp(unlockCode, "not set") != 0)
			godmode = 0;
		return true;
	}
	else if (strcmp(name, "GameSort") == 0)
	{
		GameSort = atoi(value);
		return true;
	}
	else if (strcmp(name, "LoaderIOS") == 0)
	{
		LoaderIOS = atoi(value);
		return true;
	}
	else if (strcmp(name, "cios") == 0)
	{
		cios = atoi(value);
		return true;
	}
	else if (strcmp(name, "keyset") == 0)
	{
		keyset = atoi(value);
		return true;
	}
	else if (strcmp(name, "xflip") == 0)
	{
		xflip = atoi(value);
		return true;
	}
	else if (strcmp(name, "gridRows") == 0)
	{
		gridRows = atoi(value);
		return true;
	}
	else if (strcmp(name, "quickboot") == 0)
	{
		quickboot = atoi(value);
		return true;
	}
	else if (strcmp(name, "partition") == 0)
	{
		partition = atoi(value);
		return true;
	}
	else if (strcmp(name, "wsprompt") == 0)
	{
		wsprompt = atoi(value);
		return true;
	}
	else if (strcmp(name, "gameDisplay") == 0)
	{
		gameDisplay = atoi(value);
		return true;
	}
	else if (strcmp(name, "parentalcontrol") == 0)
	{
		parentalcontrol = atoi(value);
		return true;
	}
	else if (strcmp(name, "screensaver") == 0)
	{
		screensaver = atoi(value);
		return true;
	}
	else if (strcmp(name, "titlesOverride") == 0)
	{
		titlesOverride = atoi(value);
		return true;
	}
	else if (strcmp(name, "ForceDiscTitles") == 0)
	{
		ForceDiscTitles = atoi(value);
		return true;
	}
	else if (strcmp(name, "musicloopmode") == 0)
	{
		musicloopmode = atoi(value);
		return true;
	}
	else if (strcmp(name, "gamesound") == 0)
	{
		gamesound = atoi(value);
		return true;
	}
	else if (strcmp(name, "wiilight") == 0)
	{
		wiilight = atoi(value);
		return true;
	}
	else if (strcmp(name, "marknewtitles") == 0)
	{
		marknewtitles = atoi(value);
		return true;
	}
	else if (strcmp(name, "ShowPlayCount") == 0)
	{
		ShowPlayCount = atoi(value);
		return true;
	}
	else if (strcmp(name, "ShowFreeSpace") == 0)
	{
		ShowFreeSpace = atoi(value);
		return true;
	}
	else if (strcmp(name, "HomeMenu") == 0)
	{
		HomeMenu = atoi(value);
		return true;
	}
	else if (strcmp(name, "MultiplePartitions") == 0)
	{
		MultiplePartitions = atoi(value);
		return true;
	}
	else if (strcmp(name, "BlockIOSReload") == 0)
	{
		BlockIOSReload = atoi(value);
		return true;
	}
	else if (strcmp(name, "USBPort") == 0)
	{
		USBPort = atoi(value);
		return true;
	}
	else if (strcmp(name, "USBAutoMount") == 0)
	{
		USBAutoMount = atoi(value);
		return true;
	}
	else if (strcmp(name, "CacheTitles") == 0)
	{
		CacheTitles = atoi(value);
		return true;
	}
	else if (strcmp(name, "patchcountrystrings") == 0)
	{
		patchcountrystrings = atoi(value);
		return true;
	}
	else if (strcmp(name, "discart") == 0)
	{
		discart = atoi(value);
		return true;
	}
	else if (strcmp(name, "coversfull") == 0)
	{
		coversfull = atoi(value);
		return true;
	}
	else if (strcmp(name, "autonetwork") == 0)
	{
		autonetwork = atoi(value);
		return true;
	}
	else if (strcmp(name, "InstallToDir") == 0)
	{
		InstallToDir = atoi(value);
		return true;
	}
	else if (strcmp(name, "GameSplit") == 0)
	{
		GameSplit = atoi(value);
		return true;
	}
	else if (strcmp(name, "PlaylogUpdate") == 0)
	{
		PlaylogUpdate = atoi(value);
		return true;
	}
	else if(strcmp(name, "Wiinnertag") == 0)
	{
		Wiinnertag = atoi(value);
	}
	else if(strcmp(name, "SelectedGame") == 0)
	{
		SelectedGame = atoi(value);
	}
	else if(strcmp(name, "GameListOffset") == 0)
	{
		GameListOffset = atoi(value);
	}
	else if(strcmp(name, "sneekVideoPatch") == 0)
	{
		sneekVideoPatch = atoi(value);
	}
	else if(strcmp(name, "UseSystemFont") == 0)
	{
		UseSystemFont = atoi(value);
	}
	else if(strcmp(name, "Hooktype") == 0)
	{
		Hooktype = atoi(value);
	}
	else if(strcmp(name, "WiirdDebugger") == 0)
	{
		WiirdDebugger = atoi(value);
	}
	else if(strcmp(name, "WiirdDebuggerPause") == 0)
	{
		WiirdDebuggerPause = atoi(value);
	}
	else if(strcmp(name, "NandEmuMode") == 0)
	{
		NandEmuMode = atoi(value);
	}
	else if(strcmp(name, "NandEmuChanMode") == 0)
	{
		NandEmuChanMode = atoi(value);
	}
	else if(strcmp(name, "LoaderMode") == 0)
	{
		LoaderMode = atoi(value);
	}
	else if(strcmp(name, "SearchMode") == 0)
	{
		SearchMode = atoi(value);
	}
	else if(strcmp(name, "GameAspectRatio") == 0)
	{
		GameAspectRatio = atoi(value);
	}
	else if(strcmp(name, "UseChanLauncher") == 0)
	{
		UseChanLauncher = atoi(value);
	}
	else if(strcmp(name, "AdjustOverscanX") == 0)
	{
		AdjustOverscanX = atoi(value);
	}
	else if(strcmp(name, "AdjustOverscanY") == 0)
	{
		AdjustOverscanY = atoi(value);
	}
	else if(strcmp(name, "TooltipDelay") == 0)
	{
		TooltipDelay = atoi(value);
	}
	else if(strcmp(name, "BannerZoomDuration") == 0)
	{
		BannerZoomDuration = atoi(value);
	}
	else if(strcmp(name, "GameWindowMode") == 0)
	{
		GameWindowMode = atoi(value);
	}
	else if(strcmp(name, "BannerAnimStart") == 0)
	{
		BannerAnimStart = atoi(value);
	}
	else if(strcmp(name, "CacheBNRFiles") == 0)
	{
		CacheBNRFiles = atoi(value);
	}
	else if (strcmp(name, "InstallPartitions") == 0)
	{
		InstallPartitions = strtoul(value, 0, 16);
		return true;
	}
	else if (strcmp(name, "WSFactor") == 0)
	{
		WSFactor = atof(value);
		return true;
	}
	else if (strcmp(name, "FontScaleFactor") == 0)
	{
		FontScaleFactor = atof(value);
		return true;
	}
	else if (strcmp(name, "ClockFontScaleFactor") == 0)
	{
		ClockFontScaleFactor = atof(value);
		return true;
	}
	else if (strcmp(name, "PointerSpeed") == 0)
	{
		PointerSpeed = atof(value);
		return true;
	}
	else if (strcmp(name, "BannerGridSpeed") == 0)
	{
		BannerGridSpeed = atof(value);
		return true;
	}
	else if (strcmp(name, "BannerProjectionOffsetX") == 0)
	{
		BannerProjectionOffsetX = atof(value);
		return true;
	}
	else if (strcmp(name, "BannerProjectionOffsetY") == 0)
	{
		BannerProjectionOffsetY = atof(value);
		return true;
	}
	else if (strcmp(name, "BannerProjectionWidth") == 0)
	{
		BannerProjectionWidth = atof(value);
		return true;
	}
	else if (strcmp(name, "BannerProjectionHeight") == 0)
	{
		BannerProjectionHeight = atof(value);
		return true;
	}
	else if (strcmp(name, "GCBannerScale") == 0)
	{
		GCBannerScale = atof(value);
		return true;
	}
	else if (strcmp(name, "ParentalBlocks") == 0)
	{
		ParentalBlocks = strtoul(value, 0, 16);
		return true;
	}
	else if (strcmp(name, "GameCubeMode") == 0)
	{
		GameCubeMode = atoi(value);
		return true;
	}
	else if (strcmp(name, "GameCubeSource") == 0)
	{
		GameCubeSource = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLVideo") == 0)
	{
		DMLVideo = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLProgPatch") == 0)
	{
		DMLProgPatch = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLNMM") == 0)
	{
		DMLNMM = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLActivityLED") == 0)
	{
		DMLActivityLED = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLPADHOOK") == 0)
	{
		DMLPADHOOK = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLNoDisc2") == 0)
	{
		DMLNoDisc2 = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLWidescreen") == 0)
	{
		DMLWidescreen = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLScreenshot") == 0)
	{
		DMLScreenshot = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLJPNPatch") == 0)
	{
		DMLJPNPatch = atoi(value);
		return true;
	}
	else if (strcmp(name, "DMLDebug") == 0)
	{
		DMLDebug = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINDeflicker") == 0)
	{
		NINDeflicker = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINMCEmulation") == 0)
	{
		NINMCEmulation = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINMCSize") == 0)
	{
		NINMCSize = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINAutoboot") == 0)
	{
		NINAutoboot = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINSettings") == 0)
	{
		NINSettings = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINUSBHID") == 0)
	{
		NINUSBHID = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINMaxPads") == 0)
	{
		NINMaxPads = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINNativeSI") == 0)
	{
		NINNativeSI = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINWiiUWide") == 0)
	{
		NINWiiUWide = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINOSReport") == 0)
	{
		NINOSReport = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINLED") == 0)
	{
		NINLED = atoi(value);
		return true;
	}
	else if (strcmp(name, "NINLog") == 0)
	{
		NINLog = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOMCEmulation") == 0)
	{
		DEVOMCEmulation = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOWidescreen") == 0)
	{
		DEVOWidescreen = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOActivityLED") == 0)
	{
		DEVOActivityLED = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOFZeroAX") == 0)
	{
		DEVOFZeroAX = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOTimerFix") == 0)
	{
		DEVOTimerFix = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVODButtons") == 0)
	{
		DEVODButtons = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOCropOverscan") == 0)
	{
		DEVOCropOverscan = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVODiscDelay") == 0)
	{
		DEVODiscDelay = atoi(value);
		return true;
	}
	else if (strcmp(name, "DEVOLoaderPath") == 0)
	{
		strlcpy(DEVOLoaderPath, value, sizeof(DEVOLoaderPath));
		return true;
	}
	else if (strcmp(name, "NINLoaderPath") == 0)
	{
		strlcpy(NINLoaderPath, value, sizeof(NINLoaderPath));
		return true;
	}
	else if (strcmp(name, "GCInstallCompressed") == 0)
	{
		GCInstallCompressed = atoi(value);
		return true;
	}
	else if (strcmp(name, "GCInstallAligned") == 0)
	{
		GCInstallAligned = atoi(value);
		return true;
	}
	else if (strcmp(name, "covers_path") == 0)
	{
		strlcpy(covers_path, value, sizeof(covers_path));
		return true;
	}
	else if (strcmp(name, "covers2d_path") == 0)
	{
		strlcpy(covers2d_path, value, sizeof(covers2d_path));
		return true;
	}
	else if (strcmp(name, "coversFull_path") == 0)
	{
		strlcpy(coversFull_path, value, sizeof(coversFull_path));
		return true;
	}
	else if (strcmp(name, "theme_path") == 0)
	{
		strlcpy(theme_path, value, sizeof(theme_path));
		return true;
	}
	else if (strcmp(name, "theme") == 0)
	{
		strlcpy(theme, value, sizeof(theme));
		return true;
	}
	else if (strcmp(name, "disc_path") == 0)
	{
		strlcpy(disc_path, value, sizeof(disc_path));
		return true;
	}
	else if (strcmp(name, "language_path") == 0)
	{
		strlcpy(language_path, value, sizeof(language_path));
		return true;
	}
	else if (strcmp(name, "languagefiles_path") == 0)
	{
		strlcpy(languagefiles_path, value, sizeof(languagefiles_path));
		return true;
	}
	else if (strcmp(name, "TxtCheatcodespath") == 0)
	{
		strlcpy(TxtCheatcodespath, value, sizeof(TxtCheatcodespath));
		return true;
	}
	else if (strcmp(name, "titlestxt_path") == 0)
	{
		strlcpy(titlestxt_path, value, sizeof(titlestxt_path));
		return true;
	}
	else if (strcmp(name, "dolpath") == 0)
	{
		strlcpy(dolpath, value, sizeof(dolpath));
		return true;
	}
	else if (strcmp(name, "ogg_path") == 0)
	{
		strlcpy(ogg_path, value, sizeof(ogg_path));
		return true;
	}
	else if (strcmp(name, "update_path") == 0)
	{
		strlcpy(update_path, value, sizeof(update_path));
		return true;
	}
	else if (strcmp(name, "homebrewapps_path") == 0)
	{
		strlcpy(homebrewapps_path, value, sizeof(homebrewapps_path));
		return true;
	}
	else if (strcmp(name, "BNRCachePath") == 0)
	{
		strlcpy(BNRCachePath, value, sizeof(BNRCachePath));
		return true;
	}
	else if (strcmp(name, "Cheatcodespath") == 0)
	{
		strlcpy(Cheatcodespath, value, sizeof(Cheatcodespath));
		return true;
	}
	else if (strcmp(name, "BcaCodepath") == 0)
	{
		strlcpy(BcaCodepath, value, sizeof(BcaCodepath));
		return true;
	}
	else if (strcmp(name, "WipCodepath") == 0)
	{
		strlcpy(WipCodepath, value, sizeof(WipCodepath));
		return true;
	}
	else if (strcmp(name, "WDMpath") == 0)
	{
		strlcpy(WDMpath, value, sizeof(WDMpath));
		return true;
	}
	else if (strcmp(name, "returnTo") == 0)
	{
		strlcpy(returnTo, value, sizeof(returnTo));
		return true;
	}
	else if (strcmp(name, "WiinnertagPath") == 0)
	{
		strlcpy(WiinnertagPath, value, sizeof(WiinnertagPath));
		return true;
	}
	else if (strcmp(name, "NandEmuPath") == 0)
	{
		strlcpy(NandEmuPath, value, sizeof(NandEmuPath));
		return true;
	}
	else if (strcmp(name, "NandEmuChanPath") == 0)
	{
		strlcpy(NandEmuChanPath, value, sizeof(NandEmuChanPath));
		return true;
	}
	else if (strcmp(name, "GameCubePath") == 0)
	{
		strlcpy(GameCubePath, value, sizeof(GameCubePath));
		return true;
	}
	else if (strcmp(name, "GameCubeSDPath") == 0)
	{
		strlcpy(GameCubeSDPath, value, sizeof(GameCubeSDPath));
		return true;
	}
	else if (strcmp(name, "CustomBannersURL") == 0)
	{
		if( strcmp(value, "http://dl.dropbox.com/u/101209384/") == 0 ||
			strcmp(value, "http://dl.dropboxusercontent.com/u/101209384/") == 0)
			strlcpy(CustomBannersURL, "http://copy.com/vRN3HgFVyk9u7YuB/Public/", sizeof(CustomBannersURL)); // update banner URL
		else
			strlcpy(CustomBannersURL, value, sizeof(CustomBannersURL));
		return true;
	}
	else if(strcmp(name, "PrivateServer") == 0)
	{
		PrivateServer = atoi(value);
	}
	else if (strcmp(name, "EnabledCategories") == 0)
	{
		EnabledCategories.clear();
		char * strTok = strtok(value, ",");
		while (strTok != NULL)
		{
			u32 id  = atoi(strTok);
			u32 i;
			for(i = 0; i < EnabledCategories.size(); ++i)
			{
				if(EnabledCategories[i] == id)
					break;
			}
			if(i == EnabledCategories.size())
				EnabledCategories.push_back(id);
			strTok = strtok(NULL,",");
		}
		return true;
	}
	else if (strcmp(name, "RequiredCategories") == 0)
	{
		RequiredCategories.clear();
		char * strTok = strtok(value, ",");
		while (strTok != NULL)
		{
			u32 id  = atoi(strTok);
			u32 i;
			for(i = 0; i < RequiredCategories.size(); ++i)
			{
				if(RequiredCategories[i] == id)
					break;
			}
			if(i == RequiredCategories.size())
				RequiredCategories.push_back(id);
			strTok = strtok(NULL,",");
		}
		return true;
	}
	else if (strcmp(name, "ForbiddenCategories") == 0)
	{
		ForbiddenCategories.clear();
		char * strTok = strtok(value, ",");
		while (strTok != NULL)
		{
			u32 id  = atoi(strTok);
			u32 i;
			for(i = 0; i < ForbiddenCategories.size(); ++i)
			{
				if(ForbiddenCategories[i] == id)
					break;
			}
			if(i == ForbiddenCategories.size())
				ForbiddenCategories.push_back(id);
			strTok = strtok(NULL,",");
		}
		return true;
	}

	return false;
}

bool CSettings::FindConfig()
{
	bool found = false;
	char CheckDevice[12];
	char CheckPath[300];

	// Enumerate the devices supported by libogc.
	for (int i = SD; (i < MAXDEVICES) && !found; ++i)
	{
		snprintf(CheckDevice, sizeof(CheckDevice), "%s:", DeviceName[i]);

		if(!found)
		{
			// Check for the config file in the apps directory.
			strlcpy(BootDevice, CheckDevice, sizeof(BootDevice));
			snprintf(ConfigPath, sizeof(ConfigPath), "%s/apps/usbloader_gx/", BootDevice);
			snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
			found = CheckFile(CheckPath);
		}
		if(!found)
		{
			// Check for the config file in the config directory.
			strlcpy(BootDevice, CheckDevice, sizeof(BootDevice));
			snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/", BootDevice);
			snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
			found = CheckFile(CheckPath);
		}
	}

	FILE * testFp = NULL;
	//! No existing config so try to find a place where we can write it too
	for (int i = SD; (i < MAXDEVICES) && !found; ++i)
	{
		sprintf(CheckDevice, "%s:", DeviceName[i]);

		if (!found)
		{
			// Check if we can write to the apps directory.
			strlcpy(BootDevice, CheckDevice, sizeof(BootDevice));
			snprintf(ConfigPath, sizeof(ConfigPath), "%s/apps/usbloader_gx/", BootDevice);
			snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
			testFp = fopen(CheckPath, "wb");
			found = (testFp != NULL);
			if(testFp) fclose(testFp);
		}
		if (!found)
		{
			// Check if we can write to the config directory.
			strlcpy(BootDevice, CheckDevice, sizeof(BootDevice));
			snprintf(ConfigPath, sizeof(ConfigPath), "%s/config/", BootDevice);
			CreateSubfolder(ConfigPath);
			snprintf(CheckPath, sizeof(CheckPath), "%sGXGlobal.cfg", ConfigPath);
			testFp = fopen(CheckPath, "wb");
			found = (testFp != NULL);
			if(testFp) fclose(testFp);
		}
	}

	return found;
}

void CSettings::ParseLine(char *line)
{
	char temp[1024], name[1024], value[1024];

	strncpy(temp, line, sizeof(temp));

	char * eq = strchr(temp, '=');

	if (!eq) return;

	*eq = 0;

	this->TrimLine(name, temp, sizeof(name));
	this->TrimLine(value, eq + 1, sizeof(value));

	this->SetSetting(name, value);
}

void CSettings::TrimLine(char *dest, char *src, int size)
{
	int len;
	while (*src == ' ')
		src++;
	len = strlen(src);
	while (len > 0 && strchr(" \r\n", src[len - 1]))
		len--;
	if (len >= size) len = size - 1;
	strncpy(dest, src, len);
	dest[len] = 0;
}

//! Get language code from the selected language file
//! eg. german.lang = DE and default to EN
static inline const char * GetLangCode(const char * langpath)
{
	if(strcasestr(langpath, "japanese"))
		return "JA";

	else if(strcasestr(langpath, "german"))
		return "DE";

	else if(strcasestr(langpath, "french"))
		return "FR";

	else if(strcasestr(langpath, "spanish"))
		return "ES";

	else if(strcasestr(langpath, "italian"))
		return "IT";

	else if(strcasestr(langpath, "dutch"))
		return "NL";

	else if(strcasestr(langpath, "schinese"))
		return "ZHCN";

	else if(strcasestr(langpath, "tchinese"))
		return "ZHTW";

	else if(strcasestr(langpath, "korean"))
		return "KO";

	return "EN";
}

bool CSettings::LoadLanguage(const char *path, int lang)
{
	bool ret = false;

	if (path && strlen(path) > 3)
	{
		ret = gettextLoadLanguage(path);
		if (ret)
		{
			strlcpy(language_path, path, sizeof(language_path));
			strlcpy(db_language, GetLangCode(language_path), sizeof(db_language));
		}
		else
			return LoadLanguage(NULL, CONSOLE_DEFAULT);
	}
	else if (lang >= 0)
	{
		char filepath[150];
		char langpath[150];
		strlcpy(langpath, languagefiles_path, sizeof(langpath));
		if (langpath[strlen(langpath) - 1] != '/')
		{
			char * ptr = strrchr(langpath, '/');
			if (ptr)
			{
				ptr++;
				ptr[0] = '\0';
			}
		}

		if (lang == CONSOLE_DEFAULT)
		{
			return LoadLanguage(NULL, CONF_GetLanguage());
		}
		else if (lang == JAPANESE)
		{
			snprintf(filepath, sizeof(filepath), "%s/japanese.lang", langpath);
		}
		else if (lang == ENGLISH)
		{
			snprintf(filepath, sizeof(filepath), "%s/english.lang", langpath);
		}
		else if (lang == GERMAN)
		{
			snprintf(filepath, sizeof(filepath), "%s/german.lang", langpath);
		}
		else if (lang == FRENCH)
		{
			snprintf(filepath, sizeof(filepath), "%s/french.lang", langpath);
		}
		else if (lang == SPANISH)
		{
			snprintf(filepath, sizeof(filepath), "%s/spanish.lang", langpath);
		}
		else if (lang == ITALIAN)
		{
			snprintf(filepath, sizeof(filepath), "%s/italian.lang", langpath);
		}
		else if (lang == DUTCH)
		{
			snprintf(filepath, sizeof(filepath), "%s/dutch.lang", langpath);
		}
		else if (lang == S_CHINESE)
		{
			snprintf(filepath, sizeof(filepath), "%s/schinese.lang", langpath);
		}
		else if (lang == T_CHINESE)
		{
			snprintf(filepath, sizeof(filepath), "%s/tchinese.lang", langpath);
		}
		else if (lang == KOREAN)
		{
			snprintf(filepath, sizeof(filepath), "%s/korean.lang", langpath);
		}

		strlcpy(db_language, GetLangCode(filepath), sizeof(db_language));
		ret = gettextLoadLanguage(filepath);
		if (ret)
			strlcpy(language_path, filepath, sizeof(language_path));
	}

	return ret;
}
