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
#ifndef GUICHECKSIGN_HPP_
#define GUICHECKSIGN_HPP_

#include "GUI/gui.h"

class GuiChecksign : public GuiElement
{
	public:
		GuiChecksign() : Linewidth(2.0f) { color = (GXColor) {0, 0, 0, 255}; GX_SetLineWidth((u8) (Linewidth*6.0f), 0); }
		//! Max line width is 42.5 pixel
		void SetLinewidth(float w) { LOCK(this); Linewidth = w; GX_SetLineWidth((u8) (Linewidth*6.0f), 0); }
		void SetColor(const GXColor c) { LOCK(this); color = c; }
		void SetSize(int w, int h) { LOCK(this); width = w; height = h; }
		void Draw();
	protected:
		GXColor color;
		float Linewidth;
};

#endif
