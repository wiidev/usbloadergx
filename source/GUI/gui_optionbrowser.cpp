/****************************************************************************
 * libwiigui
 *
 * gui_customoptionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"
#include "../main.h"
#include "../gecko.h"
#include "../settings/CSettings.h"
#include "gui_optionbrowser.h"
#include "themes/CTheme.h"
#include "utils/tools.h"
#include "menu.h"

#include <unistd.h>

#define GAMESELECTSIZE	  30

/**
GuiOptionBrowser * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l, const char * custombg)
	: scrollBar(h-10)
{
	width = w;
	height = h;
	options = l;
	selectable = true;
	selectedItem = 0;
	oldSelectedItem = -1;
	coL2 = 50;
	listOffset = 0;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	bgOptions = Resources::GetImageData(custombg);

	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgOptionsEntry = Resources::GetImageData("bg_options_entry.png");

	scrollBar.SetParent(this);
	scrollBar.SetAlignment(thAlign("right - options browser scrollbar align hor"), thAlign("top - options browser scrollbar align ver"));
	scrollBar.SetPosition(thInt("0 - options browser scrollbar pos x"), thInt("5 - options browser scrollbar pos y"));
	scrollBar.listChanged.connect(this, &GuiOptionBrowser::onListChange);

	optionBtn.resize(PAGESIZE);
	optionBg.resize(PAGESIZE);
	optionTxt.resize(PAGESIZE);
	optionVal.resize(PAGESIZE);

	for (int i = 0; i < PAGESIZE; i++)
	{
		optionTxt[i] = new GuiText((wchar_t *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - settings text color"));
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(24, 0);
		optionTxt[i]->SetMaxWidth(bgOptionsImg->GetWidth()-scrollBar.GetWidth()-40, DOTTED);

		optionBg[i] = new GuiImage(bgOptionsEntry);

		optionVal[i] = new GuiText((wchar_t *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - settings text color"));
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

		optionBtn[i] = new GuiButton(width - scrollBar.GetWidth(), GAMESELECTSIZE);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(10, GAMESELECTSIZE * i + 4);
		optionBtn[i]->SetRumble(false);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
	delete bgOptionsImg;
	delete bgOptions;
	delete bgOptionsEntry;

	delete trigA;

	for (int i = 0; i < PAGESIZE; i++)
	{
		delete optionTxt[i];
		delete optionVal[i];
		delete optionBg[i];
		delete optionBtn[i];
	}
}

void GuiOptionBrowser::ResetState()
{
	if (state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for (u32 i = 0; i < optionBtn.size(); i++)
	{
		optionBtn[i]->ResetState();
	}
}

int GuiOptionBrowser::GetClickedOption()
{
	for (u32 i = 0; i < optionBtn.size(); i++)
	{
		if (optionBtn[i]->GetState() == STATE_CLICKED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			return listOffset + i;
		}
	}

	return -1;
}

int GuiOptionBrowser::GetSelectedOption()
{
	for (u32 i = 0; i < optionBtn.size(); i++)
	{
		if (optionBtn[i]->GetState() == STATE_SELECTED)
		{
			return listOffset + i;
		}
	}
	return -1;
}

void GuiOptionBrowser::SetOffset(int optionnumber)
{
	listOffset = optionnumber;
	selectedItem = optionnumber;
}

void GuiOptionBrowser::onListChange(int SelItem, int SelInd)
{
	selectedItem = SelItem;
	listOffset = SelInd;
	UpdateListEntries();
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
	if (!this->IsVisible()) return;

	bgOptionsImg->Draw();

	for (u32 i = 0; i < optionBtn.size(); i++)
	{
		if (listOffset + i < (u32) options->GetLength())
		{
			optionBtn[i]->Draw();
		}
	}

	scrollBar.Draw();

	this->UpdateEffects();
}

void GuiOptionBrowser::UpdateListEntries()
{
	LOCK(this);
	if (listOffset < 0)
		listOffset = 0;

	int maxNameWidth = 0;
	for (u32 i = 0; i < optionBtn.size(); i++)
	{
		if (listOffset + i < (u32) options->GetLength())
		{
			if (optionBtn[i]->GetState() == STATE_DISABLED)
			{
				optionBtn[i]->SetVisible(true);
				optionBtn[i]->SetState(STATE_DEFAULT);
			}

			optionTxt[i]->SetText(options->GetName(listOffset+i));
			if (maxNameWidth < optionTxt[i]->GetTextWidth()) maxNameWidth = optionTxt[i]->GetTextWidth();
			optionVal[i]->SetText(options->GetValue(listOffset+i));
		}
		else
		{
			optionBtn[i]->SetVisible(false);
			optionBtn[i]->SetState(STATE_DISABLED);
		}
	}

	if (coL2 < (24 + maxNameWidth + 16))
		coL2 = 24 + maxNameWidth + 16;

	for (u32 i = 0; i < optionBtn.size(); i++)
	{
		if (optionBtn[i]->GetState() != STATE_DISABLED)
		{
			optionVal[i]->SetPosition(coL2, 0);
			optionVal[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2 + scrollBar.GetWidth()+10), DOTTED);
		}
	}

	oldSelectedItem = -1;
}

void GuiOptionBrowser::Update(GuiTrigger * t)
{
	if (state == STATE_DISABLED || !t) return;

	int listSize = optionBtn.size();
	static int pressedChan = -1;

	if((t->wpad.btns_d & (WPAD_BUTTON_B | WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT |
						  WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT)) ||
		(t->pad.btns_d & (PAD_BUTTON_UP | PAD_BUTTON_DOWN)))
		pressedChan = t->chan;

	// update the location of the scroll box based on the position in the option list
	scrollBar.Update(t);

	if(pressedChan == -1 || (!t->wpad.btns_h && !t->pad.btns_h))
	{
		for(int i = 0; i < listSize; i++)
		{
			if (i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
			{
				optionBtn[i]->ResetState();
			}
			else if (i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
			{
				optionBtn[selectedItem]->SetState(STATE_SELECTED);
			}

			optionBtn[i]->Update(t);

			if (optionBtn[i]->GetState() == STATE_SELECTED)
				selectedItem = i;
		}
	}

	if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h)
		pressedChan = -1;

	if(selectedItem != oldSelectedItem)
	{
		if(oldSelectedItem >= 0 && oldSelectedItem < listSize)
			optionVal[oldSelectedItem]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2 + scrollBar.GetWidth()+10), DOTTED);
		if(selectedItem >= 0 && selectedItem < listSize)
			optionVal[selectedItem]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2 + scrollBar.GetWidth()+10), SCROLL_HORIZONTAL);

		oldSelectedItem = selectedItem;
	}

	scrollBar.SetPageSize(listSize);
	scrollBar.SetSelectedItem(selectedItem);
	scrollBar.SetSelectedIndex(listOffset);
	scrollBar.SetEntrieCount(options->GetLength());

	if (options->IsChanged())
		UpdateListEntries();
}
