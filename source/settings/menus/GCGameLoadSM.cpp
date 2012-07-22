/****************************************************************************
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

static const char * VideoModeText[] =
{
	trNOOP( "System Default" ),
	trNOOP( "Disc Default" ),
	trNOOP( "Force PAL50" ),
	trNOOP( "Force PAL60" ),
	trNOOP( "Force NTSC" ),
	trNOOP( "Region Patch" ),
	trNOOP( "Force PAL480p" ),
	trNOOP( "Force NTSC480p" ),
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
	Options->SetName(Idx++, "%s", tr( "Video Mode" ));
	Options->SetName(Idx++, "%s", tr( "Game Language" ));
	Options->SetName(Idx++, "%s", tr( "Ocarina" ));
	Options->SetName(Idx++, "%s", tr( "Parental Control" ));
	Options->SetName(Idx++, "%s", tr( "GameCube Mode" ));
	Options->SetName(Idx++, "%s", tr( "DML Progressive Patch" ));
	Options->SetName(Idx++, "%s", tr( "DML NMM Mode" ));
	Options->SetName(Idx++, "%s", tr( "DML LED Activity" ));
	Options->SetName(Idx++, "%s", tr( "DML PAD Hook" ));
	Options->SetName(Idx++, "%s", tr( "DML No Disc" ));
	if(Settings.DMLConfigVersion > 1)
		Options->SetName(Idx++, "%s", tr( "DML No Disc+" ));
	if(Settings.DMLConfigVersion > 1)
		Options->SetName(Idx++, "%s", tr( "DML Force Widescreen" ));
	Options->SetName(Idx++, "%s", tr( "DML Debug" ));
	Options->SetName(Idx++, "%s", tr( "DEVO MemCard Emulation" ));
}

void GCGameLoadSM::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Game Lock
	Options->SetValue(Idx++, "%s", tr( OnOffText[GameConfig.Locked] ));

	//! Settings: Favorite Level
	Options->SetValue(Idx++, "%i", GameStatistics.GetFavoriteRank(Header->id));

	//! Settings: Video Mode
	if(GameConfig.video == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(VideoModeText[GameConfig.video]));

	//! Settings: Game Language
	if(GameConfig.language == INHERIT)
		GameConfig.language = GC_LANG_CONSOLE_DEFAULT;
	Options->SetValue(Idx++, "%s", tr(LanguageText[GameConfig.language]));
	
	//! Settings: Ocarina
	if(GameConfig.ocarina == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.ocarina]));

	//! Settings: Parental Control
	Options->SetValue(Idx++, "%s", tr(ParentalText[GameConfig.parentalcontrol]));

	//! Settings: GameCube Mode
	if(GameConfig.GameCubeMode == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(GCMode[GameConfig.GameCubeMode]));

	//! Settings: DML Progressive Patch
	if(GameConfig.DMLProgPatch == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLProgPatch]));

	//! Settings: DML NMM Mode
	if(GameConfig.DMLNMM == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(DMLNMMMode[GameConfig.DMLNMM]));

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

	//! Settings: DML No Disc
	if(GameConfig.DMLNoDisc == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLNoDisc]));

	//! Settings: DML Extended No Disc
	if(Settings.DMLConfigVersion > 1)
	{
		if(GameConfig.DMLNoDisc2 == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLNoDisc2]));
	}

	//! Settings: DML Force Widescreen
	if(Settings.DMLConfigVersion > 1)
	{
		if(GameConfig.DMLWidescreen == INHERIT)
			Options->SetValue(Idx++, tr("Use global"));
		else
			Options->SetValue(Idx++, "%s", tr(OnOffText[GameConfig.DMLWidescreen]));
	}

	//! Settings: DML Debug
	if(GameConfig.DMLDebug == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(DMLDebug[GameConfig.DMLDebug]));

	//! Settings: DEVO Memory Card Emulation
	if(GameConfig.DEVOMCEmulation == INHERIT)
		Options->SetValue(Idx++, tr("Use global"));
	else
		Options->SetValue(Idx++, "%s", tr(DEVOMCText[GameConfig.DEVOMCEmulation]));
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

	//! Settings: Video Mode
	else if (ret == ++Idx)
	{
		if (++GameConfig.video >= VIDEO_MODE_MAX) GameConfig.video = INHERIT;
	}

	//! Settings: Game Language
	else if (ret == ++Idx)
	{
		if (++GameConfig.language >= GC_MAX_LANGUAGE) GameConfig.language = GC_ENGLISH;
	}

	//! Settings: Ocarina
	else if (ret == ++Idx)
	{
		if (++GameConfig.ocarina >= MAX_ON_OFF) GameConfig.ocarina = INHERIT;
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
	}

	//! Settings: DML Progressive Patch
	else if (ret == ++Idx)
	{
		if (++GameConfig.DMLProgPatch >= MAX_ON_OFF) GameConfig.DMLProgPatch = INHERIT;
	}

	//! Settings: DML NMM Mode
	else if (ret == ++Idx)
	{
		if (++GameConfig.DMLNMM >= 3) GameConfig.DMLNMM = INHERIT;
	}

	//! Settings: DML LED Activity
	else if (ret == ++Idx)
	{
		if (++GameConfig.DMLActivityLED >= MAX_ON_OFF) GameConfig.DMLActivityLED = INHERIT;
	}

	//! Settings: DML PAD Hook
	else if (ret == ++Idx)
	{
		if (++GameConfig.DMLPADHOOK >= MAX_ON_OFF) GameConfig.DMLPADHOOK = INHERIT;
	}

	//! Settings: DML No Disc
	else if (ret == ++Idx)
	{
		if (++GameConfig.DMLNoDisc >= MAX_ON_OFF) GameConfig.DMLNoDisc = INHERIT;
	}

	//! Settings: DML Extended No Disc
	else if (Settings.DMLConfigVersion > 1 && ret == ++Idx)
	{
		if (++GameConfig.DMLNoDisc2 >= MAX_ON_OFF) GameConfig.DMLNoDisc2 = INHERIT;
	}

	//! Settings: DML Force Widescreen
	else if (Settings.DMLConfigVersion > 1 && ret == ++Idx)
	{
		if (++GameConfig.DMLWidescreen >= MAX_ON_OFF) GameConfig.DMLWidescreen = INHERIT;
	}

	//! Settings: DML Debug
	else if (ret == ++Idx)
	{
		if (++GameConfig.DMLDebug >= 3) GameConfig.DMLDebug = INHERIT;
	}

	//! Settings: DEVO Memory Card Emulation
	else if (ret == ++Idx)
	{
		if (++GameConfig.DEVOMCEmulation >= DEVO_MC_MAX_CHOICE) GameConfig.DEVOMCEmulation = INHERIT;
	}

	SetOptionValues();

	return MENU_NONE;
}

