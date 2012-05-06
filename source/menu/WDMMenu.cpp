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
#include "WDMMenu.hpp"
#include "FileOperations/fileops.h"
#include "menu/menus.h"
#include "themes/CTheme.h"
#include "language/gettext.h"
#include "usbloader/wbfs.h"
#include "libs/libwbfs/libwbfs.h"
#include "libs/libwbfs/wiidisc.h"
#include "usbloader/fstfile.h"
#include "settings/GameTitles.h"
#include "gecko.h"

u32 WDMMenu::AlternateDolOffset = 0;
u32 WDMMenu::AlternateDolParameter = 0;

WDMMenu::WDMMenu(const struct discHdr * header)
	: GuiWindow(screenwidth, screenheight)
{
	Options = new OptionList;

	btnOutline = Resources::GetImageData("button_dialogue_box.png");

	trigA = new GuiTrigger();
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	trigB = new GuiTrigger();
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	defaultBtnTxt = new GuiText(tr("Default"), 22, (GXColor){0, 0, 0, 255});
	defaultBtnImg = new GuiImage(btnOutline);
	defaultBtn = new GuiButton(defaultBtnImg, defaultBtnImg, 2, 3, 130, 400, trigA, btnSoundOver, btnSoundClick2, 1);
	defaultBtn->SetLabel(defaultBtnTxt);
	Append(defaultBtn);

	backBtnTxt = new GuiText(tr("Back"), 22, (GXColor){0, 0, 0, 255});
	backBtnImg = new GuiImage(btnOutline);
	backBtn = new GuiButton(backBtnImg, backBtnImg, 2, 3, -130, 400, trigA, btnSoundOver, btnSoundClick2, 1);
	backBtn->SetLabel(backBtnTxt);
	backBtn->SetTrigger(trigB);
	Append(backBtn);

	optionBrowser = new GuiOptionBrowser(396, 280, Options, "bg_options_settings.png");
	optionBrowser->SetPosition(0, 90);
	optionBrowser->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	Append(optionBrowser);

	SetEffect(EFFECT_FADE, 50);

	char WDMPath[200];
	snprintf(WDMPath, sizeof(WDMPath), "%s/%.3s.wdm", Settings.WDMpath, (char *) header->id);

	if(!CheckFile(WDMPath))
	{
		snprintf(WDMPath, sizeof(WDMPath), "%s/%.6s.wdm", Settings.WDMpath, (char *) header->id);
		if(!CheckFile(WDMPath))
			snprintf(WDMPath, sizeof(WDMPath), "%s/%.4s.wdm", Settings.WDMpath, (char *) header->id);
	}

	wdmFile = new WDMFile(WDMPath);

	CheckGameFiles(header);
}

WDMMenu::~WDMMenu()
{
	ResumeGui();

	SetEffect(EFFECT_FADE, -50);
	while(this->GetEffect() > 0)
		usleep(100);

	HaltGui();
	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();

	delete btnOutline;

	delete backBtnTxt;
	delete backBtnImg;
	delete backBtn;

	delete defaultBtnTxt;
	delete defaultBtnImg;
	delete defaultBtn;

	delete trigA;
	delete trigB;

	delete optionBrowser;
	delete wdmFile;

	ResumeGui();
}

int WDMMenu::GetChoice()
{
	if (shutdown)
		Sys_Shutdown();
	else if (reset)
		Sys_Reboot();

	if(backBtn->GetState() == STATE_CLICKED)
		return 0;

	else if(defaultBtn->GetState() == STATE_CLICKED)
		return 1;

	int choice = optionBrowser->GetClickedOption();
	if(choice >= 0 && choice < (int) DOLOffsetList.size())
	{
		AlternateDolOffset = DOLOffsetList[choice].first;
		AlternateDolParameter = DOLOffsetList[choice].second;
		return 1;
	}

	return -1;
}

int WDMMenu::Show(const struct discHdr * header)
{
	WDMMenu Menu(header);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&Menu);

	int ret = -1;

	while(ret == -1)
	{
		usleep(100);
		ret = Menu.GetChoice();
	}
	mainWindow->SetState(STATE_DEFAULT);

	return ret;
}

static inline bool stringcompare(const char * replace, const char * dolname)
{
	if(strlen(replace) == 0 || strlen(dolname) == 0)
		return false;

	for( ; *replace != 0 && *dolname != 0; replace++, dolname++)
	{
		if(*replace == '?')
			continue;

		if(toupper((int) *replace) != toupper((int) *dolname))
			return false;
	}

	return true;
}

void WDMMenu::CheckGameFiles(const struct discHdr * header)
{
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) header->id);
	if (!disc)
	{
		WindowPrompt(tr( "ERROR:" ), tr( "Could not open Disc" ), tr( "OK" ));
		return;
	}

	wiidisc_t *wdisc = wd_open_disc((int(*)(void *, u32, u32, void *)) wbfs_disc_read, disc);
	if (!wdisc)
	{
		WindowPrompt(tr( "ERROR:" ), tr( "Could not open Disc" ), tr( "OK" ));
		return;
	}

	FST_ENTRY * fstbuffer = (FST_ENTRY *) wd_extract_file(wdisc, ONLY_GAME_PARTITION, (char*) "FST");
	if (!fstbuffer)
	{
		WindowPrompt(tr( "ERROR:" ), tr( "Not enough free memory." ), tr( "OK" ));
		return;
	}

	wd_close_disc(wdisc);
	WBFS_CloseDisc(disc);

	int position = 0;
	vector<pair<int, string> > FilesNotInWDM;

	for(int i = 0; i < wdmFile->size(); ++i)
	{
		if(stringcompare(wdmFile->GetDolName(i), "main") == true)
		{
			DOLOffsetList.push_back(pair<int, int>(0, wdmFile->GetParameter(i)));
			Options->SetName(position, "%i.", position+1);
			Options->SetValue(position, wdmFile->GetReplaceName(i));
			position++;
		}
	}

	for (u32 i = 1; i < fstbuffer[0].filelen; i++)
	{
		//don't add files that aren't .dol to the list
		const char * filename = fstfiles(fstbuffer, i);
		const char * fileext = NULL;

		if(filename)
			fileext = strrchr(filename, '.');

		if (fileext && strcasecmp(fileext, ".dol") == 0)
		{
			char NameCpy[strlen(filename)+1];
			strcpy(NameCpy, filename);
			char *extension = strrchr(NameCpy, '.');
			if(extension) *extension = 0;

			int j;
			for(j = 0; j < wdmFile->size(); ++j)
			{
				if(stringcompare(wdmFile->GetDolName(j), NameCpy) == true)
				{
					DOLOffsetList.push_back(pair<int, int>(i, wdmFile->GetParameter(j)));
					Options->SetName(position, "%i.", position+1);
					Options->SetValue(position, wdmFile->GetReplaceName(j));
					position++;
					break;
				}
			}

			if(j == wdmFile->size())
				FilesNotInWDM.push_back(pair<int, string>(i, filename));
		}
	}

	for(u32 i = 0; i < FilesNotInWDM.size(); ++i)
	{
		DOLOffsetList.push_back(pair<int, int>(FilesNotInWDM[i].first, 1));
		Options->SetName(position, "%i.", position+1);
		Options->SetValue(position, FilesNotInWDM[i].second.c_str());
		position++;
	}

	free(fstbuffer);
}
