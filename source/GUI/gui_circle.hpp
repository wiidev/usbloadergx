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
#ifndef GUICIRCLE_HPP_
#define GUICIRCLE_HPP_

#include "gui.h"

class GuiCircle : public GuiElement
{
	public:
		GuiCircle();
		GuiCircle(float radius);
		void SetRadius(float r) { LOCK(this); radius = r; }
		void SetInnerRadius(float r) { SetLinewidth((radius-r)*2.0f); }
		void SetColor(const GXColor c) { LOCK(this); color = c; }
		void SetAccuracy(int a) { LOCK(this); accuracy = a; }
		void SetFilled(bool f) { LOCK(this); filled = f; }
		void SetLinewidth(float s);
		void Draw();
	protected:
		GXColor color;
		float radius;
		float Linewidth;
		bool filled;
		int accuracy;
};

#endif
