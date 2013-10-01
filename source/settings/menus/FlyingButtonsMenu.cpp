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
#include <string.h>
#include <unistd.h>

#include "FlyingButtonsMenu.hpp"
#include "utils/StringTools.h"
#include "language/gettext.h"
#include "themes/CTheme.h"
#include "menu/menus.h"
#include "sys.h"

#define FADE_SPEED 20
#define SLIDE_SPEED 35
#define DISPLAY_BUTTONS	4
#define MAX_INDICATORS	 5

FlyingButtonsMenu::FlyingButtonsMenu(const char * menu_title)
	:   GuiWindow(screenwidth, screenheight)

{
	currentPage = 0;
	returnMenu = MENU_NONE;
	ParentMenu = MENU_DISCLIST;
	CurrentMenu = NULL;
	titleTxt = NULL;
	GoLeftImg = NULL;
	GoLeftBtn = NULL;
	GoRightImg = NULL;
	GoRightBtn = NULL;
	MenuTitle = menu_title ? menu_title : " ";

	trigA = new GuiTrigger();
	trigHome = new GuiTrigger();
	trigB = new GuiTrigger();
	trigL = new GuiTrigger();
	trigR = new GuiTrigger();
	trigMinus = new GuiTrigger();
	trigPlus = new GuiTrigger();
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigHome->SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, PAD_TRIGGER_L);
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_TRIGGER_R);

	btnOutline = Resources::GetImageData("button_dialogue_box.png");
	settingsbg = Resources::GetImageData("settings_background.png");
	MainButtonImgData = Resources::GetImageData("settings_title.png");
	MainButtonImgOverData = Resources::GetImageData("settings_title_over.png");
	PageindicatorImgData = Resources::GetImageData("pageindicator.png");
	arrow_left = Resources::GetImageData("startgame_arrow_left.png");
	arrow_right = Resources::GetImageData("startgame_arrow_right.png");

	settingsbackground = new GuiImage(settingsbg);
	Append(settingsbackground);

	homeBtn = new GuiButton(1, 1);
	homeBtn->SetTrigger(trigHome);
	Append(homeBtn);

	backBtnTxt = new GuiText(tr( "Back" ), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	backBtnTxt->SetMaxWidth(btnOutline->GetWidth() - 30);
	backBtnImg = new GuiImage(btnOutline);
	if (Settings.wsprompt == ON)
	{
		backBtnTxt->SetWidescreen(Settings.widescreen);
		backBtnImg->SetWidescreen(Settings.widescreen);
	}
	backBtn = new GuiButton(backBtnImg, backBtnImg, 2, 3, -195, 400, trigA, btnSoundOver, btnSoundClick2, 1);
	backBtn->SetLabel(backBtnTxt);
	backBtn->SetTrigger(trigB);
	Append(backBtn);

	SetEffect(EFFECT_FADE, FADE_SPEED);
}

FlyingButtonsMenu::~FlyingButtonsMenu()
{
	ResumeGui();

	SetEffect(EFFECT_FADE, -FADE_SPEED);
	while(parentElement && this->GetEffect() > 0) usleep(10000);

	HaltGui();
	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();
	HideMenu();

	delete trigA;
	delete trigHome;
	delete trigB;
	delete trigL;
	delete trigR;
	delete trigMinus;
	delete trigPlus;
	delete settingsbackground;
	delete homeBtn;
	delete btnOutline;
	delete settingsbg;
	delete MainButtonImgData;
	delete MainButtonImgOverData;
	delete PageindicatorImgData;
	delete arrow_left;
	delete arrow_right;
	delete backBtnTxt;
	delete backBtnImg;
	delete backBtn;

	ResumeGui();
}

