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

#define GAMESELECTSIZE      30
extern const int vol;
//int txtscroll = 0;
int changed = 0;
int tooMuch;
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
	scrollbaron = (gameCnt > pagesize) ? 1 : 0;
	selectable = true;
	listOffset = (offset == 0) ? this->FindMenuItem(-1, 1) : offset;
	selectedItem = selected - offset;
	focus = 1; // allow focus
	char imgPath[100];
	tooMuch=(gameCnt-(gameCnt%8));

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

	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM, vol);
	
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	imgLeft = new GuiImageData(imgPath, startgame_arrow_left_png);
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	imgRight = new GuiImageData(imgPath, startgame_arrow_right_png);
	
	btnLeftImg = new GuiImage(imgLeft);
	btnLeft = new GuiButton(imgLeft->GetWidth(), imgLeft->GetHeight());
	//GuiImage btnLeftImg(&imgLeft);
	//GuiButton btnLeft(imgLeft.GetWidth(), imgLeft.GetHeight());
	btnLeft->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	btnLeft->SetPosition(20, -30);
	btnLeft->SetParent(this);
	btnLeft->SetImage(btnLeftImg);
	btnLeft->SetSoundOver(btnSoundOver);
	btnLeft->SetTrigger(trigA);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);
	btnLeft->SetEffectGrow();

	btnRightImg = new GuiImage(imgRight);
	btnRight = new GuiButton(imgRight->GetWidth(), imgRight->GetHeight());
	//GuiButton btnRight(imgRight.GetWidth(), imgRight.GetHeight());
	btnRight->SetParent(this);
	btnRight->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	btnRight->SetPosition(-20, -30);
	btnRight->SetImage(btnRightImg);
	btnRight->SetSoundOver(btnSoundOver);
	btnRight->SetTrigger(trigA);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);
	btnRight->SetEffectGrow();

	gameIndex = new int[pagesize];
	game = new GuiButton * [pagesize];
	//gameTxt = new GuiText * [pagesize];
	//gameBg = new GuiImage * [pagesize];
	coverImg = new GuiImage * [pagesize];
	cover = new GuiImageData * [pagesize];

	//char buffer[THEME.maxcharacters + 4];
	char ID[4];
	char IDfull[7];
	
	for(int i=0; i < pagesize; i++)
	{
		/*if (strlen(get_title(&gameList[i])) < (u32)(THEME.maxcharacters + 3))
		{
			sprintf(buffer, "%s", get_title(&gameList[i]));
		}
		else
		{
			sprintf(buffer, get_title(&gameList[i]),  THEME.maxcharacters);
			buffer[THEME.maxcharacters] = '\0';
			strncat(buffer, "...", 3);
		}
		*/////////////////////////////////////////////////
		
				struct discHdr *header = &gameList[i];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				//w.Remove(&DownloadBtn);

				//load game cover
				//if (cover)
				//{
				//	delete cover;
				//	cover = NULL;
				//}

				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
				cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				//if (coverImg)
				//{
				//	delete coverImg;
				//	coverImg = NULL;
				//}
				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);

				//DownloadBtn.SetImage(coverImg);// put the new image on the download button
				//w.Append(&DownloadBtn);
		/////////////////////////////////////////////////

		//gameTxt[i] = new GuiText(buffer, 20, (GXColor){THEME.gameText_r, THEME.gameText_g, THEME.gameText_b, 0xff});
		//gameTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		//gameTxt[i]->SetPosition(24,0);

		//gameBg[i] = new GuiImage(bgGamesEntry);

		game[i] = new GuiButton(coverImg[i]->GetWidth()*.45,coverImg[i]->GetHeight()*.7);
		game[i]->SetParent(this);
		game[i]->SetAlignment(ALIGN_TOP,ALIGN_LEFT);
		//game[i]->SetLabel(gameTxt[i]);
		//game[i]->SetImageOver(gameBg[i]);
		game[i]->SetImage(coverImg[i]);
		coverImg[i]->SetParent(game[i]);
		//coverImg[i]->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
		coverImg[i]->SetPosition(-10,-35);
		//game[i]->SetPosition(5,GAMESELECTSIZE*i+4);
		if (i<4)game[i]->SetPosition(117+i*110,25);
		if (i>3)game[i]->SetPosition(117+(i-4)*110,185);
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetEffectGrow();
		coverImg[i]->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	}
}

/**
 * Destructor for the GuiGameGrid class.
 */
