/****************************************************************************
 * libwiigui
 *
 * gui_gamebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "wpad.h"

#include <unistd.h>
#include "gui_gamelist.h"
#include "settings/CSettings.h"
#include "main.h"
#include "settings/newtitles.h"
#include "settings/GameTitles.h"
#include "usbloader/GameList.h"
#include "themes/CTheme.h"
#include "utils/tools.h"
#include "menu.h"

#include <string.h>
#include <sstream>

#define GAMESELECTSIZE	  30

/**
 * Constructor for the GuiGameList class.
 */
GuiGameList::GuiGameList(int w, int h, int offset)
	: scrollBar(h-10)
{
	width = w;
	height = h;
	pagesize = thInt("9 - game list browser page size");
	selectable = true;
	listOffset = LIMIT(offset, 0, MAX(0, gameList.size()-pagesize));
	selectedItem = 0;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	bgGames = Resources::GetImageData("bg_options.png");
	newGames = Resources::GetImageData("new.png");

	scrollBar.SetParent(this);
	scrollBar.SetAlignment(thAlign("right - game browser scrollbar align hor"), thAlign("top - game browser scrollbar align ver"));
	scrollBar.SetPosition(thInt("0 - game browser scrollbar pos x"), thInt("5 - game browser scrollbar pos y"));
	scrollBar.SetButtonScroll(WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B);
	scrollBar.SetPageSize(pagesize);
	scrollBar.SetSelectedItem(selectedItem);
	scrollBar.SetSelectedIndex(listOffset);
	scrollBar.SetEntrieCount(gameList.size());
	scrollBar.listChanged.connect(this, &GuiGameList::onListChange);

	bgGameImg = new GuiImage(bgGames);
	bgGameImg->SetParent(this);
	bgGameImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgGamesEntry = Resources::GetImageData("bg_options_entry.png");

	maxTextWidth = bgGameImg->GetWidth() - scrollBar.GetWidth() - 38;

	game = new GuiButton *[pagesize];
	gameTxt = new GuiText *[pagesize];
	gameTxtOver = new GuiText *[pagesize];
	gameBg = new GuiImage *[pagesize];
	newImg = new GuiImage *[pagesize];

	for (int i = 0; i < pagesize; ++i)
	{
		gameTxt[i] = new GuiText((char *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - game browser list text color"));
		gameTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		gameTxt[i]->SetPosition(24, 0);
		gameTxt[i]->SetMaxWidth(maxTextWidth, DOTTED);

		gameTxtOver[i] = new GuiText((char *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - game browser list text color over"));
		gameTxtOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		gameTxtOver[i]->SetPosition(24, 0);
		gameTxtOver[i]->SetMaxWidth(maxTextWidth, SCROLL_HORIZONTAL);

		gameBg[i] = new GuiImage(bgGamesEntry);

		newImg[i] = new GuiImage(newGames);
		newImg[i]->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
		newImg[i]->SetVisible(false);

		game[i] = new GuiButton(width - scrollBar.GetWidth(), GAMESELECTSIZE);
		game[i]->SetParent(this);
		game[i]->SetLabel(gameTxt[i]);
		game[i]->SetLabelOver(gameTxtOver[i]);
		game[i]->SetIcon(newImg[i]);
		game[i]->SetImageOver(gameBg[i]);
		game[i]->SetPosition(5, GAMESELECTSIZE * i + 4);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetVisible(false);
		game[i]->SetState(STATE_DISABLED);
	}
	UpdateListEntries();
}

/**
 * Destructor for the GuiGameList class.
 */
GuiGameList::~GuiGameList()
{
	delete bgGameImg;
	delete bgGames;
	delete bgGamesEntry;
	delete newGames;

	delete trigA;

	for (int i = 0; i < pagesize; ++i)
	{
		delete gameTxt[i];
		delete gameTxtOver[i];
		delete gameBg[i];
		delete game[i];
		delete newImg[i];
	}
	delete[] game;
	delete[] gameTxt;
	delete[] gameTxtOver;
	delete[] gameBg;
}

void GuiGameList::SetFocus(int f)
{
	LOCK( this );
	if (!gameList.size()) return;

	for (int i = 0; i < pagesize; ++i)
		game[i]->ResetState();

	if (f == 1) game[selectedItem]->SetState(STATE_SELECTED);
}

void GuiGameList::ResetState()
{
	LOCK( this );
	if (state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for (int i = 0; i < pagesize; ++i)
	{
		game[i]->ResetState();
	}
}

int GuiGameList::GetClickedOption()
{
	int found = -1;
	for (int i = 0; i < pagesize; ++i)
	{
		if (game[i]->GetState() == STATE_CLICKED)
		{
			game[i]->SetState(STATE_SELECTED);
			found = listOffset + i;
			break;
		}
	}
	return found;
}

void GuiGameList::onListChange(int SelItem, int SelInd)
{
	selectedItem = SelItem;
	listOffset = SelInd;
	UpdateListEntries();
}

void GuiGameList::setListOffset(int off)
{
	LOCK(this);
	listOffset = LIMIT(off, 0, MAX(0, gameList.size()-pagesize));
}

void GuiGameList::SetSelectedOption(int ind)
{
	LOCK(this);
	selectedItem = LIMIT(ind, 0, MIN(pagesize, MAX(0, gameList.size()-1)));
}

/**
 * Draw the button on screen
 */
void GuiGameList::Draw()
{
	LOCK( this );
	if (!this->IsVisible() || !gameList.size()) return;

	bgGameImg->Draw();

	for (int i = 0, next = listOffset; i < pagesize; ++i, ++next)
	{
		if (next < gameList.size())
			game[i]->Draw();
	}

	scrollBar.Draw();

	this->UpdateEffects();
}

void GuiGameList::UpdateListEntries()
{
	for (int i = 0, next = listOffset; i < pagesize; ++i, ++next)
	{
		if (next < gameList.size())
		{
			if (game[i]->GetState() == STATE_DISABLED)
			{
				game[i]->SetVisible(true);
				game[i]->SetState(STATE_DEFAULT);
			}
			gameTxt[i]->SetText(GameTitles.GetTitle(gameList[next]));
			gameTxt[i]->SetPosition(24, 0);
			gameTxtOver[i]->SetText(GameTitles.GetTitle(gameList[next]));
			gameTxtOver[i]->SetPosition(24, 0);

			if (Settings.marknewtitles)
			{
				bool isNew = NewTitles::Instance()->IsNew(gameList[next]->id);
				if (isNew)
				{
					gameTxt[i]->SetMaxWidth(maxTextWidth - (newGames->GetWidth() + 1), DOTTED);
					gameTxtOver[i]->SetMaxWidth(maxTextWidth - (newGames->GetWidth() + 1), SCROLL_HORIZONTAL);
				}
				else
				{
					gameTxt[i]->SetMaxWidth(maxTextWidth, DOTTED);
					gameTxtOver[i]->SetMaxWidth(maxTextWidth, SCROLL_HORIZONTAL);
				}
				newImg[i]->SetVisible(isNew);
			}
		}
		else
		{
			game[i]->SetVisible(false);
			game[i]->SetState(STATE_DISABLED);
		}
	}
}

void GuiGameList::Update(GuiTrigger * t)
{
	LOCK( this );
	if (state == STATE_DISABLED || !t || !gameList.size()) return;

	static int pressedChan = -1;

	if((t->wpad.btns_d & (WPAD_BUTTON_B | WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT |
						  WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT)) ||
		(t->pad.btns_d & (PAD_BUTTON_UP | PAD_BUTTON_DOWN)))
		pressedChan = t->chan;

	// update the location of the scroll box based on the position in the option list
	scrollBar.Update(t);

	if(pressedChan == -1 || (!t->wpad.btns_h && !t->pad.btns_h))
	{
		for (int i = 0, next = listOffset; i < pagesize; ++i, ++next)
		{
			if (next >= gameList.size())
				break;

			if (i != selectedItem && game[i]->GetState() == STATE_SELECTED)
				game[i]->ResetState();
			else if (i == selectedItem && game[i]->GetState() == STATE_DEFAULT)
				game[selectedItem]->SetState(STATE_SELECTED, -1);

			game[i]->Update(t);

			if (game[i]->GetState() == STATE_SELECTED)
				selectedItem = i;
		}
	}

	if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h)
		pressedChan = -1;

	scrollBar.SetPageSize(pagesize);
	scrollBar.SetSelectedItem(selectedItem);
	scrollBar.SetSelectedIndex(listOffset);
	scrollBar.SetEntrieCount(gameList.size());

	if (updateCB) updateCB(this);
}

