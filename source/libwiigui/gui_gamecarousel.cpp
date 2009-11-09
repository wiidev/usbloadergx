/****************************************************************************
 * libwiigui
 *
 * gui_gamecarousel.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"
#include "../menu.h"

#include <unistd.h>
#include "gui_image_async.h"
#include "gui_gamecarousel.h"
#include "../settings/cfg.h"
#include "../main.h"

#include <string.h>
#include <math.h>
#include <sstream>

#define SCALE		0.8f
#define DEG_OFFSET	7
#define RADIUS		780
#define IN_SPEED	175
#define SHIFT_SPEED	75
#define SPEED_STEP	4
#define SPEED_LIMIT	250

static inline int OFFSETLIMIT(int Offset, int gameCnt)
{
	while(Offset < 0) Offset+=gameCnt;
	return Offset%gameCnt;
}
#define GetGameIndex(pageEntry, listOffset, gameCnt) OFFSETLIMIT(listOffset+pageEntry, gameCnt)
static GuiImageData *GameCarouselLoadCoverImage(void * Arg)
{
	return LoadCoverImage((struct discHdr *)Arg, true, false);
}
/**
 * Constructor for the GuiGameCarousel class.
 */
GuiGameCarousel::GuiGameCarousel(int w, int h, struct discHdr * l, int count, const char *themePath, const u8 *imagebg, int selected, int offset) :
noCover(nocover_png)
{
	width = w;
	height = h;
	gameCnt = count;
	gameList = l;
	pagesize = (gameCnt < 11) ? gameCnt : 11;
	listOffset = 0;
	selectable = true;
	selectedItem = -1;
	focus = 1;					 // allow focus
	clickedItem = -1;
	
	speed = 0;
	char imgPath[100];

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigL = new GuiTrigger;
	trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR = new GuiTrigger;
	trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
	trigMinus = new GuiTrigger;
	trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	btnSoundClick = new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);

	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	imgLeft = new GuiImageData(imgPath, startgame_arrow_left_png);
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	imgRight = new GuiImageData(imgPath, startgame_arrow_right_png);

	int btnHeight = (int) lround(sqrt(RADIUS*RADIUS - 90000)-RADIUS-50);

	btnLeftImg = new GuiImage(imgLeft);
	if (Settings.wsprompt == yes)
		btnLeftImg->SetWidescreen(CFG.widescreen);
	btnLeft = new GuiButton(imgLeft->GetWidth(), imgLeft->GetHeight());
	btnLeft->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	btnLeft->SetPosition(20, btnHeight);
	btnLeft->SetParent(this);
	btnLeft->SetImage(btnLeftImg);
	btnLeft->SetSoundOver(btnSoundOver);
	btnLeft->SetTrigger(trigA);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);
	btnLeft->SetEffectGrow();

	btnRightImg = new GuiImage(imgRight);
	if (Settings.wsprompt == yes)
		btnRightImg->SetWidescreen(CFG.widescreen);
	btnRight = new GuiButton(imgRight->GetWidth(), imgRight->GetHeight());
	btnRight->SetParent(this);
	btnRight->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	btnRight->SetPosition(-20, btnHeight);
	btnRight->SetImage(btnRightImg);
	btnRight->SetSoundOver(btnSoundOver);
	btnRight->SetTrigger(trigA);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);
	btnRight->SetEffectGrow();

	gamename = new GuiText(" ", 18, THEME.info);
	gamename->SetParent(this);
    gamename->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    gamename->SetPosition(0, 330);
    gamename->SetMaxWidth(280, GuiText::DOTTED);

	gameIndex	= new int[pagesize];
	game		= new GuiButton * [pagesize];
	titleTT		= new GuiTooltip * [pagesize];
	coverImg	= new GuiImageAsync * [pagesize];

	for(int i=0; i < pagesize; i++)
	{
		//------------------------
		// Index
		//------------------------
		gameIndex[i] = GetGameIndex(i, listOffset, gameCnt);

		//------------------------
		// Tooltip
		//------------------------
		titleTT[i] = new GuiTooltip(get_title(&gameList[gameIndex[i]]), THEME.tooltipAlpha);

		//------------------------
		// Image
		//------------------------
		coverImg[i]	= new(std::nothrow) GuiImageAsync(GameCarouselLoadCoverImage, &gameList[gameIndex[i]], sizeof(struct discHdr), &noCover);
		if(coverImg[i])
			coverImg[i]->SetWidescreen(CFG.widescreen);

		//------------------------
		// GameButton
		//------------------------

		game[i] = new GuiButton(122,244);
		game[i]->SetParent(this);
		game[i]->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
		game[i]->SetPosition(0,740);
		game[i]->SetImage(coverImg[i]);
        game[i]->SetScale(SCALE);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetClickable(true);
		game[i]->SetEffect(EFFECT_GOROUND, IN_SPEED,  90-(pagesize-2*i-1)*DEG_OFFSET/2, RADIUS, 180, 1, 0, RADIUS);
		switch((i*3)/pagesize)
		{
			case 0:
				game[i]->SetToolTip(titleTT[i], 122/2, -244/4, ALIGN_LEFT, ALIGN_MIDDLE);
				break;
			case 1:
				game[i]->SetToolTip(titleTT[i], 0, -244/4, ALIGN_CENTRE, ALIGN_MIDDLE);
				break;
			case 2:
				game[i]->SetToolTip(titleTT[i], -122/2, -244/4, ALIGN_RIGHT, ALIGN_MIDDLE);
				break;
			default:
				break;
		}		
	}
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
	delete btnSoundClick;
	delete btnSoundOver;
    delete gamename;

	for(int i=0; i<pagesize; i++) {
		delete coverImg[i];
		delete titleTT[i];
		delete game[i];
	}
	delete [] gameIndex;
	delete [] coverImg;
	delete [] game;
	

}


