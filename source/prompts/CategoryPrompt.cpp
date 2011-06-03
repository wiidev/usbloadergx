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
#include <unistd.h>
#include "CategoryPrompt.hpp"
#include "settings/CGameCategories.hpp"
#include "themes/Resources.h"
#include "language/gettext.h"
#include "menu/menus.h"

CategoryPrompt::CategoryPrompt(const string &title)
    : GuiWindow(0, 0)
{
    int posX = 15;
    const int posY = 50;
    const int distance = 10;

    bgImgData = Resources::GetImageData("categoryPrompt.png");
    addImgData = Resources::GetImageData("add.png");
    prevImgData = Resources::GetImageData("back.png");
    forwardImgData = Resources::GetImageData("forward.png");
    trashImgData = Resources::GetImageData("remove.png");
    lineImgData = Resources::GetImageData("categoryLine.png");

    bgImg = new GuiImage(bgImgData);
    Append(bgImg);

    width = bgImgData->GetWidth();
    height = bgImgData->GetHeight();

    addImg = new GuiImage(addImgData);
    prevImg = new GuiImage(prevImgData);
    forwardImg = new GuiImage(forwardImgData);
    trashImg = new GuiImage(trashImgData);
    lineImg = new GuiImage(lineImgData);

    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigB = new GuiTrigger;
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    trigHome = new GuiTrigger;
    trigHome->SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
    trigLeft = new GuiTrigger;
    trigLeft->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
    trigRight = new GuiTrigger;
    trigRight->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
    trigMinus = new GuiTrigger;
    trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);
    trigPlus = new GuiTrigger;
    trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);

    homeButton = new GuiButton(0, 0);
    homeButton->SetTrigger(trigHome);
    Append(homeButton);

    backButton = new GuiButton(0, 0);
    backButton->SetTrigger(trigB);
    Append(backButton);

    titleTxt = new GuiText(title.c_str(), 30, (GXColor) {0, 0, 0, 255});
    titleTxt->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
    titleTxt->SetPosition(0, 10);
    Append(titleTxt);

    previousButton = new GuiButton(prevImg->GetWidth(), prevImg->GetHeight());
    previousButton->SetImage(prevImg);
    previousButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    previousButton->SetPosition(posX, posY);
    previousButton->SetSoundOver(btnSoundOver);
    previousButton->SetSoundClick(btnSoundClick);
    previousButton->SetTrigger(trigA);
    previousButton->SetTrigger(trigLeft);
    previousButton->SetTrigger(trigMinus);
    previousButton->SetEffectGrow();
    previousButton->Clicked.connect(this, &CategoryPrompt::OnPreviousButtonClick);
    Append(previousButton);
    posX += distance + previousButton->GetWidth();

    categoryTxt = new GuiText((char *) NULL, 26, (GXColor) {0, 0, 0, 255});
    categoryTxt->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
    categoryTxt->SetMaxWidth(lineImg->GetWidth()-10, DOTTED);
    categoryTxt->SetPosition(0, 2);

    editButton = new GuiButton(lineImg->GetWidth(), lineImg->GetHeight());
    editButton->SetImage(lineImg);
    editButton->SetLabel(categoryTxt);
    editButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    editButton->SetPosition(posX, posY);
    editButton->SetSoundOver(btnSoundOver);
    editButton->SetSoundClick(btnSoundClick);
    editButton->SetTrigger(trigA);
    editButton->SetEffectGrow();
    Append(editButton);
    posX += distance + editButton->GetWidth();

    enabledButton = new GuiCheckbox(32, 32);
    enabledButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    enabledButton->SetPosition(posX, posY);
    enabledButton->SetSoundOver(btnSoundOver);
    enabledButton->SetSoundClick(btnSoundClick);
    enabledButton->SetTrigger(trigA);
    enabledButton->Clicked.connect(this, &CategoryPrompt::OnEnableButtonClick);
    Append(enabledButton);
    posX += distance + enabledButton->GetWidth();

    forwardButton = new GuiButton(forwardImg->GetWidth(), forwardImg->GetHeight());
    forwardButton->SetImage(forwardImg);
    forwardButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    forwardButton->SetPosition(posX, posY);
    forwardButton->SetSoundOver(btnSoundOver);
    forwardButton->SetSoundClick(btnSoundClick);
    forwardButton->SetTrigger(trigA);
    forwardButton->SetTrigger(trigRight);
    forwardButton->SetTrigger(trigPlus);
    forwardButton->SetEffectGrow();
    forwardButton->Clicked.connect(this, &CategoryPrompt::OnForwardButtonClick);
    Append(forwardButton);
    posX += 35 + forwardImg->GetWidth();

    posTxt = new GuiText((char *) NULL, 26, (GXColor) {0, 0, 0, 255});
    posTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    posTxt->SetPosition(posX, posY+4);
    Append(posTxt);

    addTxt = new GuiText(tr("Add category"), 24, (GXColor) {0, 0, 0, 255});
    addTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    addTxt->SetMaxWidth(180, DOTTED);
    addTxt->SetPosition(10+addImg->GetWidth(), 6);

    addButton = new GuiButton(addImg->GetWidth()+10+addTxt->GetTextWidth(), addImg->GetHeight());
    addButton->SetImage(addImg);
    addButton->SetLabel(addTxt);
    addButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    addButton->SetPosition(15, 100);
    addButton->SetSoundOver(btnSoundOver);
    addButton->SetSoundClick(btnSoundClick);
    addButton->SetTrigger(trigA);
    addButton->SetEffectGrow();
    Append(addButton);

    deleteTxt = new GuiText(tr("Delete category"), 24, (GXColor) {0, 0, 0, 255});
    deleteTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    deleteTxt->SetMaxWidth(180, DOTTED);
    deleteTxt->SetPosition(trashImg->GetWidth()+10, 6);

    deleteButton = new GuiButton(trashImg->GetWidth()+10+deleteTxt->GetTextWidth(), trashImg->GetHeight());
    deleteButton->SetImage(trashImg);
    deleteButton->SetLabel(deleteTxt);
    deleteButton->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    deleteButton->SetPosition(10+180+addImg->GetWidth(), 100);
    deleteButton->SetSoundOver(btnSoundOver);
    deleteButton->SetSoundClick(btnSoundClick);
    deleteButton->SetTrigger(trigA);
    deleteButton->SetEffectGrow();
    Append(deleteButton);
}

