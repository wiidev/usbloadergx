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

#include "libwiigui/gui.h"
#include "libwiigui/gui_box.hpp"
#include "libwiigui/gui_cross.hpp"

class GuiCheckbox : public GuiButton
{
    public:
        GuiCheckbox();
        GuiCheckbox(int w, int h);
        void SetTransparent(bool b);
        void SetSize(int w, int h);
        void SetChecked(bool c) { LOCK(this); Checked = c; }
        bool IsChecked() const { return Checked; }
        virtual void SetState(int s, int c = -1);
        virtual void Draw();
    protected:
        GuiCross Cross;
        GuiBox Blackbox;
        GuiBox Whitebox;
        bool Checked;

};

#endif
