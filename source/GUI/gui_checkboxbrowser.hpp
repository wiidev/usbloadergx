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
#ifndef CHECKBOXBROWSER_HPP_
#define CHECKBOXBROWSER_HPP_

#include <vector>
#include <string>
#include "gui_checkbox.hpp"
#include "gui_scrollbar.hpp"

using namespace std;

class GuiCheckboxBrowser : public GuiElement, public sigslot::has_slots<>
{
    public:
        GuiCheckboxBrowser(int w, int h, int maxSize = 7);
        ~GuiCheckboxBrowser();
        bool AddEntrie(const string &text, bool checked = false);
        int GetSelected() const { return pageIndex+selectedItem; };
        void SetImage(GuiImage *Img);
        void RefreshList();
        void Clear();
        void Draw();
        void Update(GuiTrigger *t);
		sigslot::signal2<GuiCheckbox *, int> checkBoxClicked;
    private:
        void onListChange(int SelItem, int SelInd);
        void OnCheckboxClick(GuiButton *sender, int chan, const POINT &pointer);
        u16 maxSize;
        int selectedItem;
        int pageIndex;
        int pressedChan;
        GuiScrollbar scrollBar;
        GuiTrigger trigA;
        GuiImage *backgroundImg;
        GuiImageData *markImgData;
        GuiImage *markImg;
        vector<GuiText *> textLineDrawn;
        vector<GuiCheckbox *> checkBoxDrawn;
        vector<GuiText *> textLineList;
        vector<GuiCheckbox *> checkBoxList;
};

#endif
