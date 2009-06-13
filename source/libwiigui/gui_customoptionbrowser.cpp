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
#include "../settings/cfg.h"
#include "gui_customoptionbrowser.h"

#include <unistd.h>


#define GAMESELECTSIZE      30


customOptionList::customOptionList(int size)
{
	name = new char * [size];
	value = new char * [size];
	for (int i = 0; i < size; i++)
	{
		name[i] = 0;
		value[i] = 0;
	}
	length = size;
	changed = false;
}
customOptionList::~customOptionList()
{
	for (int i = 0; i < length; i++)
	{
		free(name[i]);
		free(value[i]);
	}
	delete [] name;
	delete [] value;
}
void customOptionList::SetName(int i, const char *format, ...)
{
	if(i >= 0 && i < length)
	{
		if(name[i]) free(name[i]);
		name[i] = 0;
		va_list va;
		va_start(va, format);
		vasprintf(&name[i], format, va);
		va_end(va);
		changed = true;
	}
}
void customOptionList::SetValue(int i, const char *format, ...)
{
	if(i >= 0 && i < length)
	{
		char *tmp=0;
		va_list va;
		va_start(va, format);
		vasprintf(&tmp, format, va);
		va_end(va);

		if(tmp)
		{
			if(value[i] && !strcmp(tmp, value[i]))
				free(tmp);
			else
			{
				free(value[i]);
				value[i] = tmp;
				changed = true;
			}
		}
	}
}

/**
 * Constructor for the GuiCustomOptionBrowser class.
 */
