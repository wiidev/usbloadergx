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
#ifndef GUICHECKBOX_HPP_
#define GUICHECKBOX_HPP_

#include "GUI/gui.h"
#include "GUI/gui_box.hpp"
#include "GUI/gui_cross.hpp"
#include "GUI/gui_checksign.hpp"
#include "GUI/gui_plus.hpp"

class GuiCheckbox : public GuiButton
{
	public:
		GuiCheckbox(int style = CHECKSIGN);
		GuiCheckbox(int w, int h, int style = CHECKSIGN);
		void SetTransparent(bool b);
		void SetSize(int w, int h);
		void SetClickSize(int w, int h);
		void SetAlignment(int h, int v);
		void SetChecked(bool c) { LOCK(this); Checked = c; }
		void SetStyle(int st) { LOCK(this); style = st; }
		void SetMultiStates(bool m) { LOCK(this); MultiStates = m; }
		bool IsChecked() const { return Checked; }
		int  GetStyle() { return style; }
		virtual void SetState(int s, int c = -1);
		virtual void Draw();
		enum
		{
			CHECKSIGN,
			CROSS,
			PLUS,
			MAX_CHECKBOX_STYLE
		};
	protected:
		GuiChecksign Checksign;
		GuiCross Cross;
		GuiPlus Plus;
		GuiBox Blackbox;
		GuiBox Whitebox;
		int style;
		bool Checked;
		bool MultiStates;

};

#endif
