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
#ifndef GUIGAMEBROWSER_H_
#define GUIGAMEBROWSER_H_

#include "gui.h"

class GuiGameBrowser : public GuiElement
{
	public:
		GuiGameBrowser() {}
		virtual ~GuiGameBrowser() {}
		virtual int GetClickedOption() { return -1; }
		virtual int GetSelectedOption() { return -1; }
		virtual void SetSelectedOption(int ind) {}
		virtual void setListOffset(int off) {}
		virtual int getListOffset() const { return 0; }
};

#endif /* GUIGAMEBROWSER_H_ */
