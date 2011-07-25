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
#include <unistd.h>
#include "SoundSettingsMenu.hpp"
#include "settings/CSettings.h"
#include "settings/SettingsPrompts.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"

static const char * GameSoundText[] =
{
	trNOOP( "Sound+Quiet" ),
	trNOOP( "Sound+BGM" ),
	trNOOP( "Loop Sound" ),
};

static const char * MusicLoopText[] =
{
	trNOOP( "Play Once" ),
	trNOOP( "Loop Music" ),
	trNOOP( "Random Directory Music" ),
	trNOOP( "Loop Directory" ),
};

SoundSettingsMenu::SoundSettingsMenu()
	: SettingsMenu(tr("Sound Settings"), &GuiOptions, MENU_NONE)
{
	int Idx = 0;
	Options->SetName(Idx++, "%s", tr( "Backgroundmusic" ));
	Options->SetName(Idx++, "%s", tr( "Music Volume" ));
	Options->SetName(Idx++, "%s", tr( "SFX Volume" ));
	Options->SetName(Idx++, "%s", tr( "Game Sound Mode" ));
	Options->SetName(Idx++, "%s", tr( "Game Sound Volume" ));
	Options->SetName(Idx++, "%s", tr( "Music Loop Mode" ));
	Options->SetName(Idx++, "%s", tr( "Reset BG Music" ));

	SetOptionValues();
}

void SoundSettingsMenu::SetOptionValues()
{
	int Idx = 0;

	//! Settings: Backgroundmusic
	const char * filename = strrchr(Settings.ogg_path, '/');
	if (filename)
		Options->SetValue(Idx++, filename+1);
	else
		Options->SetValue(Idx++, tr( "Default" ));

	//! Settings: Music Volume
	if (Settings.volume > 0)
		Options->SetValue(Idx++, "%i", Settings.volume);
	else
		Options->SetValue(Idx++, tr( "OFF" ));

	//! Settings: SFX Volume
	if (Settings.sfxvolume > 0)
		Options->SetValue(Idx++, "%i", Settings.sfxvolume);
	else
		Options->SetValue(Idx++, tr( "OFF" ));

	//! Settings: Game Sound Mode
	Options->SetValue(Idx++, "%s", tr( GameSoundText[Settings.gamesound] ));

	//! Settings: Game Sound Volume
	if (Settings.gamesoundvolume > 0)
		Options->SetValue(Idx++, "%i", Settings.gamesoundvolume);
	else
		Options->SetValue(Idx++, tr( "OFF" ));

	//! Settings: Music Loop Mode
	Options->SetValue(Idx++, tr( MusicLoopText[Settings.musicloopmode] ));

	//! Settings: Reset BG Music
	Options->SetValue(Idx++, " ");
}

int SoundSettingsMenu::GetMenuInternal()
{
	int ret = optionBrowser->GetClickedOption();

	if (ret < 0)
		return MENU_NONE;

	int Idx = -1;

	//! Settings: Backgroundmusic
	if (ret == ++Idx)
	{
		GuiWindow * parent = (GuiWindow *) parentElement;
		if(parent) parent->SetState(STATE_DISABLED);

		MenuBackgroundMusic();

		if(parent) parent->SetState(STATE_DEFAULT);
	}

	//! Settings: Music Volume
	else if (ret == ++Idx)
	{
		Settings.volume += 10;
		if (Settings.volume > 100) Settings.volume = 0;
		bgMusic->SetVolume(Settings.volume);
	}

	//! Settings: SFX Volume
	else if (ret == ++Idx)
	{
		Settings.sfxvolume += 10;
		if (Settings.sfxvolume > 100) Settings.sfxvolume = 0;
		btnSoundOver->SetVolume(Settings.sfxvolume);
		btnSoundClick->SetVolume(Settings.sfxvolume);
		btnSoundClick2->SetVolume(Settings.sfxvolume);
	}

	//! Settings: Game Sound Mode
	else if (ret == ++Idx)
	{
		if (++Settings.gamesound > 2) Settings.gamesound = 0;
	}

	//! Settings: Game Sound Volume
	else if (ret == ++Idx)
	{
		Settings.gamesoundvolume += 10;
		if (Settings.gamesoundvolume > 100) Settings.gamesoundvolume = 0;
	}

	//! Settings: Music Loop Mode
	else if (ret == ++Idx)
	{
		if (++Settings.musicloopmode > 3) Settings.musicloopmode = 0;
		bgMusic->SetLoop(Settings.musicloopmode);
	}

	//! Settings: Reset BG Music
	else if (ret == ++Idx)
	{
		int result = WindowPrompt(tr( "Reset to default BGM?" ), 0, tr( "Yes" ), tr( "No" ));
		if (result)
		{
			bgMusic->LoadStandard();
			bgMusic->Play();
		}
	}

	SetOptionValues();

	return MENU_NONE;
}
