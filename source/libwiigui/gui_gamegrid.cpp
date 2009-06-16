/****************************************************************************
 * libwiigui
 *
 * gui_gameGrid.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"

#include <unistd.h>
#include "gui_gamegrid.h"
#include "../settings/cfg.h"
#include "../prompts/PromptWindows.h"
#include "../language/language.h"
#include "../menu.h"

#include <string.h>
#include <math.h>
#include <sstream>

#define SCALE		0.8f
#define DEG_OFFSET	7
#define RADIUS		780
#define IN_SPEED	175
#define SHIFT_SPEED	100
#define SPEED_STEP	4
#define SAFETY		320

#include "../main.h"




extern const int vol;
int mover=0, mover2=0; 
u8 goback=0;
int goLeft = 0, goRight=0;
char debugbuffer[100];
int c;
int selectedOld=0;
int wait=0,wait1=0;
bool isover=false;

/**
 * Constructor for the GuiGamegrid class.
 */
GuiGameGrid::GuiGameGrid(int w, int h, struct discHdr * l, int count, const char *themePath, const u8 *imagebg, int selected, int offset)
{
	width = w;
	height = h;
	gameCnt = (count < SAFETY) ? count : SAFETY;
	gameList = l;
	c=count;
	listOffset = (offset == 0) ? this->FindMenuItem(-1, 1) : offset;
	selectable = true;
	selectedItem = selected - offset;
	focus = 1;					 // allow focus
	firstPic = 0;
	clickedItem = -1;
	speed = SHIFT_SPEED;
	char imgPath[100];
	rows =3;
	
	
	
	if ((count<42)&&(rows==3))rows=2;
	if ((count<16)&&(rows==2))rows=1;
	if (gameCnt<6)gameCnt=6;
	
	if (rows==1)pagesize = 6;
	else if (rows==2)pagesize = 16;
	else if (rows==3)pagesize = 42;
	
	//if (realCnt<pagesize)listOffset=5;//pagesize-(pagesize-realCnt)/2+1;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigL = new GuiTrigger;
	trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR = new GuiTrigger;
	trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	trig1 = new GuiTrigger;
	trig1->SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_X, PAD_BUTTON_X);
	trig2 = new GuiTrigger;
	trig2->SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_Y, PAD_BUTTON_Y);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
	trigMinus = new GuiTrigger;
	trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	btnSoundClick = new GuiSound(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);
	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);

	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	imgLeft = new GuiImageData(imgPath, startgame_arrow_left_png);
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	imgRight = new GuiImageData(imgPath, startgame_arrow_right_png);

	int btnHeight = (int) lround(sqrt(RADIUS*RADIUS - 90000)-RADIUS-50);

	btnLeftImg = new GuiImage(imgLeft);
	btnLeft = new GuiButton(0,0);
	btnLeft->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	btnLeft->SetPosition(20, btnHeight);
	btnLeft->SetParent(this);
	//btnLeft->SetImage(btnLeftImg);
	btnLeft->SetSoundOver(btnSoundOver);
	btnLeft->SetTrigger(trigA);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);
	btnLeft->SetEffectGrow();
	
	/*debugTxt = new GuiText("fag", 14, (GXColor){0,0,0, 255});
	debugTxt->SetParent(this);
	debugTxt->SetAlignment(2,5); 
	debugTxt->SetPosition(0,180);*/  
	

	btnRightImg = new GuiImage(imgRight);
	btnRight = new GuiButton(0,0);
	btnRight->SetParent(this);
	btnRight->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	btnRight->SetPosition(-20, btnHeight);
	//btnRight->SetImage(btnRightImg);
	btnRight->SetSoundOver(btnSoundOver);
	btnRight->SetTrigger(trigA);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);
	btnRight->SetEffectGrow();
	
	btnRowUp = new GuiButton(0,0);
	btnRowUp->SetParent(this);
	btnRowUp->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btnRowUp->SetPosition(0,0);
	btnRowUp->SetTrigger(trig2);
	
	btnRowDown = new GuiButton(0,0);
	btnRowDown->SetParent(this);
	btnRowDown->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btnRowDown->SetPosition(0,0);
	btnRowDown->SetTrigger(trig1);
	
	
//	titleTxt = new GuiText("test");
	
	titleTT = new GuiTooltip("test");
	titleTT->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTT->SetPosition(-100,0);
	titleTT->SetAlpha(175);
	
//        if (Settings.wsprompt == yes)
//                installBtnTT.SetWidescreen(CFG.widescreen);
	
	
	//if (count>0){

	gameIndex = new int[pagesize];
	game = new GuiButton * [pagesize];
	bob = new int[pagesize];
	coverImg = new GuiImage * [gameCnt];
	cover = new GuiImageData * [gameCnt];
	//titleTxt = new GuiText * [gameCnt];

	for(int i=0; i<pagesize; i++) {
		bob[i]=i;
	}

	char ID[4];
	char IDfull[7];
	int n = gameCnt>pagesize?gameCnt:pagesize;
	//for(int i=0; i < gameCnt; i++) {
	for(int i=0; i < n; i++) {

		struct discHdr *header = &gameList[i];
		snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
		snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

		snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, IDfull); //Load full id image
		cover[i] = new GuiImageData(imgPath,0);
		if (!cover[i]->GetImage()) {
			delete cover[i];
			snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID); //Load short id image
			cover[i] = new GuiImageData(imgPath, 0);
			if (!cover[i]->GetImage()) {
				delete cover[i];
				snprintf(imgPath, sizeof(imgPath), "%snoimage.png", Settings.covers_path); //Load no image
				cover[i] = new GuiImageData(imgPath, nocoverFlat_png);
			}
		}

		coverImg[i] = new GuiImage(cover[i]);
		coverImg[i]->SetWidescreen(CFG.widescreen);
		if (rows==2)coverImg[i]->SetScale(.6);//these are the numbers for 2 rows
		else if (rows==3)coverImg[i]->SetScale(.26);//these are the numbers for 3 rows
		
		//titleTxt[i] = new GuiText(get_title(&gameList[i]), 20, (GXColor){0,0,0, 0xff});
		
		
	}
	
	for(int i=0; i < pagesize; i++) {
		game[i] = new GuiButton(160,224);//for 1 row
		if (rows==2)game[i]->SetSize(75,133);//these are the numbers for 2 rows
		else if (rows==3)game[i]->SetSize(35,68);//these are the numbers for 3 rows
		game[i]->SetParent(this);
		game[i]->SetAlignment(ALIGN_TOP,ALIGN_LEFT);
		game[i]->SetPosition(-200,740);
		game[i]->SetImage(coverImg[((listOffset+i) % gameCnt)]);
		if (rows==3)coverImg[(listOffset+i) % gameCnt]->SetPosition(0,-80);// only for 3 rows
		if (rows==2)coverImg[(listOffset+i) % gameCnt]->SetPosition(0,-50);// only for 2 rows
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetClickable(true);
		game[i]->SetVisible(true);
		coverImg[i]->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
      
	}
	for (int i=gameCnt-1;i<pagesize;i++){ //hide games if gameCnt is less than the number of onscreen boxes 
            game[i]->SetVisible(false);
            game[i]->SetClickable(false);
			game[i]->RemoveSoundOver();}
		
	if(CFG.widescreen){
		
		if (rows==1)
		{
		game[0]->SetPosition(-70,74);
		game[1]->SetPosition(88,74);
		game[2]->SetPosition(239,74);
		game[3]->SetPosition(390,74);
		game[4]->SetPosition(550,74);
		
		game[0]->SetSkew(-10,-44,10,-26,10,26,-10,44);
		game[1]->SetSkew(-6,-22,6,-14,6,14,-6,22);
		game[2]->SetSkew(0,-11,0,-11,0,11,0,11);
		game[3]->SetSkew(-6,-14,6,-22,6,22,-6,14);
		game[4]->SetSkew(-10,-26,10,-44,10,44,-10,26);
		}
		else if (rows ==2)
		{
		game[0]->SetPosition(3,50);
		game[1]->SetPosition(3,193);
		game[2]->SetPosition(97,50);
		game[3]->SetPosition(97,193);
		game[4]->SetPosition(187,50);
		game[5]->SetPosition(187,193);
		game[6]->SetPosition(272,50);
		game[7]->SetPosition(272,193);
		game[8]->SetPosition(358,50);
		game[9]->SetPosition(358,193);
		game[10]->SetPosition(449,50);
		game[11]->SetPosition(449,193);
		game[12]->SetPosition(545,50);
		game[13]->SetPosition(545,193);
		game[14]->SetPosition(700,0);
		game[15]->SetPosition(700,0);
		
		
		game[0]->SetSkew(-4.5,-49,4.5,-27,4.5,0,-4.5,0);
		game[1]->SetSkew(-4.5,0,4.5,0,4.5,27,-4.5,49);
		game[2]->SetSkew(-4,-22,4,-14,4,0,-4,0);
		game[3]->SetSkew(-4,0,4,0,4,14,-4,22);
		game[4]->SetSkew(0,-9,0,-5,0,0,0,0);
		game[5]->SetSkew(0,0,0,0,0,5,0,9);
		game[6]->SetSkew(0,0,0,0,0,0,0,0);
		game[7]->SetSkew(0,0,0,0,0,0,0,0);
		game[8]->SetSkew(0,-5,0,-9,0,0,0,0);
		game[9]->SetSkew(0,0,0,0,0,9,0,5);
		game[10]->SetSkew(-4,-14,4,-22,4,0,-4,0);
		game[11]->SetSkew(-4,0,4,0,4,22,-4,14);
		game[12]->SetSkew(-4.5,-27,4.5,-49,4.5,0,-4.5,0);
		game[13]->SetSkew(-4.5,0,4.5,0,4.5,49,-4.5,27);
		}
		
		else if (rows==3)
		{
		game[0]->SetPosition(13,58);
		game[1]->SetPosition(13,153);
		game[2]->SetPosition(13,250);
		
		game[3]->SetPosition(68,67);
		game[4]->SetPosition(68,153);
		game[5]->SetPosition(68,239);
		
		game[6]->SetPosition(120,74);
		game[7]->SetPosition(120,153);
		game[8]->SetPosition(120,232);
		
		game[9]->SetPosition(170,78);
		game[10]->SetPosition(170,153);
		game[11]->SetPosition(170,228);
		
		game[12]->SetPosition(214,80);
		game[13]->SetPosition(214,153);
		game[14]->SetPosition(214,226);
		
		game[15]->SetPosition(258,81);
		game[16]->SetPosition(258,153);
		game[17]->SetPosition(258,224);
		
		game[18]->SetPosition(302,81);
		game[19]->SetPosition(302,153);
		game[20]->SetPosition(302,223);
		
		game[21]->SetPosition(346,81);
		game[22]->SetPosition(346,153);
		game[23]->SetPosition(346,223);
		
		game[24]->SetPosition(390,80);
		game[25]->SetPosition(390,153);
		game[26]->SetPosition(390,225);
		
		game[27]->SetPosition(434,77);
		game[28]->SetPosition(434,153);
		game[29]->SetPosition(434,227);
		
		game[30]->SetPosition(484,73);
		game[31]->SetPosition(484,153);
		game[32]->SetPosition(484,231);
		
		game[33]->SetPosition(537,67);
		game[34]->SetPosition(537,153);
		game[35]->SetPosition(537,239);
		
		game[36]->SetPosition(591,58);
		game[37]->SetPosition(591,153);
		game[38]->SetPosition(591,250);
		
		game[0]->SetSkew(-38,-110,15,-42,15,65,-38,32);
		game[1]->SetSkew(-38,-75,15,-48,15,45,-38,72);
		game[2]->SetSkew(-38,-52,15,-70,15,27,-38,100);
		
		game[3]->SetSkew(-38,-70,15,-24,15,40,-38,27);
		game[4]->SetSkew(-38,-50,15,-35,15,40,-38,50);
		game[5]->SetSkew(-38,-34,15,-47,15,24,-38,58);
		
		game[6]->SetSkew(-27,-55,19,-22,19,30,-27,22);
		game[7]->SetSkew(-27,-40,19,-30,19,30,-27,40);
		game[8]->SetSkew(-27,-20,19,-30,19,20,-27,50);
		
		game[9]->SetSkew(-19,-28,0,-17,0,15,-19,10);
		game[10]->SetSkew(-19,-30,0,-20,0,12,-19,30);
		game[11]->SetSkew(-19,-15,0,-20,0,10,-19,24);
		
		game[12]->SetSkew(-10,-20,3,-13,3,14,-10,10);
		game[13]->SetSkew(-10,-20,3,-18,3,18,-10,20);
		game[14]->SetSkew(-10,-10,3,-10,3,0,-10,10);
		
		game[15]->SetSkew(-10,-15,3,-12,3,13,-10,13);
		game[16]->SetSkew(-10,-17,3,-10,3,10,-10,17);
		game[17]->SetSkew(-10,-10,3,-15,3,10,-10,10);
		
		game[18]->SetSkew(-10,-10,3,-10,3,14,-10,14);
		game[19]->SetSkew(-10,-10,3,-10,3,10,-10,10);//middle
		game[20]->SetSkew(-10,-10,3,-10,3,10,-10,10);
		
		game[21]->SetSkew(-14,-10,4,-20,3,10,-14,10);
		game[22]->SetSkew(-14,-10,4,-17,3,17,-14,10);
		game[23]->SetSkew(-14,-10,4,-10,3,10,-14,10);
		
		game[24]->SetSkew(-10,-13,3,-20,3,14,-10,10);
		game[25]->SetSkew(-10,-18,3,-20,3,20,-10,18);
		game[26]->SetSkew(-10,-10,3,-10,3,20,-10,5);
		
		game[27]->SetSkew(-19,-17,0,-28,0,10,-19,15);
		game[28]->SetSkew(-19,-20,0,-30,0,30,-19,12);
		game[29]->SetSkew(-19,-20,0,-15,0,30,-19,10);
		
		game[30]->SetSkew(-27,-22,19,-55,19,22,-27,30);
		game[31]->SetSkew(-27,-30,19,-40,19,40,-27,30);
		game[32]->SetSkew(-27,-30,19,-20,19,55,-27,20);
		
		game[33]->SetSkew(-38,-24,15,-70,15,27,-38,40);
		game[34]->SetSkew(-38,-35,15,-50,15,50,-38,40);
		game[35]->SetSkew(-38,-47,15,-34,15,58,-38,24);
		
		game[36]->SetSkew(-38,-42,15,-110,15,32,-38,60);
		game[37]->SetSkew(-38,-48,15,-75,15,70,-38,45);
		game[38]->SetSkew(-38,-70,15,-52,15,100,-38,27);
		}
		
		}
		else 
		WindowPrompt("Oops","Your Wii must be in 16:9 mode to see the gamewall.",0, LANGUAGE.ok, 0,0);
                        
		//}

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
	delete btnRowUp;
	delete btnRowDown;

	delete trigA;
	delete trigL;
	delete trigR;
	delete trigPlus;
	delete trigMinus;
	delete trig1;
	delete trig2;
	delete btnSoundClick;
	delete btnSoundOver;

	for(int i=0; i<pagesize; i++) {
		delete game[i];
	}
	for(int i=0; i<gameCnt; i++) {
		delete coverImg[i];
		delete cover[i];
	}

	delete [] gameIndex;
	delete [] bob;
	delete [] game;
	delete [] coverImg;
	delete [] cover;
}


