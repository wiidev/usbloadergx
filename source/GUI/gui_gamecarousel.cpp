/****************************************************************************
 * libwiigui
 *
 * gui_gamecarousel.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "wpad.h"
#include "menu.h"

#include <unistd.h>
#include "gui_image_async.h"
#include "gui_gamecarousel.h"
#include "usbloader/GameList.h"
#include "settings/GameTitles.h"
#include "settings/CSettings.h"
#include "GUI/LoadCoverImage.h"
#include "themes/CTheme.h"
#include "utils/tools.h"
#include "main.h"

#include <string.h>
#include <math.h>
#include <sstream>

#define SCALE	   0.8f
#define DEG_OFFSET  7
#define RADIUS	  780
#define IN_SPEED	175
#define SHIFT_SPEED 75
#define SPEED_STEP  4
#define SPEED_LIMIT 250

static inline int OFFSETLIMIT(int Offset, int gameCnt)
{
	while (Offset < 0)
		Offset += gameCnt;
	return Offset % gameCnt;
}
#define GetGameIndex(pageEntry, listOffset, gameCnt) OFFSETLIMIT(listOffset+pageEntry, gameCnt)
static GuiImageData *GameCarouselLoadCoverImage(void * Arg)
{
	return LoadCoverImage((struct discHdr *) Arg, true, false);
}
/**
 * Constructor for the GuiGameCarousel class.
 */
GuiGameCarousel::GuiGameCarousel(int w, int h, const char *themePath, int offset) :
	noCover(Resources::GetFile("nocover.png"), Resources::GetFileSize("nocover.png"))
{
	width = w;
	height = h;
	pagesize = (gameList.size() < 11) ? gameList.size() : 11;
	listOffset = (gameList.size() < 11) ? LIMIT(offset, 0, MAX(0, gameList.size()-1)) : LIMIT(offset, 0, MAX(0, gameList.size()-1))-2;
	selectable = true;
	selectedItem = -1;
	clickedItem = -1;

	speed = 0;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigL = new GuiTrigger;
	trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR = new GuiTrigger;
	trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_TRIGGER_R);
	trigMinus = new GuiTrigger;
	trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, PAD_TRIGGER_L);

	imgLeft = Resources::GetImageData("startgame_arrow_left.png");
	imgRight = Resources::GetImageData("startgame_arrow_right.png");

	btnLeftImg = new GuiImage(imgLeft);
	if (Settings.wsprompt == ON) btnLeftImg->SetWidescreen(Settings.widescreen);
	btnLeft = new GuiButton(imgLeft->GetWidth(), imgLeft->GetHeight());
	btnLeft->SetAlignment(thAlign("left - carousel layout left arrow align hor"), thAlign("top - carousel layout left arrow align ver"));
	btnLeft->SetPosition(thInt("20 - carousel layout left arrow pos x"), thInt("65 - carousel layout left arrow pos y"));
	btnLeft->SetParent(this);
	btnLeft->SetImage(btnLeftImg);
	btnLeft->SetSoundOver(btnSoundOver);
	btnLeft->SetTrigger(trigA);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);
	btnLeft->SetEffectGrow();

	btnRightImg = new GuiImage(imgRight);
	if (Settings.wsprompt == ON) btnRightImg->SetWidescreen(Settings.widescreen);
	btnRight = new GuiButton(imgRight->GetWidth(), imgRight->GetHeight());
	btnRight->SetParent(this);
	btnRight->SetAlignment(thAlign("right - carousel layout right arrow align hor"), thAlign("top - carousel layout right arrow align ver"));
	btnRight->SetPosition(thInt("-20 - carousel layout right arrow pos x"), thInt("65 - carousel layout right arrow pos y"));
	btnRight->SetImage(btnRightImg);
	btnRight->SetSoundOver(btnSoundOver);
	btnRight->SetTrigger(trigA);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);
	btnRight->SetEffectGrow();

	gamename = new GuiText(" ", 18, thColor("r=55 g=190 b=237 a=255 - carousel game name text color"));
	gamename->SetParent(this);
	gamename->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	gamename->SetPosition(0, 330);
	gamename->SetMaxWidth(280, DOTTED);

	gameIndex = new int[pagesize];
	game.resize(pagesize);
	coverImg.resize(pagesize);

	Refresh();
}

/**
 * Destructor for the GuiGameCarousel class.
 */
GuiGameCarousel::~GuiGameCarousel()
{
	delete imgRight;
	delete imgLeft;
	delete btnLeftImg;
	delete btnRightImg;
	delete btnRight;
	delete btnLeft;

	delete trigA;
	delete trigL;
	delete trigR;
	delete trigPlus;
	delete trigMinus;
	delete gamename;

	GuiImageAsync::ClearQueue();

	for (u32 i = 0; i < game.size(); ++i)
		delete coverImg[i];
	for (u32 i = 0; i < game.size(); ++i)
		delete game[i];

	delete[] gameIndex;

}

