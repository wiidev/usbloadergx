/****************************************************************************
 * libwiigui
 *
 * gui_gamecarousel.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"

#include <unistd.h>
#include "gui_gamecarousel.h"
#include "../cfg.h"

#include <string.h>
#include <sstream>

#define GAMESELECTSIZE      30
extern const int vol;

/**
 * Constructor for the GuiGameCarousel class.
 */
GuiGameCarousel::GuiGameCarousel(int w, int h, struct discHdr * l, int gameCnt, const char *themePath, const u8 *imagebg, int selected, int offset)
{
	width = w;
	height = h;
	this->gameCnt = gameCnt;
	gameList = l;
	pagesize = 7;
	changed = 0;
	selectable = true;
	listOffset = (offset == 0) ? this->FindMenuItem(-1, 1) : offset;
	selectedItem = selected - offset;
	focus = 1;					 // allow focus
	firstPic = 0;
	speed = 50000;
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
	for(int i=0; i < pagesize; i++) {

		struct discHdr *header = &gameList[i];
		snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
		snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

		snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
								 //Load full id image
		cover[i] = new GuiImageData(imgPath,0);
		if (!cover[i]->GetImage()) {
			delete cover[i];
			snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
								 //Load short id image
			cover[i] = new GuiImageData(imgPath, 0);
			if (!cover[i]->GetImage()) {
				delete cover[i];
				snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
								 //Load no image
				cover[i] = new GuiImageData(imgPath, nocover_png);
			}
		}

		coverImg[i] = new GuiImage(cover[i]);
		coverImg[i]->SetScale(0.8);
		coverImg[i]->SetWidescreen(CFG.widescreen);
		coverImg[i]->SetAngle(-30+(i*10));
		coverImg[i]->SetPosition(-20,0);
		game[i] = new GuiButton(122,244);
		game[i]->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
		game[i]->SetImage(coverImg[i]);
		if (i==0)game[i]->SetPosition(-290,20);
		if (i==1)game[i]->SetPosition(-200,-45);
		if (i==2)game[i]->SetPosition(-100,-85);
		if (i==3)game[i]->SetPosition(0,-100);
		if (i==4)game[i]->SetPosition(100,-80);
		if (i==5)game[i]->SetPosition(200,-40);
		if (i==6)game[i]->SetPosition(290,20);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetEffectGrow();
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
	delete btnSoundClick;

	for(int i=0; i<pagesize; i++) {
		delete game[i];
		delete coverImg[i];
		delete cover[i];
	}
	delete [] gameIndex;
	delete [] game;
}


void GuiGameCarousel::SetFocus(int f)
{
	LOCK(this);
	focus = f;

	for(int i=0; i<pagesize; i++)
		game[i]->ResetState();

	if(f == 1)
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
	return changed;
}


int GuiGameCarousel::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++) {
		if(game[i]->GetState() == STATE_CLICKED) {
			game[i]->SetState(STATE_SELECTED);
			found = changed+i;
			break;
		}
	}
	return found;
}


