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
#ifndef SETTINGS_MENU_HPP_
#define SETTINGS_MENU_HPP_

#include "GUI/gui.h"
#include "GUI/gui_optionbrowser.h"
#include "menu.h"

class SettingsMenu : public GuiWindow
{
	public:
		SettingsMenu(const char * title, OptionList * option, int returnTo);
		virtual ~SettingsMenu();
		int GetClickedOption();
		int GetMenu();
	protected:
		virtual int GetMenuInternal() { return MENU_NONE; };
		int returnToMenu;

		GuiImageData * btnOutline;

		GuiTrigger * trigA;
		GuiTrigger * trigB;

		OptionList * Options;

		GuiText * titleTxt;
		GuiText * backBtnTxt;
		GuiImage * backBtnImg;
		GuiButton * backBtn;

		GuiOptionBrowser * optionBrowser;

};


#endif