void GuiGameCarousel::setListOffset(int off)
{
	LOCK( this );
	if(gameList.size() < 11)
		listOffset = MIN(off, gameList.size()-1);
	else
		listOffset = MIN(off, gameList.size()) - 2;

	Refresh();
}

int GuiGameCarousel::getListOffset() const
{
	if(gameList.size() < 11)
		return listOffset;
	else
		return (listOffset + 2) % gameList.size();
}

void GuiGameCarousel::SetSelectedOption(int ind)
{
	LOCK(this);
	selectedItem = LIMIT(ind, 0, MIN(pagesize, MAX(0, gameList.size()-1)));
}

void GuiGameCarousel::Refresh()
{
	for (int i = 0; i < pagesize; i++)
	{
		//------------------------
		// Index
		//------------------------
		gameIndex[i] = GetGameIndex( i, listOffset, gameList.size() );

		//------------------------
		// Image
		//------------------------
		delete coverImg[i];
		coverImg[i] = new (std::nothrow) GuiImageAsync(GameCarouselLoadCoverImage, gameList[gameIndex[i]], sizeof(struct discHdr), &noCover);
		if (coverImg[i]) coverImg[i]->SetWidescreen(Settings.widescreen);

		//------------------------
		// GameButton
		//------------------------
		delete game[i];
		game[i] = new GuiButton(122, 244);
		game[i]->SetParent(this);
		game[i]->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
		game[i]->SetPosition(0, 740);
		game[i]->SetImage(coverImg[i]);
		game[i]->SetScale(SCALE);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetClickable(true);
		game[i]->SetEffect(EFFECT_GOROUND, IN_SPEED, 90 - (pagesize - 2 * i - 1) * DEG_OFFSET / 2, RADIUS, 180, 1, 0, RADIUS);
	}
}

void GuiGameCarousel::SetFocus(int f)
{
	LOCK( this );
	if (!gameList.size()) return;

	for (int i = 0; i < pagesize; i++)
		game[i]->ResetState();

	if (f == 1 && selectedItem >= 0) game[selectedItem]->SetState(STATE_SELECTED);
}