//GuiGameGrid::~GuiGameGrid()
GuiGameGrid::~GuiGameGrid()
{
  
	delete imgRight;
	delete imgLeft;
	delete btnLeftImg;
	delete btnRightImg;
	delete btnRight;
	delete btnLeft;

	delete trigA;
	delete btnSoundClick;

	for(int i=0; i<pagesize; i++)
	{
		
		delete game[i];
		delete coverImg[i];
		delete cover[i];
	}
	delete [] gameIndex;
	delete [] game;
	//delete [] gameTxt;
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

	//bgGameImg->Draw();
	

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
    //static int position2;
	u8 over=0;
	//if (changed>gameCnt){changed=tooMuch;over=1;} 
	
	//int extra=(gameCnt-changed-1);
	
	
	// scrolldelay affects how fast the list scrolls
	// when the arrows are clicked
	//float scrolldelay = 3.5;

   
	btnRight->Update(t);
	btnLeft->Update(t);

	next = listOffset;
	//char buffer[THEME.maxcharacters + 4];
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
			

			/*if (strlen(get_title(&gameList[next])) < (u32)(THEME.maxcharacters + 3))
			{
				sprintf(buffer, "%s", get_title(&gameList[next]));
			}
			else
			{
				sprintf(buffer, get_title(&gameList[next]), THEME.maxcharacters);
				buffer[THEME.maxcharacters] = '\0';
				strncat(buffer, "...", 3);
			}

			gameTxt[i]->SetText(buffer);
			gameTxt[i]->SetPosition(24, 0);*/

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

    if (scrollbaron == 1) {

	if ((t->Right()  ||
	btnRight->GetState() == STATE_CLICKED) )
	//&&(changed<(tooMuch+1)))
	{
		
		////////////////////////////////////////////
		changed += pagesize;
		if (changed>gameCnt)changed=0;
		
		
		for(int i=0; i<pagesize; i++)
	{	
		
		
		//if (changed<gameCnt){
		//if (i<tooMuch){
		if(coverImg[i])delete coverImg[i];
		if(cover[i])delete cover[i];
			struct discHdr *header = &gameList[i+changed];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				
				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
				cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);
				coverImg[i]->SetParent(game[i]);
		//coverImg[i]->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
		coverImg[i]->SetPosition(-10,-35);
				game[i]->SetImage(coverImg[i]);
				coverImg[i]->SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 65);
				//if ((extra<8)&&(i>extra)){
				//if ((changed>gameCnt)){//&& (i>((changed-gameCnt)-1))){
				if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1)))
					coverImg[i]->SetVisible(false);//}
					//delete coverImg[i];}
					
				//}
				//else {	delete game[i];
					//	delete coverImg[i];
						//delete cover[i];}
				//}
		//usleep(500000);
				
				}

				//game[i] = new GuiButton(coverImg[i]->GetWidth(),coverImg[i]->GetHeight());
		//game[i]->SetParent(this);
		//game[i]->SetImage(coverImg[i]);
		//game[i]->SetPosition(5,GAMESELECTSIZE*i+4);
		//game[i]->SetRumble(false);
		//game[i]->SetTrigger(trigA);
		//game[i]->SetSoundClick(btnSoundClick);
	////////////////////////////////////////////////////
		
		
		
		WPAD_ScanPads();
        u8 cnt, buttons = NULL;
        /* Get pressed buttons */
        for (cnt = 0; cnt < 4; cnt++)
            buttons |= WPAD_ButtonsHeld(cnt);
        if (buttons == WPAD_BUTTON_A) {

        } else {
            btnRight->ResetState();

        }

	}
	else if((t->Left()  ||
	btnLeft->GetState() == STATE_CLICKED))
	//&& (changed>7)) ////////////////////////////////////////////up
	//arrowUpBtn->GetState() == STATE_HELD)
	{changed -= pagesize;if (changed<0)changed=tooMuch+pagesize;
		over=0;
	//if (changed<9)changed=0;
	
	
	
		
		for(int i=0; i<pagesize; i++)
	{	
		coverImg[i]->SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 65);

		//usleep(500000);
		
		//if (changed<gameCnt){
		if(coverImg[i])delete coverImg[i];
		if(cover[i])delete cover[i];
		
			struct discHdr *header = &gameList[i+changed];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				
				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
				cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);
				coverImg[i]->SetParent(game[i]);
		//coverImg[i]->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
		coverImg[i]->SetPosition(-10,-35);
		game[i]->ResetState();
			game[i]->SetVisible(true);
				game[i]->SetImage(coverImg[i]);
				coverImg[i]->SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 65);//}
				if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1)))
					coverImg[i]->SetVisible(false);
		//usleep(500000);
				
				}
		//prev = this->FindMenuItem(gameIndex[selectedItem], -1);

		//if(prev >= 0)
		

		WPAD_ScanPads();
        u8 cnt, buttons = NULL;
        /* Get pressed buttons */
        for (cnt = 0; cnt < 4; cnt++)
            buttons |= WPAD_ButtonsHeld(cnt);
        if (buttons == WPAD_BUTTON_A) {

        } else {
            btnLeft->ResetState();

        }
	}

   

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
	tooMuch=(gameCnt-(gameCnt%12));
	scrollbaron = (gameCnt > pagesize) ? 1 : 0;
	selectedItem = 0;
	listOffset = 0;
	char ID[4];
	char IDfull[7];
	char imgPath[100];

	for(int i=0; i<pagesize; i++)
		{	
		coverImg[i]->SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);

		//usleep(500000);
		
		//if (changed<gameCnt){
		if(coverImg[i])delete coverImg[i];
		if(cover[i])delete cover[i];
		
			struct discHdr *header = &gameList[i+changed];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				
				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
				cover[i] = new GuiImageData(imgPath,0); //load short id
				if (!cover[i]->GetImage()) //if could not load the short id image
				{
					delete cover[i];
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
					cover[i] = new GuiImageData(imgPath, 0); //load full id image
					if (!cover[i]->GetImage())
					{
						delete cover[i];
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover[i] = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				coverImg[i] = new GuiImage(cover[i]);
				coverImg[i]->SetWidescreen(CFG.widescreen);
				coverImg[i]->SetScale(0.6);
				coverImg[i]->SetParent(game[i]);
		//coverImg[i]->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
		coverImg[i]->SetPosition(-10,-35);
		game[i]->ResetState();
			game[i]->SetVisible(true);
				game[i]->SetImage(coverImg[i]);
				coverImg[i]->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);//}
				if (((changed+pagesize)>gameCnt)&&(i>((gameCnt-changed)-1)))
					coverImg[i]->SetVisible(false);
		//usleep(500000);
				
				game[i]->ResetState();}
}
