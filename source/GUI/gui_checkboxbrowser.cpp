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
#include "gui_checkboxbrowser.hpp"
#include "themes/Resources.h"
#include "themes/gettheme.h"
#include "wstring.hpp"


GuiCheckboxBrowser::GuiCheckboxBrowser(int w, int h, int s)
	: scrollBar(h-10)
{
	width = w;
	height = h;
	backgroundImg = NULL;
	selectedItem = 0;
	pageIndex = 0;
	pressedChan = -1;
	maxTextWidth = 280;
	maxSize = s;
	scrollBar.SetParent(this);
	scrollBar.SetAlignment(thAlign("right - checkbox browser scrollbar align hor"), thAlign("top - checkbox browser scrollbar align ver"));
	scrollBar.SetPosition(thInt("0 - checkbox browser scrollbar pos x"), thInt("5 - checkbox browser scrollbar pos y"));
	scrollBar.SetButtonScroll(WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B);
	scrollBar.listChanged.connect(this, &GuiCheckboxBrowser::onListChange);
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	markImgData = Resources::GetImageData("checkBoxSelection.png");
	markImg = new GuiImage(markImgData);
	markImg->SetParent(this);
}

GuiCheckboxBrowser::~GuiCheckboxBrowser()
{
	Clear();

	delete markImg;
	delete markImgData;
}

void GuiCheckboxBrowser::SetImage(GuiImage *Img)
{
	LOCK(this);
	backgroundImg = Img;
	if(backgroundImg)
		backgroundImg->SetParent(this);
}

void GuiCheckboxBrowser::Clear()
{
	LOCK(this);
	checkBoxDrawn.clear();
	textLineDrawn.clear();

	for(u32 i = 0; i < checkBoxList.size(); ++i)
	{
		delete textLineList[i];
		delete checkBoxList[i];
	}

	textLineList.clear();
	checkBoxList.clear();
}

bool GuiCheckboxBrowser::AddEntrie(const string &text, bool checked, int style, bool multistates)
{
	LOCK(this);
	int currentSize = checkBoxList.size();
	textLineList.resize(currentSize+1);
	checkBoxList.resize(currentSize+1);

	checkBoxList[currentSize] = new GuiCheckbox(30, 30);
	checkBoxList[currentSize]->SetParent(this);
	checkBoxList[currentSize]->SetChecked(checked);
	checkBoxList[currentSize]->SetStyle(style);
	checkBoxList[currentSize]->SetMultiStates(multistates);
	checkBoxList[currentSize]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	checkBoxList[currentSize]->SetTrigger(&trigA);
	checkBoxList[currentSize]->SetClickSize(width-30-scrollBar.GetWidth(), 30);
	checkBoxList[currentSize]->Clicked.connect(this, &GuiCheckboxBrowser::OnCheckboxClick);

	textLineList[currentSize] = new GuiText(text.c_str(), 18, thColor("r=0 g=0 b=0 a=255 - checkbox browser text color"));
	textLineList[currentSize]->SetParent(this);
	textLineList[currentSize]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	textLineList[currentSize]->SetMaxWidth(maxTextWidth, DOTTED);

	if(textLineDrawn.size() < (u32) maxSize)
	{
		textLineDrawn.push_back(textLineList[currentSize]);
		checkBoxDrawn.push_back(checkBoxList[currentSize]);
	}

	return true;
}

void GuiCheckboxBrowser::OnCheckboxClick(GuiButton *sender, int chan, const POINT &pointer)
{
	LOCK(this);
	sender->ResetState();

	for(u32 i = 0; i < checkBoxDrawn.size(); ++i)
	{
		if(sender == checkBoxDrawn[i])
		{
			checkBoxClicked(checkBoxDrawn[i], pageIndex+i);
			return;
		}
	}
}

void GuiCheckboxBrowser::onListChange(int SelItem, int SelInd)
{
	LOCK(this);
	selectedItem = SelItem;
	pageIndex = SelInd;
	RefreshList();
}

void GuiCheckboxBrowser::RefreshList()
{
	LOCK(this);
	while(pageIndex+checkBoxDrawn.size() > checkBoxList.size())
		--pageIndex;

	if(checkBoxDrawn.size() == 0)
		selectedItem = 0;
	else if(selectedItem >= (int) checkBoxDrawn.size())
		selectedItem = checkBoxDrawn.size()-1;

	for(u32 i = 0; i < checkBoxDrawn.size(); i++)
	{
		checkBoxDrawn[i] = checkBoxList[pageIndex+i];
		checkBoxDrawn[i]->SetPosition(-scrollBar.GetWidth()-10, 15+i*(checkBoxDrawn[i]->GetHeight()+6));

		textLineDrawn[i] = textLineList[pageIndex+i];
		textLineDrawn[i]->SetPosition(25, 10+i*(checkBoxDrawn[i]->GetHeight()+6)+(checkBoxDrawn[i]->GetHeight()-textLineDrawn[i]->GetFontSize())/2+2);
	}
	scrollBar.SetSelectedItem(selectedItem);
	scrollBar.SetSelectedIndex(pageIndex);
}

void GuiCheckboxBrowser::Draw()
{
	LOCK(this);
	if(backgroundImg)
		backgroundImg->Draw();

	for(u32 i = 0; i < checkBoxDrawn.size(); ++i)
	{
		textLineDrawn[i]->Draw();
		checkBoxDrawn[i]->Draw();
	}

	markImg->Draw();

	if(checkBoxList.size() >= (u32) maxSize)
		scrollBar.Draw();
}

void GuiCheckboxBrowser::Update(GuiTrigger *t)
{
	if(state == STATE_DISABLED || !t)
		return;

	LOCK(this);
	if(checkBoxList.size() >= maxSize)
		scrollBar.Update(t);

	if((t->wpad.btns_d & (WPAD_BUTTON_B | WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT |
						  WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT)) ||
		(t->pad.btns_d & (PAD_BUTTON_UP | PAD_BUTTON_DOWN)))
		pressedChan = t->chan;

	for(u32 i = 0; i < checkBoxDrawn.size(); i++)
	{
		if(pressedChan == -1 || (!t->wpad.btns_h && !t->pad.btns_h))
		{
			if(i != (u32) selectedItem && checkBoxDrawn[i]->GetState() == STATE_SELECTED) {
				textLineList[i]->SetMaxWidth(maxTextWidth, DOTTED);
				checkBoxDrawn[i]->ResetState();
			}
			else if(i == (u32) selectedItem && checkBoxDrawn[i]->GetState() == STATE_DEFAULT) {
				checkBoxDrawn[selectedItem]->SetState(STATE_SELECTED, -1);
				textLineList[i]->SetMaxWidth(maxTextWidth, SCROLL_HORIZONTAL);
			}

			checkBoxDrawn[i]->Update(t);

			if(checkBoxDrawn[i]->GetState() == STATE_SELECTED)
				selectedItem = i;
		}

		if(i == (u32) selectedItem)
			markImg->SetPosition(5, 15+i*(checkBoxDrawn[i]->GetHeight()+6)+(checkBoxDrawn[i]->GetHeight()-markImg->GetHeight())/2);
	}

	if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h)
		pressedChan = -1;

	scrollBar.SetPageSize(checkBoxDrawn.size());
	scrollBar.SetSelectedItem(selectedItem);
	scrollBar.SetSelectedIndex(pageIndex);
	scrollBar.SetEntrieCount(checkBoxList.size());
}