void GuiGameGrid::SetFocus(int f)
{
	LOCK(this);
	focus = f;

	for(int i=0; i<pagesize; i++)
		game[i]->ResetState();

	if(f == 1)
		game[bob[selectedItem]]->SetState(STATE_SELECTED);
}


void GuiGameGrid::ResetState()
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


int GuiGameGrid::GetOffset()
{
	return listOffset;
}


int GuiGameGrid::GetClickedOption()
{
	int found = -1;
	if (clickedItem>-1){
		game[bob[clickedItem]]->SetState(STATE_SELECTED);
		found= (clickedItem+listOffset) % gameCnt;
		clickedItem=-1;
	}
	return found;
}


int GuiGameGrid::GetSelectedOption()
{
	int found = -1;
	for(int i=0; i<pagesize; i++) {
		if(game[bob[i]]->GetState() == STATE_SELECTED) {
			game[bob[i]]->SetState(STATE_SELECTED);
			found = (listOffset+i) % gameCnt;
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
		if(gameCnt <= pagesize)
			return -1;
		else
			nextItem = (nextItem < 0) ? nextItem + gameCnt : nextItem - gameCnt;

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
	
	if(c>0){
	
	int next = listOffset;

	for(int i=0; i<pagesize; i++) {
		if(next >= 0) {
			game[bob[i]]->Draw();
			next = this->FindMenuItem(next, 1);
		} else break;
	}

	if(gameCnt > pagesize) {
		btnRight->Draw();
		btnLeft->Draw();
	}

	btnRowUp->Draw();
	btnRowDown->Draw();
	//debugTxt->Draw();
	if ((wait>75)&&(Settings.tooltips == TooltipsOn))
	titleTT->Draw();
	
	
	

	this->UpdateEffects();
	}
}



/**
 * Change the number of rows 
 */
void GuiGameGrid::ChangeRows(int n)
{

	rows=n;
	for(int i=0; i < gameCnt; i++) {
		coverImg[i] = new GuiImage(cover[i]);
		coverImg[i]->SetWidescreen(CFG.widescreen);
		if (rows==2)coverImg[i]->SetScale(.6);//these are the numbers for 2 rows
		else if (rows==3)coverImg[i]->SetScale(.26);//these are the numbers for 3 rows
		
	}
	//set  pagesize
	if (n==1)pagesize=6;
	else if (n==2)pagesize=16;
	else if (n==3)pagesize=42;
	

	firstPic=0;
	
	// create new buttons based on pagesize
	for(int i=0; i < pagesize; i++) {
		if (n==1)game[i]->SetSize(160,224);//for 1 row
		if (n==2)game[i]->SetSize(75,133);//these are the numbers for 2 rows
		else if (n==3)game[i]->SetSize(35,68);//these are the numbers for 3 rows
		game[i]->SetPosition(0,740);//hide unused buttons 
		game[i]->SetImage(listOffset+i<gameCnt ? coverImg[listOffset+i] : coverImg[listOffset+i-gameCnt]);
		if (n==1){listOffset+i<gameCnt ? coverImg[listOffset+i]->SetPosition(0,0):coverImg[listOffset+i-gameCnt]->SetPosition(0,0);}// only for 1 row
		if (n==2){listOffset+i<gameCnt ? coverImg[listOffset+i]->SetPosition(0,-50):coverImg[listOffset+i-gameCnt]->SetPosition(0,-50);}// only for 2 row
		if (n==3){listOffset+i<gameCnt ? coverImg[listOffset+i]->SetPosition(0,-80):coverImg[listOffset+i-gameCnt]->SetPosition(0,-80);}// only for 3 row
		bob[i] = i;
		
		
	}
	if(CFG.widescreen)
	{
		
		if (n==1)
				{
		game[bob[0]]->SetPosition(-70,74);
		game[bob[1]]->SetPosition(88,74);
		game[bob[2]]->SetPosition(239,74);
		game[bob[3]]->SetPosition(390,74);
		game[bob[4]]->SetPosition(550,74);
		
		game[bob[0]]->SetSkew(-10,-44,10,-26,10,26,-10,44);
		game[bob[1]]->SetSkew(-6,-22,6,-14,6,14,-6,22);
		game[bob[2]]->SetSkew(0,-11,0,-11,0,11,0,11);
		game[bob[3]]->SetSkew(-6,-14,6,-22,6,22,-6,14);
		game[bob[4]]->SetSkew(-10,-26,10,-44,10,44,-10,26);
		}
		else if (n == 2)
		{
		game[bob[0]]->SetPosition(3,50);
		game[bob[1]]->SetPosition(3,193);
		game[bob[2]]->SetPosition(97,50);
		game[bob[3]]->SetPosition(97,193);
		game[bob[4]]->SetPosition(187,50);
		game[bob[5]]->SetPosition(187,193);
		game[bob[6]]->SetPosition(272,50);
		game[bob[7]]->SetPosition(272,193);
		game[bob[8]]->SetPosition(358,50);
		game[bob[9]]->SetPosition(358,193);
		game[bob[10]]->SetPosition(449,50);
		game[bob[11]]->SetPosition(449,193);
		game[bob[12]]->SetPosition(545,50);
		game[bob[13]]->SetPosition(545,193);
		
		
		game[bob[0]]->SetSkew(-4.5,-49,4.5,-27,4.5,0,-4.5,0);
		game[bob[1]]->SetSkew(-4.5,0,4.5,0,4.5,27,-4.5,49);
		game[bob[2]]->SetSkew(-4,-22,4,-14,4,0,-4,0);
		game[bob[3]]->SetSkew(-4,0,4,0,4,14,-4,22);
		game[bob[4]]->SetSkew(0,-9,0,-5,0,0,0,0);
		game[bob[5]]->SetSkew(0,0,0,0,0,5,0,9);
		game[bob[6]]->SetSkew(0,0,0,0,0,0,0,0);
		game[bob[7]]->SetSkew(0,0,0,0,0,0,0,0);
		game[bob[8]]->SetSkew(0,-5,0,-9,0,0,0,0);
		game[bob[9]]->SetSkew(0,0,0,0,0,9,0,5);
		game[bob[10]]->SetSkew(-4,-14,4,-22,4,0,-4,0);
		game[bob[11]]->SetSkew(-4,0,4,0,4,22,-4,14);
		game[bob[12]]->SetSkew(-4.5,-27,4.5,-49,4.5,0,-4.5,0);
		game[bob[13]]->SetSkew(-4.5,0,4.5,0,4.5,49,-4.5,27);
		}
		else if (n==3)
		{
		game[bob[0]]->SetPosition(13,58);
		game[bob[1]]->SetPosition(13,153);
		game[bob[2]]->SetPosition(13,250);
		
		game[bob[3]]->SetPosition(68,67);
		game[bob[4]]->SetPosition(68,153);
		game[bob[5]]->SetPosition(68,239);
		
		game[bob[6]]->SetPosition(120,74);
		game[bob[7]]->SetPosition(120,153);
		game[bob[8]]->SetPosition(120,232);
		
		game[bob[9]]->SetPosition(170,78);
		game[bob[10]]->SetPosition(170,153);
		game[bob[11]]->SetPosition(170,228);
		
		game[bob[12]]->SetPosition(214,80);
		game[bob[13]]->SetPosition(214,153);
		game[bob[14]]->SetPosition(214,226);
		
		game[bob[15]]->SetPosition(258,81);
		game[bob[16]]->SetPosition(258,153);
		game[bob[17]]->SetPosition(258,224);
		
		game[bob[18]]->SetPosition(302,81);
		game[bob[19]]->SetPosition(302,153);
		game[bob[20]]->SetPosition(302,223);
		
		game[bob[21]]->SetPosition(346,81);
		game[bob[22]]->SetPosition(346,153);
		game[bob[23]]->SetPosition(346,223);
		
		game[bob[24]]->SetPosition(390,80);
		game[bob[25]]->SetPosition(390,153);
		game[bob[26]]->SetPosition(390,225);
		
		game[bob[27]]->SetPosition(434,77);
		game[bob[28]]->SetPosition(434,153);
		game[bob[29]]->SetPosition(434,227);
		
		game[bob[30]]->SetPosition(484,73);
		game[bob[31]]->SetPosition(484,153);
		game[bob[32]]->SetPosition(484,231);
		
		game[bob[33]]->SetPosition(537,67);
		game[bob[34]]->SetPosition(537,153);
		game[bob[35]]->SetPosition(537,239);
		
		game[bob[36]]->SetPosition(591,58);
		game[bob[37]]->SetPosition(591,153);
		game[bob[38]]->SetPosition(591,250);
		
		game[bob[0]]->SetSkew(-38,-110,15,-42,15,65,-38,32);
		game[bob[1]]->SetSkew(-38,-75,15,-48,15,45,-38,72);
		game[bob[2]]->SetSkew(-38,-52,15,-70,15,27,-38,100);
		
		game[bob[3]]->SetSkew(-38,-70,15,-24,15,40,-38,27);
		game[bob[4]]->SetSkew(-38,-50,15,-35,15,40,-38,50);
		game[bob[5]]->SetSkew(-38,-34,15,-47,15,24,-38,58);
		
		game[bob[6]]->SetSkew(-27,-55,19,-22,19,30,-27,22);
		game[bob[7]]->SetSkew(-27,-40,19,-30,19,30,-27,40);
		game[bob[8]]->SetSkew(-27,-20,19,-30,19,20,-27,50);
		
		game[bob[9]]->SetSkew(-19,-28,0,-17,0,15,-19,10);
		game[bob[10]]->SetSkew(-19,-30,0,-20,0,12,-19,30);
		game[bob[11]]->SetSkew(-19,-15,0,-20,0,10,-19,24);
		
		game[bob[12]]->SetSkew(-10,-20,3,-13,3,14,-10,10);
		game[bob[13]]->SetSkew(-10,-20,3,-18,3,18,-10,20);
		game[bob[14]]->SetSkew(-10,-10,3,-10,3,0,-10,10);
		
		game[bob[15]]->SetSkew(-10,-15,3,-12,3,13,-10,13);
		game[bob[16]]->SetSkew(-10,-17,3,-10,3,10,-10,17);
		game[bob[17]]->SetSkew(-10,-10,3,-15,3,10,-10,10);
		
		game[bob[18]]->SetSkew(-10,-10,3,-10,3,14,-10,14);
		game[bob[19]]->SetSkew(-10,-10,3,-10,3,10,-10,10);//middle
		game[bob[20]]->SetSkew(-10,-10,3,-10,3,10,-10,10);
		
		game[bob[21]]->SetSkew(-14,-10,4,-20,3,10,-14,10);
		game[bob[22]]->SetSkew(-14,-10,4,-17,3,17,-14,10);
		game[bob[23]]->SetSkew(-14,-10,4,-10,3,10,-14,10);
		
		game[bob[24]]->SetSkew(-10,-13,3,-20,3,14,-10,10);
		game[bob[25]]->SetSkew(-10,-18,3,-20,3,20,-10,18);
		game[bob[26]]->SetSkew(-10,-10,3,-10,3,20,-10,5);
		
		game[bob[27]]->SetSkew(-19,-17,0,-28,0,10,-19,15);
		game[bob[28]]->SetSkew(-19,-20,0,-30,0,30,-19,12);
		game[bob[29]]->SetSkew(-19,-20,0,-15,0,30,-19,10);
		
		game[bob[30]]->SetSkew(-27,-22,19,-55,19,22,-27,30);
		game[bob[31]]->SetSkew(-27,-30,19,-40,19,40,-27,30);
		game[bob[32]]->SetSkew(-27,-30,19,-20,19,55,-27,20);
		
		game[bob[33]]->SetSkew(-38,-24,15,-70,15,27,-38,40);
		game[bob[34]]->SetSkew(-38,-35,15,-50,15,50,-38,40);
		game[bob[35]]->SetSkew(-38,-47,15,-34,15,58,-38,24);
		
		game[bob[36]]->SetSkew(-38,-42,15,-110,15,32,-38,60);
		game[bob[37]]->SetSkew(-38,-48,15,-75,15,70,-38,45);
		game[bob[38]]->SetSkew(-38,-70,15,-52,15,100,-38,27);
		}
		
	}
}


void GuiGameGrid::Update(GuiTrigger * t)
{
	LOCK(this);
	if(state == STATE_DISABLED || !t)
		return;

	if(!(game[0]->GetEffect() || game[0]->GetEffectOnOver())) {
		for(int i=0; i<pagesize; i++) {
			game[i]->SetEffectGrow();
		}
	}
	//if (realCnt!=0)goRight=(12*(pagesize-realCnt)/2);
	// for debugging
	//snprintf(debugbuffer, sizeof(debugbuffer), "count: %i listOffset: %i", count,listOffset);
	//debugTxt->SetText(debugbuffer);
	//debugTxt->Draw();
	
	btnRight->Update(t);
	btnLeft->Update(t);
	btnRowUp->Update(t);
	btnRowDown->Update(t);

	int next = listOffset;


	for(int i=0; i<pagesize; i++) {
		if(next >= 0) {
			if(game[bob[i]]->GetState() == STATE_DISABLED) {
				game[bob[i]]->SetVisible(true);
				game[bob[i]]->SetState(STATE_DEFAULT);
			}
			gameIndex[i] = next;
			next = this->FindMenuItem(next, 1);
		} else {
			game[bob[i]]->SetVisible(false);
			game[bob[i]]->SetState(STATE_DISABLED);
		}

		if(focus) {
			if(i != selectedItem && game[bob[i]]->GetState() == STATE_SELECTED)
				game[bob[i]]->ResetState();
			else if(i == selectedItem && game[bob[i]]->GetState() == STATE_DEFAULT);
				game[bob[selectedItem]]->SetState(STATE_SELECTED, t->chan);
		}
		game[bob[i]]->Update(t);

		if(game[bob[i]]->GetState() == STATE_SELECTED) {
			selectedItem = i;
		}
		if(game[bob[i]]->GetState() == STATE_CLICKED) {
			clickedItem = i;
		}

	}
	
	this->SetPosition(this->GetLeft()+mover2,this->GetTop());
	if (goback==1)mover2= (mover2<0? mover2+1:mover2-1);
	if (mover2==0)goback=0;
	
	/*u16 buttons = ButtonsHold();
		if(buttons & WPAD_BUTTON_B) {
			int x = t->wpad.ir.x;
			int center = this->GetWidth()/2;
			if (x<center)goLeft=12;
			else if (x>center)goRight=12;
			usleep(x<center? x*250:((center*2 - x)*250));
			for(int i=0; i<pagesize; i++) {game[i]->Draw();}
			//return;
		}*/
	
	
	// navigation
	if(!focus || gameCnt < pagesize || (c==0)||(game[bob[0]]->GetEffect() && game[bob[pagesize-1]]->GetEffect()))
		return; // skip navigation

	if (t->Left()  || btnLeft->GetState() == STATE_CLICKED) {
		WPAD_ScanPads();
		u16 buttons = 0;
		for(int i=0; i<4; i++)
			buttons |= WPAD_ButtonsHeld(i);
		if(!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_MINUS) || t->Left())) {
			btnLeft->ResetState();
			speed = SHIFT_SPEED;
			return;
		}goLeft=12;
		
		
	}
	if (goLeft>0){
	
	for (int i=1; i<rows+1; i++){ 
			game[bob[pagesize-i]]->SetImage(coverImg[(listOffset + pagesize-i) % gameCnt]);
			if (rows==1)coverImg[(listOffset+i) % gameCnt]->SetPosition(0,0);// only for 1 row
			if (rows==3)coverImg[(listOffset + pagesize-i) % gameCnt]->SetPosition(0,-80);// only for 3 rows
			if (rows==2)coverImg[(listOffset + pagesize-i) % gameCnt]->SetPosition(0,-50);// only for 2 rows
		
			}
		if (mover<11){
		if(CFG.widescreen)
				{
		if (rows==1){
		game[bob[0]]->SetPosition(-70-(mover * 16),74);
		game[bob[1]]->SetPosition(88-(mover * 15.8),74);
		game[bob[2]]->SetPosition(239-(mover * 15.1),74);
		game[bob[3]]->SetPosition(390-(mover * 15.1),74);
		game[bob[4]]->SetPosition(550-(mover * 16),74);
		game[bob[5]]->SetPosition(710-(mover * 16),74);
		
		//if (mover>5)game[bob[0]]->SetSkew(-10,-26,10,-44,10,44,-10,26);
		//SetSkew(-10(mover * ),-44(mover * ),10(mover * ),-26(mover * ),
			//					10(mover * ),26(mover * ),-10(mover * ),44(mover * ));
								
		game[bob[1]]->SetSkew(-6-(mover * .4),-22-(mover * 2.2),6+(mover * .4),-14-(mover * 1.2),
								6+(mover * .4),14+(mover * 1.2),-6-(mover * .4),22+(mover * 2.2));
		
		game[bob[2]]->SetSkew(0-(mover * .6),-11-(mover * 1.1),0+(mover * .6),-11-(mover * .3)
								,0+(mover * .6),11+(mover * .3),0-(mover * .6),11+(mover * 1.1));
								
		game[bob[3]]->SetSkew(-6+(mover * .6),-14+(mover * .3),6-(mover * .6),-22+(mover * 1.1),
								6-(mover * .6),22-(mover * 1.1),-6+(mover * .6),14-(mover * .3));
								
		game[bob[4]]->SetSkew(-10+(mover * .4),-26+(mover * 1.2),10-(mover * .4),-44+(mover * 2.2),
								10-(mover * .4),44-(mover * 2.2),-10+(mover * .4),26-(mover * 1.2));
		
		game[bob[5]]->SetSkew(-14+(mover * .4),-34+(mover * 1.2),14-(mover * .4),-66+(mover * 2.2),
								14-(mover * .4),66-(mover * 2.2),-14+(mover * .4),34-(mover * 1.2));
		}
		else if (rows ==2)
		{
		game[bob[0]]->SetPosition(3-(mover * 9.4),50);
		game[bob[1]]->SetPosition(3-(mover * 9.4),193);
		game[bob[2]]->SetPosition(97-(mover * 9.4),50);
		game[bob[3]]->SetPosition(97-(mover * 9.4),193);
		game[bob[4]]->SetPosition(187-(mover * 9),50);
		game[bob[5]]->SetPosition(187-(mover * 9),193);
		game[bob[6]]->SetPosition(272-(mover * 8.5),50);
		game[bob[7]]->SetPosition(272-(mover * 8.5),193);
		game[bob[8]]->SetPosition(358-(mover * 8.5),50);
		game[bob[9]]->SetPosition(358-(mover * 8.5),193);
		game[bob[10]]->SetPosition(449-(mover * 9),50);
		game[bob[11]]->SetPosition(449-(mover * 9),193);
		game[bob[12]]->SetPosition(545-(mover * 9.6),50);
		game[bob[13]]->SetPosition(545-(mover * 9.6),193);
		game[bob[14]]->SetPosition(641-(mover * 9.6),50);
		game[bob[15]]->SetPosition(641-(mover * 9.6),193);
		
		game[bob[2]]->SetSkew(-4-(mover * .05),-22-(mover * 2.7),4+(mover * .05),-14-(mover * 1.3),
								4+(mover * .05),0,-4-(mover * .05),0);
		
		game[bob[3]]->SetSkew(-4-(mover * .05),0,4+(mover * .05),0,
								4+(mover * .05),14+(mover * 1.3),-4-(mover * .05),22+(mover * 2.7));
		
		game[bob[4]]->SetSkew(0-(mover * .4),-9-(mover * 1.3),0+(mover * .4),-5-(mover * .9),
								0+(mover * .4),0,0-(mover * .4),0);
		
		game[bob[5]]->SetSkew(0-(mover * .4),0,0+(mover * .4),0,
								0+(mover *.4),5+(mover * .9),0-(mover * .4),9+(mover * 1.3));
		
		game[bob[6]]->SetSkew(0,0-(mover * .9),0,0-(mover * .5),
								0,0,0,0);
		
		game[bob[7]]->SetSkew(0,0,0,0,
								0,0+(mover * .5),0,0+(mover * .9));
		
		game[bob[8]]->SetSkew(0,-5+(mover * .5),0,-9+(mover * .9),
								0,0,0,0);
		
		game[bob[9]]->SetSkew(0,0,0,0,
								0,9-(mover * .9),0,5-(mover * .5));
		
		game[bob[10]]->SetSkew(-4+(mover * .4),-14+(mover * .9),4-(mover * .4),-22+(mover *1.3),
								4-(mover * .4),0,-4+(mover * .4),0);
		
		game[bob[11]]->SetSkew(-4+(mover * .4),0,4-(mover * .4),0,
								4-(mover * .4),22-(mover * 1.3),-4+(mover * .4),14-(mover * .9));
		
		game[bob[12]]->SetSkew(-4.5+(mover *.05),-27+(mover *1.3),4.5-(mover *.05),-49+(mover *2.7),
								4.5-(mover *.05),0,-4.5+(mover *.05),0);
		
		game[bob[13]]->SetSkew(-4.5+(mover *.05),0,4.5-(mover *.05),0,4.5-(mover *.05),
								49-(mover *2.7),-4.5+(mover *.05),27-(mover *1.3));
								
		game[bob[14]]->SetSkew(-4.5,-27,4.5,-49,4.5,0,-4.5,0);
		game[bob[15]]->SetSkew(-4.5,0,4.5,0,4.5,49,-4.5,27);
		}
		else if (rows==3)
		{
		game[bob[0]]->SetPosition(13-(mover * 5.5),58-(mover * .9));
		game[bob[1]]->SetPosition(13-(mover * 5.5),153);
		game[bob[2]]->SetPosition(13-(mover * 5.5),250+(mover * 1.1));
		
		game[bob[3]]->SetPosition(68-(mover * 5.5),67-(mover * .9));
		game[bob[4]]->SetPosition(68-(mover * 5.5),153);
		game[bob[5]]->SetPosition(68-(mover * 5.5),239+(mover * 1.1));
		
		game[bob[6]]->SetPosition(120-(mover * 5.2),74-(mover * .7));
		game[bob[7]]->SetPosition(120-(mover * 5.2),153);
		game[bob[8]]->SetPosition(120-(mover * 5.2),232+(mover * .7));
		
		game[bob[9]]->SetPosition(170-(mover * 5),78-(mover * .4));
		game[bob[10]]->SetPosition(170-(mover * 5),153);
		game[bob[11]]->SetPosition(170-(mover * 5),228+(mover * .4));
		
		game[bob[12]]->SetPosition(214-(mover * 4.4),80-(mover * .2));
		game[bob[13]]->SetPosition(214-(mover * 4.4),153);
		game[bob[14]]->SetPosition(214-(mover * 4.4),226+(mover * .2));
		
		game[bob[15]]->SetPosition(258-(mover * 4.4),81-(mover * .1));
		game[bob[16]]->SetPosition(258-(mover * 4.4),153);
		game[bob[17]]->SetPosition(258-(mover * 4.4),224+(mover * .2));
		
		game[bob[18]]->SetPosition(302-(mover * 4.4),81);
		game[bob[19]]->SetPosition(302-(mover * 4.4),153);
		game[bob[20]]->SetPosition(302-(mover * 4.4),223+(mover * .1));
		
		game[bob[21]]->SetPosition(346-(mover * 4.4),81);
		game[bob[22]]->SetPosition(346-(mover * 4.4),153);
		game[bob[23]]->SetPosition(346-(mover * 4.4),223);
		
		game[bob[24]]->SetPosition(390-(mover * 4.4),80+(mover * .1));
		game[bob[25]]->SetPosition(390-(mover * 4.4),153);
		game[bob[26]]->SetPosition(390-(mover * 4.4),225-(mover * .2));
		
		game[bob[27]]->SetPosition(434-(mover * 4.4),77+(mover * .3));
		game[bob[28]]->SetPosition(434-(mover * 4.4),153);
		game[bob[29]]->SetPosition(434-(mover * 4.4),227-(mover * .2));
		
		game[bob[30]]->SetPosition(484-(mover * 5),73+(mover * .4));
		game[bob[31]]->SetPosition(484-(mover * 5),153);
		game[bob[32]]->SetPosition(484-(mover * 5),231-(mover * .4));
		
		game[bob[33]]->SetPosition(537-(mover * 5.3),67+(mover * .6));
		game[bob[34]]->SetPosition(537-(mover * 5.3),153);
		game[bob[35]]->SetPosition(537-(mover * 5.3),239-(mover * .8));
		
		game[bob[36]]->SetPosition(591-(mover * 5.4),58+(mover * 1.1));
		game[bob[37]]->SetPosition(591-(mover * 5.4),153);
		game[bob[38]]->SetPosition(591-(mover * 5.4),250-(mover * 1.1));
		
		game[bob[39]]->SetPosition(645-(mover * 5.4),58);
		game[bob[40]]->SetPosition(645-(mover * 5.4),153);
		game[bob[41]]->SetPosition(645-(mover * 5.4),250);
		
		//game[bob[0]]->SetSkew(-38,-110,15,-42,15,65,-38,32);
		//game[bob[1]]->SetSkew(-38,-75,15,-48,15,45,-38,72);
		//game[bob[2]]->SetSkew(-38,-52,15,-70,15,27,-38,100);
		
		game[bob[3]]->SetSkew(-38,-70-(mover * 4),15,-24-(mover * 1.8),15,40+(mover * 1.5),-38,27+(mover * .5));
		game[bob[4]]->SetSkew(-38,-50-(mover * 2.5),15,-35-(mover * .7),15,40-(mover * .5),-38,50+(mover * 1.2));
		game[bob[5]]->SetSkew(-38,-34-(mover * 1.8),15,-47-(mover * 2.3),15,24+(mover * .3),-38,58+(mover * 4.2));
		
		game[bob[6]]->SetSkew(-27-(mover * 1.1),-55-(mover * 1.5),19-(mover * .4),-22-(mover * .2),19-(mover * .4),30+(mover * 1),-27-(mover * 1.1),22+(mover * .5));
		game[bob[7]]->SetSkew(-27-(mover * 1.1),-40-(mover * 1),19-(mover * .4),-30-(mover * .5),19-(mover * .4),30-(mover * 1),-27-(mover * 1.1),40+(mover * 1));
		game[bob[8]]->SetSkew(-27-(mover * 1.1),-20-(mover * 1.4),19-(mover * .4),-30-(mover * 1.7),19-(mover * .4),20+(mover * .4),-27-(mover * 1.1),50+(mover * .8));
		
		game[bob[9]]->SetSkew(-19-(mover * .8),-28-(mover * 1.7),0+(mover * 1.9),-17-(mover * .5),0+(mover * 1.9),15+(mover * 1.5),-19-(mover * .8),10+(mover * 1.2));
		game[bob[10]]->SetSkew(-19-(mover * .8),-30-(mover * 1),0+(mover * 1.9),-20-(mover * 1),0+(mover * 1.9),12+(mover * 1.8),-19-(mover * .8),30+(mover * 1));
		game[bob[11]]->SetSkew(-19-(mover * .8),-15-(mover * .5),0+(mover * 1.9),-20-(mover * 1),0+(mover * 1.9),10+(mover * 1),-19-(mover * .8),24+(mover * 2.2));
		
		game[bob[12]]->SetSkew(-10-(mover * .9),-20-(mover * .8),3-(mover * .3),-13-(mover * .4),3-(mover * .3),14+(mover * .1),-10-(mover * .9),10);
		game[bob[13]]->SetSkew(-10-(mover * .9),-20-(mover * 1),3-(mover * .3),-18-(mover * .2),3-(mover * .3),18-(mover * .1),-10-(mover * .9),20+(mover * 1));
		game[bob[14]]->SetSkew(-10-(mover * .9),-10-(mover * .5),3-(mover * .3),-10-(mover * 1),3-(mover * .3),0+(mover * 1),-10-(mover * .9),10+(mover * 1.4));
		
		game[bob[15]]->SetSkew(-10,-15-(mover * .5),3,-12-(mover * .1),3,13+(mover * .1),-10,13-(mover * .3));
		game[bob[16]]->SetSkew(-10,-17-(mover * .3),3,-10-(mover * .8),3,10+(mover * .8),-10,17+(mover * .3));
		game[bob[17]]->SetSkew(-10,-10,3,-15+(mover * .5),3,10-(mover * 1),-10,10);
		
		game[bob[18]]->SetSkew(-10,-10-(mover * .5),3,-10-(mover * .2),3,14-(mover * .1),-10,14-(mover * .1));
		game[bob[19]]->SetSkew(-10,-10-(mover * .7),3,-10,3,10,-10,10+(mover * .7));//middle
		game[bob[20]]->SetSkew(-10,-10,3,-10-(mover * .5),3,10,-10,10);
		
		game[bob[21]]->SetSkew(-14,-10,4-(mover * .1),-20+(mover * 1),3,10+(mover * .4),-14,10);
		game[bob[22]]->SetSkew(-14,-10,4-(mover * .1),-17+(mover * .7),3,17-(mover * .7),-14,10);
		game[bob[23]]->SetSkew(-14,-10,4-(mover * .1),-10,3,10,-14+(mover * .4),10);
		
		game[bob[24]]->SetSkew(-10-(mover * .4),-13-(mover * .3),3+(mover * .1),-20,3,14-(mover * .4),-10-(mover * .4),10);
		game[bob[25]]->SetSkew(-10-(mover * .4),-18-(mover * .8),3+(mover * .1),-20+(mover * .3),3,20-(mover * .3),-10-(mover * .4),18-(mover * .8));
		game[bob[26]]->SetSkew(-10-(mover * .4),-10,3+(mover * .1),-10,3,20-(mover * 1),-10-(mover * .4),5+(mover * .5));
		
		game[bob[27]]->SetSkew(-19+(mover * .9),-17+(mover * .4),0+(mover * .3),-28+(mover * .8),0+(mover * .3),10+(mover * .4),-19+(mover * .9),15-(mover * .5));
		game[bob[28]]->SetSkew(-19+(mover * .9),-20+(mover * .2),0+(mover * .3),-30+(mover * 1),0+(mover * .3),20-(mover * 1),-19+(mover * .9),12+(mover * .6));
		game[bob[29]]->SetSkew(-19+(mover * .9),-20+(mover * 1),0+(mover * .3),-15+(mover * .5),0+(mover * .3),30-(mover * 1),-19+(mover * .9),10);
		
		game[bob[30]]->SetSkew(-27+(mover * .8),-22+(mover * .5),19-(mover * .9),-55+(mover * 1.7),19-(mover * .9),22-(mover * 1.2),-27+(mover * .8),30-(mover * 1.5));
		game[bob[31]]->SetSkew(-27+(mover * .8),-30+(mover * 1),19-(mover * .9),-40+(mover * 1),19-(mover * .9),40-(mover * 2),-27+(mover * .8),30-(mover * 1.8));
		game[bob[32]]->SetSkew(-27+(mover * .8),-30+(mover * 1),19-(mover * .9),-20+(mover * .5),19-(mover * .9),55-(mover * 1.5),-27+(mover * .8),20-(mover * 1));
		
		game[bob[33]]->SetSkew(-38+(mover * 1.1),-24+(mover * .2),15+(mover * .4),-70+(mover * 1.5),15+(mover * .4),27-(mover * .5),-38+(mover * 1.1),40-(mover * 1));
		game[bob[34]]->SetSkew(-38+(mover * 1.1),-35+(mover * .5),15+(mover * .4),-50+(mover * 1),15+(mover * .4),50-(mover * 1),-38+(mover * 1.1),40-(mover * 1));
		game[bob[35]]->SetSkew(-38+(mover * 1.1),-47+(mover * 1.7),15+(mover * .4),-34+(mover * 1.4),15+(mover * .4),58-(mover * .3),-38+(mover * 1.1),24-(mover * .4));
		
		game[bob[36]]->SetSkew(-38,-42+(mover * 1.8),15,-110+(mover * 4),15,32-(mover * .5),-38,60-(mover * 2));
		game[bob[37]]->SetSkew(-38,-48+(mover * 1.3),15,-75+(mover * 2.5),15,70-(mover * 2),-38,45-(mover * .5));
		game[bob[38]]->SetSkew(-38,-70+(mover * 2.3),15,-52+(mover * 1.8),15,100-(mover * 4.2),-38,27-(mover * .3));
		
		game[bob[39]]->SetSkew(-38,-42,15,-110,15,32,-38,60);
		game[bob[40]]->SetSkew(-38,-48,15,-75,15,65,-38,45);
		game[bob[41]]->SetSkew(-38,-70,15,-52,15,100,-38,27);
		}
		
		}
		mover++;
		goLeft--;
		
		}
		else {goLeft=0;mover=0;
		listOffset = (listOffset+rows < gameCnt) ? listOffset+rows : ((listOffset+rows) - gameCnt);
		firstPic = (firstPic+rows < pagesize) ? firstPic+rows : 0;

		for (int i=0; i<pagesize; i++) {
			bob[i] = (firstPic+i)%pagesize;
		}
		
	}
	}
	
	else if(t->Right()  || btnRight->GetState() == STATE_CLICKED) {
		WPAD_ScanPads();
		u16 buttons = 0;
		for(int i=0; i<4; i++)
			buttons |= WPAD_ButtonsHeld(i);
		if(!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_PLUS) || t->Right())) {
			btnRight->ResetState();
			speed=SHIFT_SPEED;
			return;
		}
		goRight=12;
		
		
		
		
	}
	if (goRight>0){
	if (mover<11){
	
	
	
	for (int i=1; i<(rows+1); i++){ 
		int tmp = listOffset-i;
	
		if (tmp<0)tmp=(gameCnt-i);
		
			game[bob[pagesize-(i)]]->SetImage(coverImg[tmp]);
			
			if (rows==1)coverImg[tmp]->SetPosition(0,0);// only for 1 row
			if (rows==3)coverImg[tmp]->SetPosition(0,-80);// only for 3 rows
			if (rows==2)coverImg[tmp]->SetPosition(0,-50);// only for 2 rows   
			}
		if(CFG.widescreen)
				{
		if (rows==1){
		game[bob[0]]->SetPosition(-70+(mover * 15.8),74);
		game[bob[1]]->SetPosition(88+(mover * 15.1),74);
		game[bob[2]]->SetPosition(239+(mover * 15.1),74);
		game[bob[3]]->SetPosition(390+(mover * 16),74);
		game[bob[4]]->SetPosition(550+(mover * 16),74);
		game[bob[5]]->SetPosition(-230+(mover * 16),74);
		
		game[bob[0]]->SetSkew(-10+(mover * .4),-44+(mover * 2.2),10-(mover * .4),-26+(mover * 1.2),
								10-(mover * .4),26-(mover * 1.2),-10+(mover * .4),44-(mover * 2.2));
								
		game[bob[1]]->SetSkew(-6+(mover * .6),-22+(mover * 1.1),6-(mover * .6),-14+(mover * .3),
								6-(mover * .6),14-(mover * .3),-6+(mover * .6),22-(mover * 1.1));
		
		game[bob[2]]->SetSkew(0-(mover * .6),-11-(mover * .3),0+(mover * .6),-11-(mover * 1.1)
								,0+(mover * .6),11+(mover * 1.1),0-(mover * .6),11+(mover * .3));
								
		game[bob[3]]->SetSkew(-6-(mover * .4),-14-(mover * 1.2),6+(mover * .4),-22-(mover * 2.2),
								6+(mover * .4),22+(mover * 2.2),-6-(mover * .4),14+(mover * 1.2));
								
		game[bob[4]]->SetSkew(-10-(mover * .4),-26+(mover * 1.2),10+(mover * .4),-44-(mover * 2.2),
								10+(mover * .4),44+(mover * 2.2),-10-(mover * .4),26+(mover * 1.2));
								
		game[bob[5]]->SetSkew(-10,-44,10,-26,10,26,-10,44);
		}
		else if (rows==2)
		{
		game[bob[0]]->SetPosition(3+(mover * 9.4),50);
		game[bob[1]]->SetPosition(3+(mover * 9.4),193);
		game[bob[2]]->SetPosition(97+(mover * 9),50);
		game[bob[3]]->SetPosition(97+(mover * 9),193);
		game[bob[4]]->SetPosition(187+(mover * 8.5),50);
		game[bob[5]]->SetPosition(187+(mover * 8.5),193);
		game[bob[6]]->SetPosition(272+(mover * 8.5),50);
		game[bob[7]]->SetPosition(272+(mover * 8.5),193);
		game[bob[8]]->SetPosition(358+(mover * 9),50);
		game[bob[9]]->SetPosition(358+(mover * 9),193);
		game[bob[10]]->SetPosition(449+(mover * 9.6),50);
		game[bob[11]]->SetPosition(449+(mover * 9.6),193);
		game[bob[12]]->SetPosition(545+(mover * 9.6),50);
		game[bob[13]]->SetPosition(545+(mover * 9.6),193);
		game[bob[14]]->SetPosition(-93+(mover * 9.6),50);
		game[bob[15]]->SetPosition(-93+(mover * 9.6),193);
		
		game[bob[0]]->SetSkew(-4.5+(mover * .05),-49+(mover * 2.7),4.5-(mover * .05),-27+(mover * 1.3),
								4.5-(mover * .05),0,-4.5+(mover * .05),0);
		
		game[bob[1]]->SetSkew(-4.5+(mover * .05),0,4.5-(mover * .05),0,
								4.5-(mover * .05),27-(mover * 1.3),-4.5+(mover * .05),49-(mover * 2.7));
		
		game[bob[2]]->SetSkew(-4+(mover * .4),-22+(mover * 1.3),4-(mover * .4),-14+(mover * .9),
								4-(mover * .4),0,-4+(mover * .4),0);
		
		game[bob[3]]->SetSkew(-4+(mover * .4),0,4-(mover * .4),0,
								4-(mover * .4),14-(mover * .9),-4+(mover * .4),22-(mover * 1.3));
		
		game[bob[4]]->SetSkew(0,-9+(mover * .9),0,-5+(mover * .5),
								0,0,0,0);
		
		game[bob[5]]->SetSkew(0,0,0,0,
								0,5-(mover * .5),0,9-(mover * .9));
		
		game[bob[6]]->SetSkew(0,0-(mover * .5),0,0-(mover * .9),
								0,0,0,0);
		
		game[bob[7]]->SetSkew(0,0,0,0,
								0,0+(mover * .9),0,0+(mover * .5));
		
		game[bob[8]]->SetSkew(0-(mover * .4),-5-(mover * .9),0+(mover * .4),-9-(mover * 1.3),
								0+(mover * .4),0,0-(mover * .4),0);
		
		game[bob[9]]->SetSkew(0-(mover * .4),0,0+(mover * .4),0,
								0+(mover * .4),9+(mover * 1.3),0-(mover * .4),5+(mover * .9));
		
		game[bob[10]]->SetSkew(-4-(mover * .05),-14-(mover * 1.3),4+(mover * .05),-22-(mover * 2.7),
								4+(mover * .05),0,-4-(mover * .05),0);
		
		game[bob[11]]->SetSkew(-4-(mover * .05),0,4+(mover * .05),0,
								4+(mover * .05),22+(mover * 2.7),-4-(mover * .05),14+(mover * 1.3));
		
		game[bob[12]]->SetSkew(-4.5,-27,4.5,-49,4.5,0,-4.5,0);
		
		game[bob[13]]->SetSkew(-4.5,0,4.5,0,4.5,49,-4.5,27);
		game[bob[14]]->SetSkew(-4.5,-49,4.5,-27,4.5,0,-4.5,0);
		game[bob[15]]->SetSkew(-4.5,0,4.5,0,4.5,27,-4.5,49);
		}
		else if (rows==3)
		{
		game[bob[39]]->SetPosition(-42+(mover *5.5),58);
		game[bob[40]]->SetPosition(-42+(mover *5.5),153);
		game[bob[41]]->SetPosition(-42+(mover *5.5),250);
		
		game[bob[0]]->SetPosition(13+(mover *5.5),58+(mover *.9));
		game[bob[1]]->SetPosition(13+(mover *5.5),153);
		game[bob[2]]->SetPosition(13+(mover *5.5),250-(mover *1.1));
		
		game[bob[3]]->SetPosition(68+(mover *5.2),67+(mover *.7));
		game[bob[4]]->SetPosition(68+(mover *5.2),153);
		game[bob[5]]->SetPosition(68+(mover *5.2),239-(mover *.7));
		
		game[bob[6]]->SetPosition(120+(mover *5),74+(mover *.4));
		game[bob[7]]->SetPosition(120+(mover *5),153);
		game[bob[8]]->SetPosition(120+(mover *5),232-(mover *.4));
		
		game[bob[9]]->SetPosition(170+(mover *4.4),78+(mover *.2));
		game[bob[10]]->SetPosition(170+(mover *4.4),153);
		game[bob[11]]->SetPosition(170+(mover *4.4),228-(mover *.2));
		
		game[bob[12]]->SetPosition(214+(mover *4.4),80+(mover *.1));
		game[bob[13]]->SetPosition(214+(mover *4.4),153);
		game[bob[14]]->SetPosition(214+(mover *4.4),226-(mover *.2));
		
		game[bob[15]]->SetPosition(258+(mover *4.4),81);
		game[bob[16]]->SetPosition(258+(mover *4.4),153);
		game[bob[17]]->SetPosition(258+(mover *4.4),224-(mover *.1));
		
		game[bob[18]]->SetPosition(302+(mover *4.4),81);
		game[bob[19]]->SetPosition(302+(mover *4.4),153);
		game[bob[20]]->SetPosition(302+(mover *4.4),223);
		
		game[bob[21]]->SetPosition(346+(mover *4.4),81-(mover *.1));
		game[bob[22]]->SetPosition(346+(mover *4.4),153);
		game[bob[23]]->SetPosition(346+(mover *4.4),223+(mover *.2));
		
		game[bob[24]]->SetPosition(390+(mover *4.4),80-(mover *.3));
		game[bob[25]]->SetPosition(390+(mover *4.4),153);
		game[bob[26]]->SetPosition(390+(mover *4.4),225+(mover *.2));
		
		game[bob[27]]->SetPosition(434+(mover *5),77-(mover *.4));
		game[bob[28]]->SetPosition(434+(mover *5),153);
		game[bob[29]]->SetPosition(434+(mover *5),227+(mover *.4));
		
		game[bob[30]]->SetPosition(484+(mover *5.3),73-(mover *.6));
		game[bob[31]]->SetPosition(484+(mover *5.3),153);
		game[bob[32]]->SetPosition(484+(mover *5.3),231+(mover *.8));
		
		game[bob[33]]->SetPosition(537+(mover *5.4),67-(mover *.9));
		game[bob[34]]->SetPosition(537+(mover *5.4),153);
		game[bob[35]]->SetPosition(537+(mover *5.4),239+(mover *1.1));
		
		game[bob[36]]->SetPosition(591+(mover *5.4),58);
		game[bob[37]]->SetPosition(591+(mover *5.4),153);
		game[bob[38]]->SetPosition(591+(mover *5.4),250);
		
		game[bob[39]]->SetSkew(-38,-110,15,-42,15,65,-38,32);
		game[bob[40]]->SetSkew(-38,-75,15,-48,15,45,-38,72);
		game[bob[41]]->SetSkew(-38,-52,15,-70,15,27,-38,100);
		
		game[bob[0]]->SetSkew(-38,-110+(mover * .4),15,-42+(mover * 1.8),15,65-(mover * 2.5),-38,32-(mover * .5));
		game[bob[1]]->SetSkew(-38,-75+(mover * 2.5),15,-48+(mover * 1.3),15,45-(mover * .5),-38,72-(mover * 2.2));
		game[bob[2]]->SetSkew(-38,-52+(mover * 1.8),15,-70+(mover * 2.3),15,27+(mover * .3),-38,100-(mover * 4.2));
		
		game[bob[3]]->SetSkew(-38+(mover * 1.1),-70+(mover * 1.5),15+(mover * .4),-24-(mover * .2),15+(mover * .4),40-(mover * 1),-38+(mover * 1.1),27-(mover * .5));
		game[bob[4]]->SetSkew(-38+(mover * 1.1),-50+(mover * 1),15+(mover * .4),-35-(mover * .5),15+(mover * .4),40-(mover * 1),-38+(mover * 1.1),50-(mover * 1));
		game[bob[5]]->SetSkew(-38+(mover * 1.1),-34+(mover * 1.4),15+(mover * .4),-47+(mover * 1.7),15+(mover * .4),24-(mover * .4),-38+(mover * 1.1),58-(mover * .8));
		
		game[bob[6]]->SetSkew(-27+(mover * .8),-55+(mover * 1.7),19-(mover * .9),-22+(mover * .5),19-(mover * .9),30-(mover * 1.5),-27+(mover * .8),22-(mover * 1.2));
		game[bob[7]]->SetSkew(-27+(mover * .8),-40+(mover * 1),19-(mover * .9),-30+(mover * 1),19-(mover * .9),30-(mover * 1.8),-27+(mover * .8),40-(mover * 1));
		game[bob[8]]->SetSkew(-27+(mover * .8),-20+(mover * .5),19-(mover * .9),-30+(mover * 1),19-(mover * .9),20-(mover * 1),-27+(mover * .8),50-(mover * 2.6));
		
		game[bob[9]]->SetSkew(-19+(mover * .9),-28+(mover * .8),0+(mover * .3),-17-(mover * .4),0+(mover * .3),15-(mover * 1),-19+(mover * .9),10);
		game[bob[10]]->SetSkew(-19+(mover * .9),-30+(mover * 1),0+(mover * .3),-20-(mover * .2),0+(mover * .3),12+(mover * .6),-19+(mover * .9),30-(mover * 1));
		game[bob[11]]->SetSkew(-19+(mover * .9),-15+(mover * .5),0+(mover * .3),-20+(mover * 1),0+(mover * .3),10,-19+(mover * .9),24-(mover * 1.4));
		
		game[bob[12]]->SetSkew(-10,-20+(mover * .5),3,-13+(mover * .1),3,14-(mover * .1),-10,10+(mover * .3));
		game[bob[13]]->SetSkew(-10,-20+(mover * .3),3,-18+(mover * .8),3,18-(mover * .8),-10,20-(mover * .3));
		game[bob[14]]->SetSkew(-10,-10,3,-10-(mover * .5),3,0+(mover * 1),-10,10);
		
		game[bob[15]]->SetSkew(-10,-15+(mover * .5),3,-12+(mover * .2),3,13+(mover * .1),-10,13+(mover * .1));
		game[bob[16]]->SetSkew(-10,-17+(mover * .7),3,-10,3,10,-10,17-(mover * .7));
		game[bob[17]]->SetSkew(-10,-10,3,-15+(mover * .5),3,10,-10,10);
		
		game[bob[18]]->SetSkew(-10,-10,3+(mover * .1),-10-(mover * 1),3,14-(mover * .4),-10-(mover * .4),14-(mover * .4));
		game[bob[19]]->SetSkew(-10,-10,3+(mover * .1),-10-(mover * .7),3,10+(mover * .7),-10-(mover * .4),10);//middle
		game[bob[20]]->SetSkew(-10,-10,3+(mover * .1),-10,3,10,-10-(mover * .4),10);
		
		game[bob[21]]->SetSkew(-14+(mover * .4),-10-(mover * .3),4-(mover * .1),-20,3,10+(mover * .4),-14,10);
		game[bob[22]]->SetSkew(-14+(mover * .4),-10-(mover * .8),4-(mover * .1),-17-(mover * .3),3,17+(mover * .3),-14,10+(mover * .8));
		game[bob[23]]->SetSkew(-14+(mover * .4),-10,4-(mover * .1),-10,3,10+(mover * 1),-14,10-(mover * .5));
		
		game[bob[24]]->SetSkew(-10-(mover * .9),-13,3-(mover * .3),-20,3-(mover * .3),14,-10-(mover * .9),10);
		game[bob[25]]->SetSkew(-10-(mover * .9),-18,3-(mover * .3),-20,3-(mover * .3),20,-10-(mover * .9),18);
		game[bob[26]]->SetSkew(-10-(mover * .9),-10,3-(mover * .3),-10,3-(mover * .3),20,-10-(mover * .9),5);
		
		game[bob[27]]->SetSkew(-19-(mover * .8),-17,0+(mover * 1.9),-28,0+(mover * 1.9),10+(mover * 1.2),-19-(mover * .8),15+(mover * 1.5));
		game[bob[28]]->SetSkew(-19-(mover * .8),-20,0+(mover * 1.9),-30,0+(mover * 1.9),30+(mover * 1),-19-(mover * .8),12+(mover * 1.8));
		game[bob[29]]->SetSkew(-19-(mover * .8),-20,0+(mover * 1.9),-15,0+(mover * 1.9),30+(mover * 1.5),-19-(mover * .8),10+(mover * 1));
		
		game[bob[30]]->SetSkew(-27-(mover * 1.1),-22-(mover * .2),19-(mover * .4),-55-(mover * 1.5),19-(mover * .4),22+(mover * .5),-27-(mover * 1.1),30+(mover * 1));
		game[bob[31]]->SetSkew(-27-(mover * 1.1),-30-(mover * .5),19-(mover * .4),-40-(mover * 1),19-(mover * .4),40+(mover * 1),-27-(mover * 1.1),30+(mover * 1));
		game[bob[32]]->SetSkew(-27-(mover * 1.1),-30-(mover * 1.7),19-(mover * .4),-20-(mover * 1.4),19-(mover * .4),55+(mover * .3),-27-(mover * 1.1),20+(mover * .4));
		
		game[bob[33]]->SetSkew(-38,-24-(mover * 1.8),15,-70-(mover * 4),15,27+(mover * .5),-38,40+(mover * .2));
		game[bob[34]]->SetSkew(-38,-35-(mover * 1.3),15,-50-(mover * 2.5),15,50+(mover * 2),-38,40+(mover * .5));
		game[bob[35]]->SetSkew(-38,-47-(mover * 2.7),15,-34-(mover * 1.8),15,58+(mover * 4.2),-38,24+(mover * .3));
		
		game[bob[36]]->SetSkew(-38,-42+(mover * 1.8),15,-110,15,32,-38,60);
		game[bob[37]]->SetSkew(-38,-48+(mover * 1.3),15,-75,15,70,-38,45);
		game[bob[38]]->SetSkew(-38,-70+(mover * 2.3),15,-52+(mover * 1.8),15,100,-38,27);
		}
		}
		mover++;
		goRight--;
		
		}
		else {goRight=0;mover=0;
		listOffset = (listOffset-rows < 0) ? gameCnt-rows : listOffset-rows;
		firstPic = (firstPic-rows < 0) ? pagesize-rows : firstPic-rows;

		for(int i=0; i<pagesize; i++) {
			bob[i] = (firstPic+i)%pagesize;
			
		//snprintf(debugbuffer, sizeof(debugbuffer), "listOffset: %i  firstPic: %i 3: %i   2: %i   1: %i", listOffset,firstPic,((listOffset -3) % gameCnt),((listOffset -2) % gameCnt),((listOffset -1) % gameCnt));
		//debugTxt->SetText(debugbuffer);
		
		}
		}
	}
	
	
	int ttoffset=0;
	if (rows==1)ttoffset=70;
	if (rows==2)ttoffset=35;
	char titlebuffer[50];
	int selected = this->GetSelectedOption();
	//3 different loops here with different alignment for tooltips
	//depending on where on the screen the game is
	for(int i=0; i < (pagesize/3); i++) {
		game[i]->RemoveToolTip();
		
		if (game[bob[i]]->GetState()==STATE_SELECTED)
		{
		
		game[bob[i]]->SetToolTip(titleTT,ttoffset,0,0,5);
		}
	}	
	for(int i=(pagesize/3); i < (2* pagesize/3); i++) {
		game[i]->RemoveToolTip();
		
		if (game[bob[i]]->GetState()==STATE_SELECTED)
		{
		game[bob[i]]->SetToolTip(titleTT,0,0,2,5);
		isover=true;
		}
	}	
	for(int i=(2* pagesize/3); i < pagesize; i++) {
		game[i]->RemoveToolTip();
		
		if (game[bob[i]]->GetState()==STATE_SELECTED)
		{
		game[bob[i]]->SetToolTip(titleTT,-ttoffset,0,1,5);
		}
	}	
	snprintf(titlebuffer, sizeof(titlebuffer), "%s",get_title(&gameList[this->GetSelectedOption()]));
	if (selected!=selectedOld){
		delete titleTT;
		titleTT = new GuiTooltip(titlebuffer);
		titleTT->SetAlpha(175);
		wait=0;wait1=0;
	}
	selectedOld=selected;
	if (wait1==0){wait++;if(wait>500)wait1=1;}//500 *2 is the time that the tooltips stay on screen
	if ((wait1==1)&&(wait>-1)){wait--;}
	
		
	//snprintf(debugbuffer, sizeof(debugbuffer), "faggot %i %s", GetOverImage(t),get_title(&gameList[this->GetSelectedOption()]));
	//debugTxt->SetText(debugbuffer);
	if ((btnRowUp->GetState() == STATE_CLICKED)&&(c>0)) {
		if ((rows==1)&&(gameCnt>=16))this->ChangeRows(2);
		else if ((rows==2)&&(gameCnt>=42))this->ChangeRows(3);
		btnRowUp->ResetState();
		return;
	}

	if ((btnRowDown->GetState() == STATE_CLICKED)&&(c>0)) {
		if (rows==3)this->ChangeRows(2);
		else if (rows==2)this->ChangeRows(1);
		btnRowDown->ResetState();
		return;
	}

	if(updateCB)
		updateCB(this);
}


