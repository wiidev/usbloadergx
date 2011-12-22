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
#ifndef _CHECKBOXPROMPT_HPP_
#define _CHECKBOXPROMPT_HPP_

#include "GUI/gui_checkbox.hpp"
#include "PromptWindow.hpp"

enum
{
	CheckedNone = -0x01,
	CheckedBox1 = 0x01,
	CheckedBox2 = 0x02,
	CheckedBox3 = 0x04,
	CheckedBox4 = 0x08,
	CheckedBox5 = 0x10,
	CheckedBox6 = 0x20,
};

class CheckboxPrompt : private PromptWindow, public sigslot::has_slots<>
{
	public:
		//! Constructor
		CheckboxPrompt(const char * title = 0, const char *msg = 0);
		//! Destructor
		virtual ~CheckboxPrompt();
		//! Add new checkbox
		void AddCheckBox(const char *text);
		//! Default function to get the button pressed
		int GetChoice();
		//! Set a checkbox checked/unchecked
		void SetChecked(int box, bool checked);
		//! Show window and wait for the user to press OK/Cancel
		static int Show(const char *title = 0, const char *msg = 0,
						const char *chbx1 = 0, const char *chbx2 = 0,
						const char *chbx3 = 0, const char *chbx4 = 0,
						const char *chbx5 = 0, const char *chbx6 = 0,
						int initChecks = 0);
	protected:
		void OnCheckBoxClick(GuiButton *sender, int chan, const POINT &pointer);
		std::vector<GuiText *> CheckboxTxt;
		std::vector<GuiCheckbox *> Checkbox;
};

#define CheckboxWindow CheckboxPrompt::Show

#endif