void GuiGameCarousel::SetFocus(int f)
{
	LOCK(this);
	if(!gameCnt) return;

	focus = f;

	for(int i=0; i<pagesize; i++)
		game[i]->ResetState();

	if(f == 1 && selectedItem>=0)
		game[selectedItem]->SetState(STATE_SELECTED);
}


void GuiGameCarousel::ResetState()
{
	LOCK(this);
	if(state != STATE_DISABLED) {
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i=0; i<pagesize; i++) {
		game[i]->ResetState();
	}
}


int GuiGameCarousel::GetOffset()
{
	LOCK(this);
	return listOffset;
}


int GuiGameCarousel::GetClickedOption()
{
	LOCK(this);
	int found = -1;
	if (clickedItem>=0)
	{
		game[clickedItem]->SetState(STATE_SELECTED);
		found = gameIndex[clickedItem];
		clickedItem=-1;
	}
	return found;
}


int GuiGameCarousel::GetSelectedOption()
{
	LOCK(this);
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

/**
 * Draw the button on screen
 */
void GuiGameCarousel::Draw()
{
	LOCK(this);
	if(!this->IsVisible() || !gameCnt)
		return;

	for(int i=0; i<pagesize; i++)
		game[i]->Draw();


    gamename->Draw();

	if(gameCnt > 6)
	{
		btnRight->Draw();
		btnLeft->Draw();
	}
	
	//!Draw tooltip after the Images to have it on top
	if (focus && Settings.tooltips == TooltipsOn)
		for(int i=0; i<pagesize; i++)
			game[i]->DrawTooltip();

	this->UpdateEffects();
}


void GuiGameCarousel::Update(GuiTrigger * t)
{
	LOCK(this);
	if(state == STATE_DISABLED || !t || !gameCnt)
		return;

	btnRight->Update(t);
	btnLeft->Update(t);
	
	if(game[0]->GetEffect() & EFFECT_GOROUND || game[pagesize-1]->GetEffect() & EFFECT_GOROUND)
	{
		return; // skip when rotate
	}

	// find selected + clicked
	int selectedItem_old = selectedItem;
	selectedItem = -1;
	clickedItem = -1;
	for(int i=pagesize-1; i>=0; i--)
	{
		game[i]->Update(t);
		if(game[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
		}
		if(game[i]->GetState() == STATE_CLICKED)
		{
			clickedItem = i;
		}

	}

	/// OnOver-Effect + GameText + Tooltop
	if(selectedItem_old != selectedItem)
	{
		if(selectedItem>=0)
		{
			game[selectedItem]->SetEffect(EFFECT_SCALE, 1, 130);
			char *gameTitle = get_title(&gameList[gameIndex[selectedItem]]);
			gamename->SetText(gameTitle);
		}
		else
			gamename->SetText((char*)NULL);
		if(selectedItem_old>=0)
			game[selectedItem_old]->SetEffect(EFFECT_SCALE, -1, 100);
	}
	// navigation
	if(focus && gameCnt>6)
	{

		int newspeed = 0;
		// Left/Right Navigation
		if (btnLeft->GetState() == STATE_CLICKED)
		{
			WPAD_ScanPads();
			u16 buttons = 0;
			for(int i=0; i<4; i++)
				buttons |= WPAD_ButtonsHeld(i);
			if(!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_MINUS) || t->Left()))
			{
				btnLeft->ResetState();
				return;
			}

			if (Settings.xflip==sysmenu || Settings.xflip==yes || Settings.xflip==disk3d)
				newspeed	= SHIFT_SPEED;
			else
				newspeed	= -SHIFT_SPEED;
		}
		else if(btnRight->GetState() == STATE_CLICKED)
		{
			WPAD_ScanPads();
			u16 buttons = 0;
			for(int i=0; i<4; i++)
				buttons |= WPAD_ButtonsHeld(i);
			if(!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_PLUS) || t->Right()))
			{
				btnRight->ResetState();
				return;
			}
			if (Settings.xflip==sysmenu ||Settings.xflip==yes || Settings.xflip==disk3d)
				newspeed	= -SHIFT_SPEED;
			else
				newspeed	= SHIFT_SPEED;
		}
		if(newspeed)
		{
			if(speed==0)
				speed = newspeed;
			else if(speed>0)
			{
				if((speed+=SPEED_STEP) > SPEED_LIMIT)
					speed = SPEED_LIMIT;
			}
			else
			{
				if((speed-=SPEED_STEP) < -SPEED_LIMIT)
					speed = -SPEED_LIMIT;
			}
		}
		else
			speed = 0;


		if(speed > 0) // rotate right
		{
			GuiButton *tmpButton;
			GuiTooltip *tmpTT;
			listOffset = OFFSETLIMIT(listOffset - 1, gameCnt); // set the new listOffset
			// Save right Button + TollTip and destroy right Image + Image-Data 
			delete coverImg[pagesize-1]; coverImg[pagesize-1] = NULL;game[pagesize-1]->SetImage(NULL);
			tmpButton	= game[pagesize-1];
			tmpTT		= titleTT[pagesize-1];
			
			// Move all Page-Entries one step right
			for (int i=pagesize-1; i>=1; i--)
			{
				titleTT[i]		= titleTT[i-1];
				coverImg[i]		= coverImg[i-1];
				game[i]			= game[i-1];
				gameIndex[i]	= gameIndex[i-1];
			}
			// set saved Button & gameIndex to right
			gameIndex[0]		= listOffset;
			titleTT[0]			= tmpTT;
			titleTT[0]			->SetText(get_title(&gameList[gameIndex[0]]));
			coverImg[0]			= new GuiImageAsync(GameCarouselLoadCoverImage, &gameList[gameIndex[0]], sizeof(struct discHdr), &noCover);
			coverImg[0]			->SetWidescreen(CFG.widescreen);

			game[0]				= tmpButton;
			game[0]				->SetImage(coverImg[0]);

			
			for(int i=0; i<pagesize; i++)
			{
				game[i]->StopEffect();
				game[i]->ResetState();
				game[i]->SetEffect(EFFECT_GOROUND, speed, DEG_OFFSET, RADIUS, 270-(pagesize-2*i+1)*DEG_OFFSET/2, 1, 0, RADIUS);
				game[i]->UpdateEffects(); // rotate one step for liquid scrolling

				// Set Tooltip-Position
				switch((i*3)/pagesize)
				{
					case 0:
						game[i]->SetToolTip(titleTT[i], 122/4, -244/4, ALIGN_LEFT, ALIGN_MIDDLE);
						break;
					case 1:
						game[i]->SetToolTip(titleTT[i], 0, -244/4, ALIGN_CENTRE, ALIGN_MIDDLE);
						break;
					case 2:
						game[i]->SetToolTip(titleTT[i], -122/4, -244/4, ALIGN_RIGHT, ALIGN_MIDDLE);
						break;
					default:
						break;
				}		
			}
		}
		else if(speed < 0) // rotate left
		{
			GuiButton *tmpButton;
			GuiTooltip *tmpTT;
			listOffset = OFFSETLIMIT(listOffset + 1, gameCnt); // set the new listOffset
			// Save left Button + TollTip and destroy left Image + Image-Data 
			delete coverImg[0]; coverImg[0] = NULL;game[0]->SetImage(NULL);
			tmpButton	= game[0];
			tmpTT		= titleTT[0];
			
			// Move all Page-Entries one step left
			for (int i=0; i<(pagesize-1); i++)
			{
				titleTT[i]		= titleTT[i+1];
				coverImg[i]		= coverImg[i+1];
				game[i]			= game[i+1];
				gameIndex[i]	= gameIndex[i+1];
			}
			// set saved Button & gameIndex to right
			int ii = pagesize-1;
			gameIndex[ii]		= OFFSETLIMIT(listOffset + ii, gameCnt);
			titleTT[ii]			= tmpTT;
			titleTT[ii]			->SetText(get_title(&gameList[gameIndex[ii]]));
			coverImg[ii]		= new GuiImageAsync(GameCarouselLoadCoverImage, &gameList[gameIndex[ii]], sizeof(struct discHdr), &noCover);
			coverImg[ii]		->SetWidescreen(CFG.widescreen);

			game[ii]			= tmpButton;
			game[ii]			->SetImage(coverImg[ii]);

			
			for(int i=0; i<pagesize; i++)
			{
				game[i]->StopEffect();
				game[i]->ResetState();
				game[i]->SetEffect(EFFECT_GOROUND, speed, DEG_OFFSET, RADIUS, 270-(pagesize-2*i-3)*DEG_OFFSET/2, 1, 0, RADIUS);
				game[i]->UpdateEffects(); // rotate one step for liquid scrolling

				// Set Tooltip-Position
				switch((i*3)/pagesize)
				{
					case 0:
						game[i]->SetToolTip(titleTT[i], 122/4, -244/4, ALIGN_LEFT, ALIGN_MIDDLE);
						break;
					case 1:
						game[i]->SetToolTip(titleTT[i], 0, -244/4, ALIGN_CENTRE, ALIGN_MIDDLE);
						break;
					case 2:
						game[i]->SetToolTip(titleTT[i], -122/4, -244/4, ALIGN_RIGHT, ALIGN_MIDDLE);
						break;
					default:
						break;
				}		
			}
		}

	}
	if(updateCB)
		updateCB(this);
}