GuiCustomOptionBrowser::GuiCustomOptionBrowser(int w, int h, customOptionList * l, const char *themePath, const char *custombg, const u8 *imagebg, int scrollon,int col2)
{
	width = w;
	height = h;
	options = l;
	size = ((l->GetLength() > PAGESIZE)? PAGESIZE: l->GetLength());
	scrollbaron = scrollon;
	selectable = true;
	listOffset = this->FindMenuItem(-1, 1);
	selectedItem = 0;
	focus = 1; // allow focus
	coL2 = col2;
	char imgPath[100];

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A, PAD_BUTTON_A);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM, Settings.sfxvolume);

	snprintf(imgPath, sizeof(imgPath), "%s%s", themePath, custombg);
	bgOptions = new GuiImageData(imgPath, imagebg);

	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	snprintf(imgPath, sizeof(imgPath), "%sbg_options_entry.png", themePath);
	bgOptionsEntry = new GuiImageData(imgPath, bg_options_entry_png);

	snprintf(imgPath, sizeof(imgPath), "%sscrollbar.png", themePath);
	scrollbar = new GuiImageData(imgPath, scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 4);

	snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowdown.png", themePath);
	arrowDown = new GuiImageData(imgPath, scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData(imgPath, scrollbar_arrowdown_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowup.png", themePath);
	arrowUp = new GuiImageData(imgPath, scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData(imgPath, scrollbar_arrowup_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);
	snprintf(imgPath, sizeof(imgPath), "%sscrollbar_box.png", themePath);
	scrollbarBox = new GuiImageData(imgPath, scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOver = new GuiImageData(imgPath, scrollbar_box_png);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetImageHold(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	arrowUpBtn->SetPosition(width/2-18+7,-18);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetTrigger(trigA);
	arrowUpBtn->SetEffectOnOver(EFFECT_SCALE, 50, 130);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetImageHold(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	arrowDownBtn->SetPosition(width/2-18+7,18);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetTrigger(trigA);
	arrowDownBtn->SetEffectOnOver(EFFECT_SCALE, 50, 130);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetImageHold(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetEffectOnOver(EFFECT_SCALE, 50, 120);
    scrollbarBoxBtn->SetMinY(0);
	scrollbarBoxBtn->SetMaxY(height-30);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);

	optionIndex = new int[size];
	optionVal = new GuiText * [size];
	optionValOver = new GuiText * [size];
	optionBtn = new GuiButton * [size];
	optionTxt = new GuiText * [size];
	optionBg = new GuiImage * [size];

	for(int i=0; i < size; i++)
	{
		optionTxt[i] = new GuiText(options->GetName(i), 20, (GXColor){THEME.settingsTxt_r, THEME.settingsTxt_g, THEME.settingsTxt_b, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(24,0);

		optionBg[i] = new GuiImage(bgOptionsEntry);

		optionVal[i] = new GuiText(NULL, 20, (GXColor){THEME.settingsTxt_r, THEME.settingsTxt_g, THEME.settingsTxt_b, 0xff});
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

		optionValOver[i] = new GuiText(NULL, 20, (GXColor){THEME.settingsTxt_r, THEME.settingsTxt_g, THEME.settingsTxt_b, 0xff});
		optionValOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

		optionBtn[i] = new GuiButton(width-28,GAMESELECTSIZE);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetLabelOver(optionValOver[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(10,GAMESELECTSIZE*i+4);
		optionBtn[i]->SetRumble(false);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);

	}
	UpdateListEntries();
}

/**
 * Destructor for the GuiCustomOptionBrowser class.
 */
GuiCustomOptionBrowser::~GuiCustomOptionBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;

    delete bgOptionsImg;
	delete bgOptions;
	delete bgOptionsEntry;

	delete trigA;
	delete trigHeldA;
	delete btnSoundClick;

	for(int i = 0; i < size; i++)
	{
		delete optionTxt[i];
		delete optionVal[i];
		delete optionValOver[i];
		delete optionBg[i];
		delete optionBtn[i];
	}
	delete [] optionIndex;
	delete [] optionVal;
	delete [] optionValOver;
	delete [] optionBtn;
	delete [] optionTxt;
	delete [] optionBg;
}

void GuiCustomOptionBrowser::SetFocus(int f)
{
	LOCK(this);
	focus = f;

	for(int i = 0; i < size; i++)
		optionBtn[i]->ResetState();

	if(f == 1)
		optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiCustomOptionBrowser::ResetState()
{
	LOCK(this);
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i = 0; i < size; i++)
	{
		optionBtn[i]->ResetState();
	}
}

int GuiCustomOptionBrowser::GetClickedOption()
{
	int found = -1;
	for(int i = 0; i < size; i++)
	{
		if(optionBtn[i]->GetState() == STATE_CLICKED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

int GuiCustomOptionBrowser::GetSelectedOption()
{
	int found = -1;
	for(int i = 0; i < size; i++)
	{
		if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

void GuiCustomOptionBrowser::SetClickable(bool enable)
{
	for(int i = 0; i < size; i++)
	{
		optionBtn[i]->SetClickable(enable);
	}
}

void GuiCustomOptionBrowser::SetScrollbar(int enable)
{
	scrollbaron = enable;
}

void GuiCustomOptionBrowser::SetOffset(int optionnumber)
{
    listOffset = optionnumber;
    selectedItem = optionnumber;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiCustomOptionBrowser::FindMenuItem(int currentItem, int direction)
{
	int nextItem = currentItem + direction;

	if(nextItem < 0 || nextItem >= options->GetLength())
		return -1;

	if(strlen(options->GetName(nextItem)) > 0)
		return nextItem;
	else
		return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiCustomOptionBrowser::Draw()
{
	LOCK(this);
	if(!this->IsVisible())
		return;

	bgOptionsImg->Draw();

	int next = listOffset;

	for(int i=0; i < size; i++)
	{
		if(next >= 0)
		{
			optionBtn[i]->Draw();
			next = this->FindMenuItem(next, 1);
		}
		else
			break;
	}

    if(scrollbaron == 1) {
	scrollbarImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();
	scrollbarBoxBtn->Draw();
    }
	this->UpdateEffects();
}

void GuiCustomOptionBrowser::UpdateListEntries()
{
	if(listOffset<0) listOffset = this->FindMenuItem(-1, 1);
	int next = listOffset;

	int maxNameWidth = 0;
	for(int i=0; i < size; i++)
	{
		if(next >= 0)
		{
			if(optionBtn[i]->GetState() == STATE_DISABLED)
			{
				optionBtn[i]->SetVisible(true);
				optionBtn[i]->SetState(STATE_DEFAULT);
			}

			optionTxt[i]->SetText(options->GetName(next));
			if(maxNameWidth < optionTxt[i]->GetTextWidth())
				maxNameWidth = optionTxt[i]->GetTextWidth();
			optionVal[i]->SetText(options->GetValue(next));
			optionValOver[i]->SetText(options->GetValue(next));

			optionIndex[i] = next;
			next = this->FindMenuItem(next, 1);
		}
		else
		{
			optionBtn[i]->SetVisible(false);
			optionBtn[i]->SetState(STATE_DISABLED);
		}
	}
	if(coL2 < (24+maxNameWidth+16))
		coL2 = 24+maxNameWidth+16;
	for(int i=0; i < size; i++)
	{
		if(optionBtn[i]->GetState() != STATE_DISABLED)
		{
			optionVal[i]->SetPosition(coL2,0);
			optionVal[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2+24), GuiText::DOTTED);

			optionValOver[i]->SetPosition(coL2,0);
			optionValOver[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2+24), GuiText::SCROLL);
		}
	}
}

void GuiCustomOptionBrowser::Update(GuiTrigger * t)
{
	LOCK(this);
	int next, prev, lang = options->GetLength();

	if(state == STATE_DISABLED || !t)
		return;

	if(options->IsChanged()) {
		coL2 = 0;
		UpdateListEntries();
	}
	int old_listOffset = listOffset;

    if (scrollbaron == 1)
	{
		// update the location of the scroll box based on the position in the option list
		arrowUpBtn->Update(t);
		arrowDownBtn->Update(t);
		scrollbarBoxBtn->Update(t);
    }

	next = listOffset;

    u32 buttonshold = ButtonsHold();

    if(buttonshold != WPAD_BUTTON_UP && buttonshold != WPAD_BUTTON_DOWN) {
        for(int i=0; i < size; i++)
        {
		if(next >= 0)
			next = this->FindMenuItem(next, 1);

		if(focus)
		{
			if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED) {
				optionBtn[i]->ResetState();
			} else if(i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT) {
				optionBtn[selectedItem]->SetState(STATE_SELECTED);
			}
		}


		optionBtn[i]->Update(t);

		if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
		}
        }
	}

	// pad/joystick navigation
	if(!focus)
		return; // skip navigation

    if(t->Down())
		{
			next = this->FindMenuItem(optionIndex[selectedItem], 1);

			if(next >= 0)
			{
				if(selectedItem == size-1)
				{
					// move list down by 1
					listOffset = this->FindMenuItem(listOffset, 1);
				}
				else if(optionBtn[selectedItem+1]->IsVisible())
				{
					optionBtn[selectedItem]->ResetState();
					optionBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
					selectedItem++;
				}
			}
		}
		else if(t->Up())
		{
			prev = this->FindMenuItem(optionIndex[selectedItem], -1);

			if(prev >= 0)
			{
				if(selectedItem == 0)
				{
					// move list up by 1
					listOffset = prev;
				}
				else
				{
					optionBtn[selectedItem]->ResetState();
					optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
					selectedItem--;
				}
			}
		}

    if (scrollbaron == 1)
	{
	    if (arrowDownBtn->GetState() == STATE_CLICKED ||
            arrowDownBtn->GetState() == STATE_HELD)
		{

			next = this->FindMenuItem(optionIndex[selectedItem], 1);

			if(next >= 0)
			{
				if(selectedItem == size-1)
				{
					// move list down by 1
					listOffset = this->FindMenuItem(listOffset, 1);
				}
				else if(optionBtn[selectedItem+1]->IsVisible())
				{
					optionBtn[selectedItem]->ResetState();
					optionBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
					selectedItem++;
				}
				scrollbarBoxBtn->Draw();
				usleep(35000);
			}
			if (buttonshold != WPAD_BUTTON_A) {
				arrowDownBtn->ResetState();
			}
		}
		else if(arrowUpBtn->GetState() == STATE_CLICKED ||
                arrowUpBtn->GetState() == STATE_HELD)
		{
			prev = this->FindMenuItem(optionIndex[selectedItem], -1);

			if(prev >= 0)
			{
				if(selectedItem == 0)
				{
					// move list up by 1
					listOffset = prev;
				}
				else
				{
					optionBtn[selectedItem]->ResetState();
					optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
					selectedItem--;
				}
				scrollbarBoxBtn->Draw();
				usleep(35000);
			}
			if (buttonshold != WPAD_BUTTON_A) {
				arrowUpBtn->ResetState();
			}
		}

		if(scrollbarBoxBtn->GetState() == STATE_HELD &&
			scrollbarBoxBtn->GetStateChan() == t->chan &&
			t->wpad.ir.valid && options->GetLength() > size)
		{
			scrollbarBoxBtn->SetPosition(width/2-18+7,0);

			int position = t->wpad.ir.y - 50 - scrollbarBoxBtn->GetTop();

			listOffset = (position * lang)/180 - selectedItem;

			if(listOffset <= 0) {
				listOffset = 0;
				selectedItem = 0;
			} else if(listOffset+size >= lang) {
				listOffset = lang-size;
				selectedItem = size-1;
			}
		}
		int positionbar = 237*(listOffset + selectedItem) / lang;

		if(positionbar > 216)
			positionbar = 216;
		scrollbarBoxBtn->SetPosition(width/2-18+7, positionbar+8);

		if(t->Right())
		{
			if(listOffset < lang && lang > size)
			{
				listOffset =listOffset+ size;
				if(listOffset+size >= lang)
				listOffset = lang-size;
			}
		}
		else if(t->Left())
		{
			if(listOffset > 0)
			{
				listOffset =listOffset- size;
				if(listOffset < 0)
					listOffset = 0;
			}
		}
    }

	if(old_listOffset != listOffset)
		UpdateListEntries();

	if(updateCB)
		updateCB(this);
}
