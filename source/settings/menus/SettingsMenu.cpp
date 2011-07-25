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
#include "SettingsMenu.hpp"
#include "themes/CTheme.h"
#include "language/gettext.h"

SettingsMenu::SettingsMenu(const char * title, OptionList * opts, int returnTo)
	: GuiWindow(screenwidth, screenheight)
{
	Options = opts;
	returnToMenu = returnTo;
	trigA = NULL;
	trigB = NULL;
	backBtnTxt = NULL;
	backBtnImg = NULL;
	backBtn = NULL;
	btnOutline = NULL;

	//! Skipping back button if there is no menu defined to go back to
	if(returnToMenu != MENU_NONE)
	{
		btnOutline = Resources::GetImageData("button_dialogue_box.png");

		trigA = new GuiTrigger();
		trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

		trigB = new GuiTrigger();
		trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

		backBtnTxt = new GuiText(tr("Back"), 22, (GXColor){0, 0, 0, 255});
		backBtnImg = new GuiImage(btnOutline);
		backBtn = new GuiButton(backBtnImg, backBtnImg, 2, 3, -180, 400, trigA, btnSoundOver, btnSoundClick2, 1);
		backBtn->SetLabel(backBtnTxt);
		backBtn->SetTrigger(trigB);
		Append(backBtn);
	}

	optionBrowser = new GuiOptionBrowser(396, 280, Options, "bg_options_settings.png");
	optionBrowser->SetAlignment(thAlign("center - settings option browser align hor"), thAlign("top - settings option browser align ver"));
	optionBrowser->SetPosition(thInt("0 - settings option browser pos x"), thInt("90 - settings option browser pos y"));

	titleTxt = new GuiText(title, 28, thColor("r=0 g=0 b=0 a=255 - settings title text color"));
	titleTxt->SetAlignment(thAlign("center - settings title text align hor"), thAlign("top - settings title text align ver"));
	titleTxt->SetPosition(thInt("0 - settings title text pos x"), thInt("40 - settings title text pos y"));
	titleTxt->SetMaxWidth(thInt("310 - settings title text max width"), SCROLL_HORIZONTAL);

	Append(optionBrowser);
	Append(titleTxt);

	SetEffect(EFFECT_FADE, 50);
}

SettingsMenu::~SettingsMenu()
{
	ResumeGui();

	SetEffect(EFFECT_FADE, -50);
	while(this->GetEffect() > 0)
		usleep(100);

	HaltGui();
	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();

	if(btnOutline)
		delete btnOutline;

	if(backBtnTxt)
		delete backBtnTxt;
	if(backBtnImg)
		delete backBtnImg;
	if(backBtn)
		delete backBtn;

	if(trigA)
		delete trigA;
	if(trigB)
		delete trigB;

	delete titleTxt;

	delete optionBrowser;

	ResumeGui();
}

int SettingsMenu::GetClickedOption()
{
	if(!optionBrowser)
		return -1;

	return optionBrowser->GetClickedOption();
}

int SettingsMenu::GetMenu()
{
	if(backBtn && backBtn->GetState() == STATE_CLICKED)
		return returnToMenu;

	return GetMenuInternal();
}
