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
#ifndef _PROMPTWINDOW_HPP_
#define _PROMPTWINDOW_HPP_

#include "GUI/gui.h"

class PromptWindow : public GuiWindow
{
	public:
		//! Constructor
		PromptWindow(const char *title = 0, const char *msg = 0);
		//! Destructor
		virtual ~PromptWindow();
		//! Set title text
		void SetTitle(const char *text) { titleTxt->SetText(text); };
		//! Set message text
		void SetMessageText(const char *text) { msgTxt->SetText(text); };
		//! Add new button and rearrange all buttons position. MAX 4 buttons.
		void AddButton(const char *text);
		//! Removes/deletes the last button and rearranges positions
		void RemoveButton();
		//! Removes a button in the position from the window but does not completely delete it
		void RemoveButton(int pos);
		//! Default function to get the button pressed
		int GetChoice();
		//! Forbid = operation
		PromptWindow& operator=(const PromptWindow &w);
	protected:
		void PositionButtons();

		GuiImageData *btnOutline;
		GuiImageData *dialogBox;
		GuiImage *dialogBoxImg;
		GuiText *titleTxt;
		GuiText *msgTxt;
		GuiTrigger *trigA;
		GuiTrigger *trigB;
		std::vector<GuiText *> ButtonTxt;
		std::vector<GuiImage *> ButtonImg;
		std::vector<GuiButton *> Button;
};

#endif