void FlyingButtonsMenu::SetPageIndicators()
{
	HaltGui();

	for(u32 i = 0; i < PageIndicatorBtn.size(); ++i)
	{
		Remove(PageIndicatorBtn[i]);
		delete PageindicatorImg[i];
		delete PageindicatorTxt[i];
		delete PageIndicatorBtn[i];
	}
	PageindicatorImg.clear();
	PageindicatorTxt.clear();
	PageIndicatorBtn.clear();

	int IndicatorCount = ceil(MainButton.size()/(1.0f*DISPLAY_BUTTONS));

	FirstIndicator = 0;
	if(currentPage > (int) floor(MAX_INDICATORS/2.0f))
	{
		if(IndicatorCount-MAX_INDICATORS < currentPage - (int) floor(MAX_INDICATORS/2.0f))
			FirstIndicator = IndicatorCount-MAX_INDICATORS;
		else
			FirstIndicator = currentPage - (int) floor(MAX_INDICATORS/2.0f);

		if(FirstIndicator < 0)
			FirstIndicator = 0;
	}

	int DisplayedIndicators = IndicatorCount > MAX_INDICATORS ? MAX_INDICATORS : IndicatorCount;

	for(int i = 0, n = FirstIndicator; i < MAX_INDICATORS && n < IndicatorCount; ++i, ++n)
	{
		PageindicatorImg.push_back(new GuiImage(PageindicatorImgData));
		PageindicatorTxt.push_back(new GuiText(fmt("%i", n+1), 22, ( GXColor ) {0, 0, 0, 255}));
		PageIndicatorBtn.push_back(new GuiButton(PageindicatorImgData->GetWidth(), PageindicatorImgData->GetHeight()));
		PageIndicatorBtn[i]->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
		PageIndicatorBtn[i]->SetPosition(270-DisplayedIndicators*35+35*i, 400);
		PageIndicatorBtn[i]->SetImage(PageindicatorImg[i]);
		PageIndicatorBtn[i]->SetLabel(PageindicatorTxt[i]);
		PageIndicatorBtn[i]->SetSoundOver(btnSoundOver);
		PageIndicatorBtn[i]->SetSoundClick(btnSoundClick);
		PageIndicatorBtn[i]->SetTrigger(trigA);
		PageIndicatorBtn[i]->SetEffectGrow();
		PageIndicatorBtn[i]->SetAlpha(n == currentPage ? 255 : 50);
		Append(PageIndicatorBtn[i]);
	}
}

void FlyingButtonsMenu::SetMainButton(int position, const char * ButtonText, GuiImageData * imageData, GuiImageData * imageOver)
{
	//! Don't allow operating gui mode while adding/setting the buttons
	HaltGui();

	if(position < (int) MainButton.size())
	{
		delete MainButtonImg[position];
		delete MainButtonImgOver[position];
		delete MainButtonTxt[position];
		delete MainButton[position];
	}
	else
	{
		MainButtonImg.resize(position+1);
		MainButtonImgOver.resize(position+1);
		MainButtonTxt.resize(position+1);
		MainButton.resize(position+1);
	}

	MainButtonImg[position] = new GuiImage(imageData);
	MainButtonImgOver[position] = new GuiImage(imageOver);

	MainButtonTxt[position] = new GuiText(ButtonText, 22, ( GXColor ) {0, 0, 0, 255});
	MainButtonTxt[position]->SetMaxWidth(MainButtonImg[position]->GetWidth());

	MainButton[position] = new GuiButton(imageData->GetWidth(), imageData->GetHeight());
	MainButton[position]->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	MainButton[position]->SetPosition(0, 90+(position % DISPLAY_BUTTONS)*70);
	MainButton[position]->SetImage(MainButtonImg[position]);
	MainButton[position]->SetImageOver(MainButtonImgOver[position]);
	MainButton[position]->SetLabel(MainButtonTxt[position]);
	MainButton[position]->SetSoundOver(btnSoundOver);
	MainButton[position]->SetSoundClick(btnSoundClick);
	MainButton[position]->SetEffectGrow();
	MainButton[position]->SetTrigger(trigA);
}

