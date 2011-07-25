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
#ifndef HOMEBREWPROMPT_HPP_
#define HOMEBREWPROMPT_HPP_

#include "GUI/gui_scrollbar.hpp"
#include "GUI/Text.hpp"
#include "PromptWindow.hpp"

class HomebrewPrompt : public PromptWindow, public sigslot::has_slots<>
{
	public:
		HomebrewPrompt(const char *name, const char *coder, const char *version,
					   const char *release_date, const char *long_description,
					   GuiImageData * iconImgData, u64 filesize);
		virtual ~HomebrewPrompt();
		int MainLoop();
	private:
		void onListChange(int SelItem, int SelInd);

		GuiImageData *whiteBox;

		GuiImage *whiteBoxImg;
		GuiImage *iconImg;

		GuiText *nameTxt;
		GuiText *coderTxt;
		GuiText *versionTxt;
		GuiText *release_dateTxt;
		GuiText *filesizeTxt;
		Text *long_descriptionTxt;
		GuiScrollbar *scrollBar;
};

#endif