CategoryPrompt::~CategoryPrompt()
{
    delete bgImgData;
    delete addImgData;
    delete prevImgData;
    delete forwardImgData;
    delete trashImgData;
    delete lineImgData;

    delete bgImg;
    delete addImg;
    delete prevImg;
    delete forwardImg;
    delete trashImg;
    delete lineImg;

    delete addButton;
    delete previousButton;
    delete backButton;
    delete forwardButton;
    delete deleteButton;
    delete editButton;
    delete homeButton;
    delete enabledButton;

    delete titleTxt;
    delete categoryTxt;
    delete posTxt;
    delete addTxt;
    delete deleteTxt;

    delete trigA;
    delete trigB;
    delete trigHome;
    delete trigLeft;
    delete trigRight;
    delete trigPlus;
    delete trigMinus;
}

void CategoryPrompt::OnForwardButtonClick(GuiButton *sender, int chan, const POINT &pointer)
{
    nextCategory();
    categoryChanged(categoryTxt, posTxt, enabledButton);
    sender->ResetState();
}

void CategoryPrompt::OnPreviousButtonClick(GuiButton *sender, int chan, const POINT &pointer)
{
    previousCategory();
    categoryChanged(categoryTxt, posTxt, enabledButton);
    sender->ResetState();
}

void CategoryPrompt::OnEnableButtonClick(GuiButton *sender, int chan, const POINT &pointer)
{
    changed = true;
    checkBoxClicked(enabledButton);
    sender->ResetState();
}

int CategoryPrompt::MainLoop()
{
    categoryChanged(categoryTxt, posTxt, enabledButton);

    while(backButton->GetState() != STATE_CLICKED)
    {
        usleep(100);

        if (shutdown)
            Sys_Shutdown();
        else if (reset)
            Sys_Reboot();

        else if (homeButton->GetState() == STATE_CLICKED)
        {
            gprintf("\thomeButton clicked\n");
            WindowExitPrompt();

            homeButton->ResetState();
        }

        else if(addButton->GetState() == STATE_CLICKED)
        {
            if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD))
            {
                WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked to be able to use this." ), tr( "OK" ));
                addButton->ResetState();
                continue;
            }

            char entered[512] = "";

            int result = OnScreenKeyboard(entered, sizeof(entered), 0);
            if(result)
            {
                GameCategories.CategoryList.AddCategory(entered);
                GameCategories.Save();
                GameCategories.CategoryList.findCategory(entered);
                categoryChanged(categoryTxt, posTxt, enabledButton);
            }

            addButton->ResetState();
        }

        else if(deleteButton->GetState() == STATE_CLICKED)
        {
            if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD))
            {
                WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked to be able to use this." ), tr( "OK" ));
                deleteButton->ResetState();
                continue;
            }

            if(GameCategories.CategoryList.getCurrentID() == 0)
            {
                WindowPrompt(tr("Error"), tr("You cannot delete this category."), tr("OK"));
                deleteButton->ResetState();
                continue;
            }

            int choice = WindowPrompt(tr("Warning"), tr("Are you sure you want to delete this category?"), tr("Yes"), tr("Cancel"));
            if(choice)
            {
                int pos = GameCategories.CategoryList.pos()-1;
                int categoryID = GameCategories.CategoryList.getCurrentID();
                GameCategories.CategoryList.RemoveCategory(categoryID);
                GameCategories.RemoveCategory(categoryID);
                GameCategories.Save();
                GameCategories.CategoryList.goToFirst();
                for(int i = 0; i < pos; ++i)
                    GameCategories.CategoryList.goToNext();

                categoryChanged(categoryTxt, posTxt, enabledButton);
            }

            deleteButton->ResetState();
        }

        else if(editButton->GetState() == STATE_CLICKED)
        {
            if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD))
            {
                WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked to be able to use this." ), tr( "OK" ));
                editButton->ResetState();
                continue;
            }

            char entered[512];
            snprintf(entered, sizeof(entered), tr(GameCategories.CategoryList.getCurrentName().c_str()));

            int result = OnScreenKeyboard(entered, sizeof(entered), 0);
            if(result)
            {
                GameCategories.CategoryList.SetCategory(GameCategories.CategoryList.getCurrentID(), entered);
                int pos = GameCategories.CategoryList.pos();
                GameCategories.Save();
                GameCategories.CategoryList.goToFirst();
                for(int i = 0; i < pos; ++i)
                    GameCategories.CategoryList.goToNext();
                categoryChanged(categoryTxt, posTxt, enabledButton);
            }

            editButton->ResetState();
        }
    }

    return 0;
}

