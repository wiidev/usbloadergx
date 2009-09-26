/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_optionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"

#include <unistd.h>


#define GAMESELECTSIZE      30

static int scrollbaron, startat, loaded = 0;
/**
 * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l, const u8 *imagebg, int scrollon)
{
	width = w;
	height = h;
	options = l;
	scrollbaron = scrollon;
	selectable = true;
	listOffset = this->FindMenuItem(-1, 1);
	listChanged = true; // trigger an initial list update
	selectedItem = 0;
	focus = 1; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A, PAD_BUTTON_A);

	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgOptions = new GuiImageData(imagebg);
	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgOptionsEntry = new GuiImageData(bg_options_entry_png);
    if (scrollbaron == 1) {
	scrollbar = new GuiImageData(scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 4);

	arrowDown = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUp = new GuiImageData(scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData(scrollbar_arrowup_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);
	scrollbarBox = new GuiImageData(scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOver = new GuiImageData(scrollbar_box_png);
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
	scrollbarBoxBtn->SetMaxY(height);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);
    }

//	optionBg = new GuiImage(bgOptionsEntry);
	for(int i=0; i<PAGESIZE; i++)
	{
		optionTxt[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(24,0);

		optionBg[i] = new GuiImage(bgOptionsEntry);

		optionVal[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionVal[i]->SetPosition(250,0);

		optionBtn[i] = new GuiButton(width-28,GAMESELECTSIZE);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(5,GAMESELECTSIZE*i+4);
		optionBtn[i]->SetRumble(false);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l, const char *themePath, const u8 *imagebg, int scrollon, int start)
{
	width = w;
	height = h;
	options = l;
	startat  = start;
	loaded = 0;
	scrollbaron = scrollon;
	selectable = true;
	listOffset = this->FindMenuItem(-1, 1);
	selectedItem = 0;
	focus = 1; // allow focus
	char imgPath[100];

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A, PAD_BUTTON_A);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	snprintf(imgPath, sizeof(imgPath), "%sbg_options.png", themePath);
	bgOptions = new GuiImageData(imgPath, imagebg);

	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	snprintf(imgPath, sizeof(imgPath), "%sbg_options_entry.png", themePath);
	bgOptionsEntry = new GuiImageData(imgPath, bg_options_entry_png);
    if (scrollbaron == 1) {
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
    }

//	optionBg = new GuiImage(bgOptionsEntry);
	for(int i=0; i<PAGESIZE; i++)
	{
		optionTxt[i] = new GuiText(options->name[i], 20, (GXColor){0, 0, 0, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(24,0);

		optionBg[i] = new GuiImage(bgOptionsEntry);

		optionVal[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionVal[i]->SetPosition(250,0);

		optionBtn[i] = new GuiButton(width-28,GAMESELECTSIZE);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(5,GAMESELECTSIZE*i+4);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
    if (scrollbaron == 1) {
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
    }
    delete bgOptionsImg;
	delete bgOptions;
	delete bgOptionsEntry;
	loaded = 0;

	delete trigA;
	delete btnSoundClick;

//	delete optionBg;
	for(int i=0; i<PAGESIZE; i++)
	{
		delete optionTxt[i];
		delete optionVal[i];
		delete optionBg[i];
		delete optionBtn[i];
	}
}

void GuiOptionBrowser::SetCol2Position(int x)
{
	LOCK(this);
	for(int i=0; i<PAGESIZE; i++)
		optionVal[i]->SetPosition(x,0);
}

void GuiOptionBrowser::SetFocus(int f)
{
	LOCK(this);
	focus = f;

	for(int i=0; i<PAGESIZE; i++)
		optionBtn[i]->ResetState();

	if(f == 1)
		optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiOptionBrowser::ResetState()
{
	LOCK(this);
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i=0; i<PAGESIZE; i++)
	{
		optionBtn[i]->ResetState();
	}
}

int GuiOptionBrowser::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<PAGESIZE; i++)
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

int GuiOptionBrowser::GetSelectedOption()
{
	int found = -1;
	for(int i=0; i<PAGESIZE; i++)
	{
		if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiOptionBrowser::FindMenuItem(int currentItem, int direction)
{
	int nextItem = currentItem + direction;

	if(nextItem < 0 || nextItem >= options->length)
		return -1;

	if(strlen(options->name[nextItem]) > 0)
		return nextItem;
	else
		return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
	LOCK(this);
	if(!this->IsVisible())
		return;

	bgOptionsImg->Draw();

	int next = listOffset;

	for(int i=0; i<PAGESIZE; i++)
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


void GuiOptionBrowser::TriggerUpdate()
{
	listChanged = true;
}	

void GuiOptionBrowser::Update(GuiTrigger * t)
{
	LOCK(this);
	int next, prev, lang = options->length;

	//go to the last game selected
	if ((loaded == 0) && (startat>0))
	{

			if (startat > (lang-9)){
				listOffset= (lang-9);
				selectedItem=startat;
				optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);
				}
			else if (startat < 9){
				selectedItem=startat;
				optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);
				}
			else {
				listOffset = (startat-4);
				selectedItem=startat;
				optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);}
			this->SetFocus(1);
			loaded = 1;
		}

	if(state == STATE_DISABLED || !t)
		return;


	// scrolldelay affects how fast the list scrolls
	// when the arrows are clicked
	float scrolldelay = 3.5;


    if (scrollbaron == 1) {
	// update the location of the scroll box based on the position in the option list


	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);
	scrollbarBoxBtn->Update(t);
    }
	next = listOffset;

	if(listChanged)
	{
		for(int i=0; i<PAGESIZE; i++)
		{
			if(next >= 0)
			{
				if(optionBtn[i]->GetState() == STATE_DISABLED)
				{
					optionBtn[i]->SetVisible(true);
					optionBtn[i]->SetState(STATE_DEFAULT);
				}

				optionTxt[i]->SetText(options->name[next]);
				optionVal[i]->SetText(options->value[next]);
				optionIndex[i] = next;
				next = this->FindMenuItem(next, 1);
			}
			else
			{
				optionBtn[i]->SetVisible(false);
				optionBtn[i]->SetState(STATE_DISABLED);
			}
		}
	}
	for(int i=0; i<PAGESIZE; i++)
	{
		if(focus)
		{
			if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
				optionBtn[i]->ResetState();
			else if(i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
				optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);
		}

		optionBtn[i]->Update(t);

		if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
		}
	}

	// pad/joystick navigation
	if(!focus)
		return; // skip navigation

    if (scrollbaron == 1) {

	if (t->Down() ||
	arrowDownBtn->GetState() == STATE_CLICKED || ////////////////////////////////////////////down
	arrowDownBtn->GetState() == STATE_HELD)
	{

		next = this->FindMenuItem(optionIndex[selectedItem], 1);

		if(next >= 0)
		{
			if(selectedItem == PAGESIZE-1)
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
			usleep(10000 * scrolldelay);


		}WPAD_ScanPads();
        u8 cnt, buttons = NULL;
        /* Get pressed buttons */
        for (cnt = 0; cnt < 4; cnt++)
            buttons |= WPAD_ButtonsHeld(cnt);
        if (buttons == WPAD_BUTTON_A) {

        } else {
            arrowDownBtn->ResetState();

        }

	}
	else if(t->Up() ||
	arrowUpBtn->GetState() == STATE_CLICKED || ////////////////////////////////////////////up
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
			usleep(10000 * scrolldelay);


		}WPAD_ScanPads();
        u8 cnt, buttons = NULL;
        /* Get pressed buttons */
        for (cnt = 0; cnt < 4; cnt++)
            buttons |= WPAD_ButtonsHeld(cnt);
        if (buttons == WPAD_BUTTON_A) {

        } else {
            arrowUpBtn->ResetState();

        }
	}

    if(scrollbarBoxBtn->GetState() == STATE_HELD &&
		scrollbarBoxBtn->GetStateChan() == t->chan &&
		t->wpad.ir.valid && options->length > PAGESIZE)
    {
		scrollbarBoxBtn->SetPosition(width/2-18+7,0);
		int position = t->wpad.ir.y - 50 - scrollbarBoxBtn->GetTop();

		listOffset = (position * lang)/180 - selectedItem;

		if(listOffset <= 0)
		{
			listOffset = 0;
			selectedItem = 0;
		}
		else if(listOffset+PAGESIZE >= lang)
		{
			listOffset = lang-PAGESIZE;
			selectedItem = PAGESIZE-1;
		}

	}
        int positionbar = 237*(listOffset + selectedItem) / lang;

        if(positionbar > 216)
		positionbar = 216;
		scrollbarBoxBtn->SetPosition(width/2-18+7, positionbar+8);


    if(t->Right())
	{
		if(listOffset < lang && lang > PAGESIZE)
		{
			listOffset =listOffset+ PAGESIZE;
			if(listOffset+PAGESIZE >= lang)
            listOffset = lang-PAGESIZE;
		}
	}
	else if(t->Left())
	{
		if(listOffset > 0)
		{
			listOffset =listOffset- PAGESIZE;
			if(listOffset < 0)
				listOffset = 0;
		}
	}

    } else {


	if(t->Down())
	{
		next = this->FindMenuItem(optionIndex[selectedItem], 1);

		if(next >= 0)
		{
			if(selectedItem == PAGESIZE-1)
			{
				// move list down by 1
				listOffset = this->FindMenuItem(listOffset, 1);
				listChanged = true;
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
				listChanged = true;
			}
			else
			{
				optionBtn[selectedItem]->ResetState();
				optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
				selectedItem--;
			}
		}
	}
    }

	if(updateCB)
		updateCB(this);
}
