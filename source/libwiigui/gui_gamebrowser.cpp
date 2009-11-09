/****************************************************************************
 * libwiigui
 *
 * gui_gamebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"

#include <unistd.h>
#include "gui_gamebrowser.h"
#include "../settings/cfg.h"
#include "../main.h"

#include <string.h>
#include <sstream>

#define GAMESELECTSIZE      30
int txtscroll = 0;
/**
 * Constructor for the GuiGameBrowser class.
 */
GuiGameBrowser::GuiGameBrowser(int w, int h, struct discHdr * l, int gameCnt, const char *themePath, const u8 *imagebg, int selected, int offset)
{
	width = w;
	height = h;
	this->gameCnt = gameCnt;
	gameList = l;
	pagesize = THEME.pagesize;
	scrollbaron = (gameCnt > pagesize) ? 1 : 0;
	selectable = true;
	listOffset = (offset == 0) ? this->FindMenuItem(-1, 1) : offset;
	selectedItem = selected - offset;
	focus = 1; // allow focus
	char imgPath[100];

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A, PAD_BUTTON_A);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, Settings.sfxvolume);

	snprintf(imgPath, sizeof(imgPath), "%sbg_options.png", themePath);
	bgGames = new GuiImageData(imgPath, imagebg);

	bgGameImg = new GuiImage(bgGames);
	bgGameImg->SetParent(this);
	bgGameImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	maxTextWidth = bgGameImg->GetWidth() - 24 - 4;

	snprintf(imgPath, sizeof(imgPath), "%sbg_options_entry.png", themePath);
	bgGamesEntry = new GuiImageData(imgPath, bg_options_entry_png);

	snprintf(imgPath, sizeof(imgPath), "%sscrollbar.png", themePath);
	scrollbar = new GuiImageData(imgPath, scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 4);

	maxTextWidth -= scrollbarImg->GetWidth() + 4;

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

	gameIndex = new int[pagesize];
	game = new GuiButton * [pagesize];
	gameTxt = new GuiText * [pagesize];
	gameTxtOver = new GuiText * [pagesize];
	gameBg = new GuiImage * [pagesize];

	for(int i=0; i < pagesize; i++)
	{
		gameTxt[i] = new GuiText(get_title(&gameList[i]), 20, THEME.gametext);
		gameTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		gameTxt[i]->SetPosition(24,0);
		gameTxt[i]->SetMaxWidth(maxTextWidth, GuiText::DOTTED);


		gameTxtOver[i] = new GuiText(get_title(&gameList[i]), 20, THEME.gametext);
		gameTxtOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		gameTxtOver[i]->SetPosition(24,0);
		gameTxtOver[i]->SetMaxWidth(maxTextWidth, GuiText::SCROLL);

		gameBg[i] = new GuiImage(bgGamesEntry);

		game[i] = new GuiButton(width-28,GAMESELECTSIZE);
		game[i]->SetParent(this);
		game[i]->SetLabel(gameTxt[i]);
		game[i]->SetLabelOver(gameTxtOver[i]);
		game[i]->SetImageOver(gameBg[i]);
		game[i]->SetPosition(5,GAMESELECTSIZE*i+4);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);

		gameIndex[i] = i;
	}
	UpdateListEntries();
}

/**
 * Destructor for the GuiGameBrowser class.
 */
GuiGameBrowser::~GuiGameBrowser()
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
	delete bgGameImg;
	delete bgGames;
	delete bgGamesEntry;

	delete trigA;
	delete trigHeldA;
	delete btnSoundClick;

	for(int i=0; i<pagesize; i++)
	{
		delete gameTxt[i];
		delete gameTxtOver[i];
		delete gameBg[i];
		delete game[i];
	}
	delete [] gameIndex;
	delete [] game;
	delete [] gameTxt;
	delete [] gameTxtOver;
	delete [] gameBg;
}

void GuiGameBrowser::SetFocus(int f)
{
	LOCK(this);
    if(!gameCnt)
        return;

	focus = f;

	for(int i=0; i<pagesize; i++)
		game[i]->ResetState();

	if(f == 1)
		game[selectedItem]->SetState(STATE_SELECTED);
}

void GuiGameBrowser::ResetState()
{
	LOCK(this);
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i=0; i<pagesize; i++)
	{
		game[i]->ResetState();
	}
}

