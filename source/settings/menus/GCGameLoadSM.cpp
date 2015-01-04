/****************************************************************************
 * Copyright (C) 2012-2014 by Cyan
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <unistd.h>
#include <gccore.h>
#include "settings/CSettings.h"
#include "settings/CGameStatistics.h"
#include "themes/CTheme.h"
#include "prompts/PromptWindows.h"
#include "prompts/DiscBrowser.h"
#include "prompts/filebrowser.h"
#include "usbloader/AlternateDOLOffsets.h"
#include "language/gettext.h"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "GCGameLoadSM.hpp"

static const char * OnOffText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Auto" )
};

static const char * LanguageText[] =
{
	trNOOP( "English" ),
	trNOOP( "German" ),
	trNOOP( "French" ),
	trNOOP( "Spanish" ),
	trNOOP( "Italian" ),
	trNOOP( "Dutch" ),
	trNOOP( "Console Default" ),
};

static const char * ParentalText[] =
{
	trNOOP( "0 (Everyone)" ),
	trNOOP( "1 (Child 7+)" ),
	trNOOP( "2 (Teen 12+)" ),
	trNOOP( "3 (Mature 16+)" ),
	trNOOP( "4 (Adults Only 18+)" )
};

static const char * GCMode[] =
{
	trNOOP( "MIOS (Default & Customs)" ),
	trNOOP( "Devolution" ),
	trNOOP( "Nintendont" ),
};

static const char * DMLVideoText[] =
{
	trNOOP( "Auto" ),
	trNOOP( "System Default" ),
	trNOOP( "Disc Default" ),
	trNOOP( "Force PAL50" ),
	trNOOP( "Force PAL60" ),
	trNOOP( "Force NTSC" ),
	"", // unused
	trNOOP( "Force PAL480p" ),
	trNOOP( "Force NTSC480p" ),
	trNOOP( "None" ),
};

static const char * DMLNMMMode[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Debug" ),
};

static const char * DMLDebug[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Debug Wait" ),
};

static const char * DEVOMCText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "ON" ),
	trNOOP( "Individual" ),
};

static const char * NINMCText[] =
{
	trNOOP( "OFF" ),
	trNOOP( "Individual" ),
	trNOOP( "ON (Multi)" ),
};

static int currentGCmode = 0;

GCGameLoadSM::GCGameLoadSM(struct discHdr *hdr)
	: SettingsMenu(tr("Game Load"), &GuiOptions, MENU_NONE),
	  Header(hdr)
{
	GameConfig = *GameSettings.GetGameCFG((const char *) Header->id);

	if(!btnOutline)
		btnOutline = Resources::GetImageData("button_dialogue_box.png");
	if(!trigA)
		trigA = new GuiTrigger();
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	saveBtnTxt = new GuiText(tr( "Save" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	saveBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
	saveBtnImg = new GuiImage (btnOutline);
	if (Settings.wsprompt == ON)
	{
		saveBtnTxt->SetWidescreen(Settings.widescreen);
		saveBtnImg->SetWidescreen(Settings.widescreen);
	}
	saveBtn = new GuiButton(saveBtnImg, saveBtnImg, 2, 3, 180, 400, trigA, btnSoundOver, btnSoundClick2, 1);
	saveBtn->SetLabel(saveBtnTxt);
	Append(saveBtn);

	currentGCmode = GameConfig.GameCubeMode == INHERIT ? Settings.GameCubeMode : GameConfig.GameCubeMode;
	
	SetOptionNames();
	SetOptionValues();
}

GCGameLoadSM::~GCGameLoadSM()
{
	HaltGui();
	//! The rest is destroyed in SettingsMenu.cpp
	Remove(saveBtn);
	delete saveBtnTxt;
	delete saveBtnImg;
	delete saveBtn;
	ResumeGui();
}

void GCGameLoadSM::SetDefaultConfig()
{
	char id[7];
	snprintf(id, sizeof(id), GameConfig.id);
	GameSettings.SetDefault(GameConfig);
	snprintf(GameConfig.id, sizeof(GameConfig.id), id);
}

void GCGameLoadSM::SetOptionNames()
{
	int Idx = 0;

	Options->SetName(Idx++, "%s", tr( "Game Lock" ));
	Options->SetName(Idx++, "%s", tr( "Favorite Level" ));
	Options->SetName(Idx++, "%s", tr( "Game Language" ));
	Options->SetName(Idx++, "%s", tr( "Parental Control" ));
	Options->SetName(Idx++, "%s", tr( "GameCube Mode" ));
	if(currentGCmode == GC_MODE_MIOS &&IosLoader::GetMIOSInfo() > DEFAULT_MIOS)
	{
		Options->SetName(Idx++, "%s", tr( "--==   DIOS MIOS (Lite) " ));
		Options->SetName(Idx++, "%s", tr( "Video Mode" ));
		Options->SetName(Idx++, "%s", tr( "Progressive Patch" ));
		if(IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_1)
			Options->SetName(Idx++, "%s", tr( "Force Widescreen" ));
		Options->SetName(Idx++, "%s", tr( "Ocarina" ));
		Options->SetName(Idx++, "%s", tr( "NMM Mode" ));
		Options->SetName(Idx++, "%s", tr( "Debug" ));
		Options->SetName(Idx++, "%s", tr( "LED Activity" ));
		Options->SetName(Idx++, "%s", tr( "PAD Hook" ));
		if(IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_2_2 && IosLoader::GetDMLVersion() <= DML_VERSION_DML_2_2_1)
			Options->SetName(Idx++, "%s", tr( "No Disc+" ));
		if(IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_5)
			Options->SetName(Idx++, "%s", tr( "Screenshot" ));
		Options->SetName(Idx++, "%s", tr( "Japanese Patch" ));
	}
	if(currentGCmode == GC_MODE_NINTENDONT)
	{
		Options->SetName(Idx++, "%s", tr( "--==       Nintendont" ));
		Options->SetName(Idx++, "%s", tr( "Video Mode" ));
		Options->SetName(Idx++, "%s", tr( "Progressive Patch" ));
		Options->SetName(Idx++, "%s", tr( "Video Deflicker" ));
		Options->SetName(Idx++, "%s", tr( "Force Widescreen" ));
		Options->SetName(Idx++, "%s", tr( "Ocarina" ));
		Options->SetName(Idx++, "%s", tr( "Memory Card Emulation" ));
		Options->SetName(Idx++, "%s", tr( "Memory Card Blocks Size" ));
		Options->SetName(Idx++, "%s", tr( "USB-HID Controller" ));
		Options->SetName(Idx++, "%s", tr( "GameCube Controller" ));
		Options->SetName(Idx++, "%s", tr( "Native Controller" ));
		Options->SetName(Idx++, "%s", tr( "WiiU Widescreen" ));
		Options->SetName(Idx++, "%s", tr( "LED Activity" ));
		Options->SetName(Idx++, "%s", tr( "Debug" ));
		Options->SetName(Idx++, "%s", tr( "OSReport" ));
		Options->SetName(Idx++, "%s", tr( "Log to file" ));
		Options->SetName(Idx++, "%s", tr( "Nintendont Loader Path" ));
	}
	if(currentGCmode == GC_MODE_DEVOLUTION)
	{
		Options->SetName(Idx++, "%s", tr( "--==       Devolution" ));
		Options->SetName(Idx++, "%s", tr( "Memory Card Emulation" ));
		Options->SetName(Idx++, "%s", tr( "Force Widescreen" ));
		Options->SetName(Idx++, "%s", tr( "LED Activity" ));
		Options->SetName(Idx++, "%s", tr( "F-Zero AX" ));
		Options->SetName(Idx++, "%s", tr( "Timer Fix" ));
		Options->SetName(Idx++, "%s", tr( "D Buttons" ));
		Options->SetName(Idx++, "%s", tr( "Crop Overscan" ));
		Options->SetName(Idx++, "%s", tr( "Disc Read Delay" ));
	}
}

void GCGameLoadSM::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Game Lock
	Options->SetValue(Idx++, "%s", tr( OnOffText[GameConfig.Locked] ));

	//! Settings: Favorite Level
	Options->SetValue(Idx++, "%i", GameStatistics.GetFavoriteRank(Header->id));

	//! Settings: Game Language
	if(GameConfig.language == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(LanguageText[GameConfig.language]));

	//! Settings: Parental Control
	Options->SetValue(Idx++, "%s", tr(ParentalText[GameConfig.parentalcontrol]));

	//! Settings: GameCube Mode
	if(GameConfig.GameCubeMode == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(GCMode[GameConfig.GameCubeMode]));
	
	if(currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS)
	{

		//! Settings: GameCube TITLE : DIOS MIOS (Lite) + Nintendont
		Options->SetValue(Idx++, "==--   ");
	
		//! Settings: DML + NIN Video Mode
		if(GameConfig.DMLVideo == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(DMLVideoText[GameConfig.DMLVideo]));

		//! Settings: DML + NIN Progressive Patch
		if(GameConfig.DMLProgPatch == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLProgPatch]));

		//! Settings: DML + NIN Force Widescreen
		if(IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_1)
		{
			if(GameConfig.DMLWidescreen == INHERIT)
				Options->SetValue(Idx++, tr("Use global"));
			else
				Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLWidescreen]));
		}

		//! Settings: Ocarina
		if(GameConfig.ocarina == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.ocarina]));

		//! Settings: DML + NIN NMM Mode
		if(GameConfig.DMLNMM == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(DMLNMMMode[GameConfig.DMLNMM]));

		//! Settings: DML + NIN Debug
		if(GameConfig.DMLDebug == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(DMLDebug[GameConfig.DMLDebug]));
		
		//! Settings: DML LED Activity
		if(GameConfig.DMLActivityLED == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLActivityLED]));

		//! Settings: DML PAD Hook
		if(GameConfig.DMLPADHOOK == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLPADHOOK]));

		//! Settings: DML Extended No Disc
		if(IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_2_2 && IosLoader::GetDMLVersion() <= DML_VERSION_DML_2_2_1)
		{
			if(GameConfig.DMLNoDisc2 == INHERIT)
				Options->SetValue(Idx++, tr("Use global"));
			else
				Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLNoDisc2]));
		}

		//! Settings: DML Screenshot
		if(IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_5)
		{
			if(GameConfig.DMLScreenshot == INHERIT)
				Options->SetValue(Idx++, tr("Use global"));
			else
				Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLScreenshot]));
		}

		//! Settings: DML Japanese Patch
		if(GameConfig.DMLJPNPatch == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLJPNPatch]));
	}
	
	if(currentGCmode == GC_MODE_NINTENDONT)
	{

		//! Settings: GameCube TITLE : DIOS MIOS (Lite) + Nintendont
		Options->SetValue(Idx++, "==--   ");
	
		//! Settings: DML + NIN Video Mode
		if(GameConfig.DMLVideo == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(DMLVideoText[GameConfig.DMLVideo]));

		//! Settings: DML + NIN Progressive Patch
		if(GameConfig.DMLProgPatch == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLProgPatch]));

		//! Settings: NIN Video Deflicker
		if(GameConfig.NINDeflicker == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINDeflicker]));

		//! Settings: DML + NIN Force Widescreen
		if(GameConfig.DMLWidescreen == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLWidescreen]));

		//! Settings: Ocarina
		if(GameConfig.ocarina == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.ocarina]));

		//! Settings: NIN Memory Card Emulation
		if(GameConfig.NINMCEmulation == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(NINMCText[GameConfig.NINMCEmulation]));

		//! Settings: NIN Memory Card Blocks Size
		if(GameConfig.NINMCSize == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%d", MEM_CARD_BLOCKS(GameConfig.NINMCSize));
		
		//! Settings: NIN USB-HID Controller
		if(GameConfig.NINUSBHID == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINUSBHID]));
		
		//! Settings: NIN MaxPads - Number of GameCube Controllers
		if(GameConfig.NINMaxPads == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%i", GameConfig.NINMaxPads);
		
		//! Settings: NIN Native Controller
		if(GameConfig.NINNativeSI == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINNativeSI]));
			
		//! Settings: WiiU Widescreen
		if(GameConfig.NINWiiUWide == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINWiiUWide]));	
		
		//! Settings: NIN LED Activity
		if(GameConfig.NINLED == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINLED]));
		
		//! Settings: DML + NIN Debug
		if(GameConfig.DMLDebug == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(DMLDebug[GameConfig.DMLDebug]));
		
		//! Settings: NIN OS Report
		if(GameConfig.NINOSReport == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINOSReport]));
		
		//! Settings: NIN Log to file
		if(GameConfig.NINLog == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.NINLog]));
		
		//! Settings: NIN Individual Loader path setting
		if(GameConfig.NINLoaderPath.size() == 0)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", GameConfig.NINLoaderPath.c_str());
		
	}
	
	if(currentGCmode == GC_MODE_DEVOLUTION)
	{
		
		//! Settings: GameCube TITLE : Devolution
		Options->SetValue(Idx++, "==--   ");
		
		//! Settings: DEVO Memory Card Emulation
		if(GameConfig.DEVOMCEmulation == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(DEVOMCText[GameConfig.DEVOMCEmulation]));

		//! Settings: DEVO Widescreen Patch
		if(GameConfig.DEVOWidescreen == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVOWidescreen]));

		//! Settings: DEVO Activity LED
		if(GameConfig.DEVOActivityLED == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVOActivityLED]));

		//! Settings: DEVO F-Zero AX unlock patch
		if(GameConfig.DEVOFZeroAX == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVOFZeroAX]));

		//! Settings: DEVO Timer Fix
		if(GameConfig.DEVOTimerFix == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVOTimerFix]));

		//! Settings: DEVO Direct Button Mapping
		if(GameConfig.DEVODButtons == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVODButtons]));

		//! Settings: DEVO Crop Overscan
		if(GameConfig.DEVOCropOverscan == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVOCropOverscan]));

		//! Settings: DEVO Disc Read Delay
		if(GameConfig.DEVODiscDelay == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DEVODiscDelay]));
		
	}
}

int GCGameLoadSM::GetMenuInternal()
{
	if (saveBtn->GetState() == STATE_CLICKED)
	{
		if (GameSettings.AddGame(GameConfig) && GameSettings.Save())
		{
			WindowPrompt(tr( "Successfully Saved" ), 0, tr( "OK" ));
		}
		else
			WindowPrompt(tr( "Save Failed. No device inserted?" ), 0, tr( "OK" ));

		saveBtn->ResetState();
	}

	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: Game Lock
	if (ret == ++Idx)
	{
		if (++GameConfig.Locked >= MAX_ON_OFF) GameConfig.Locked = 0;
	}

	//! Settings: Favorite Level
	else if (ret == ++Idx)
	{
		int Level = GameStatistics.GetFavoriteRank(Header->id);
		if (++Level > 5) Level = 0;

		GameStatistics.SetFavoriteRank(Header->id, Level);
		GameStatistics.Save();
	}

	//! Settings: Game Language
	else if (ret == ++Idx)
	{
		if (++GameConfig.language >= GC_MAX_LANGUAGE) GameConfig.language = INHERIT;
	}

	//! Settings: Parental Control
	else if (ret == ++Idx)
	{
		if (++GameConfig.parentalcontrol >= 5) GameConfig.parentalcontrol = 0;
	}

	//! Settings: GameCube Mode
	else if (ret == ++Idx)
	{
		if (++GameConfig.GameCubeMode >= CG_MODE_MAX_CHOICE) GameConfig.GameCubeMode = INHERIT;
		currentGCmode = GameConfig.GameCubeMode == INHERIT ? Settings.GameCubeMode : GameConfig.GameCubeMode;
			Options->ClearList();
			SetOptionNames();
			SetOptionValues();
	}

	//! Settings: GameCube TITLE : DIOS MIOS (Lite) + Nintendont
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: DML Video Mode
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		GameConfig.DMLVideo++;
		if(GameConfig.DMLVideo == DML_VIDEO_FORCE_PATCH) // Skip Force Patch
			GameConfig.DMLVideo++;
		if(GameConfig.DMLVideo >= DML_VIDEO_MAX_CHOICE) GameConfig.DMLVideo = INHERIT;
	}

	//! Settings: DML Progressive Patch
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.DMLProgPatch >= MAX_ON_OFF) GameConfig.DMLProgPatch = INHERIT;
	}

	//! Settings: DML Force Widescreen
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_1 && ret == ++Idx)
	{
		if (++GameConfig.DMLWidescreen >= MAX_ON_OFF) GameConfig.DMLWidescreen = INHERIT;
	}

	//! Settings: Ocarina
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.ocarina >= MAX_ON_OFF) GameConfig.ocarina = INHERIT;
	}

	//! Settings: DML NMM Mode
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.DMLNMM >= 3) GameConfig.DMLNMM = INHERIT;
	}

	//! Settings: DML Debug
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.DMLDebug >= 3) GameConfig.DMLDebug = INHERIT;
	}

	//! Settings: DML LED Activity
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.DMLActivityLED >= MAX_ON_OFF) GameConfig.DMLActivityLED = INHERIT;
	}

	//! Settings: DML PAD Hook
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.DMLPADHOOK >= MAX_ON_OFF) GameConfig.DMLPADHOOK = INHERIT;
	}

	//! Settings: DML Extended No Disc
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_2_2 && IosLoader::GetDMLVersion() <= DML_VERSION_DML_2_2_1 && ret == ++Idx)
	{
		if (++GameConfig.DMLNoDisc2 >= MAX_ON_OFF) GameConfig.DMLNoDisc2 = INHERIT;
	}

	//! Settings: DML Screenshot
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && IosLoader::GetDMLVersion() >= DML_VERSION_DM_2_5 && ret == ++Idx)
	{
		if (++GameConfig.DMLScreenshot >= MAX_ON_OFF) GameConfig.DMLScreenshot = INHERIT;
	}

	//! Settings: DML Japanese Patch
	else if (currentGCmode == GC_MODE_MIOS && IosLoader::GetMIOSInfo() > DEFAULT_MIOS && ret == ++Idx)
	{
		if (++GameConfig.DMLJPNPatch >= MAX_ON_OFF) GameConfig.DMLJPNPatch = INHERIT;
	}

	//! Settings: GameCube TITLE : DIOS MIOS (Lite) + Nintendont
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: NIN Video Mode
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		GameConfig.DMLVideo++;
		if(GameConfig.DMLVideo == DML_VIDEO_FORCE_PATCH) // Skip Force Patch
			GameConfig.DMLVideo++;
		if(GameConfig.DMLVideo >= DML_VIDEO_MAX_CHOICE) GameConfig.DMLVideo = INHERIT;
	}

	//! Settings: NIN Progressive Patch
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.DMLProgPatch >= MAX_ON_OFF) GameConfig.DMLProgPatch = INHERIT;
	}

	//! Settings: NIN Video Deflicker
	if(currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINDeflicker >= MAX_ON_OFF) GameConfig.NINDeflicker = INHERIT;
	}

	//! Settings: NIN Force Widescreen
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.DMLWidescreen >= MAX_ON_OFF) GameConfig.DMLWidescreen = INHERIT;
	}

	//! Settings: Ocarina
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.ocarina >= MAX_ON_OFF) GameConfig.ocarina = INHERIT;
	}

	//! Settings: NIN Memory Card Emulation
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINMCEmulation >= NIN_MC_MAX_CHOICE) GameConfig.NINMCEmulation = INHERIT;
	}

	//! Settings: NIN Memory Card Blocks Size
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINMCSize >= 6) GameConfig.NINMCSize = INHERIT;
		if (GameConfig.NINMCSize == 5)
			WindowPrompt(tr("Warning:"), tr("Memory Card with 2043 blocs has issues with Nintendont. Use at your own risk."), tr("Ok"));
	}

	//! Settings: NIN USB-HID Controller
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINUSBHID >= MAX_ON_OFF) GameConfig.NINUSBHID = INHERIT;
	}

	//! Settings: NIN MaxPads - Number of GameCube Controllers
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINMaxPads >= 5) GameConfig.NINMaxPads = INHERIT;
	}

	//! Settings: NIN Native Controller
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINNativeSI >= MAX_ON_OFF) GameConfig.NINNativeSI = INHERIT;
	}
	
	//! Settings: WiiU Widescreen
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINWiiUWide >= MAX_ON_OFF) GameConfig.NINWiiUWide = INHERIT;
	}

	//! Settings: NIN LED Activity
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINLED >= MAX_ON_OFF) GameConfig.NINLED = INHERIT;
	}

	//! Settings: NIN Debug
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.DMLDebug >= 3) GameConfig.DMLDebug = INHERIT;
	}

	//! Settings: NIN OS Report
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINOSReport >= MAX_ON_OFF) GameConfig.NINOSReport = INHERIT;
	}

	//! Settings: NIN Log to file
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		if (++GameConfig.NINLog >= MAX_ON_OFF) GameConfig.NINLog = INHERIT;
	}

	//! Settings: NIN Individual Loader path setting
	else if (currentGCmode == GC_MODE_NINTENDONT && ret == ++Idx)
	{
		char entered[100];
		snprintf(entered, sizeof(entered), GameConfig.NINLoaderPath.c_str());

		HaltGui();
		GuiWindow * parent = (GuiWindow *) parentElement;
		if(parent) parent->SetState(STATE_DISABLED);
		this->SetState(STATE_DEFAULT);
		this->Remove(optionBrowser);
		ResumeGui();

		int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);

		if(parent) parent->SetState(STATE_DEFAULT);
		this->Append(optionBrowser);

		if (result == 1)
		{
			if (entered[strlen(entered)-1] != '/')
				strcat(entered, "/");

			GameConfig.NINLoaderPath = entered;
			WindowPrompt(tr( "Path Changed" ), 0, tr( "OK" ));
		}
	}

	//! Settings: GameCube TITLE : Devolution
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		// This one is a category title
	}

	//! Settings: DEVO MemCard emulation
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVOMCEmulation >= DEVO_MC_MAX_CHOICE) GameConfig.DEVOMCEmulation = INHERIT;
	}

	//! Settings: DEVO Widescreen Patch
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVOWidescreen >= MAX_ON_OFF) GameConfig.DEVOWidescreen = INHERIT;
	}

	//! Settings: DEVO Activity LED
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVOActivityLED >= MAX_ON_OFF) GameConfig.DEVOActivityLED = INHERIT;
	}

	//! Settings: DEVO F-Zero AX unlock patch
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVOFZeroAX >= MAX_ON_OFF) GameConfig.DEVOFZeroAX = INHERIT;
	}

	//! Settings: DEVO Timer Fix
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVOTimerFix >= MAX_ON_OFF) GameConfig.DEVOTimerFix = INHERIT;
	}

	//!Settings: DEVO Direct Button Mapping
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVODButtons >= MAX_ON_OFF) GameConfig.DEVODButtons = INHERIT;
	}

	//!Settings: DEVO Crop Overscan
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVOCropOverscan >= MAX_ON_OFF) GameConfig.DEVOCropOverscan = INHERIT;
	}

	//!Settings: DEVO Disc Read Delay
	else if (currentGCmode == GC_MODE_DEVOLUTION && ret == ++Idx)
	{
		if (++GameConfig.DEVODiscDelay >= MAX_ON_OFF) GameConfig.DEVODiscDelay = INHERIT;
	}

	SetOptionValues();

	return MENU_NONE;
}