void GuiGameCarousel::ResetState()
{
	LOCK( this );
	if (state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for (int i = 0; i < pagesize; i++)
	{
		game[i]->ResetState();
	}
}

int GuiGameCarousel::GetClickedOption()
{
	LOCK( this );
	int found = -1;
	if (clickedItem >= 0)
	{
		for (int i = pagesize - 1; i >= 0; i--)
			game[i]->ResetState();

		game[clickedItem]->SetState(STATE_SELECTED);
		found = gameIndex[clickedItem];
		clickedItem = -1;
	}
	return found;
}

int GuiGameCarousel::GetSelectedOption()
{
	LOCK( this );
	int found = -1;
	for (int i = 0; i < pagesize; i++)
	{
		if (game[i]->GetState() == STATE_SELECTED)
		{
			game[i]->SetState(STATE_SELECTED);
			found = gameIndex[i];
			break;
		}
	}
	return found;
}

/**
 * Draw the button on screen
 */
void GuiGameCarousel::Draw()
{
	LOCK( this );
	if (!this->IsVisible() || !gameList.size()) return;

	for (int i = 0; i < pagesize; i++)
		game[i]->Draw();

	gamename->Draw();

	if (gameList.size() > 6)
	{
		btnRight->Draw();
		btnLeft->Draw();
	}

	//!Draw tooltip after the Images to have it on top
	if (Settings.tooltips == ON)
		for (int i = 0; i < pagesize; i++)
			game[i]->DrawTooltip();

	this->UpdateEffects();
}

void GuiGameCarousel::Update(GuiTrigger * t)
{
	LOCK( this );
	if (state == STATE_DISABLED || !t || !gameList.size() || !pagesize) return;

	btnRight->Update(t);
	btnLeft->Update(t);

	if ((game[0]->GetEffect() & EFFECT_GOROUND) || (game[pagesize - 1]->GetEffect() & EFFECT_GOROUND))
	{
		return; // skip when rotate
	}

	// find selected + clicked
	int selectedItem_old = selectedItem;
	selectedItem = -1;
	clickedItem = -1;
	for (int i = pagesize - 1; i >= 0; i--)
	{
		game[i]->Update(t);
		if (game[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
		}
		if (game[i]->GetState() == STATE_CLICKED)
		{
			clickedItem = i;
		}

	}

	/// OnOver-Effect + GameText + Tooltop
	if (selectedItem_old != selectedItem)
	{
		if (selectedItem >= 0)
		{
			game[selectedItem]->SetEffect(EFFECT_SCALE, 1, 130);
			gamename->SetText(GameTitles.GetTitle(gameList[gameIndex[selectedItem]]));
		}
		else gamename->SetText((char*) NULL);
		if (selectedItem_old >= 0) game[selectedItem_old]->SetEffect(EFFECT_SCALE, -1, 100);
	}
	// navigation
	if (gameList.size() > 6)
	{

		int newspeed = 0;
		// Left/Right Navigation
		if (btnLeft->GetState() == STATE_CLICKED)
		{
			u32 buttons = t->wpad.btns_h;
			u32 buttonsPAD = t->pad.btns_h;
			if (!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_MINUS) ||
				  (buttons & WPAD_CLASSIC_BUTTON_A) || (buttons & WPAD_CLASSIC_BUTTON_MINUS) ||
				  (buttonsPAD & PAD_BUTTON_A) || (buttonsPAD & PAD_TRIGGER_L)  || t->Left()))
			{
				btnLeft->ResetState();
				return;
			}

			if (Settings.xflip == XFLIP_SYSMENU || Settings.xflip == XFLIP_YES || Settings.xflip == XFLIP_DISK3D)
				newspeed = SHIFT_SPEED;
			else newspeed = -SHIFT_SPEED;
		}
		else if (btnRight->GetState() == STATE_CLICKED)
		{
			u32 buttons = t->wpad.btns_h;
			u32 buttonsPAD = t->pad.btns_h;
			if (!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_PLUS) ||
				  (buttons & WPAD_CLASSIC_BUTTON_A) || (buttons & WPAD_CLASSIC_BUTTON_PLUS) ||
				  (buttonsPAD & PAD_BUTTON_A) || (buttonsPAD & PAD_TRIGGER_R)  || t->Right()))
			{
				btnRight->ResetState();
				return;
			}
			if (Settings.xflip == XFLIP_SYSMENU || Settings.xflip == XFLIP_YES || Settings.xflip == XFLIP_DISK3D)
				newspeed = -SHIFT_SPEED;
			else newspeed = SHIFT_SPEED;
		}
		if (newspeed)
		{
			if (speed == 0)
				speed = newspeed;
			else if (speed > 0)
			{
				if ((speed += SPEED_STEP) > SPEED_LIMIT) speed = SPEED_LIMIT;
			}
			else
			{
				if ((speed -= SPEED_STEP) < -SPEED_LIMIT) speed = -SPEED_LIMIT;
			}
		}
		else speed = 0;

		if (speed > 0) // rotate right
		{
			GuiButton *tmpButton;
			listOffset = OFFSETLIMIT(listOffset - 1, gameList.size()); // set the new listOffset
			// Save right Button + TollTip and destroy right Image + Image-Data
			delete coverImg[pagesize - 1];
			coverImg[pagesize - 1] = NULL;
			game[pagesize - 1]->SetImage(NULL);
			tmpButton = game[pagesize - 1];

			// Move all Page-Entries one step right
			for (int i = pagesize - 1; i >= 1; i--)
			{
				coverImg[i] = coverImg[i - 1];
				game[i] = game[i - 1];
				gameIndex[i] = gameIndex[i - 1];
			}
			// set saved Button & gameIndex to right
			gameIndex[0] = listOffset;
			coverImg[0] = new GuiImageAsync(GameCarouselLoadCoverImage, gameList[gameIndex[0]], sizeof(struct discHdr),
					&noCover);
			coverImg[0] ->SetWidescreen(Settings.widescreen);

			game[0] = tmpButton;
			game[0] ->SetImage(coverImg[0]);

			for (int i = 0; i < pagesize; i++)
			{
				game[i]->StopEffect();
				game[i]->ResetState();
				game[i]->SetEffect(EFFECT_GOROUND, speed, DEG_OFFSET, RADIUS, 270 - (pagesize - 2 * i + 1) * DEG_OFFSET
						/ 2, 1, 0, RADIUS);
				game[i]->UpdateEffects(); // rotate one step for liquid scrolling
			}
		}
		else if (speed < 0) // rotate left
		{
			GuiButton *tmpButton;
			listOffset = OFFSETLIMIT(listOffset + 1, gameList.size()); // set the new listOffset
			// Save left Button + TollTip and destroy left Image + Image-Data
			delete coverImg[0];
			coverImg[0] = NULL;
			game[0]->SetImage(NULL);
			tmpButton = game[0];

			// Move all Page-Entries one step left
			for (int i = 0; i < (pagesize - 1); i++)
			{
				coverImg[i] = coverImg[i + 1];
				game[i] = game[i + 1];
				gameIndex[i] = gameIndex[i + 1];
			}
			// set saved Button & gameIndex to right
			int ii = pagesize - 1;
			gameIndex[ii] = OFFSETLIMIT(listOffset + ii, gameList.size());
			coverImg[ii] = new GuiImageAsync(GameCarouselLoadCoverImage, gameList[gameIndex[ii]],
					sizeof(struct discHdr), &noCover);
			coverImg[ii] ->SetWidescreen(Settings.widescreen);

			game[ii] = tmpButton;
			game[ii] ->SetImage(coverImg[ii]);

			for (int i = 0; i < pagesize; i++)
			{
				game[i]->StopEffect();
				game[i]->ResetState();
				game[i]->SetEffect(EFFECT_GOROUND, speed, DEG_OFFSET, RADIUS, 270 - (pagesize - 2 * i - 3) * DEG_OFFSET
						/ 2, 1, 0, RADIUS);
				game[i]->UpdateEffects(); // rotate one step for liquid scrolling
			}
		}

	}
	if (updateCB) updateCB(this);
}

