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
#include "gui_checkbox.hpp"

#define WHITEBOX_RED_SIZE   4

GuiCheckbox::GuiCheckbox()
    : GuiButton(30, 30), Checked(false)
{
    Cross.SetParent(this);
    Blackbox.SetParent(this);
    Whitebox.SetParent(this);
    Cross.SetColor((GXColor) {0, 0, 0, 255});
    Blackbox.SetColor((GXColor) {0, 0, 0, 255});
    Whitebox.SetColor((GXColor) {255, 255, 255, 255});

    SetSize(30, 30);
}

GuiCheckbox::GuiCheckbox(int w, int h)
        : GuiButton(w, h), Checked(false)
{
    Cross.SetParent(this);
    Blackbox.SetParent(this);
    Whitebox.SetParent(this);
    Cross.SetColor((GXColor) {0, 0, 0, 255});
    Blackbox.SetColor((GXColor) {0, 0, 0, 255});
    Whitebox.SetColor((GXColor) {255, 255, 255, 255});

    SetSize(w, h);
}

void GuiCheckbox::SetSize(int w, int h)
{
    width = w;
    height = h;
    Cross.SetSize(w-WHITEBOX_RED_SIZE, h-WHITEBOX_RED_SIZE);
    Cross.SetPosition(WHITEBOX_RED_SIZE/2, WHITEBOX_RED_SIZE/2);
    Blackbox.SetSize(w, h);
    Whitebox.SetSize(w-WHITEBOX_RED_SIZE, h-WHITEBOX_RED_SIZE);
    Whitebox.SetPosition(WHITEBOX_RED_SIZE/2, WHITEBOX_RED_SIZE/2);
}

void GuiCheckbox::SetTransparent(bool b)
{
    Blackbox.SetFilled(b);
    Whitebox.SetFilled(b);
}

void GuiCheckbox::SetState(int s, int c)
{
    if(s == STATE_CLICKED)
        Checked = !Checked;
    else
        GuiButton::SetState(s, c);
}

void GuiCheckbox::Draw()
{
    GuiButton::Draw();
    Blackbox.Draw();
    Whitebox.Draw();
    if(Checked)
        Cross.Draw();
}
