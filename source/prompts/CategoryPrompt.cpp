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
#include "settings/CSettings.h"
#include "language/gettext.h"
#include "themes/gettheme.h"
#include "themes/Resources.h"
#include "menu/menus.h"

CategoryPrompt::CategoryPrompt(const string &title)
	: GuiWindow(0, 0)
{
	changed = false;

	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB.SetSimpleTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_TRIGGER_R);
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, PAD_TRIGGER_L);
	trig1.SetButtonOnlyTrigger(-1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y, PAD_TRIGGER_Z);

	btnOutline = Resources::GetImageData("button_dialogue_box.png");
	bgImgData = Resources::GetImageData("categoryPrompt.png");
	browserImgData = Resources::GetImageData("bg_options.png");
	addImgData = Resources::GetImageData("add.png");
	deleteImgData = Resources::GetImageData("remove.png");
	editImgData = Resources::GetImageData("one.png");

	bgImg = new GuiImage(bgImgData);
	Append(bgImg);

	width = bgImg->GetWidth();
	height = bgImg->GetHeight()+btnOutline->GetHeight()*0.9f;

	titleTxt = new GuiText(title.c_str(), 30, thColor("r=0 g=0 b=0 a=255 - category prompt title text color"));
	titleTxt->SetAlignment(thAlign("center - category prompt title text align hor"), thAlign("top - category prompt title text align ver"));
	titleTxt->SetPosition(thInt("0 - category prompt title text pos x"), thInt("10 - category prompt title text pos y"));
	Append(titleTxt);

	browserImg = new GuiImage(browserImgData);
	browser = new GuiCheckboxBrowser(browserImg->GetWidth(), browserImg->GetHeight());
	browser->SetImage(browserImg);
	browser->SetAlignment(thAlign("center - category prompt browser align hor"), thAlign("top - category prompt browser align ver"));
	browser->SetPosition(thInt("0 - category prompt browser pos x"), thInt("45 - category prompt browser pos y"));
	Append(browser);

	homeButton = new GuiButton(0, 0);
	homeButton->SetTrigger(&trigHome);
	Append(homeButton);

	addImg = new GuiImage(addImgData);
	addTxt = new GuiText(tr("Add category"), 24, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	addTxt->SetMaxWidth(180, DOTTED);
	addTxt->SetAlignment(thAlign("left - category prompt add button text align hor"), thAlign("top - category prompt add button text align ver"));
	addTxt->SetPosition(thInt("10 - category prompt add button text pos x")+addImg->GetWidth(), thInt("6 - category prompt add button text pos y"));

	addButton = new GuiButton(addImg->GetWidth()+10+addTxt->GetTextWidth(), addImg->GetHeight());
	addButton->SetImage(addImg);
	addButton->SetLabel(addTxt);
	addButton->SetAlignment(thAlign("left - category prompt add button align hor"), thAlign("top - category prompt add button align ver"));
	addButton->SetPosition(width/2-thInt("180 - category prompt add button pos x")-addImg->GetWidth()/2, thInt("330 - category prompt add button pos y"));
	addButton->SetSoundOver(btnSoundOver);
	addButton->SetSoundClick(btnSoundClick);
	addButton->SetTrigger(&trigA);
	addButton->SetTrigger(&trigPlus);
	addButton->SetEffectGrow();
	Append(addButton);

	deleteImg = new GuiImage(deleteImgData);
	deleteTxt = new GuiText(tr("Delete category"), 24, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	deleteTxt->SetMaxWidth(180, DOTTED);
	deleteTxt->SetAlignment(thAlign("left - category prompt delete button text align hor"), thAlign("top - category prompt delete button text align ver"));
	deleteTxt->SetPosition(thInt("10 - category prompt delete button text pos x")+deleteImg->GetWidth(), thInt("6 - category prompt delete button text pos y"));

	deleteButton = new GuiButton(deleteImg->GetWidth()+10+deleteTxt->GetTextWidth(), deleteImg->GetHeight());
	deleteButton->SetImage(deleteImg);
	deleteButton->SetLabel(deleteTxt);
	deleteButton->SetAlignment(thAlign("left - category prompt delete button align hor"), thAlign("top - category prompt delete button align ver"));
	deleteButton->SetPosition(width/2+thInt("5 - category prompt delete button pos x"), thInt("330 - category prompt delete button pos y"));
	deleteButton->SetSoundOver(btnSoundOver);
	deleteButton->SetSoundClick(btnSoundClick);
	deleteButton->SetTrigger(&trigA);
	deleteButton->SetTrigger(&trigMinus);
	deleteButton->SetEffectGrow();
	Append(deleteButton);

	editImg = new GuiImage(editImgData);
	editTxt = new GuiText(tr("Rename category"), 24, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	editTxt->SetAlignment(thAlign("left - category prompt edit button text align hor"), thAlign("top - category prompt edit button text align ver"));
	editTxt->SetPosition(thInt("10 - category prompt edit button text pos x")+editImg->GetWidth(), thInt("6 - category prompt edit button text pos y"));
	editTxt->SetMaxWidth(180, DOTTED);

	editButton = new GuiButton(editImg->GetWidth()+10+editTxt->GetTextWidth(), editImg->GetHeight());
	editButton->SetImage(editImg);
	editButton->SetLabel(editTxt);
	editButton->SetAlignment(thAlign("left - category prompt edit button align hor"), thAlign("top - category prompt edit button align ver"));
	editButton->SetPosition(width/2-thInt("180 - category prompt edit button pos x")-addImg->GetWidth()/2, thInt("362 - category prompt edit button pos y"));
	editButton->SetSoundOver(btnSoundOver);
	editButton->SetSoundClick(btnSoundClick);
	editButton->SetTrigger(&trigA);
	editButton->SetTrigger(&trig1);
	editButton->SetEffectGrow();
	Append(editButton);

	saveImg = new GuiImage(btnOutline);
	saveImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	saveImg->SetScale(0.9f);
	saveTxt = new GuiText(tr("Save"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	saveButton = new GuiButton(saveImg->GetWidth()*0.9f, saveImg->GetHeight()*0.9f);
	saveButton->SetImage(saveImg);
	saveButton->SetLabel(saveTxt);
	saveButton->SetAlignment(thAlign("center - category prompt save button align hor"), thAlign("bottom - category prompt save button align ver"));
	saveButton->SetPosition(thInt("-110 - category prompt save button pos x"), thInt("0 - category prompt save button pos y"));
	saveButton->SetSoundOver(btnSoundOver);
	saveButton->SetSoundClick(btnSoundClick);
	saveButton->SetTrigger(&trigA);
	saveButton->SetEffectGrow();
	Append(saveButton);

	backImg = new GuiImage(btnOutline);
	backImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	backImg->SetScale(0.9f);
	backTxt = new GuiText(tr("Back"), 22, thColor("r=0 g=0 b=0 a=255 - prompt windows button text color"));
	backBtn = new GuiButton(backImg->GetWidth()*0.9f, backImg->GetHeight()*0.9f);
	backBtn->SetImage(backImg);
	backBtn->SetLabel(backTxt);
	backBtn->SetAlignment(thAlign("center - category prompt back button align hor"), thAlign("bottom - category prompt back button align ver"));
	backBtn->SetPosition(thInt("110 - category prompt back button pos x"), thInt("0 - category prompt back button pos y"));
	backBtn->SetSoundOver(btnSoundOver);
	backBtn->SetSoundClick(btnSoundClick);
	backBtn->SetTrigger(&trigA);
	backBtn->SetEffectGrow();
	Append(backBtn);

	browserRefresh();
}

CategoryPrompt::~CategoryPrompt()
{
	RemoveAll();
	delete browser;

	delete btnOutline;
	delete bgImgData;
	delete bgImg;
	delete browserImgData;
	delete browserImg;
	delete addImgData;
	delete addImg;
	delete deleteImgData;
	delete deleteImg;
	delete editImgData;
	delete editImg;
	delete backImg;
	delete saveImg;

	delete backBtn;
	delete homeButton;
	delete addButton;
	delete deleteButton;
	delete editButton;
	delete saveButton;

	delete titleTxt;
	delete addTxt;
	delete deleteTxt;
	delete editTxt;
	delete backTxt;
	delete saveTxt;
}

int CategoryPrompt::Show()
{
	while(backBtn->GetState() != STATE_CLICKED)
	{
		usleep(10000);

		if (shutdown)
			Sys_Shutdown();
		else if (reset)
			Sys_Reboot();

		else if (homeButton->GetState() == STATE_CLICKED)
		{
			gprintf("\thomeButton clicked\n");
			WindowExitPrompt();
			mainWindow->SetState(STATE_DISABLED);
			SetState(STATE_DEFAULT);
			homeButton->ResetState();
		}

		else if(saveButton->GetState() == STATE_CLICKED)
		{
			if(categoriesChanged())
				GameCategories.Save();
			return 1;
		}

		else if(addButton->GetState() == STATE_CLICKED)
		{
			if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD))
			{
				WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked to be able to use this." ), tr( "OK" ));
				mainWindow->SetState(STATE_DISABLED);
				SetState(STATE_DEFAULT);
				addButton->ResetState();
				continue;
			}

			char entered[512] = "";

			int result = OnScreenKeyboard(entered, sizeof(entered), 0);
			if(result)
			{
				GameCategories.CategoryList.AddCategory(entered);
				browserRefresh();
				markChanged();
			}

			mainWindow->SetState(STATE_DISABLED);
			SetState(STATE_DEFAULT);
			addButton->ResetState();
		}

		else if(deleteButton->GetState() == STATE_CLICKED)
		{
			if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD))
			{
				WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked to be able to use this." ), tr( "OK" ));
				mainWindow->SetState(STATE_DISABLED);
				SetState(STATE_DEFAULT);
				deleteButton->ResetState();
				continue;
			}

			if(browser->GetSelected() == 0)
			{
				WindowPrompt(tr("Error"), tr("You cannot delete this category."), tr("OK"));
				mainWindow->SetState(STATE_DISABLED);
				SetState(STATE_DEFAULT);
				deleteButton->ResetState();
				continue;
			}

			int choice = WindowPrompt(tr("Warning"), tr("Are you sure you want to delete this category?"), tr("Yes"), tr("Cancel"));
			if(choice)
			{
				GameCategories.CategoryList.goToFirst();
				for(int i = 0; i < browser->GetSelected(); ++i)
					GameCategories.CategoryList.goToNext();
				int categoryID = GameCategories.CategoryList.getCurrentID();
				GameCategories.CategoryList.RemoveCategory(categoryID);
				GameCategories.RemoveCategory(categoryID);

				browserRefresh();
				markChanged();
			}

			mainWindow->SetState(STATE_DISABLED);
			SetState(STATE_DEFAULT);
			deleteButton->ResetState();
		}

		else if(editButton->GetState() == STATE_CLICKED)
		{
			if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_CATEGORIES_MOD))
			{
				WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked to be able to use this." ), tr( "OK" ));
				mainWindow->SetState(STATE_DISABLED);
				SetState(STATE_DEFAULT);
				continue;
			}

			GameCategories.CategoryList.goToFirst();
			for(int i = 0; i < browser->GetSelected(); ++i)
				GameCategories.CategoryList.goToNext();

			char entered[512];
			snprintf(entered, sizeof(entered), tr(GameCategories.CategoryList.getCurrentName().c_str()));

			int result = OnScreenKeyboard(entered, sizeof(entered), 0);
			if(result)
			{
				GameCategories.CategoryList.SetCategory(GameCategories.CategoryList.getCurrentID(), entered);
				browserRefresh();
				markChanged();
			}

			mainWindow->SetState(STATE_DISABLED);
			SetState(STATE_DEFAULT);
			editButton->ResetState();
		}
	}

	//! Reset to old file in case of cancel
	if(categoriesChanged())
		resetChanges();

	return 0;
}


