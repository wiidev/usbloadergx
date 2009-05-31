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
#include "gui_gamegrid.h"
#include "../cfg.h"

#include <string.h>
#include <sstream>


extern const int vol;

/**
 * Constructor for the GuiGameGrid class.
 */
GuiGameGrid::GuiGameGrid(int w, int h, struct discHdr * l, int gameCnt, const char *themePath, const u8 *imagebg, int selected, int offset)
{
	width = 640;
	height = h;
	this->gameCnt = gameCnt;
	gameList = l;
	pagesize = 8;
	changed = 0;
	scrollbaron = (gameCnt > pagesize) ? 1 : 0;
	selectable = true;
	listOffset = (offset == 0) ? this->FindMenuItem(-1, 1) : offset;
	selectedItem = selected - offset;
	focus = 1; // allow focus
	char imgPath[100];

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
	trigL = new GuiTrigger;
	trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR = new GuiTrigger;
	trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
    trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
	trigMinus = new GuiTrigger;
	trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	btnSoundClick = new GuiSound(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);
	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);

	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	imgLeft = new GuiImageData(imgPath, startgame_arrow_left_png);
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	imgRight = new GuiImageData(imgPath, startgame_arrow_right_png);

	btnLeftImg = new GuiImage(imgLeft);
	btnLeft = new GuiButton(imgLeft->GetWidth(), imgLeft->GetHeight());
	btnLeft->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	btnLeft->SetPosition(20, -30);
	btnLeft->SetParent(this);
	btnLeft->SetImage(btnLeftImg);
	btnLeft->SetSoundOver(btnSoundOver);
	btnLeft->SetSoundClick(btnSoundClick);
	btnLeft->SetTrigger(trigA);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);
	btnLeft->SetEffectGrow();

	btnRightImg = new GuiImage(imgRight);
	btnRight = new GuiButton(imgRight->GetWidth(), imgRight->GetHeight());
	btnRight->SetParent(this);
	btnRight->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	btnRight->SetPosition(-20, -30);
	btnRight->SetImage(btnRightImg);
	btnRight->SetSoundOver(btnSoundOver);
	btnRight->SetSoundClick(btnSoundClick);
	btnRight->SetTrigger(trigA);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);
	btnRight->SetEffectGrow();

	gameIndex = new int[pagesize];
	game = new GuiButton * [pagesize];
	coverImg = new GuiImage * [pagesize];
	cover = new GuiImageData * [pagesize];

	char ID[4];
	char IDfull[7];

	for(int i=0; i < pagesize; i++)
	{

				struct discHdr *header = &gameList[i];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
				cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					cover[i] = NULL;
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						cover[i] = NULL;
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);

		game[i] = new GuiButton(coverImg[i]->GetWidth()*.45,coverImg[i]->GetHeight()*.7);
		game[i]->SetParent(this);
		game[i]->SetAlignment(ALIGN_TOP,ALIGN_LEFT);
		game[i]->SetImage(coverImg[i]);
		coverImg[i]->SetParent(game[i]);
		coverImg[i]->SetPosition(-10,-35);
		if (i<4)game[i]->SetPosition(117+i*110,25);
		if (i>3)game[i]->SetPosition(117+(i-4)*110,185);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundOver(btnSoundOver);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetEffectGrow();
		game[i]->SetVisible(true);
		game[i]->SetClickable(true);
        coverImg[i]->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
        if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1))) {
            game[i]->SetVisible(false);
            game[i]->SetClickable(false);
			game[i]->RemoveSoundOver();
        }
	}
}

/**
 * Destructor for the GuiGameGrid class.
 */
GuiGameGrid::~GuiGameGrid()
{

	delete imgRight;
	delete imgLeft;
	delete btnLeftImg;
	delete btnRightImg;
	delete btnRight;
	delete btnLeft;

	delete trigA;
	delete trigHeldA;
	delete trigL;
	delete trigR;
	delete trigPlus;
	delete trigMinus;
	delete btnSoundClick;
	delete btnSoundOver;

	for(int i=0; i<pagesize; i++)
	{

		delete game[i];
		delete coverImg[i];
		delete cover[i];
	}
	delete [] gameIndex;
	delete [] game;
	delete [] cover;
	delete [] coverImg;
}

void GuiGameGrid::SetFocus(int f)
{
	LOCK(this);
	focus = f;

	for(int i=0; i<pagesize; i++)
		game[i]->ResetState();

	if(f == 1)
		game[selectedItem]->SetState(STATE_SELECTED);
}

void GuiGameGrid::ResetState()
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

int GuiGameGrid::GetOffset()
{
	return changed;
}
int GuiGameGrid::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++)
	{
		if(game[i]->GetState() == STATE_CLICKED)
		{
			game[i]->SetState(STATE_SELECTED);
			found = changed+i;
			break;
		}
	}
	return found;
}