void GuiGameGrid::Reload(struct discHdr * l, int count)
{
	for(int i=0; i<42; i++) {
		delete game[i];
	}
	for(int i=0; i<gameCnt; i++) {
		delete coverImg[i];
		delete cover[i];
	}

	//delete [] bob;
	delete [] game;
	delete [] coverImg;
	delete [] cover;

	LOCK(this);

	gameList = l;
	listOffset = 0;
	selectable = true;
	selectedItem = 0;
	focus = 1;					 // allow focus
	firstPic = 0;
	clickedItem = -1;
	gameCnt=count;
	//speed = SHIFT_SPEED;
	char imgPath[100];
	
	if (count<42)rows=2;
	if (count<16)rows=1;
	//rows=1;
	if (rows==1)pagesize = 6;
	else if (rows==2)pagesize = 16;
	else if (rows==3)pagesize = 42;
	
	game = new GuiButton * [pagesize];
	//bob = new int[pagesize];
	coverImg = new GuiImage * [gameCnt];
	cover = new GuiImageData * [gameCnt];

	for(int i=0; i<pagesize; i++) {
		bob[i]=i;
	}
	
	firstPic=0;

	char ID[4];
	char IDfull[7];

	for(int i=0; i < gameCnt; i++) {

		struct discHdr *header = &gameList[i];
		snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
		snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

		snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, IDfull); //Load full id image
		cover[i] = new GuiImageData(imgPath,0);
		if (!cover[i]->GetImage()) {
			delete cover[i];
			snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID); //Load short id image
			cover[i] = new GuiImageData(imgPath, 0);
			if (!cover[i]->GetImage()) {
				delete cover[i];
				snprintf(imgPath, sizeof(imgPath), "%snoimage.png", Settings.covers_path); //Load no image
				cover[i] = new GuiImageData(imgPath, nocoverFlat_png);
			}
		}

		coverImg[i] = new GuiImage(cover[i]);
		coverImg[i]->SetWidescreen(CFG.widescreen);
		if (rows==2)coverImg[i]->SetScale(.6);//these are the numbers for 2 rows
		else if (rows==3)coverImg[i]->SetScale(.26);//these are the numbers for 3 rows
		
	}
	
	for(int i=0; i < pagesize; i++) {
		game[i] = new GuiButton(160,224);//for 1 row
		if (rows==2)game[i]->SetSize(75,133);//these are the numbers for 2 rows
		else if (rows==3)game[i]->SetSize(35,68);//these are the numbers for 3 rows
		game[i]->SetParent(this);
		game[i]->SetAlignment(ALIGN_TOP,ALIGN_LEFT);
		game[i]->SetPosition(-200,740);
		game[i]->SetImage(coverImg[(listOffset+i) % gameCnt]);
		if (rows==3)coverImg[(listOffset+i) % gameCnt]->SetPosition(0,-80);// only for 3 rows
		if (rows==2)coverImg[(listOffset+i) % gameCnt]->SetPosition(0,-50);// only for 2 rows
		game[i]->SetRumble(false);
		game[i]->SetTrigger(trigA);
		game[i]->SetSoundClick(btnSoundClick);
		game[i]->SetClickable(true);
	}
	for (int i=gameCnt-1;i<pagesize;i++){ //hide games if gameCnt is less than the number of onscreen boxes 
            game[i]->SetVisible(false);
            game[i]->SetClickable(false);
			game[i]->RemoveSoundOver();}
		
	if(CFG.widescreen)
		
		if (rows==1)
		{
		game[bob[0]]->SetPosition(-70,74);
		game[bob[1]]->SetPosition(88,74);
		game[bob[2]]->SetPosition(239,74);
		game[bob[3]]->SetPosition(390,74);
		game[bob[4]]->SetPosition(550,74);
		
		game[bob[0]]->SetSkew(-10,-44,10,-26,10,26,-10,44);
		game[bob[1]]->SetSkew(-6,-22,6,-14,6,14,-6,22);
		game[bob[2]]->SetSkew(0,-11,0,-11,0,11,0,11);
		game[bob[3]]->SetSkew(-6,-14,6,-22,6,22,-6,14);
		game[bob[4]]->SetSkew(-10,-26,10,-44,10,44,-10,26);
		}
		else if (rows ==2)
		{
		game[bob[0]]->SetPosition(3,50);
		game[bob[1]]->SetPosition(3,193);
		game[bob[2]]->SetPosition(97,50);
		game[bob[3]]->SetPosition(97,193);
		game[bob[4]]->SetPosition(187,50);
		game[bob[5]]->SetPosition(187,193);
		game[bob[6]]->SetPosition(272,50);
		game[bob[7]]->SetPosition(272,193);
		game[bob[8]]->SetPosition(358,50);
		game[bob[9]]->SetPosition(358,193);
		game[bob[10]]->SetPosition(449,50);
		game[bob[11]]->SetPosition(449,193);
		game[bob[12]]->SetPosition(545,50);
		game[bob[13]]->SetPosition(545,193);
		game[bob[14]]->SetPosition(700,0);
		game[bob[15]]->SetPosition(700,0);
		
		
		game[bob[0]]->SetSkew(-4.5,-49,4.5,-27,4.5,0,-4.5,0);
		game[bob[1]]->SetSkew(-4.5,0,4.5,0,4.5,27,-4.5,49);
		game[bob[2]]->SetSkew(-4,-22,4,-14,4,0,-4,0);
		game[bob[3]]->SetSkew(-4,0,4,0,4,14,-4,22);
		game[bob[4]]->SetSkew(0,-9,0,-5,0,0,0,0);
		game[bob[5]]->SetSkew(0,0,0,0,0,5,0,9);
		game[bob[6]]->SetSkew(0,0,0,0,0,0,0,0);
		game[bob[7]]->SetSkew(0,0,0,0,0,0,0,0);
		game[bob[8]]->SetSkew(0,-5,0,-9,0,0,0,0);
		game[bob[9]]->SetSkew(0,0,0,0,0,9,0,5);
		game[bob[10]]->SetSkew(-4,-14,4,-22,4,0,-4,0);
		game[bob[11]]->SetSkew(-4,0,4,0,4,22,-4,14);
		game[bob[12]]->SetSkew(-4.5,-27,4.5,-49,4.5,0,-4.5,0);
		game[bob[13]]->SetSkew(-4.5,0,4.5,0,4.5,49,-4.5,27);
		}
		
		else if (rows==3)
		{
		game[bob[0]]->SetPosition(13,58);
		game[bob[1]]->SetPosition(13,153);
		game[bob[2]]->SetPosition(13,250);
		
		game[bob[3]]->SetPosition(68,67);
		game[bob[4]]->SetPosition(68,153);
		game[bob[5]]->SetPosition(68,239);
		
		game[bob[6]]->SetPosition(120,74);
		game[bob[7]]->SetPosition(120,153);
		game[bob[8]]->SetPosition(120,232);
		
		game[bob[9]]->SetPosition(170,78);
		game[bob[10]]->SetPosition(170,153);
		game[bob[11]]->SetPosition(170,228);
		
		game[bob[12]]->SetPosition(214,80);
		game[bob[13]]->SetPosition(214,153);
		game[bob[14]]->SetPosition(214,226);
		
		game[bob[15]]->SetPosition(258,81);
		game[bob[16]]->SetPosition(258,153);
		game[bob[17]]->SetPosition(258,224);
		
		game[bob[18]]->SetPosition(302,81);
		game[bob[19]]->SetPosition(302,153);
		game[bob[20]]->SetPosition(302,223);
		
		game[bob[21]]->SetPosition(346,81);
		game[bob[22]]->SetPosition(346,153);
		game[bob[23]]->SetPosition(346,223);
		
		game[bob[24]]->SetPosition(390,80);
		game[bob[25]]->SetPosition(390,153);
		game[bob[26]]->SetPosition(390,225);
		
		game[bob[27]]->SetPosition(434,77);
		game[bob[28]]->SetPosition(434,153);
		game[bob[29]]->SetPosition(434,227);
		
		game[bob[30]]->SetPosition(484,73);
		game[bob[31]]->SetPosition(484,153);
		game[bob[32]]->SetPosition(484,231);
		
		game[bob[33]]->SetPosition(537,67);
		game[bob[34]]->SetPosition(537,153);
		game[bob[35]]->SetPosition(537,239);
		
		game[bob[36]]->SetPosition(591,58);
		game[bob[37]]->SetPosition(591,153);
		game[bob[38]]->SetPosition(591,250);
		
		game[bob[0]]->SetSkew(-38,-110,15,-42,15,65,-38,32);
		game[bob[1]]->SetSkew(-38,-75,15,-48,15,45,-38,72);
		game[bob[2]]->SetSkew(-38,-52,15,-70,15,27,-38,100);
		
		game[bob[3]]->SetSkew(-38,-70,15,-24,15,40,-38,27);
		game[bob[4]]->SetSkew(-38,-50,15,-35,15,40,-38,50);
		game[bob[5]]->SetSkew(-38,-34,15,-47,15,24,-38,58);
		
		game[bob[6]]->SetSkew(-27,-55,19,-22,19,30,-27,22);
		game[bob[7]]->SetSkew(-27,-40,19,-30,19,30,-27,40);
		game[bob[8]]->SetSkew(-27,-20,19,-30,19,20,-27,50);
		
		game[bob[9]]->SetSkew(-19,-28,0,-17,0,15,-19,10);
		game[bob[10]]->SetSkew(-19,-30,0,-20,0,12,-19,30);
		game[bob[11]]->SetSkew(-19,-15,0,-20,0,10,-19,24);
		
		game[bob[12]]->SetSkew(-10,-20,3,-13,3,14,-10,10);
		game[bob[13]]->SetSkew(-10,-20,3,-18,3,18,-10,20);
		game[bob[14]]->SetSkew(-10,-10,3,-10,3,0,-10,10);
		
		game[bob[15]]->SetSkew(-10,-15,3,-12,3,13,-10,13);
		game[bob[16]]->SetSkew(-10,-17,3,-10,3,10,-10,17);
		game[bob[17]]->SetSkew(-10,-10,3,-15,3,10,-10,10);
		
		game[bob[18]]->SetSkew(-10,-10,3,-10,3,14,-10,14);
		game[bob[19]]->SetSkew(-10,-10,3,-10,3,10,-10,10);//middle
		game[bob[20]]->SetSkew(-10,-10,3,-10,3,10,-10,10);
		
		game[bob[21]]->SetSkew(-14,-10,4,-20,3,10,-14,10);
		game[bob[22]]->SetSkew(-14,-10,4,-17,3,17,-14,10);
		game[bob[23]]->SetSkew(-14,-10,4,-10,3,10,-14,10);
		
		game[bob[24]]->SetSkew(-10,-13,3,-20,3,14,-10,10);
		game[bob[25]]->SetSkew(-10,-18,3,-20,3,20,-10,18);
		game[bob[26]]->SetSkew(-10,-10,3,-10,3,20,-10,5);
		
		game[bob[27]]->SetSkew(-19,-17,0,-28,0,10,-19,15);
		game[bob[28]]->SetSkew(-19,-20,0,-30,0,30,-19,12);
		game[bob[29]]->SetSkew(-19,-20,0,-15,0,30,-19,10);
		
		game[bob[30]]->SetSkew(-27,-22,19,-55,19,22,-27,30);
		game[bob[31]]->SetSkew(-27,-30,19,-40,19,40,-27,30);
		game[bob[32]]->SetSkew(-27,-30,19,-20,19,55,-27,20);
		
		game[bob[33]]->SetSkew(-38,-24,15,-70,15,27,-38,40);
		game[bob[34]]->SetSkew(-38,-35,15,-50,15,50,-38,40);
		game[bob[35]]->SetSkew(-38,-47,15,-34,15,58,-38,24);
		
		game[bob[36]]->SetSkew(-38,-42,15,-110,15,32,-38,60);
		game[bob[37]]->SetSkew(-38,-48,15,-75,15,70,-38,45);
		game[bob[38]]->SetSkew(-38,-70,15,-52,15,100,-38,27);
		}
		
	
}

