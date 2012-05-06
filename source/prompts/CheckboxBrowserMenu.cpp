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
#include "CheckboxBrowserMenu.h"
#include "GameCube/GCGames.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "language/gettext.h"
#include "themes/gettheme.h"
#include "themes/Resources.h"
#include "menu/menus.h"

CheckboxBrowserMenu::CheckboxBrowserMenu(void)
	: GuiWindow(0, 0)
{
	changed = false;

	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);

	btnOutline = Resources::GetImageData("button_dialogue_box.png");
	bgImgData = Resources::GetImageData("categoryprompt.png");
	browserImgData = Resources::GetImageData("bg_options.png");

	bgImg = new GuiImage(bgImgData);
	Append(bgImg);

	width = bgImg->GetWidth();
	height = bgImg->GetHeight()+btnOutline->GetHeight()*0.9f;

	titleTxt = new GuiText("", 30, thColor("r=0 g=0 b=0 a=255 - check box browser prompt title text color"));
	titleTxt->SetAlignment(thAlign("center - check box browser prompt title text align hor"), thAlign("top - check box browser prompt title text align ver"));
	titleTxt->SetPosition(thInt("0 - check box browser prompt title text pos x"), thInt("10 - check box browser prompt title text pos y"));
	Append(titleTxt);

	browserImg = new GuiImage(browserImgData);
	browser = new GuiCheckboxBrowser(browserImg->GetWidth(), browserImg->GetHeight());
	browser->SetImage(browserImg);
	browser->SetAlignment(thAlign("center - check box browser prompt browser align hor"), thAlign("top - check box browser prompt browser align ver"));
	browser->SetPosition(thInt("0 - check box browser prompt browser pos x"), thInt("45 - check box browser prompt browser pos y"));
	browser->checkBoxClicked.connect(this, &CheckboxBrowserMenu::OnCheckboxClick);
	Append(browser);

	homeButton = new GuiButton(0, 0);
	homeButton->SetTrigger(&trigHome);
	Append(homeButton);

	button1Img = new GuiImage(btnOutline);
	button1Img->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	button1Img->SetScale(0.9f);
	button1Txt = new GuiText(tr("Install"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	button1 = new GuiButton(button1Img->GetWidth()*0.9f, button1Img->GetHeight()*0.9f);
	button1->SetImage(button1Img);
	button1->SetLabel(button1Txt);
	button1->SetAlignment(thAlign("center - check box browser prompt install button align hor"), thAlign("bottom - check box browser prompt install button align ver"));
	button1->SetPosition(thInt("-110 - check box browser prompt install button pos x"), thInt("0 - check box browser prompt install button pos y"));
	button1->SetSoundOver(btnSoundOver);
	button1->SetSoundClick(btnSoundClick);
	button1->SetTrigger(&trigA);
	button1->SetEffectGrow();
	Append(button1);

	backImg = new GuiImage(btnOutline);
	backImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	backImg->SetScale(0.9f);
	backTxt = new GuiText(tr("Back"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	backBtn = new GuiButton(backImg->GetWidth()*0.9f, backImg->GetHeight()*0.9f);
	backBtn->SetImage(backImg);
	backBtn->SetLabel(backTxt);
	backBtn->SetAlignment(thAlign("center - check box browser prompt back button align hor"), thAlign("bottom - check box browser prompt back button align ver"));
	backBtn->SetPosition(thInt("110 - check box browser prompt back button pos x"), thInt("0 - check box browser prompt back button pos y"));
	backBtn->SetSoundOver(btnSoundOver);
	backBtn->SetSoundClick(btnSoundClick);
	backBtn->SetTrigger(&trigA);
	backBtn->SetTrigger(&trigB);
	backBtn->SetEffectGrow();
	Append(backBtn);
}

CheckboxBrowserMenu::~CheckboxBrowserMenu()
{
	RemoveAll();
	delete browser;

	delete btnOutline;
	delete bgImgData;
	delete bgImg;
	delete browserImgData;
	delete browserImg;
	delete backImg;
	delete button1Img;

	delete backBtn;
	delete homeButton;
	delete button1;

	delete titleTxt;
	delete backTxt;
	delete button1Txt;
}