void FlyingButtonsMenu::HideMenu()
{
	HaltGui();

	if(titleTxt)
		Remove(titleTxt);
	if(GoLeftBtn)
		Remove(GoLeftBtn);
	if(GoRightBtn)
		Remove(GoRightBtn);

	if(titleTxt)
		delete titleTxt;
	if(GoLeftImg)
		delete GoLeftImg;
	if(GoLeftBtn)
		delete GoLeftBtn;
	if(GoRightImg)
		delete GoRightImg;
	if(GoRightBtn)
		delete GoRightBtn;
	titleTxt = NULL;
	GoLeftImg = NULL;
	GoLeftBtn = NULL;
	GoRightImg = NULL;
	GoRightBtn = NULL;

	for(u32 i = 0; i < MainButton.size(); ++i)
	{
		Remove(MainButton[i]);
		delete MainButtonImg[i];
		delete MainButtonImgOver[i];
		delete MainButtonTxt[i];
		delete MainButton[i];
	}
	for(u32 i = 0; i < PageIndicatorBtn.size(); ++i)
	{
		Remove(PageIndicatorBtn[i]);
		delete PageindicatorImg[i];
		delete PageindicatorTxt[i];
		delete PageIndicatorBtn[i];
	}

	PageindicatorImg.clear();
	PageindicatorTxt.clear();
	PageIndicatorBtn.clear();
	MainButtonImg.clear();
	MainButtonImgOver.clear();
	MainButtonTxt.clear();
	MainButton.clear();
}

void FlyingButtonsMenu::ShowMenu()
{
	//! Free memory if not done yet because new is allocated
	HideMenu();

	titleTxt = new GuiText(MenuTitle.c_str(), 28, thColor("r=0 g=0 b=0 a=255 - settings title text color"));
	titleTxt->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	titleTxt->SetPosition(0, 40);
	titleTxt->SetMaxWidth(310, SCROLL_HORIZONTAL);
	Append(titleTxt);

	GoLeftImg = new GuiImage(arrow_left);
	GoLeftBtn = new GuiButton(GoLeftImg->GetWidth(), GoLeftImg->GetHeight());
	GoLeftBtn->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	GoLeftBtn->SetPosition(25, -25);
	GoLeftBtn->SetImage(GoLeftImg);
	GoLeftBtn->SetSoundOver(btnSoundOver);
	GoLeftBtn->SetSoundClick(btnSoundClick2);
	GoLeftBtn->SetEffectGrow();
	GoLeftBtn->SetTrigger(trigA);
	GoLeftBtn->SetTrigger(trigL);
	GoLeftBtn->SetTrigger(trigMinus);
	Append(GoLeftBtn);

	GoRightImg = new GuiImage(arrow_right);
	GoRightBtn = new GuiButton(GoRightImg->GetWidth(), GoRightImg->GetHeight());
	GoRightBtn->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	GoRightBtn->SetPosition(-25, -25);
	GoRightBtn->SetImage(GoRightImg);
	GoRightBtn->SetSoundOver(btnSoundOver);
	GoRightBtn->SetSoundClick(btnSoundClick2);
	GoRightBtn->SetEffectGrow();
	GoRightBtn->SetTrigger(trigA);
	GoRightBtn->SetTrigger(trigR);
	GoRightBtn->SetTrigger(trigPlus);
	Append(GoRightBtn);

	SetupMainButtons();
	AddMainButtons();

	ShowButtonsEffects(EFFECT_FADE, FADE_SPEED);
}

void FlyingButtonsMenu::AddMainButtons()
{
	HaltGui();

	for(u32 i = 0; i < MainButton.size(); ++i)
		Remove(MainButton[i]);

	int FirstItem = currentPage*DISPLAY_BUTTONS;

	for(int i = FirstItem; i < (int) MainButton.size() && i < FirstItem+DISPLAY_BUTTONS; ++i)
	{
		Append(MainButton[i]);
	}

	SetPageIndicators();
}

