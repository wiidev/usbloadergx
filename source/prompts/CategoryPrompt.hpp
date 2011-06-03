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
#ifndef CATEGORYPROMPT_HPP_
#define CATEGORYPROMPT_HPP_

#include "libwiigui/gui.h"
#include "libwiigui/gui_checkbox.hpp"

class CategoryPrompt : public GuiWindow, public sigslot::has_slots<>
{
    public:
        CategoryPrompt(const std::string &title);
        ~CategoryPrompt();
        bool categoriesChanged() const { return changed; }
    protected:
        int MainLoop();
		sigslot::signal3<GuiText *, GuiText *, GuiCheckbox *> categoryChanged;
		sigslot::signal0<> nextCategory;
		sigslot::signal0<> previousCategory;
		sigslot::signal1<GuiCheckbox *> checkBoxClicked;
    private:
        void OnForwardButtonClick(GuiButton *sender, int chan, const POINT &pointer);
        void OnPreviousButtonClick(GuiButton *sender, int chan, const POINT &pointer);
        void OnEnableButtonClick(GuiButton *sender, int chan, const POINT &pointer);

        bool changed;

        GuiImageData *bgImgData;
        GuiImageData *addImgData;
        GuiImageData *prevImgData;
        GuiImageData *forwardImgData;
        GuiImageData *trashImgData;
        GuiImageData *lineImgData;

        GuiImage *bgImg;
        GuiImage *addImg;
        GuiImage *prevImg;
        GuiImage *forwardImg;
        GuiImage *trashImg;
        GuiImage *lineImg;

        GuiButton *addButton;
        GuiButton *previousButton;
        GuiButton *forwardButton;
        GuiButton *backButton;
        GuiButton *deleteButton;
        GuiButton *editButton;
        GuiButton *homeButton;
        GuiCheckbox *enabledButton;

        GuiText *titleTxt;
        GuiText *categoryTxt;
        GuiText *posTxt;
        GuiText *addTxt;
        GuiText *deleteTxt;

        GuiTrigger *trigA;
        GuiTrigger *trigB;
        GuiTrigger *trigHome;
        GuiTrigger *trigLeft;
        GuiTrigger *trigRight;
        GuiTrigger *trigMinus;
        GuiTrigger *trigPlus;
};

#endif
