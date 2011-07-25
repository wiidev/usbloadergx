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
#ifndef GUIBOX_HPP_
#define GUIBOX_HPP_

#include "GUI/gui.h"

class GuiBox : public GuiElement
{
	public:
		GuiBox() : filled(true) { SetColor((GXColor) {255, 255, 255, 255}); }
		GuiBox(int w, int h) : filled(true) { width = w; height = h; SetColor((GXColor) {255, 255, 255, 255}); }
		//! Set one color for the whole square
		void SetColor(const GXColor c) { LOCK(this); for(int i = 0; i < 4; ++i) color[i] = c; }
		//! Set Color for each corner having a nice fluent flow into the color of the other corners
		//! 0 = up/left, 1 = up/right, 2 = buttom/left, 3 = buttom/right
		void SetColor(int i, const GXColor c) { LOCK(this); if(i < 4) color[i] = c; }
		void SetSize(int w, int h) { LOCK(this); width = w; height = h; }
		void SetFilled(bool f) { LOCK(this); filled = f; }
		void Draw();
	protected:
		GXColor color[4];
		bool filled;
};

#endif