void FlyingButtonsMenu::ShowButtonsEffects(int effect, int effect_speed)
{
	int FirstItem = currentPage*DISPLAY_BUTTONS;
	if(FirstItem < 0)
		FirstItem = 0;

	HaltGui();

	for(int i = FirstItem; i < (int) MainButton.size() && i < FirstItem+DISPLAY_BUTTONS; ++i)
	{
		MainButton[i]->StopEffect();
		MainButton[i]->SetEffect(effect, effect_speed);
	}

	ResumeGui();

	if(FirstItem < 0 || FirstItem >= (int) MainButton.size())
		return;

	//! Don't lock on fade in for initiation purpose
	if((effect & EFFECT_FADE) && effect_speed > 0)
		return;

	while (parentElement && MainButton[FirstItem]->GetEffect() > 0)
		usleep(10000);

	//! Allow button grow only after slide animation is done
	for(int i = FirstItem; i < (int) MainButton.size() && i < FirstItem+DISPLAY_BUTTONS; ++i)
		MainButton[i]->SetEffectGrow();
}

void FlyingButtonsMenu::SlideButtons(int direction)
{
	int SlideEffectIN = 0;

	if(direction == SLIDE_LEFT)
	{
		ShowButtonsEffects(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, SLIDE_SPEED);
		SlideEffectIN = EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN;

		currentPage--;

		if(currentPage < 0)
			currentPage = MainButton.size() > 0 ? ceil(MainButton.size()/4.0f)-1 : 0;
	}
	else
	{
		ShowButtonsEffects(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, SLIDE_SPEED);
		SlideEffectIN = EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN;

		currentPage++;

		if(currentPage >= ceil(MainButton.size()/4.0f))
			currentPage = 0;
	}

	AddMainButtons();

	ShowButtonsEffects(SlideEffectIN, SLIDE_SPEED);
}

int FlyingButtonsMenu::MainLoop()
{
	usleep(50000);

	if(shutdown)
		Sys_Shutdown();
	else if(reset)
		Sys_Reboot();

	if(backBtn->GetState() == STATE_CLICKED)
	{
		if(CurrentMenu)
		{
			DeleteSettingsMenu();
			ShowMenu();
		}
		else
		{
			return ParentMenu;
		}
		backBtn->ResetState();
	}
	else if(GoRightBtn && GoRightBtn->GetState() == STATE_CLICKED)
	{
		SlideButtons(SLIDE_RIGHT);
		GoRightBtn->ResetState();
	}
	else if(GoLeftBtn && GoLeftBtn->GetState() == STATE_CLICKED)
	{
		SlideButtons(SLIDE_LEFT);
		GoLeftBtn->ResetState();
	}
	else if(homeBtn->GetState() == STATE_CLICKED)
	{
		Settings.Save();
		if(CurrentMenu) CurrentMenu->SetState(STATE_DISABLED);
		WindowExitPrompt();
		homeBtn->ResetState();
		if(CurrentMenu) CurrentMenu->SetState(STATE_DEFAULT);
	}
	else if(CurrentMenu)
	{
		int newMenu = CurrentMenu->GetMenu();
		if(newMenu != MENU_NONE)
			return newMenu;
	}

	for(u32 i = 0; i < PageIndicatorBtn.size(); ++i)
	{
		if(PageIndicatorBtn[i]->GetState() != STATE_CLICKED)
			continue;

		if(FirstIndicator+i < (u32) currentPage)
		{
			ShowButtonsEffects(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, SLIDE_SPEED);
			currentPage = FirstIndicator+i;
			AddMainButtons();
			ShowButtonsEffects(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, SLIDE_SPEED);
		}
		else if(FirstIndicator+i > (u32) currentPage)
		{
			ShowButtonsEffects(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, SLIDE_SPEED);
			currentPage = FirstIndicator+i;
			AddMainButtons();
			ShowButtonsEffects(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, SLIDE_SPEED);
		}

		PageIndicatorBtn[i]->ResetState();
	}

	for(u32 i = 0; i < MainButton.size(); ++i)
	{
		if(MainButton[i]->GetState() != STATE_CLICKED)
			continue;

		MainButton[i]->ResetState();

		CreateSettingsMenu(i);
		break;
	}

	return returnMenu;
}