int GuiGameBrowser::GetOffset()
{
	return listOffset;
}
int GuiGameBrowser::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++)
	{
		if(game[i]->GetState() == STATE_CLICKED)
		{
			game[i]->SetState(STATE_SELECTED);
			found = gameIndex[i];
			break;
		}
	}
	return found;
}

int GuiGameBrowser::GetSelectedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++)
	{
		if(game[i]->GetState() == STATE_SELECTED)
		{
			game[i]->SetState(STATE_SELECTED);
			found = gameIndex[i];
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

int GuiGameBrowser::FindMenuItem(int currentItem, int direction)
{
	int nextItem = currentItem + direction;

	if(nextItem < 0 || nextItem >= gameCnt)
		return -1;

	if(strlen(get_title(&gameList[nextItem])) > 0)
		return nextItem;
	else
		return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiGameBrowser::Draw()
{
	LOCK(this);
	if(!this->IsVisible() || !gameCnt)
		return;

	bgGameImg->Draw();

	int next = listOffset;

	for(int i=0; i<pagesize; i++)
	{
		if(next >= 0)
		{
			game[i]->Draw();
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

void GuiGameBrowser::UpdateListEntries()
{
	int next = listOffset;
	for(int i=0; i<pagesize; i++)
	{
		if(next >= 0)
		{
			if(game[i]->GetState() == STATE_DISABLED)
			{
				game[i]->SetVisible(true);
				game[i]->SetState(STATE_DEFAULT);
			}
			gameTxt[i]->SetText(get_title(&gameList[next]));
			gameTxt[i]->SetPosition(24, 0);
			gameTxtOver[i]->SetText(get_title(&gameList[next]));
			gameTxtOver[i]->SetPosition(24, 0);

			gameIndex[i] = next;
			next = this->FindMenuItem(next, 1);
		}
		else
		{
			game[i]->SetVisible(false);
			game[i]->SetState(STATE_DISABLED);
		}
	}
}

void GuiGameBrowser::Update(GuiTrigger * t)
{
	LOCK(this);
	if(state == STATE_DISABLED || !t || !gameCnt)
		return;

	int next, prev;
	int old_listOffset = listOffset;
	static int position2;
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

    u32 buttonshold = ButtonsHold();

    if(buttonshold != WPAD_BUTTON_UP && buttonshold != WPAD_BUTTON_DOWN) {

        for(int i=0; i<pagesize; i++)
        {
            if(next >= 0)
                next = this->FindMenuItem(next, 1);

            if(focus)
            {
                if(i != selectedItem && game[i]->GetState() == STATE_SELECTED)
                    game[i]->ResetState();
                else if(i == selectedItem && game[i]->GetState() == STATE_DEFAULT)
                    game[selectedItem]->SetState(STATE_SELECTED, t->chan);
            }

            game[i]->Update(t);

            if(game[i]->GetState() == STATE_SELECTED)
            {
                selectedItem = i;
            }
        }
    }

	// pad and joystick navigation
	if(!focus || !gameCnt)
		return; // skip navigation

	if (scrollbaron == 1)
	{

		if (t->Down() || arrowDownBtn->GetState() == STATE_CLICKED || arrowDownBtn->GetState() == STATE_HELD) //down
		{

			next = this->FindMenuItem(gameIndex[selectedItem], 1);

			if(next >= 0)
			{
				if(selectedItem == pagesize-1)
				{
					// move list down by 1
					listOffset = this->FindMenuItem(listOffset, 1);
				}
				else if(game[selectedItem+1]->IsVisible())
				{
					game[selectedItem]->ResetState();
					game[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
					selectedItem++;
				}
//				scrollbarBoxBtn->Draw();
				usleep(10000 * scrolldelay);
			}
			if (!(ButtonsHold() & WPAD_BUTTON_A))
				arrowDownBtn->ResetState();
		}
		else if(t->Up() || arrowUpBtn->GetState() == STATE_CLICKED || arrowUpBtn->GetState() == STATE_HELD) //up
		{
			prev = this->FindMenuItem(gameIndex[selectedItem], -1);

			if(prev >= 0)
			{
				if(selectedItem == 0)
				{
					// move list up by 1
					listOffset = prev;
				}
				else
				{
					game[selectedItem]->ResetState();
					game[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
					selectedItem--;
				}
//				scrollbarBoxBtn->Draw();
				usleep(10000 * scrolldelay);
			}
			if (!(ButtonsHold() & WPAD_BUTTON_A))
				arrowUpBtn->ResetState();
		}
		int position1 = t->wpad.ir.y;

		if (position2 == 0 && position1 > 0)
		{
			position2 = position1;
		}

		if ((buttonshold & WPAD_BUTTON_B) && position1 > 0)
		{
			scrollbarBoxBtn->ScrollIsOn(1);
			if (position2 > position1)
			{

				prev = this->FindMenuItem(gameIndex[selectedItem], -1);

				if(prev >= 0)
				{
					if(selectedItem == 0)
					{
						// move list up by 1
						listOffset = prev;
					}
					else
					{
						game[selectedItem]->ResetState();
						game[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
						selectedItem--;
					}
//					scrollbarBoxBtn->Draw();
					usleep(10000 * scrolldelay);
				}
			}
			else if (position2 < position1)
			{
				next = this->FindMenuItem(gameIndex[selectedItem], 1);

				if(next >= 0)
				{
					if(selectedItem == pagesize-1)
					{
						// move list down by 1
						listOffset = this->FindMenuItem(listOffset, 1);
					}
					else if(game[selectedItem+1]->IsVisible())
					{
						game[selectedItem]->ResetState();
						game[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
						selectedItem++;
					}
//					scrollbarBoxBtn->Draw();
					usleep(10000 * scrolldelay);
				}
			}

		}
		else if(!(buttonshold & WPAD_BUTTON_B))
		{
			scrollbarBoxBtn->ScrollIsOn(0);
			position2 = 0;
		}

		if(scrollbarBoxBtn->GetState() == STATE_HELD && scrollbarBoxBtn->GetStateChan() == t->chan && t->wpad.ir.valid && gameCnt > pagesize)
		{
			// allow dragging of scrollbar box
			scrollbarBoxBtn->SetPosition(width/2-18+7,0);
			int position = t->wpad.ir.y - 32 - scrollbarBoxBtn->GetTop();

			listOffset = (position * gameCnt)/(25.2 * pagesize) - selectedItem;

			if(listOffset <= 0)
			{
				listOffset = 0;
				selectedItem = 0;
			}
			else if(listOffset+pagesize >= gameCnt)
			{
				listOffset = gameCnt - pagesize;
				selectedItem = pagesize-1;
			}

		}
		int positionbar = (25.2 * pagesize)*(listOffset + selectedItem) / gameCnt;

		if(positionbar > (24 * pagesize))
			positionbar = (24 * pagesize);
		scrollbarBoxBtn->SetPosition(width/2-18+7, positionbar+8);


		if(t->Right()) //skip pagesize # of games if right is pressed
		{
			if(listOffset < gameCnt && gameCnt > pagesize)
			{
				listOffset =listOffset+ pagesize;
				if(listOffset+pagesize >= gameCnt)
					listOffset = gameCnt-pagesize;
			}
		}
		else if(t->Left())
		{
			if(listOffset > 0)
			{
				listOffset =listOffset- pagesize;
				if(listOffset < 0)
					listOffset = 0;
			}
		}

	}
	else
	{
		if(t->Down()) //if there isn't a scrollbar and down is pressed
		{
			next = this->FindMenuItem(gameIndex[selectedItem], 1);

			if(next >= 0)
			{
				if(selectedItem == pagesize-1)
				{
					// move list down by 1
					listOffset = this->FindMenuItem(listOffset, 1);
				}
				else if(game[selectedItem+1]->IsVisible())
				{
					game[selectedItem]->ResetState();
					game[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
					selectedItem++;
				}
			}
		}
		else if(t->Up()) //up
		{
			prev = this->FindMenuItem(gameIndex[selectedItem], -1);

			if(prev >= 0)
			{
				if(selectedItem == 0)
				{
					// move list up by 1
					listOffset = prev;
				}
				else
				{
					game[selectedItem]->ResetState();
					game[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
					selectedItem--;
				}
			}
		}
	}

	if(old_listOffset != listOffset)
		UpdateListEntries();

	if(updateCB)
		updateCB(this);
}

void GuiGameBrowser::Reload(struct discHdr * l, int count)
{
	LOCK(this);
	gameList = l;
	gameCnt = count;
	scrollbaron = (gameCnt > pagesize) ? 1 : 0;
	selectedItem = 0;
	listOffset = 0;
	focus = 1;
	UpdateListEntries();

	for(int i=0; i<pagesize; i++)
		game[i]->ResetState();
}
