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
#ifndef GUICROSS_HPP_
#define GUICROSS_HPP_

#include "libwiigui/gui.h"

class GuiCross : public GuiElement
{
    public:
        GuiCross() : Linewidth(1.5f) { color = (GXColor) {0, 0, 0, 255}; }
        void SetLinewidth(float w) { LOCK(this); Linewidth = w; }
        void SetColor(const GXColor c) { LOCK(this); color = c; }
        void SetSize(int w, int h) { LOCK(this); width = w; height = h; }
        void Draw();
    protected:
        GXColor color;
        float Linewidth;
};

#endif
