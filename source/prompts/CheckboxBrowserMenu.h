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
#ifndef CHECKBOXBROWSERMENU_H
#define CHECKBOXBROWSERMENU_H

#include "GUI/gui_checkboxbrowser.hpp"

class CheckboxBrowserMenu : public GuiWindow, public sigslot::has_slots<>
{
	public:
		CheckboxBrowserMenu();
		virtual ~CheckboxBrowserMenu();
	private:
		void markChanged() { changed = true; }
		void OnCheckboxClick(GuiCheckbox *, int) { markChanged(); }
	protected:
		bool changed;

		GuiCheckboxBrowser *browser;

		GuiImageData *bgImgData;
		GuiImageData *browserImgData;
		GuiImageData *btnOutline;

		GuiImage *browserImg;
		GuiImage *bgImg;
		GuiImage *backImg;
		GuiImage *button1Img;

		GuiButton *backBtn;
		GuiButton *homeButton;
		GuiButton *button1;

		GuiText *titleTxt;
		GuiText *backTxt;
		GuiText *button1Txt;

		GuiTrigger trigA;
		GuiTrigger trigB;
		GuiTrigger trigHome;
};

#endif