int GuiGameGrid::GetSelectedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++)
	{
		if(game[i]->GetState() == STATE_SELECTED)
		{
			game[i]->SetState(STATE_SELECTED);
			found = changed+i;
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

int GuiGameGrid::FindMenuItem(int currentItem, int direction)
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
void GuiGameGrid::Draw()
{
	LOCK(this);
	if(!this->IsVisible())
		return;

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

   	btnRight->Draw();
	btnLeft->Draw();
	this->UpdateEffects();
}

void GuiGameGrid::Update(GuiTrigger * t)
{
	LOCK(this);
	if(state == STATE_DISABLED || !t)
		return;

	int next; //prev;

	btnRight->Update(t);
	btnLeft->Update(t);

	next = listOffset;

	char ID[4];
	char IDfull[7];
	char imgPath[100];

	for(int i=0; i<pagesize; i++)
	{
		if(next >= 0)
		{
			if(game[i]->GetState() == STATE_DISABLED)
			{
				game[i]->SetVisible(true);
				game[i]->SetState(STATE_DEFAULT);
			}
			gameIndex[i] = next;
			next = this->FindMenuItem(next, 1);
		}
		else
		{
			game[i]->SetVisible(false);
			game[i]->SetState(STATE_DISABLED);
		}

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

	// pad/joystick navigation
	if(!focus)
		return; // skip navigation

	if ((t->Right()  || btnRight->GetState() == STATE_CLICKED)) {

		changed += pagesize;
		if (changed>gameCnt-1)
		changed=0;

		for(int i=0; i<pagesize; i++) {

		if(coverImg[i]) {
        delete coverImg[i];
        coverImg[i] = NULL;
		}
		if(cover[i]) {
        delete cover[i];
        cover[i] = NULL;
		}
            struct discHdr *header = &gameList[i+changed];
            snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
            snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

            snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
            cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					cover[i] = NULL;
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						cover[i] = NULL;
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);
				coverImg[i]->SetParent(game[i]);
                coverImg[i]->SetPosition(-10,-35);
				game[i]->SetImage(coverImg[i]);
				game[i]->SetVisible(true);
				game[i]->SetClickable(true);
				game[i]->SetSoundOver(btnSoundOver);
				coverImg[i]->SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 65);
				if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1))) {
					game[i]->SetVisible(false);
					game[i]->SetClickable(false);
					game[i]->RemoveSoundOver();
				}
        }
    btnRight->ResetState();

	} else if((t->Left()  || btnLeft->GetState() == STATE_CLICKED)){

	    changed -= pagesize;
	    if (changed < 0)
	    changed=gameCnt-(gameCnt%8);

		for(int i=0; i<pagesize; i++) {
		coverImg[i]->SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 65);

		if(coverImg[i]) {
        delete coverImg[i];
        coverImg[i] = NULL;
		}
		if(cover[i]) {
        delete cover[i];
        cover[i] = NULL;
		}

			struct discHdr *header = &gameList[i+changed];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
				cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					cover[i] = NULL;
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						cover[i] = NULL;
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);
				coverImg[i]->SetParent(game[i]);
                coverImg[i]->SetPosition(-10,-35);
                game[i]->ResetState();
                game[i]->SetVisible(true);
				game[i]->SetImage(coverImg[i]);
				game[i]->SetSoundOver(btnSoundOver);
				game[i]->SetClickable(true);
				coverImg[i]->SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 65);
				if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1))) {
					game[i]->SetVisible(false);
					game[i]->SetClickable(false);
					game[i]->RemoveSoundOver();
				}
		}
            btnLeft->ResetState();
	}

	if(updateCB)
		updateCB(this);
}

void GuiGameGrid::Reload(struct discHdr * l, int count)
{
	LOCK(this);
	gameList = l;
	gameCnt = count;
	changed=0;
	scrollbaron = (gameCnt > pagesize) ? 1 : 0;
	selectedItem = 0;
	listOffset = 0;
	char ID[4];
	char IDfull[7];
	char imgPath[100];

	for(int i=0; i<pagesize; i++) {

		if(coverImg[i]) {
        delete coverImg[i];
        coverImg[i] = NULL;
		}
		if(cover[i]) {
        delete cover[i];
        cover[i] = NULL;
		}

			struct discHdr *header = &gameList[i+changed];
            snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
            snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

            snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
            cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					cover[i] = NULL;
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						cover[i] = NULL;
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);
				coverImg[i]->SetParent(game[i]);
                coverImg[i]->SetPosition(-10,-35);
                game[i]->ResetState();
                game[i]->SetVisible(true);
				game[i]->SetImage(coverImg[i]);
				game[i]->SetClickable(true);
				coverImg[i]->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
				if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1))) {
					game[i]->SetVisible(false);
					game[i]->SetClickable(false);
				}
    }
}
