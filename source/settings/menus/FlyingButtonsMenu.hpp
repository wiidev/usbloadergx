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
#ifndef FLYINGBUTTONSMENU_HPP_
#define FLYINGBUTTONSMENU_HPP_

#include <vector>
#include <string>
#include "GUI/gui.h"
#include "SettingsMenu.hpp"
#include "menu.h"

class FlyingButtonsMenu : public GuiWindow
{
	public:
		FlyingButtonsMenu(const char * menu_title);
		virtual ~FlyingButtonsMenu();
		virtual int MainLoop();
		virtual void HideMenu();
		virtual void ShowMenu();
	protected:
		virtual void CreateSettingsMenu(int index) { };
		virtual void DeleteSettingsMenu() { };
		virtual void SetupMainButtons() { };
		virtual void AddMainButtons();
		virtual void ShowButtonsEffects(int effect, int effect_speed);
		virtual void SlideButtons(int slide_direction);
		virtual void SetPageIndicators();
		virtual void SetMainButton(int position, const char * ButtonText, GuiImageData * imageData, GuiImageData * imageOver);

		int currentPage;
		int returnMenu;
		int ParentMenu;
		int FirstIndicator;
		std::string MenuTitle;
		enum
		{
			SLIDE_LEFT, SLIDE_RIGHT
		};

		//!The main settings gui with browser
		SettingsMenu * CurrentMenu;

		GuiImageData * btnOutline;
		GuiImageData * settingsbg;
		GuiImageData * MainButtonImgData;
		GuiImageData * MainButtonImgOverData;
		GuiImageData * PageindicatorImgData;
		GuiImageData * arrow_left;
		GuiImageData * arrow_right;

		GuiImage * settingsbackground;
		GuiImage * backBtnImg;
		GuiImage * PageindicatorImg2;
		GuiImage * GoLeftImg;
		GuiImage * GoRightImg;

		GuiTrigger * trigA;
		GuiTrigger * trigHome;
		GuiTrigger * trigB;
		GuiTrigger * trigL;
		GuiTrigger * trigR;
		GuiTrigger * trigMinus;
		GuiTrigger * trigPlus;

		GuiText * titleTxt;
		GuiText * backBtnTxt;
		GuiText * PageindicatorTxt1;

		GuiButton * backBtn;
		GuiButton * homeBtn;
		GuiButton * GoLeftBtn;
		GuiButton * GoRightBtn;

		std::vector<GuiImage *>PageindicatorImg;
		std::vector<GuiText *>PageindicatorTxt;
		std::vector<GuiButton *>PageIndicatorBtn;

		std::vector<GuiImage *> MainButtonImg;
		std::vector<GuiImage *> MainButtonImgOver;
		std::vector<GuiText *> MainButtonTxt;
		std::vector<GuiButton *> MainButton;
};

#endif
