/****************************************************************************
 * Copyright (C) 2011
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
#ifndef _THEME_MENU_H_
#define _THEME_MENU_H_

#include <vector>
#include <string>
#include "settings/menus/FlyingButtonsMenu.hpp"
#include "FileOperations/DirList.h"
#include "themes/Theme_List.h"

class ThemeMenu : public FlyingButtonsMenu
{
	public:
		ThemeMenu();
		virtual ~ThemeMenu();
		static int Execute();
		int MainLoop();
	protected:
		void CreateSettingsMenu(int index) { MainButtonClicked(index); };
		void MainButtonClicked(int button);
		void AddMainButtons();
		void SetupMainButtons();
		void SetMainButton(int position, const char * ButtonText, GuiImageData * imageData, GuiImageData * imageOver);
		GuiImageData * GetImageData(int theme);
		bool GetNodeText(const u8 *buffer, const char *node, std::string &outtext);

		struct ThemeInfoStruct
		{
			std::string Filepath;
			std::string Title;
			std::string Team;
			std::string Version;
			std::string ImageFolder;
		};
		std::vector<ThemeInfoStruct> ThemeList;
		GuiText * defaultBtnTxt;
		GuiImage * defaultBtnImg;
		GuiButton * defaultBtn;
		GuiImageData * ThemePreviews[4];
};

#endif