int GuiGameCarousel::GetSelectedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++) {
		if(game[i]->GetState() == STATE_SELECTED) {
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

int GuiGameCarousel::FindMenuItem(int currentItem, int direction)
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
void GuiGameCarousel::Draw()
{
	LOCK(this);
	if(!this->IsVisible())
		return;

	int next = listOffset;

	for(int i=0; i<pagesize; i++) {
		if(next >= 0) {
			game[i]->Draw();
			next = this->FindMenuItem(next, 1);
		} else break;
	}

	btnRight->Draw();
	btnLeft->Draw();
	this->UpdateEffects();
}


void GuiGameCarousel::Update(GuiTrigger * t)
{
	LOCK(this);
	if(state == STATE_DISABLED || !t)
		return;

	btnRight->Update(t);
	btnLeft->Update(t);

	int next = listOffset;

	char ID[4];
	char IDfull[7];
	char imgPath[100];

	for(int i=0; i<pagesize; i++) {
		if(next >= 0) {
			if(game[i]->GetState() == STATE_DISABLED) {
				game[i]->SetVisible(true);
				game[i]->SetState(STATE_DEFAULT);
			}
			gameIndex[i] = next;
			next = this->FindMenuItem(next, 1);
		} else {
			game[i]->SetVisible(false);
			game[i]->SetState(STATE_DISABLED);
		}

		if(focus) {
			if(i != selectedItem && game[i]->GetState() == STATE_SELECTED)
				game[i]->ResetState();
			else if(i == selectedItem && game[i]->GetState() == STATE_DEFAULT);
				game[selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
		game[i]->Update(t);

		if(game[i]->GetState() == STATE_SELECTED) {
			selectedItem = i;
		}
	}

	// pad/joystick navigation
	if(!focus)
		return;					 // skip navigation

	if ((t->Right()  || btnRight->GetState() == STATE_CLICKED)) {

		if (firstPic<0)
			firstPic=6;
		changed++;
		if (changed > (gameCnt-1))
			changed=0;
		int bob[7];
		for(int i=0; i<7; i++) {
			bob[i] = (firstPic+i < 7) ? firstPic+i : firstPic+i-7;
		}

		for(int i=0; i<20; i++) {

			game[bob[1]]->SetPosition((-200-(4.5*i)),(-45+(3.25*i)));
			coverImg[bob[1]]->SetAngle(-20-(i/2));

			game[bob[2]]->SetPosition((-100-(5*i)),(-85+(2*i)));
			coverImg[bob[2]]->SetAngle(-10-(i/2));

			game[bob[3]]->SetPosition((0-(5*i)),(-100+(0.75*i)));
			coverImg[bob[3]]->SetAngle(0-(i/2));

			game[bob[4]]->SetPosition((100-(5*i)),(-80-(1*i)));
			coverImg[bob[4]]->SetAngle(10-(i/2));

			game[bob[5]]->SetPosition((200-(4.5*i)),(-40-(2*i)));
			coverImg[bob[5]]->SetAngle(20-(i/2));

			game[bob[6]]->SetPosition((290-(4.5*i)),(20-(3*i)));
			coverImg[bob[6]]->SetAngle(30-(i/2));

			if (i==1) {
				struct discHdr *header = &gameList[changed];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				delete cover[bob[0]];
				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull); //Load full id image
				cover[bob[0]] = new GuiImageData(imgPath,0);
				if (!cover[bob[0]]->GetImage()) {
					delete cover[bob[0]];
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID); //Load short id image
					cover[bob[0]] = new GuiImageData(imgPath, 0);
					if (!cover[bob[0]]->GetImage()) {
						delete cover[bob[0]];
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path); //Load no image
						cover[bob[0]] = new GuiImageData(imgPath, nocover_png);
					}
				}
				delete coverImg[bob[0]];
				coverImg[bob[0]] = new GuiImage(cover[bob[0]]);
				coverImg[bob[0]]->SetScale(0.8);
				coverImg[bob[0]]->SetWidescreen(CFG.widescreen);
				game[bob[0]]->SetImage(coverImg[bob[0]]);
			}

			game[bob[0]]->SetPosition((380-(4.5*i)),(80-(3*i)));
			coverImg[bob[0]]->SetAngle(40-(i/2));

			usleep(speed/25);
			for(int j=0; j<pagesize; j++) {
				game[bob[6-j]]->Draw();
			}
		}
		firstPic++;
		btnRight->ResetState();
	}

	else if((t->Left()  || btnLeft->GetState() == STATE_CLICKED)) {

		if (firstPic<0)
			firstPic=6;
		changed--;
		if (changed<0)
			changed=(gameCnt-1);
		int bob[7];
		for(int i=0; i<7; i++) {
			bob[i] = (firstPic+i < 7) ? firstPic+i : firstPic+i-7;
		}

		for(int i=0; i<20; i++) {

			game[bob[0]]->SetPosition((-290+(4.5*i)),(20-(3.25*i)));
			coverImg[bob[0]]->SetAngle(-30+(i/2));

			game[bob[1]]->SetPosition((-200+(5*i)),(-45-(2*i)));
			coverImg[bob[1]]->SetAngle(-20+(i/2));

			game[bob[2]]->SetPosition((-100+(5*i)),(-85-(.75*i)));
			coverImg[bob[2]]->SetAngle(-10+(i/2));

			game[bob[3]]->SetPosition((0+(5*i)),(-100+(1*i)));
			coverImg[bob[3]]->SetAngle(0+(i/2));

			game[bob[4]]->SetPosition((100+(5*i)),(-80+(2*i)));
			coverImg[bob[4]]->SetAngle(10+(i/2));

			game[bob[5]]->SetPosition((200+(4.5*i)),(-40+(3*i)));
			coverImg[bob[5]]->SetAngle(20+(i/2));

			if (i==1) {
				struct discHdr *header = &gameList[changed];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				delete cover[bob[6]];
				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull); //Load full id image
				cover[bob[6]] = new GuiImageData(imgPath,0);
				if (!cover[bob[6]]->GetImage()) {
					delete cover[bob[6]];
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID); //Load short id image
					cover[bob[6]] = new GuiImageData(imgPath, 0);
					if (!cover[bob[6]]->GetImage()) {
						delete cover[bob[6]];
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path); //Load no image
						cover[bob[6]] = new GuiImageData(imgPath, nocover_png);
					}
				}
				delete coverImg[bob[6]];
				coverImg[bob[6]] = new GuiImage(cover[bob[6]]);
				coverImg[bob[6]]->SetScale(0.8);
				coverImg[bob[6]]->SetWidescreen(CFG.widescreen);
				game[bob[6]]->SetImage(coverImg[bob[6]]);
			}

			game[bob[6]]->SetPosition((-380+(4.5*i)),(80-(3*i)));
			coverImg[bob[6]]->SetAngle(-40+(i/2));

			usleep(speed/25);
			for(int j=0; j<pagesize; j++) {
				game[bob[6-j]]->Draw();
			}
		}
		firstPic--;
		btnLeft->ResetState();
	}

	if(updateCB)
		updateCB(this);
}


void GuiGameCarousel::Reload(struct discHdr * l, int count)
{
	LOCK(this);
	gameList = l;
	gameCnt = count;
	changed=0;
	selectedItem = 0;
	listOffset = 0;
	firstPic = 0;
}
