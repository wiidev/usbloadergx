/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h> //CLOCK
#include <time.h> //CLOCK
#include <dirent.h>

#include "libwiigui/gui.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "http.h"
#include "dns.h"
#include "partition.h"
#include "wbfs.h"
#include "utils.h"
#include "usbstorage.h"
#include "disc.h"
#include "filelist.h"
#include "wdvd.h"
#include "libwbfs/libwbfs.h"
#include "sys.h"
#include "patchcode.h"
#include "wpad.h"
#include "cfg.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "libwiigui/gui_gamebrowser.h"

#define MAX_CHARACTERS		38

extern FreeTypeGX *fontClock; //CLOCK

static GuiImage * coverImg = NULL;
static GuiImageData * cover = NULL;

//char GamesHDD[320][14];

static struct discHdr *gameList = NULL;
static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
static GuiButton * btnLogo = NULL;
static GuiImageData * background = NULL;
static char prozent[10] = "0%";
static char timet[50] = " ";
static char sizeshow[20] = " ";
static GuiText prTxt(prozent, 26, (GXColor){0, 0, 0, 255});
static GuiText timeTxt(prozent, 26, (GXColor){0, 0, 0, 255});
static GuiText sizeTxt(sizeshow, 26, (GXColor){0, 0, 0, 255});
static GuiText *GameIDTxt = NULL;
static GuiText *GameRegionTxt = NULL;
static GuiSound * bgMusic = NULL;
static GuiSound * creditsMusic = NULL;
static wbfs_t *hdd = NULL;
static u32 gameCnt = 0;
static s32 gameSelected = 0, gameStart = 0;
static GuiWindow * mainWindow = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static GuiImageData progressbar(progressbar_png);
static GuiImage progressbarImg(&progressbar);
int godmode = 0;
int height = 224;
int width = 160;
static float gamesize = 0.00;
static int startat = 0;
static int offset = 0, networkisinitialized = 0;
int vol = Settings.volume;

//downloadvariables
static char missingFiles[500][12]; //fixed
static int cntMissFiles = 0;

int direction = 0; // direction the gameprompt slides in

static char gameregion[7];
//power button fix
extern u8 shutdown;
extern u8 reset;

//Wiilight stuff
static vu32 *_wiilight_reg = (u32*)0xCD0000C0;
void wiilight(int enable){             // Toggle wiilight (thanks Bool for wiilight source)
    u32 val = (*_wiilight_reg&~0x20);
    if(enable) val |= 0x20;
    *_wiilight_reg=val;}

//Prototypes
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *btn3Label, const char *btn4Label);
static void HaltGui();
static void ResumeGui();


//libfat helper functions
int isSdInserted() {    return __io_wiisd.isInserted(); }

//Initialise SD CARD
int SDCard_Init()
{
    __io_wiisd.startup();
    if (!isSdInserted()){
        printf("No SD card inserted!");
        return -1;

    }    if (!fatMountSimple ("SD", &__io_wiisd)){
        printf("Failed to mount front SD card!");
        return -1;
    }

    return 1;
}

void SDCARD_deInit()
{
    //First unmount all the devs...
    fatUnmount ("SD");
    //...and then shutdown em!
    __io_wiisd.shutdown();
}

bool findfile(const char * filename, const char * path)
{
DIR *dir;
struct dirent *file;

dir = opendir(path);

char temp[11];
while ((file = readdir(dir)))
{
	snprintf(temp,sizeof(temp),"%s",file->d_name);
    if (!strncmpi(temp,filename,11))
		{
		//WindowPrompt(path, filename,"go" ,0);
		closedir(dir);
		return true;
		}
	}
  closedir(dir);
  return false;
}

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
static void
ResumeGui()
{
	guiHalt = false;
	LWP_ResumeThread (guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
static void
HaltGui()
{
	guiHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(50);
}

/****************************************************************************
 * WindowCredits
 * Display credits
 ***************************************************************************/
static void WindowCredits(void * ptr)
{
	int angle = 0;

	if(btnLogo->GetState() != STATE_CLICKED) {
		return;
		}

	bgMusic->Stop();
	creditsMusic = new GuiSound(credits_music_ogg, credits_music_ogg_size, SOUND_OGG, 55);
	creditsMusic->SetVolume(55);
	creditsMusic->SetLoop(1);
	creditsMusic->Play();

	btnLogo->ResetState();

	bool exit = false;
	int i = 0;
	int y = 95;

	GuiWindow creditsWindow(screenwidth,screenheight);
	GuiWindow creditsWindowBox(580,448);
	creditsWindowBox.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	GuiImageData creditsBox(credits_bg_png);
	GuiImage creditsBoxImg(&creditsBox);
	creditsBoxImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	creditsWindowBox.Append(&creditsBoxImg);

	GuiImageData star(little_star_png);
	GuiImage starImg(&star);
	starImg.SetWidescreen(CFG.widescreen); //added
	starImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	starImg.SetPosition(500,335);

	int numEntries = 15;
	GuiText * txt[numEntries];

	txt[i] = new GuiText("Official Site: http://code.google.com/p/usbloader-gui/", 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y); i++; y+=26;

	txt[i]->SetPresets(22, (GXColor){255, 255, 255,  255}, 0,
			FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP, ALIGN_LEFT, ALIGN_TOP);

	txt[i] = new GuiText("Coding:");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(100,y);
	i++;

	txt[i] = new GuiText("dimok");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=22;

	txt[i] = new GuiText("nIxx");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=22;

	txt[i] = new GuiText("hungyip84");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=22;

	txt[i] = new GuiText("giantpune");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=24;

	txt[i] = new GuiText("Design:");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(100,y);
	i++;

	txt[i] = new GuiText("cyrex");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=22;

	txt[i] = new GuiText("NeoRame");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=22;

	txt[i] = new GuiText("WiiShizza");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=24;

	txt[i] = new GuiText("Ocarina & Vidpatch thanks to:");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(100,y);
	i++;
	y+=22;

	txt[i] = new GuiText("Fishears & WiiPower");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=26;

	txt[i] = new GuiText("Special thanks to Tantric for libwiigui");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y);
	i++;
	y+=22;

	txt[i] = new GuiText("and to Waninkoko & Kwiirk for the USB Loader ");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y);
	i++;
	y+=22;

	txt[i] = new GuiText("and releasing the source code ;)");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y);
	i++;
	y+=22;

	for(i=0; i < numEntries; i++)
		creditsWindowBox.Append(txt[i]);


	creditsWindow.Append(&creditsWindowBox);
	creditsWindow.Append(&starImg);

	while(!exit)
	{
		creditsWindow.Draw();

		angle ++;
		angle = int(angle) % 360;
		usleep(12000);
		starImg.SetAngle(angle);

		for(i=3; i >= 0; i--)
		{
			#ifdef HW_RVL
			if(userInput[i].wpad.ir.valid)
				Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48,
					96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, CFG.widescreen? 0.8 : 1, 1, 255);
			if(Settings.rumble == RumbleOn){
				DoRumble(i);
				}
			#endif
		}

		Menu_Render();

		for(i=0; i < 4; i++)
		{
			if(userInput[i].wpad.btns_d || userInput[i].pad.btns_d)
				exit = true;
		}
	}

	// clear buttons pressed
	for(i=0; i < 4; i++)
	{
		userInput[i].wpad.btns_d = 0;
		userInput[i].pad.btns_d = 0;
	}
	creditsMusic->Stop();
	for(i=0; i < numEntries; i++)
		delete txt[i];

	delete creditsMusic;
	bgMusic->SetLoop(1);
	bgMusic->Play();
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 4 Buttons.
 *
 * Give him 1 Titel, 1 Subtitel and 4 Buttons
 * If titel/subtitle or one of the buttons is not needed give him a 0 on that
 * place.
 ***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label,
                const char *btn2Label, const char *btn3Label,
                const char *btn4Label)
{
	int choice = -1;

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);
	GuiImageData btnOutline(button_dialogue_box_png);


	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}///////////

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,55);
	GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-40);
	msgTxt.SetMaxWidth(430);

	GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnClick);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn2Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetSoundOver(&btnSoundOver);
	btn2.SetSoundClick(&btnClick);
	if(!btn3Label && !btn4Label)
	btn2.SetTrigger(&trigB);
	btn2.SetTrigger(&trigA);
	btn2.SetEffectGrow();

    GuiText btn3Txt(btn3Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn3Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn3Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn3(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn3.SetLabel(&btn3Txt);
	btn3.SetImage(&btn3Img);
	btn3.SetSoundOver(&btnSoundOver);
	btn3.SetSoundClick(&btnClick);
	if(!btn4Label)
	btn3.SetTrigger(&trigB);
	btn3.SetTrigger(&trigA);
	btn3.SetEffectGrow();

    GuiText btn4Txt(btn4Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn4Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn4Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn4(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn4.SetLabel(&btn4Txt);
	btn4.SetImage(&btn4Img);
	btn4.SetSoundOver(&btnSoundOver);
	btn4.SetSoundClick(&btnClick);
	if(btn4Label)
	btn4.SetTrigger(&trigB);
	btn4.SetTrigger(&trigA);
	btn4.SetEffectGrow();

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust buttons for widescreen
		msgTxt.SetMaxWidth(330);
        btn1Txt.SetFontSize(20);
		btn2Txt.SetFontSize(20);
		btn3Txt.SetFontSize(20);
		btn4Txt.SetFontSize(20);

		if(btn2Label && !btn3Label && !btn4Label)
        {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(70, -80);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -80);
            btn3.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn3.SetPosition(-70, -55);
            btn4.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn4.SetPosition(70, -55);
        } else if(btn2Label && btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(70, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -120);
            btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn3.SetPosition(0, -55);
            btn4.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn4.SetPosition(70, -55);
        } else if(btn2Label && btn3Label && btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(70, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(70, -55);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-70, -55);
        }   else if(!btn2Label && btn3Label && btn4Label) {
            btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn1.SetPosition(0, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(70, -55);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-70, -55);
        } else {
            btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn1.SetPosition(0, -80);
            btn2.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn2.SetPosition(70, -120);
            btn3.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn3.SetPosition(-70, -55);
            btn4.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn4.SetPosition(70, -55);
        }
	} else {

	    if(btn2Label && !btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(40, -45);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-40, -45);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(50, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
	    } else if(btn2Label && btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(50, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-50, -120);
            btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn3.SetPosition(0, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
	    } else if(btn2Label && btn3Label && btn4Label) {
	        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(50, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-50, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(50, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
	    } else if(!btn2Label && btn3Label && btn4Label) {
	        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn1.SetPosition(0, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-50, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(50, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
	    } else {
	        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn1.SetPosition(0, -45);
            btn2.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn2.SetPosition(50, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(50, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
	    }

	}

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	if(btn1Label)
	promptWindow.Append(&btn1);
	if(btn2Label)
		promptWindow.Append(&btn2);
    if(btn3Label)
		promptWindow.Append(&btn3);
    if(btn4Label)
		promptWindow.Append(&btn4);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	while(choice == -1)
	{
		VIDEO_WaitVSync();
		if(shutdown == 1)
		{
			wiilight(0);
			Sys_Shutdown();
		}
		if(reset == 1)
			Sys_Reboot();
		if(btn1.GetState() == STATE_CLICKED) {
			choice = 1;
		}
		else if(btn2.GetState() == STATE_CLICKED) {
		    if(!btn3Label)
			choice = 0;
			else
			choice = 2;
		}
		else if(btn3.GetState() == STATE_CLICKED) {
		    if(!btn4Label)
			choice = 0;
			else
			choice = 3;
		}
		else if(btn4.GetState() == STATE_CLICKED) {
			choice = 0;
		}
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(50);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return choice;
}

/****************************************************************************
 * GameWindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int GameWindowPrompt()
{
	int choice = -1, angle = 0;
	char sizeText[15];
	f32 size = 0.0;
	char ID[4];
	char IDFull[7];
	char gameName[CFG.maxcharacters + 4];

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);
	GuiImageData btnOutline(button_dialogue_box_png);
	GuiImageData imgLeft(startgame_arrow_left_png);
	GuiImageData imgRight(startgame_arrow_right_png);


	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	GuiTrigger trigL;
	trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	GuiTrigger trigR;
	trigR.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
    GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
	GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

	char imgPath[100];

	if (CFG.widescreen)
		snprintf(imgPath, sizeof(imgPath), "%swdialogue_box_startgame.png", CFG.theme_path);
	else
		snprintf(imgPath, sizeof(imgPath), "%sdialogue_box_startgame.png", CFG.theme_path);

	GuiImageData dialogBox(imgPath, CFG.widescreen ? wdialogue_box_startgame_png : dialogue_box_startgame_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText msgTxt("", 22, (GXColor){50, 50, 50, 255});
	GuiButton nameBtn(120,50);
	nameBtn.SetLabel(&msgTxt);
	nameBtn.SetLabelOver(&msgTxt);
	nameBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	nameBtn.SetPosition(0,-122);
	nameBtn.SetSoundOver(&btnSoundOver);
	nameBtn.SetSoundClick(&btnClick);

	if (CFG.godmode == 1){
		nameBtn.SetTrigger(&trigA);
		nameBtn.SetEffectGrow();
	}

    GuiText sizeTxt("", 22, (GXColor){50, 50, 50, 255}); //TODO: get the size here
	sizeTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	sizeTxt.SetPosition(-60,70);

	GuiImage diskImg;
	diskImg.SetWidescreen(CFG.widescreen);
	diskImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	diskImg.SetAngle(angle);

	GuiButton btn1(160, 160);
	btn1.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	btn1.SetPosition(0, -20);
	btn1.SetImage(&diskImg);

	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnClick);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	//btn1.SetEffectGrow(); just commented it out if anybody wants to use it again.

	GuiText btn2Txt("Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn2Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	//check if unlocked
	if (CFG.godmode == 1)
	{
		btn2.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn2.SetPosition(40, -40);
	}
	else
	{
		btn2.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn2.SetPosition(0, -40);
	}

	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetSoundOver(&btnSoundOver);
	btn2.SetSoundClick(&btnClick);
	btn2.SetTrigger(&trigB);
	btn2.SetTrigger(&trigA);
	btn2.SetEffectGrow();

	GuiText btn3Txt("Settings", 22, (GXColor){0, 0, 0, 255});
	GuiImage btn3Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn3Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn3(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn3.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn3.SetPosition(-50, -40);
	btn3.SetLabel(&btn3Txt);
	btn3.SetImage(&btn3Img);
	btn3.SetSoundOver(&btnSoundOver);
	btn3.SetSoundClick(&btnClick);
	btn3.SetTrigger(&trigA);
	btn3.SetEffectGrow();

	GuiImage btnLeftImg(&imgLeft);
	GuiButton btnLeft(imgLeft.GetWidth(), imgLeft.GetHeight());
	btnLeft.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	btnLeft.SetPosition(20, 0);
	btnLeft.SetImage(&btnLeftImg);
	btnLeft.SetSoundOver(&btnSoundOver);
	//btnLeft.SetSoundClick(&btnClick);
	btnLeft.SetTrigger(&trigA);
	btnLeft.SetTrigger(&trigL);
	btnLeft.SetTrigger(&trigMinus);
	btnLeft.SetEffectGrow();

	GuiImage btnRightImg(&imgRight);
	GuiButton btnRight(imgRight.GetWidth(), imgRight.GetHeight());
	btnRight.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	btnRight.SetPosition(-20, 0);
	btnRight.SetImage(&btnRightImg);
	btnRight.SetSoundOver(&btnSoundOver);
	//btnRight.SetSoundClick(&btnClick);
	btnRight.SetTrigger(&trigA);
	btnRight.SetTrigger(&trigR);
	btnRight.SetTrigger(&trigPlus);
	btnRight.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&nameBtn);
	promptWindow.Append(&sizeTxt);
	promptWindow.Append(&btn1);
	promptWindow.Append(&btn2);
	promptWindow.Append(&btnLeft);
	promptWindow.Append(&btnRight);

	//check if unlocked
	if (CFG.godmode == 1)
	{
    promptWindow.Append(&btn3);
	}

	short changed = 3;
	GuiImageData * diskCover = NULL;

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	while (changed)
	{
		if (changed == 1){
			promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
		}
		if (changed == 2){
			promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
		}

		changed = 0;
		//load disc image based or what game is seleted
		struct discHdr * header = &gameList[gameSelected];
		WBFS_GameSize(header->id, &size);

		snprintf(sizeText, sizeof(sizeText), "%.2fGB", size); //set size text

		snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
		snprintf (IDFull,sizeof(IDFull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

		//set name
		if (strlen(get_title(header)) < (u32)(CFG.maxcharacters + 3)) {
			sprintf(gameName, "%s", get_title(header));
		}
		else {
			strncpy(gameName, get_title(header), CFG.maxcharacters);
			gameName[CFG.maxcharacters] = '\0';
			strncat(gameName, "...", 3);
		}

		snprintf(imgPath,sizeof(imgPath),"%s%s.png", CFG.disc_path, ID); //changed to current id

		if (diskCover)
			delete diskCover;

		diskCover = new GuiImageData(imgPath,0);

		if (!diskCover->GetImage())
		{
			delete diskCover;
			snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.disc_path, IDFull); //changed to current full id
			diskCover = new GuiImageData(imgPath, 0);
			if (!diskCover->GetImage())
			{
				delete diskCover;
				diskCover = new GuiImageData(imgPath,nodisc_png);
			}
		}

		diskImg.SetImage(diskCover);
		sizeTxt.SetText(sizeText);
		msgTxt.SetText(gameName);

		HaltGui();
		mainWindow->SetState(STATE_DISABLED);
		mainWindow->Append(&promptWindow);
		mainWindow->ChangeFocus(&promptWindow);
		ResumeGui();

		float speedup = 1; //speedup increases while disc is selected

		while(choice == -1)
		{
			VIDEO_WaitVSync();
			//angle++;
			angle = int(angle+speedup) % 360;
			//disc speedup and slowdown
			if (btn1.GetState() == STATE_SELECTED) { //if mouse over
				if (speedup < 11) // speed up
				{
					speedup = (speedup+0.20);
				}
			}
			else //if not mouse over
			{
				if (speedup > 1) {speedup = (speedup-0.05);} //slow down
			}
			if (speedup < 1)
			{
				speedup = 1;
			}
			diskImg.SetAngle(angle);

			if(shutdown == 1) //for power button
			{
				wiilight(0);
				Sys_Shutdown();
			}
			if(reset == 1)
				Sys_Reboot();

			if(btn1.GetState() == STATE_CLICKED) { //boot
				choice = 1;
				SDCARD_deInit();
			}

			else if(btn2.GetState() == STATE_CLICKED) { //back
				choice = 0;
				promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
				mainWindow->SetState(STATE_DEFAULT);
				wiilight(0);
			}

			else if(btn3.GetState() == STATE_CLICKED) { //settings
				choice = 2;
				promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			}

			else if(nameBtn.GetState() == STATE_CLICKED) { //rename
				choice = 3;
				promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			}

			else if((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == no)){//next game
				promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
				changed = 1;
				btnClick.Play();
				gameSelected = (gameSelected + 1) % gameCnt;
				btnRight.ResetState();
				break;
			}

			else if((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == no)){//previous game
				promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
				changed = 2;
				btnClick.Play();
				gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
				btnLeft.ResetState();
				break;
			}

			else if((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == yes)){//previous game
				promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
				changed = 2;
				btnClick.Play();
				gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
				btnRight.ResetState();
				break;
			}

			else if((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == yes)){//netx game
				promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
				changed = 1;
				btnClick.Play();
				gameSelected = (gameSelected + 1) % gameCnt;
				btnLeft.ResetState();
				break;
			}
		}


		while(promptWindow.GetEffect() > 0) usleep(50);
		HaltGui();
		mainWindow->Remove(&promptWindow);

		ResumeGui();
	}
	delete diskCover;

	return choice;
}

/****************************************************************************
 * DiscWait
 ***************************************************************************/
int
DiscWait(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, int IsDeviceWait)
{
	int i = 30, ret = 0;
    u32 cover = 0;

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);
	GuiImageData btnOutline(button_dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}///////////

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);
	GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-40);
	msgTxt.SetMaxWidth(430);

	GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

	if(btn2Label)
	{
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn1.SetPosition(40, -45);
	}
	else
	{
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -45);
	}

	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnClick);
	btn1.SetTrigger(&trigB);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn2Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -25);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetSoundOver(&btnSoundOver);
	btn2.SetSoundClick(&btnClick);
	btn2.SetTrigger(&trigA);
	btn2.SetEffectGrow();

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust buttons for widescreen
		msgTxt.SetMaxWidth(380);
		if(btn2Label)
	{
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn2.SetPosition(-70, -80);
		btn1.SetPosition(70, -80);
	}
	else
	{
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -80);
	}
	}

    char timer[20];
	GuiText timerTxt(timer, 26, (GXColor){0, 0, 0, 255});
	timerTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	timerTxt.SetPosition(0,160);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	if(btn1Label)
	promptWindow.Append(&btn1);
	if(btn2Label)
		promptWindow.Append(&btn2);
    if(IsDeviceWait)
	promptWindow.Append(&timerTxt);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	if(IsDeviceWait) {
        while(i >= 0)
        {
            sprintf(timer, "%u secs left", i);
            timerTxt.SetText(timer);
            VIDEO_WaitVSync();
            if(Settings.cios == ios222) {
            ret = IOS_ReloadIOS(222);
            } else {
            ret = IOS_ReloadIOS(249);
            }
            sleep(1);
            ret = WBFS_Init(WBFS_DEVICE_USB);
            if(ret>=0)
            break;

            i--;
        }
	} else {
        while(!(cover & 0x2))
        {
            VIDEO_WaitVSync();
            if(btn1.GetState() == STATE_CLICKED) {
                btn1.ResetState();
                break;
            }
            ret = WDVD_GetCoverStatus(&cover);
            if (ret < 0)
                break;
        }
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(50);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return ret;
}

/****************************************************************************
 * FormatingPartition
 ***************************************************************************/
int
FormatingPartition(const char *title, partitionEntry *entry)
{
    int ret;
	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

	GuiImageData btnOutline(button_dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}///////////

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);


	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	VIDEO_WaitVSync();
	ret = WBFS_Format(entry->sector, entry->size);


	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(50);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return ret;
}


/****************************************************************************
 * NetworkInit
 ***************************************************************************/
int NetworkInitPromp(int choice2)
{
    char hostip[16];
    char * IP = NULL;
    s32 ret = -1;

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	GuiImageData btnOutline(button_dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}///////////

	GuiText titleTxt("Initializing Network", 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);

	char msg[20] = " ";
	GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-40);

    GuiText btn1Txt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Img.SetWidescreen(CFG.widescreen);}///////////
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -45);
	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnClick);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust buttons for widescreen
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -80);
	}

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);

	ResumeGui();

	while (!IP)
	{

        VIDEO_WaitVSync();

        ret = Net_Init(hostip);

		if (ret > 0) {
		IP = hostip;
		}

		if (ret <= 0) {
        msgTxt.SetText("Could not initialize network!");
		}

		if (IP && ret > 0) {
			sprintf(msg, "IP: %s", IP);
			msgTxt.SetText(msg);
			cntMissFiles = 0;
			u32 i = 0;
			char filename[11];

			bool found1 = false;/////add Ids of games that are missing covers to cntMissFiles
			bool found2 = false;
			for (i = 0; i < gameCnt && cntMissFiles < 500; i++)
			{
				struct discHdr* header = &gameList[i];
				if (choice2 != 3) {

					snprintf (filename,sizeof(filename),"%c%c%c.png", header->id[0], header->id[1], header->id[2]);
					found2 = findfile(filename, CFG.covers_path);
					snprintf(filename,sizeof(filename),"%c%c%c%c%c%c.png",header->id[0], header->id[1], header->id[2],
																		header->id[3], header->id[4], header->id[5]); //full id
					found1 = findfile(filename, CFG.covers_path);
					if (!found1 && !found2) //if could not find any image
					{
						snprintf(missingFiles[cntMissFiles],11,"%s",filename);
						cntMissFiles++;
					}
				}
				else if (choice2 == 3) {
					snprintf (filename,sizeof(filename),"%c%c%c.png", header->id[0], header->id[1], header->id[2]);
					found2 = findfile(filename, CFG.disc_path);
					snprintf(filename,sizeof(filename),"%c%c%c%c%c%c.png",header->id[0], header->id[1], header->id[2],
																		header->id[3], header->id[4], header->id[5]); //full id
					found1 = findfile(filename,CFG.disc_path);
					if (!found1 && !found2)
					{
						snprintf(missingFiles[cntMissFiles],11,"%s",filename);
						cntMissFiles++;
					}
				}
			}
			break;
		}

		if(btn1.GetState() == STATE_CLICKED) {
			IP = 0;
			ret = -1;
			break;
		}

    }
	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(50);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	return ret;
}

/****************************************************************************
 * ShowProgress
 *
 * Updates the variables used by the progress window for drawing a progress
 * bar. Also resumes the progress window thread if it is suspended.
 ***************************************************************************/
void
ShowProgress (s32 done, s32 total)
{

    static time_t start;
	static u32 expected;

    f32 percent; //, size;
	u32 d, h, m, s;

	//first time
	if (!done) {
		start    = time(0);
		expected = 300;
	}

	//Elapsed time
	d = time(0) - start;

	if (done != total) {
		//Expected time
		if (d)
			expected = (expected * 3 + d * total / done) / 4;

		//Remaining time
		d = (expected > d) ? (expected - d) : 0;
	}

	//Calculate time values
	h =  d / 3600;
	m = (d / 60) % 60;
	s =  d % 60;

	//Calculate percentage/size
	percent = (done * 100.0) / total;
	//size    = (hdd->wbfs_sec_sz / GB_SIZE) * total;

	sprintf(prozent, "%0.2f", percent);
    prTxt.SetText(prozent);
	//prTxt.SetFont(fontClock);
    sprintf(timet,"Time left: %d:%02d:%02d",h,m,s);
    timeTxt.SetText(timet);

    float gamesizedone = 0.00;

    gamesizedone = gamesize * done/total;

    sprintf(sizeshow,"%0.2fGB/%0.2fGB", gamesizedone, gamesize);
    sizeTxt.SetText(sizeshow);

//	timeTxt.SetFont(fontClock);
	if ((Settings.wsprompt == yes) && (CFG.widescreen)){
	progressbarImg.SetTile(80*done/total);}
	else {progressbarImg.SetTile(100*done/total);}

}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
int
ProgressWindow(const char *title, const char *msg)
{

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiImageData btnOutline(button_dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	GuiImageData progressbarOutline(progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	if (Settings.wsprompt == yes){
	progressbarOutlineImg.SetWidescreen(CFG.widescreen);}
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 40);

	GuiImageData progressbarEmpty(progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 40);
	progressbarEmptyImg.SetTile(100);

	GuiImageData progressbar(progressbar_png);

	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 40);

	GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);
	GuiText msgTxt(msg, 26, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,120);

	GuiText prsTxt("%", 26, (GXColor){0, 0, 0, 255});
	prsTxt.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	prsTxt.SetPosition(-188,40);

    timeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	timeTxt.SetPosition(275,-50);

    sizeTxt.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	sizeTxt.SetPosition(50, -50);

	prTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	prTxt.SetPosition(200, 40);

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
		progressbarOutlineImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
		progressbarOutlineImg.SetPosition(0, 40);
		progressbarEmptyImg.SetPosition(80,40);
		progressbarEmptyImg.SetTile(78);
		progressbarImg.SetPosition(80, 40);
		msgTxt.SetMaxWidth(380);

		timeTxt.SetPosition(250,-50);
		timeTxt.SetFontSize(22);
		sizeTxt.SetPosition(90, -50);
		sizeTxt.SetFontSize(22);
	}

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
    promptWindow.Append(&progressbarEmptyImg);
    promptWindow.Append(&progressbarImg);
    promptWindow.Append(&progressbarOutlineImg);
    promptWindow.Append(&prTxt);
	promptWindow.Append(&prsTxt);
    promptWindow.Append(&timeTxt);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();
	promptWindow.Append(&prTxt);
	promptWindow.Append(&sizeTxt);

    s32 ret;

    __Disc_SetLowMem();

    ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, ShowProgress, ONLY_GAME_PARTITION, 0);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	if (ret < 0) {
    return ret;
	}
	return 0;
}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
int
ProgressDownloadWindow(int choice2)
{

    int i = 0, cntNotFound = 0;
//    char filename[11];

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	GuiImageData btnOutline(button_dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	GuiImageData progressbarOutline(progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	if (Settings.wsprompt == yes){
	progressbarOutlineImg.SetWidescreen(CFG.widescreen);}
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 40);

	GuiImageData progressbarEmpty(progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 40);
	progressbarEmptyImg.SetTile(100);

	GuiImageData progressbar(progressbar_png);
	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 40);

	GuiText titleTxt("Downloading file:", 26, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);
    char msg[25] = " ";
	GuiText msgTxt(msg, 26, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,130);
	char msg2[15] = " ";
	GuiText msg2Txt(msg2, 26, (GXColor){0, 0, 0, 255});
	msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msg2Txt.SetPosition(0,100);

	prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	prTxt.SetPosition(0, 40);

    GuiText btn1Txt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Img.SetWidescreen(CFG.widescreen);}
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -45);
	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnClick);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
		progressbarOutlineImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
		progressbarOutlineImg.SetPosition(0, 40);
		progressbarEmptyImg.SetPosition(80,40);
		progressbarEmptyImg.SetTile(78);
		progressbarImg.SetPosition(80, 40);
	}

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&msg2Txt);
    promptWindow.Append(&progressbarEmptyImg);
    promptWindow.Append(&progressbarImg);
    promptWindow.Append(&progressbarOutlineImg);
    promptWindow.Append(&prTxt);
    promptWindow.Append(&btn1);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

    //check if directory exist and if not create one
    struct stat st;
    if(stat(CFG.covers_path, &st) != 0) {
        char dircovers[100];
        snprintf(dircovers,strlen(CFG.covers_path),"%s",CFG.covers_path);
        if (mkdir(dircovers, 0777) == -1) {
        WindowPrompt("Error:","Can't create directory","OK",0,0,0);
        cntMissFiles = 0;
        }
    }
    if(stat(CFG.disc_path,&st) != 0) {
        char dirdiscs[100];
        snprintf(dirdiscs,strlen(CFG.disc_path),"%s",CFG.disc_path);
        if (mkdir(dirdiscs, 0777) == -1) {
        WindowPrompt("Error:","Can't create directory","OK",0,0,0);
        cntMissFiles = 0;
        }
    }

	while (i < cntMissFiles) {



	sprintf(prozent, "%i%%", 100*i/cntMissFiles);
	prTxt.SetText(prozent);
	//prTxt.SetFont(fontClock);

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
		progressbarImg.SetPosition(80,40);
		progressbarImg.SetTile(80*i/cntMissFiles);
	}
	else{
	progressbarImg.SetTile(100*i/cntMissFiles);}

    sprintf(msg, "%i file(s) left", cntMissFiles - i);
    msgTxt.SetText(msg);
    sprintf(msg2, "%s", missingFiles[i]);
    msg2Txt.SetText(msg2);

    //download boxart image
    char imgPath[100];
    char URLFile[100];
    if (choice2 == 2) {
		sprintf(URLFile,"http://www.theotherzone.com/wii/3d/176/248/%s",missingFiles[i]); // For 3D Covers
		sprintf(imgPath,"%s%s", CFG.covers_path, missingFiles[i]);
    }
    if(choice2 == 3) {
		sprintf(URLFile,"http://www.theotherzone.com/wii/diskart/160/160/%s",missingFiles[i]);
		sprintf(imgPath,"%s%s", CFG.disc_path, missingFiles[i]);
    }
    if(choice2 == 1) {
		sprintf(URLFile,"http://www.theotherzone.com/wii/resize/160/224/%s",missingFiles[i]);
		sprintf(imgPath,"%s%s", CFG.covers_path, missingFiles[i]);
    }

    struct block file = downloadfile(URLFile);//reject known bad images

    if (file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.data == NULL) {
        cntNotFound++;
        i++;
    } else {

    if(file.data != NULL)
    {
        // save png to sd card
        FILE *pfile;
        pfile = fopen(imgPath, "wb");
		if (!pfile)
			return -1;
        fwrite(file.data,1,file.size,pfile);
        fclose (pfile);
        free(file.data);
    }

    i++;
    }

    if(btn1.GetState() == STATE_CLICKED) {
        cntNotFound = cntMissFiles-i+cntNotFound;
        break;
    }

	}


	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	if (cntNotFound != 0) {
	    return cntNotFound;
	} else {
	return 0;
	}
}


/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/

static void *
UpdateGUI (void *arg)
{
	while(1)
	{
		if(guiHalt)
		{
			LWP_SuspendThread(guithread);
		}
		else
		{
			mainWindow->Draw();
			if (Settings.tooltips == TooltipsOn && THEME.showToolTip != 0)
				mainWindow->DrawTooltip();

			#ifdef HW_RVL
			for(int i=3; i >= 0; i--) // so that player 1's cursor appears on top!
			{
				if(userInput[i].wpad.ir.valid)
					Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48,
						96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, CFG.widescreen? 0.8 : 1, 1, 255);
				if(Settings.rumble == RumbleOn)
				{
					DoRumble(i);
				}
			}
			#endif

			Menu_Render();

			for(int i=0; i < 4; i++)
				mainWindow->Update(&userInput[i]);

			if(ExitRequested)
			{
				for(int a = 0; a < 255; a += 15)
				{
					mainWindow->Draw();
					Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, a},1);
					Menu_Render();
				}
				ExitApp();
			}
		}
	}
	return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void
InitGUIThreads()
{
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 70);
}

/****************************************************************************
 * EntryCmp
 ***************************************************************************/
s32 __Menu_EntryCmp(const void *a, const void *b)
{
	struct discHdr *hdr1 = (struct discHdr *)a;
	struct discHdr *hdr2 = (struct discHdr *)b;

	/* Compare strings */
	return stricmp(get_title(hdr1), get_title(hdr2));
}

/****************************************************************************
 * Get Gamelist
 ***************************************************************************/

s32 __Menu_GetEntries(void)
{
	struct discHdr *buffer = NULL;
	struct discHdr *buffer2 = NULL;
	struct discHdr *header = NULL;

	u32 cnt, len;
	s32 ret;

	/* Get list length */
	ret = WBFS_GetCount(&cnt);
	if (ret < 0)
		return ret;

	/* Buffer length */
	len = sizeof(struct discHdr) * cnt;

	/* Allocate memory */
	buffer = (struct discHdr *)memalign(32, len);
	if (!buffer)
		return -1;

	/* Clear buffer */
	memset(buffer, 0, len);

	/* Get header list */
	ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
	if (ret < 0)
		goto err;

	if (CFG.parentalcontrol && !CFG.godmode)
	{
		u32 cnt2 = 0;

		for (u32 i = 0; i < cnt; i++)
		{
			header = &buffer[i];
			if (get_block(header) < CFG.parentalcontrol)
			{
				buffer2 = (discHdr *) realloc(buffer2, (cnt2+1) * sizeof(struct discHdr));
				if (!buffer2)
				{
					free(buffer);
					return -1;
				}

				memcpy((buffer2 + cnt2), (buffer + i), sizeof(struct discHdr));
				cnt2++;
			}
		}
		free(buffer);
		buffer = buffer2;
		buffer2 = NULL;
		cnt = cnt2;
	}

	/* Sort entries */
	qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);

	/* Free memory */
	if (gameList)
		free(gameList);

	/* Set values */
	gameList = buffer;
	gameCnt  = cnt;

	/* Reset variables */
	gameSelected = gameStart = 0;

	return 0;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	return ret;
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
static int OnScreenKeyboard(char * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	GuiImageData btnOutline(button_dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetSimpleTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	okBtnImg.SetWidescreen(CFG.widescreen);}///////////
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(5, 15);//(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetSoundClick(&btnClick);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	cancelBtnImg.SetWidescreen(CFG.widescreen);}///////////
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-5, 15);//(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetSoundClick(&btnClick);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetTrigger(&trigB);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		VIDEO_WaitVSync();

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}


/****************************************************************************
 * MenuInstall
 ***************************************************************************/

static int MenuInstall()
{
	int menu = MENU_NONE;
    static struct discHdr headerdisc ATTRIBUTE_ALIGN(32);

    if(Settings.cios == ios222) {
    Disc_SetUSB(NULL, 1);
    } else {
    Disc_SetUSB(NULL, 0);
    }

    int ret, choice = 0;
	char *name;
	static char buffer[MAX_CHARACTERS + 4];


	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);

    GuiImageData battery(battery_png);
	GuiImageData batteryRed(battery_red_png);
	GuiImageData batteryBar(battery_bar_png);


	#ifdef HW_RVL
	int i = 0, level;
	char txt[3];
	GuiText * batteryTxt[4];
	GuiImage * batteryImg[4];
	GuiImage * batteryBarImg[4];
	GuiButton * batteryBtn[4];

	for(i=0; i < 4; i++)
	{

		if(i == 0)
			sprintf(txt, "P%d", i+1);
		else
			sprintf(txt, "P%d", i+1);

		batteryTxt[i] = new GuiText(txt, 22, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
		batteryTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i] = new GuiImage(&battery);
		batteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i]->SetPosition(36, 0);
		batteryImg[i]->SetTile(0);
		batteryBarImg[i] = new GuiImage(&batteryBar);
		batteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBarImg[i]->SetPosition(33, 0);

		batteryBtn[i] = new GuiButton(40, 20);
		batteryBtn[i]->SetLabel(batteryTxt[i]);
		batteryBtn[i]->SetImage(batteryBarImg[i]);
		batteryBtn[i]->SetIcon(batteryImg[i]);
		batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		batteryBtn[i]->SetRumble(false);
		batteryBtn[i]->SetAlpha(70);
	}


	batteryBtn[0]->SetPosition(THEME.battery1_x, THEME.battery1_y);
	batteryBtn[1]->SetPosition(THEME.battery2_x, THEME.battery2_y);
	batteryBtn[2]->SetPosition(THEME.battery3_x, THEME.battery3_y);
	batteryBtn[3]->SetPosition(THEME.battery4_x, THEME.battery4_y);
	#endif

    HaltGui();
	GuiWindow w(screenwidth, screenheight);

	if (THEME.showBattery)
	{
		#ifdef HW_RVL
		w.Append(batteryBtn[0]);
		w.Append(batteryBtn[1]);
		w.Append(batteryBtn[2]);
		w.Append(batteryBtn[3]);
		#endif
	}

    mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
	    VIDEO_WaitVSync ();

		#ifdef HW_RVL
		for(i=0; i < 4; i++)
		{
			if(WPAD_Probe(i, NULL) == WPAD_ERR_NONE) // controller connected
			{
				level = (userInput[i].wpad.battery_level / 100.0) * 4;
				if(level > 4) level = 4;
				batteryImg[i]->SetTile(level);

				if(level == 0)
					batteryBarImg[i]->SetImage(&batteryRed);
				else
					batteryBarImg[i]->SetImage(&batteryBar);

				batteryBtn[i]->SetAlpha(255);
			}
			else // controller not connected
			{
				batteryImg[i]->SetTile(0);
				batteryImg[i]->SetImage(&battery);
				batteryBtn[i]->SetAlpha(70);
			}
		}
		#endif

		ret = DiscWait("Insert Disk","Waiting...","Cancel",0,0);
		if (ret < 0) {
			WindowPrompt ("Error reading Disc",0,"Back",0,0,0);
			menu = MENU_DISCLIST;
			break;
		}
		ret = Disc_Open();
		if (ret < 0) {
			WindowPrompt ("Could not open Disc",0,"Back",0,0,0);
			menu = MENU_DISCLIST;
			break;
		}

		ret = Disc_IsWii();
		if (ret < 0) {
			choice = WindowPrompt ("Not a Wii Disc","Insert a Wii Disc!","OK","Back",0,0);

			if (choice == 1) {
				menu = MENU_INSTALL;
				break;
			} else
				menu = MENU_DISCLIST;
				break;
			}

		Disc_ReadHeader(&headerdisc);
		name = headerdisc.title;
		if (strlen(name) < (34 + 3)) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(name, "%s", name);
			} else {
			strncpy(buffer, name,  34);
			strncat(buffer, "...", 3);
			sprintf(name, "%s", buffer);
		}

		ret = WBFS_CheckGame(headerdisc.id);
		if (ret) {
			WindowPrompt ("Game is already installed:",name,"Back",0,0,0);
			menu = MENU_DISCLIST;
			break;
		}
		hdd = GetHddInfo();
		if (!hdd) {
			WindowPrompt ("No HDD found!","Error!!","Back",0,0,0);
			menu = MENU_DISCLIST;
			break;
			}

		f32 freespace, used;

		WBFS_DiskSpace(&used, &freespace);
		u32 estimation = wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
		gamesize = ((f32) estimation)/1073741824;
		char gametxt[50];

		sprintf(gametxt, "%s : %.2fGB", name, gamesize);

		choice = WindowPrompt("Continue install game?",gametxt,"OK","Cancel",0,0);

		if(choice == 1) {

		sprintf(gametxt, "Installing game:");

		if (gamesize > freespace) {
			char errortxt[50];
			sprintf(errortxt, "Game Size: %.2fGB, Free Space: %.2fGB", gamesize, freespace);
			choice = WindowPrompt("Not enough free space!",errortxt,"Go on", "Return",0,0);
			if (choice == 1) {
				ret = ProgressWindow(gametxt, name);
				if (ret != 0) {
					WindowPrompt ("Install error!",0,"Back",0,0,0);
					menu = MENU_DISCLIST;
					break;
				}
				else {
					__Menu_GetEntries(); //get the entries again
					wiilight(1);
					WindowPrompt ("Successfully installed:",name,"OK",0,0,0);
					menu = MENU_DISCLIST;
					wiilight(0);
					break;
				}
			} else {
				menu = MENU_DISCLIST;
				break;
			}

		}
		else {
			ret = ProgressWindow(gametxt, name);
			if (ret != 0) {
				WindowPrompt ("Install error!",0,"Back",0,0,0);
				menu = MENU_DISCLIST;
					break;
			} else {
				__Menu_GetEntries(); //get the entries again
				wiilight(1);
				WindowPrompt ("Successfully installed:",name,"OK",0,0,0);
				menu = MENU_DISCLIST;
				wiilight(0);
				break;
			}
		}
		} else {
		    menu = MENU_DISCLIST;
		    wiilight(0);
		    break;
		}

		if (shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();
	}


	HaltGui();

	#ifdef HW_RVL
	for(i=0; i < 4; i++)
	{
		delete batteryTxt[i];
		delete batteryImg[i];
		delete batteryBarImg[i];
		delete batteryBtn[i];
	}
	#endif

	mainWindow->Remove(&w);
	ResumeGui();
	return menu;
}

/****************************************************************************
 * MenuDiscList
 ***************************************************************************/

static int MenuDiscList()
{
	int menu = MENU_NONE;
	char imgPath[100];

	f32 freespace, used, size = 0.0;
	u32 nolist;
	char text[MAX_CHARACTERS + 4]; //text2[20];
	int choice = 0, selectedold = 100;
	s32 ret;

	//CLOCK
	struct tm * timeinfo;
	char theTime[80];
	int counter = 0;
	time_t lastrawtime=0;

	WBFS_DiskSpace(&used, &freespace);

    if (!gameCnt) { //if there is no list of games to display
        nolist = 1;
    }

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	snprintf(imgPath, sizeof(imgPath), "%sbutton_install.png", CFG.theme_path);
	GuiImageData btnInstall(imgPath, button_install_png);
	snprintf(imgPath, sizeof(imgPath), "%sbutton_install_over.png", CFG.theme_path);
	GuiImageData btnInstallOver(imgPath, button_install_over_png);

	snprintf(imgPath, sizeof(imgPath), "%ssettings_button.png", CFG.theme_path);
	GuiImageData btnSettings(imgPath, settings_button_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_button_over.png", CFG.theme_path);
	GuiImageData btnSettingsOver(imgPath, settings_button_over_png);

	snprintf(imgPath, sizeof(imgPath), "%swiimote_poweroff.png", CFG.theme_path);
	GuiImageData btnpwroff(imgPath, wiimote_poweroff_png);
	snprintf(imgPath, sizeof(imgPath), "%swiimote_poweroff_over.png", CFG.theme_path);
	GuiImageData btnpwroffOver(imgPath, wiimote_poweroff_over_png);
	snprintf(imgPath, sizeof(imgPath), "%smenu_button.png", CFG.theme_path);
	GuiImageData btnhome(imgPath, menu_button_png);
	snprintf(imgPath, sizeof(imgPath), "%smenu_button_over.png", CFG.theme_path);
	GuiImageData btnhomeOver(imgPath, menu_button_over_png);
	snprintf(imgPath, sizeof(imgPath), "%sSDcard.png", CFG.theme_path);
	GuiImageData btnsdcard(imgPath, sdcard_png);

    GuiImageData battery(battery_png);
	GuiImageData batteryRed(battery_red_png);
	GuiImageData batteryBar(battery_bar_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    char spaceinfo[30];
	sprintf(spaceinfo,"%.2fGB of %.2fGB free",freespace,(freespace+used));
	GuiText usedSpaceTxt(spaceinfo, 18, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
	usedSpaceTxt.SetAlignment(THEME.hddInfoAlign, ALIGN_TOP);
	usedSpaceTxt.SetPosition(THEME.hddInfo_x, THEME.hddInfo_y);

	char GamesCnt[15];
	sprintf(GamesCnt,"Games: %i",gameCnt);
	GuiText gamecntTxt(GamesCnt, 18, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
	gamecntTxt.SetAlignment(THEME.gameCntAlign, ALIGN_TOP);
	gamecntTxt.SetPosition(THEME.gameCnt_x,THEME.gameCnt_y);

	GuiTooltip installBtnTT("Install a game");
	if (Settings.wsprompt == yes)
		installBtnTT.SetWidescreen(CFG.widescreen);///////////
	GuiImage installBtnImg(&btnInstall);
	GuiImage installBtnImgOver(&btnInstallOver);
	installBtnImg.SetWidescreen(CFG.widescreen); //added
	installBtnImgOver.SetWidescreen(CFG.widescreen); //added
	GuiButton installBtn(btnInstall.GetWidth(), btnInstall.GetHeight());
	installBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	installBtn.SetPosition(THEME.install_x, THEME.install_y);
	installBtn.SetImage(&installBtnImg);
	installBtn.SetImageOver(&installBtnImgOver);
	installBtn.SetSoundOver(&btnSoundOver);
	installBtn.SetSoundClick(&btnClick);
	installBtn.SetTrigger(&trigA);
	installBtn.SetEffectGrow();
	installBtn.SetToolTip(&installBtnTT,24,-30, ALIGN_LEFT);

	GuiTooltip settingsBtnTT("Settings");
	if (Settings.wsprompt == yes)
		settingsBtnTT.SetWidescreen(CFG.widescreen);///////////

	GuiImage settingsBtnImg(&btnSettings);
	settingsBtnImg.SetWidescreen(CFG.widescreen); //added
	GuiImage settingsBtnImgOver(&btnSettingsOver);
	settingsBtnImgOver.SetWidescreen(CFG.widescreen); //added
	GuiButton settingsBtn(btnSettings.GetWidth(), btnSettings.GetHeight());
	settingsBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsBtn.SetPosition(THEME.setting_x, THEME.setting_y);
	settingsBtn.SetImage(&settingsBtnImg);
	settingsBtn.SetImageOver(&settingsBtnImgOver);
	settingsBtn.SetSoundOver(&btnSoundOver);
	settingsBtn.SetSoundClick(&btnClick);
	settingsBtn.SetTrigger(&trigA);
	settingsBtn.SetEffectGrow();
	settingsBtn.SetToolTip(&settingsBtnTT,65,-30);

	GuiTooltip homeBtnTT("Back to HBC or Wii Menu");
	if (Settings.wsprompt == yes)
		homeBtnTT.SetWidescreen(CFG.widescreen);///////////

	GuiImage homeBtnImg(&btnhome);
	homeBtnImg.SetWidescreen(CFG.widescreen); //added
	GuiImage homeBtnImgOver(&btnhomeOver);
	homeBtnImgOver.SetWidescreen(CFG.widescreen); //added
	GuiButton homeBtn(btnhome.GetWidth(), btnhome.GetHeight());
	homeBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	homeBtn.SetPosition(THEME.home_x, THEME.home_y);
	homeBtn.SetImage(&homeBtnImg);
	homeBtn.SetImageOver(&homeBtnImgOver);
	homeBtn.SetSoundOver(&btnSoundOver);
	homeBtn.SetSoundClick(&btnClick);
	homeBtn.SetTrigger(&trigA);
	homeBtn.SetTrigger(&trigHome);
	homeBtn.SetEffectGrow();
	homeBtn.SetToolTip(&homeBtnTT,15,-30);

	GuiTooltip poweroffBtnTT("Power off the Wii");
	if (Settings.wsprompt == yes)
		poweroffBtnTT.SetWidescreen(CFG.widescreen);///////////


    GuiImage poweroffBtnImg(&btnpwroff);
	GuiImage poweroffBtnImgOver(&btnpwroffOver);
	poweroffBtnImg.SetWidescreen(CFG.widescreen);
	poweroffBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton poweroffBtn(btnpwroff.GetWidth(), btnpwroff.GetHeight());
	poweroffBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	poweroffBtn.SetPosition(THEME.power_x, THEME.power_y);
	poweroffBtn.SetImage(&poweroffBtnImg);
	poweroffBtn.SetImageOver(&poweroffBtnImgOver);
	poweroffBtn.SetSoundOver(&btnSoundOver);
	poweroffBtn.SetSoundClick(&btnClick);
	poweroffBtn.SetTrigger(&trigA);
	poweroffBtn.SetEffectGrow();
	poweroffBtn.SetToolTip(&poweroffBtnTT,-10,-30);


	GuiTooltip sdcardBtnTT("Reload SD");
	if (Settings.wsprompt == yes)
		sdcardBtnTT.SetWidescreen(CFG.widescreen);///////////

	GuiImage sdcardImg(&btnsdcard);
	sdcardImg.SetWidescreen(CFG.widescreen);
	GuiButton sdcardBtn(btnsdcard.GetWidth(), btnsdcard.GetHeight());
	sdcardBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	sdcardBtn.SetPosition(THEME.sdcard_x, THEME.sdcard_y);
	sdcardBtn.SetImage(&sdcardImg);
	sdcardBtn.SetSoundOver(&btnSoundOver);
	sdcardBtn.SetSoundClick(&btnClick);
	sdcardBtn.SetTrigger(&trigA);
	sdcardBtn.SetEffectGrow();
	sdcardBtn.SetToolTip(&sdcardBtnTT,95,-40);

	//Downloading Covers
	GuiTooltip DownloadBtnTT("Click to Download Covers");
	if (Settings.wsprompt == yes)
		DownloadBtnTT.SetWidescreen(CFG.widescreen);///////////

	GuiButton DownloadBtn(160,224);
	DownloadBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	DownloadBtn.SetPosition(THEME.cover_x,THEME.cover_y);//(20, 300);

	if (CFG.godmode == 1)
	{//only make the button have trigger & tooltip if in godmode
		DownloadBtn.SetSoundOver(&btnSoundOver);
		DownloadBtn.SetTrigger(&trigA);
		DownloadBtn.SetToolTip(&DownloadBtnTT,205,-30);
    }
	else
		DownloadBtn.SetRumble(false);
	#ifdef HW_RVL
	int i = 0, level;
	char txt[3];
	GuiText * batteryTxt[4];
	GuiImage * batteryImg[4];
	GuiImage * batteryBarImg[4];
	GuiButton * batteryBtn[4];

	for(i=0; i < 4; i++)
	{

		if(i == 0)
			sprintf(txt, "P%d", i+1);
		else
			sprintf(txt, "P%d", i+1);

		batteryTxt[i] = new GuiText(txt, 22, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
		batteryTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i] = new GuiImage(&battery);
		batteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i]->SetPosition(36, 0);
		batteryImg[i]->SetTile(0);
		batteryBarImg[i] = new GuiImage(&batteryBar);
		batteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBarImg[i]->SetPosition(33, 0);

		batteryBtn[i] = new GuiButton(40, 20);
		batteryBtn[i]->SetLabel(batteryTxt[i]);
		batteryBtn[i]->SetImage(batteryBarImg[i]);
		batteryBtn[i]->SetIcon(batteryImg[i]);
		batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		batteryBtn[i]->SetRumble(false);
		batteryBtn[i]->SetAlpha(70);
	}

	batteryBtn[0]->SetPosition(THEME.battery1_x, THEME.battery1_y);
	batteryBtn[1]->SetPosition(THEME.battery2_x, THEME.battery2_y);
	batteryBtn[2]->SetPosition(THEME.battery3_x, THEME.battery3_y);
	batteryBtn[3]->SetPosition(THEME.battery4_x, THEME.battery4_y);
	#endif

	GuiGameBrowser gameBrowser(THEME.selection_w, THEME.selection_h, gameList, gameCnt, CFG.theme_path, bg_options_png, startat, offset);
	gameBrowser.SetPosition(THEME.selection_x, THEME.selection_y);
	gameBrowser.SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);

    GuiText clockTimeBack("88:88", 40, (GXColor){138, 138, 138, 40});
    clockTimeBack.SetAlignment(THEME.clockAlign, ALIGN_TOP);
    clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y);
	clockTimeBack.SetFont(fontClock);

    GuiText clockTime(theTime, 40, (GXColor){138, 138, 138, 240});
    clockTime.SetAlignment(THEME.clockAlign, ALIGN_TOP);
    clockTime.SetPosition(THEME.clock_x, THEME.clock_y);
	clockTime.SetFont(fontClock);


    HaltGui();
	GuiWindow w(screenwidth, screenheight);

	if((Settings.hddinfo == HDDInfo && THEME.showHDD == -1) || THEME.showHDD == 1) //force show hdd info
	{
		w.Append(&usedSpaceTxt);
	}
	if((Settings.hddinfo == HDDInfo && THEME.showGameCnt == -1) || THEME.showGameCnt == 1) //force show game cnt info
	{
		w.Append(&gamecntTxt);
	}

	w.Append(&sdcardBtn);
	w.Append(&poweroffBtn);
	if (CFG.godmode)
		w.Append(&installBtn);
	w.Append(&homeBtn);
    w.Append(&settingsBtn);
	w.Append(&DownloadBtn);

    if(Settings.hddinfo == Clock)
    {
		w.Append(&clockTimeBack);
		w.Append(&clockTime);
    }

	if (THEME.showBattery)
	{
		#ifdef HW_RVL
		w.Append(batteryBtn[0]);
		w.Append(batteryBtn[1]);
		w.Append(batteryBtn[2]);
		w.Append(batteryBtn[3]);
		#endif
	}

    mainWindow->Append(&gameBrowser);
    mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{

	    VIDEO_WaitVSync ();

        //CLOCK
		time_t rawtime = time(0);								//this fixes code dump caused by the clock
        if (Settings.hddinfo == Clock && rawtime != lastrawtime) {//only update the clock every 2000 loops
            lastrawtime = rawtime;
			timeinfo = localtime (&rawtime);
			if(rawtime & 1)
				strftime(theTime, sizeof(theTime), "%H:%M", timeinfo);
			else
				strftime(theTime, sizeof(theTime), "%H %M", timeinfo);
            clockTime.SetText(theTime);
        }
		counter++;

	    #ifdef HW_RVL
		for(i=0; i < 4; i++)
		{
			if(WPAD_Probe(i, NULL) == WPAD_ERR_NONE) // controller connected
			{
				level = (userInput[i].wpad.battery_level / 100.0) * 4;
				if(level > 4) level = 4;
				batteryImg[i]->SetTile(level);

				if(level == 0)
					batteryBarImg[i]->SetImage(&batteryRed);
				else
					batteryBarImg[i]->SetImage(&batteryBar);

				batteryBtn[i]->SetAlpha(255);
			}
			else // controller not connected
			{
				batteryImg[i]->SetTile(0);
				batteryImg[i]->SetImage(&battery);
				batteryBtn[i]->SetAlpha(70);
			}
		}
		#endif
			// respond to button presses
		if(shutdown == 1)
		{
			Sys_Shutdown();
		}
		if(reset == 1)
			Sys_Reboot();

	    if(poweroffBtn.GetState() == STATE_CLICKED)
		{

		    choice = WindowPrompt("How to Shutdown?",0,"Full Shutdown", "Shutdown to Idle", "Cancel",0);
			if(choice == 2)
			{
			    WPAD_Flush(0);
                WPAD_Disconnect(0);
                WPAD_Shutdown();

                /* Set LED mode */
                ret = CONF_GetIdleLedMode();
                if(ret >= 0 && ret <= 2)
                    STM_SetLedMode(ret);

				STM_ShutdownToIdle();

			} else if(choice == 1) {
			    WPAD_Flush(0);
                WPAD_Disconnect(0);
                WPAD_Shutdown();
                STM_ShutdownToStandby();
			} else {
			    poweroffBtn.ResetState();
			    gameBrowser.SetFocus(1);
			}

		}
		else if(homeBtn.GetState() == STATE_CLICKED)
		{

			choice = WindowPrompt("Exit USB ISO Loader ?",0, "Back to Loader","Wii Menu","Back",0);
			if(choice == 2)
			{
                SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0); // Back to System Menu
			}
			else if (choice == 1)
			{
				if (*(unsigned int*) 0x80001800) exit(0);
				// Channel Version
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
				//exit(0); //Back to HBC
			} else {
			homeBtn.ResetState();
			gameBrowser.SetFocus(1);
			}

        }
		else if(installBtn.GetState() == STATE_CLICKED)
		{
				choice = WindowPrompt("Install a game?",0,"Yes","No",0,0);
				if (choice == 1)
				{
					menu = MENU_INSTALL;
					break;
				}
				else
				{
					installBtn.ResetState();
					gameBrowser.SetFocus(1);
				}
		}

		else if(sdcardBtn.GetState() == STATE_CLICKED)
		{
            __io_wiisd.shutdown();
			__io_wiisd.startup();
			break;
		}

		else if(DownloadBtn.GetState() == STATE_CLICKED)
		{
		    if(isSdInserted() == 1) {
			choice = WindowPrompt("Cover Download", 0, "Normal Covers", "3D Covers", "Disc Images", "Back"); // ask for download choice

			if (choice != 0)
			{
				int netset;
				int choice2 = choice;

				netset = NetworkInitPromp(choice2);
				networkisinitialized = 1;

				if(netset < 0)
				{
					WindowPrompt("Network init error", 0, "OK",0,0,0);
					netcheck = false;

				} else  {
                    netcheck = true;
				}

				if (netcheck)
				{

					if (missingFiles != NULL && cntMissFiles > 0)

					{
						char tempCnt[40];
						i = 0;

						sprintf(tempCnt,"Missing %i files",cntMissFiles);
						choice = WindowPrompt("Download Boxart image?",tempCnt,"Yes","No",0,0);
						//WindowPrompt("Downloading","Please Wait Downloading Covers",0,0);
						if (choice == 1)
						{
							ret = ProgressDownloadWindow(choice2);
							if (ret == 0) {
							WindowPrompt("Download finished",0,"OK",0,0,0);
							} else {
                            sprintf(tempCnt,"%i files not found on the server!",ret);
                            WindowPrompt("Download finished",tempCnt,"OK",0,0,0);
							}
						}
					}
					else
					{
						WindowPrompt("No file missing!",0,"OK",0,0,0);
					}
				}
			}
            } else {
			WindowPrompt("No SD-Card inserted!", "Insert a SD-Card to download images.", "OK", 0,0,0);
            }
			DownloadBtn.ResetState();
			gameBrowser.SetFocus(1);
		}//end download

		else if(settingsBtn.GetState() == STATE_CLICKED)
		{		startat = gameBrowser.GetSelectedOption();
				offset = gameBrowser.GetOffset();
				menu = MENU_SETTINGS;
			    break;

		}


		//Get selected game under cursor
		int selectimg;//, promptnumber;
		//promptnumber = 0;
		char ID[4];
		char IDfull[7];
		selectimg = gameBrowser.GetSelectedOption();
	    gameSelected = gameBrowser.GetClickedOption();



		if (gameSelected > 0) //if click occured
			selectimg = gameSelected;

		if ((selectimg >= 0) && (selectimg < (s32) gameCnt))
		{
			if (selectimg != selectedold)
			{
				selectedold = selectimg;//update displayed cover, game ID, and region if the selected game changes
				struct discHdr *header = &gameList[selectimg];
				snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
				snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
				w.Remove(&DownloadBtn);
//				w.Remove(coverImg);

				if (GameIDTxt)
				{
					w.Remove(GameIDTxt);
					delete GameIDTxt;
					GameIDTxt = NULL;
				}
				if(GameRegionTxt)
				{
					w.Remove(GameRegionTxt);
					delete GameRegionTxt;
					GameRegionTxt = NULL;
				}

				switch(header->id[3])
				{
					case 'E':
					sprintf(gameregion,"NTSC U");
					break;

					case 'J':
					sprintf(gameregion,"NTSC J");
					break;

					case 'P':
					case 'D':
					case 'F':
					case 'X':
					case 'S':
					case 'Y':
					sprintf(gameregion,"  PAL ");
					break;
				}

				//load game cover
				if (cover)
				{
					delete cover;
					cover = NULL;
				}

				snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
				cover = new GuiImageData(imgPath,0); //load short id
				if (!cover->GetImage()) //if could not load the short id image
				{
					delete cover;
					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
					cover = new GuiImageData(imgPath, 0); //load full id image
					if (!cover->GetImage())
					{
						delete cover;
						snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.covers_path);
						cover = new GuiImageData(imgPath, nocover_png); //load no image
					}
				}

				if (coverImg)
				{
					delete coverImg;
					coverImg = NULL;
				}
				coverImg = new GuiImage(cover);
				coverImg->SetWidescreen(CFG.widescreen);

				DownloadBtn.SetImage(coverImg);// put the new image on the download button
				w.Append(&DownloadBtn);

				if ((Settings.sinfo == GameID) || (Settings.sinfo == Both)){
					GameIDTxt = new GuiText(IDfull, 22, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
					GameIDTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
					GameIDTxt->SetPosition(THEME.id_x,THEME.id_y);
					GameIDTxt->SetEffect(EFFECT_FADE, 20);
					w.Append(GameIDTxt);
				}

				if ((Settings.sinfo == GameRegion) || (Settings.sinfo == Both)){
					GameRegionTxt = new GuiText(gameregion, 22, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
					GameRegionTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
					GameRegionTxt->SetPosition(THEME.region_x, THEME.region_y);
					GameRegionTxt->SetEffect(EFFECT_FADE, 20);
					w.Append(GameRegionTxt);
				}
			}
		}

		if ((gameSelected >= 0) && (gameSelected < (s32)gameCnt))
		{
			struct discHdr *header = &gameList[gameSelected];
			WBFS_GameSize(header->id, &size);
			if (strlen(get_title(header)) < (MAX_CHARACTERS + 3)) {
				sprintf(text, "%s", get_title(header));
			}
			else {
				strncpy(text, get_title(header),  MAX_CHARACTERS);
				text[MAX_CHARACTERS] = '\0';
				strncat(text, "...", 3);
			}

			if (Settings.qboot == yes)//quickboot game
			{

					wiilight(0);

                    int ios2;
                    switch(iosChoice)
                    {
                        case i249:
                            ios2 = 0;
                            break;

                        case i222:
                            ios2 = 1;
                            break;

                        default:
                            ios2 = 0;
                            break;
                    }

                    // if we have used the network or cios222 we need to reload the disklist
                    if(networkisinitialized == 1 || ios2 == 1 || Settings.cios == ios222) {


                    if(ios2 == 1) {

					ret = Sys_IosReload(222);

					if(ret < 0) {
                    Wpad_Init();
                    WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
                    WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

					WindowPrompt("You don't have cIOS222!","Loading in cIOS249!","OK", 0,0,0);

					Sys_IosReload(249);
					ios2 = 0;
					}

                    } else {

                    ret = Sys_IosReload(249);

                    }
                    }

					/* Set USB mode */
					ret = Disc_SetUSB(header->id, ios2);
					if (ret < 0) {
						sprintf(text, "Error: %i", ret);
						WindowPrompt(
						"Failed to set USB:",
						text,
						"OK",0,0,0);
					}
					else {
						/* Open disc */
						ret = Disc_Open();
						if (ret < 0) {
							sprintf(text, "Error: %i", ret);
							WindowPrompt(
							"Failed to boot:",
							text,
							"OK",0,0,0);
						}
						else {
							menu = MENU_EXIT;
						}
					}
					break;
				}
			bool returnHere = true;// prompt to start game
			while (returnHere)
			{
				returnHere = false;
				wiilight(1);
				choice = GameWindowPrompt();
				header = &gameList[gameSelected]; //reset header

				if(choice == 1)
				{

					wiilight(0);

                    int ios2;
                    switch(iosChoice)
                    {
                        case i249:
                            ios2 = 0;
                            break;

                        case i222:
                            ios2 = 1;
                            break;

                        default:
                            ios2 = 0;
                            break;
                    }

                                        // if we have used the network or cios222 we need to reload the disklist
                    if(networkisinitialized == 1 || ios2 == 1 || Settings.cios == ios222) {


                    if(ios2 == 1) {

					ret = Sys_IosReload(222);

					if(ret < 0) {
                    Wpad_Init();
                    WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
                    WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

					WindowPrompt("You don't have cIOS222!","Loading in cIOS249!","OK", 0,0,0);

					Sys_IosReload(249);
					ios2 = 0;
					}

                    } else {

                    ret = Sys_IosReload(249);

                    }
                    }



					/* Set USB mode */
					ret = Disc_SetUSB(header->id, ios2);
					if (ret < 0) {
						sprintf(text, "Error: %i", ret);
						WindowPrompt(
						"Failed to set USB:",
						text,
						"OK",0,0,0);
					}
					else {
						/* Open disc */
						ret = Disc_Open();
						if (ret < 0) {
							sprintf(text, "Error: %i", ret);
							WindowPrompt(
							"Failed to boot:",
							text,
							"OK",0,0,0);
						}
						else {
							menu = MENU_EXIT;
						}
					}
				}
				else if (choice == 2)
				{
					wiilight(0);
					if (GameSettings(header) == 1) //if deleted
					{
						menu = MENU_DISCLIST;
						break;
					}
					returnHere = true;
				}

				else if (choice == 3) //WBFS renaming
				{
					wiilight(0);
										//enter new game title
					char entered[40];
					snprintf(entered, sizeof(entered), "%s", get_title(header));
					entered[39] = '\0';
					OnScreenKeyboard(entered, 40);
					WBFS_RenameGame(header->id, entered);
					__Menu_GetEntries();
					menu = MENU_DISCLIST;
				}


				else if(choice == 0)
					gameBrowser.SetFocus(1);
			}
		}
	}

	HaltGui();

	#ifdef HW_RVL
	for(i=0; i < 4; i++)
	{
		delete batteryTxt[i];
		delete batteryImg[i];
		delete batteryBarImg[i];
		delete batteryBtn[i];
	}
	#endif

	mainWindow->Remove(&gameBrowser);
	mainWindow->Remove(&w);
	ResumeGui();
	return menu;
}

/****************************************************************************
 * MenuFormat
 ***************************************************************************/

static int MenuFormat()
{
	int menu = MENU_NONE;
	char imgPath[100];

	OptionList options;
    partitionEntry partitions[MAX_PARTITIONS];

	u32 cnt, sector_size, selected = 2000;
	int choice, ret;
	char text[ISFS_MAXPATH];

	s32 ret2;
    ret2 = Partition_GetEntries(partitions, &sector_size);

	//create the partitionlist
    for (cnt = 0; cnt < MAX_PARTITIONS; cnt++) {
		partitionEntry *entry = &partitions[cnt];

		/* Calculate size in gigabytes */
		f32 size = entry->size * (sector_size / GB_SIZE);

        if (size) {
            sprintf(options.name[cnt], "Partition %d:", cnt+1);
            sprintf (options.value[cnt],"%.2fGB", size);
        } else {
            sprintf(options.name[cnt], "Partition %d:", cnt+1);
            sprintf (options.value[cnt],"(Can't be formated)");
        }
    }

    options.length = cnt;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);
	snprintf(imgPath, sizeof(imgPath), "%swiimote_poweroff.png", CFG.theme_path);
	GuiImageData btnpwroff(imgPath, wiimote_poweroff_png);
	snprintf(imgPath, sizeof(imgPath), "%swiimote_poweroff_over.png", CFG.theme_path);
	GuiImageData btnpwroffOver(imgPath, wiimote_poweroff_over_png);
	snprintf(imgPath, sizeof(imgPath), "%smenu_button.png", CFG.theme_path);
	GuiImageData btnhome(imgPath, menu_button_png);
	snprintf(imgPath, sizeof(imgPath), "%smenu_button_over.png", CFG.theme_path);
	GuiImageData btnhomeOver(imgPath, menu_button_over_png);
    GuiImageData battery(battery_png);
	GuiImageData batteryRed(battery_red_png);
	GuiImageData batteryBar(battery_bar_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    GuiText titleTxt("Select the Partition", 18, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(10,40);

    GuiText titleTxt2("you want to format:", 18, (GXColor){0, 0, 0, 255});
	titleTxt2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt2.SetPosition(20,60);

    GuiImage poweroffBtnImg(&btnpwroff);
	GuiImage poweroffBtnImgOver(&btnpwroffOver);
	poweroffBtnImg.SetWidescreen(CFG.widescreen);
	poweroffBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton poweroffBtn(btnpwroff.GetWidth(), btnpwroff.GetHeight());
	poweroffBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	poweroffBtn.SetPosition(THEME.power_x, THEME.power_y);
	poweroffBtn.SetImage(&poweroffBtnImg);
	poweroffBtn.SetImageOver(&poweroffBtnImgOver);
	poweroffBtn.SetSoundOver(&btnSoundOver);
	poweroffBtn.SetSoundClick(&btnClick);
	poweroffBtn.SetTrigger(&trigA);
	poweroffBtn.SetEffectGrow();

	GuiImage exitBtnImg(&btnhome);
	GuiImage exitBtnImgOver(&btnhomeOver);
	exitBtnImg.SetWidescreen(CFG.widescreen);
	exitBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton exitBtn(btnhome.GetWidth(), btnhome.GetHeight());
	exitBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	exitBtn.SetPosition(240, 367);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetSoundClick(&btnClick);
	exitBtn.SetTrigger(&trigA);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	#ifdef HW_RVL
	int i = 0, level;
	char txt[3];
	GuiText * batteryTxt[4];
	GuiImage * batteryImg[4];
	GuiImage * batteryBarImg[4];
	GuiButton * batteryBtn[4];

	for(i=0; i < 4; i++)
	{

		if(i == 0)
			sprintf(txt, "P%d", i+1);
		else
			sprintf(txt, "P%d", i+1);

		batteryTxt[i] = new GuiText(txt, 22, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
		batteryTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i] = new GuiImage(&battery);
		batteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i]->SetPosition(36, 0);
		batteryImg[i]->SetTile(0);
		batteryBarImg[i] = new GuiImage(&batteryBar);
		batteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBarImg[i]->SetPosition(33, 0);

		batteryBtn[i] = new GuiButton(40, 20);
		batteryBtn[i]->SetLabel(batteryTxt[i]);
		batteryBtn[i]->SetImage(batteryBarImg[i]);
		batteryBtn[i]->SetIcon(batteryImg[i]);
		batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		batteryBtn[i]->SetRumble(false);
		batteryBtn[i]->SetAlpha(70);
	}

	batteryBtn[0]->SetPosition(THEME.battery1_x, THEME.battery1_y);
	batteryBtn[1]->SetPosition(THEME.battery2_x, THEME.battery2_y);
	batteryBtn[2]->SetPosition(THEME.battery3_x, THEME.battery3_y);
	batteryBtn[3]->SetPosition(THEME.battery4_x, THEME.battery4_y);
	#endif

	GuiOptionBrowser optionBrowser(THEME.selection_w, THEME.selection_h, &options, CFG.theme_path, bg_options_png, 1, 0);
	optionBrowser.SetPosition(THEME.selection_x, THEME.selection_y);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);

    HaltGui();
	GuiWindow w(screenwidth, screenheight);
    w.Append(&titleTxt);
    w.Append(&titleTxt2);
    w.Append(&poweroffBtn);
	w.Append(&exitBtn);

	if (THEME.showBattery)
	{
		#ifdef HW_RVL
		w.Append(batteryBtn[0]);
		w.Append(batteryBtn[1]);
		w.Append(batteryBtn[2]);
		w.Append(batteryBtn[3]);
		#endif
	}

    mainWindow->Append(&w);
    mainWindow->Append(&optionBrowser);

	ResumeGui();

	while(menu == MENU_NONE)
	{
	    VIDEO_WaitVSync ();

		#ifdef HW_RVL
		for(i=0; i < 4; i++)
		{
			if(WPAD_Probe(i, NULL) == WPAD_ERR_NONE) // controller connected
			{
				level = (userInput[i].wpad.battery_level / 100.0) * 4;
				if(level > 4) level = 4;
				batteryImg[i]->SetTile(level);

				if(level == 0)
					batteryBarImg[i]->SetImage(&batteryRed);
				else
					batteryBarImg[i]->SetImage(&batteryBar);

				batteryBtn[i]->SetAlpha(255);
			}
			else // controller not connected
			{
				batteryImg[i]->SetTile(0);
				batteryImg[i]->SetImage(&battery);
				batteryBtn[i]->SetAlpha(70);
			}
		}
		#endif

	    selected = optionBrowser.GetClickedOption();

            for (cnt = 0; cnt < MAX_PARTITIONS; cnt++) {
                if (cnt == selected) {
                    partitionEntry *entry = &partitions[selected];
                        if (entry->size) {
                        sprintf(text, "Partition %d : %.2fGB", selected+1, entry->size * (sector_size / GB_SIZE));
                        choice = WindowPrompt(
                        "Do you want to format:",
                        text,
                        "Yes",
                        "No",0,0);
                    if(choice == 1) {
                    ret = FormatingPartition("Formatting, please wait...", entry);
                        if (ret < 0) {
                            WindowPrompt("Error:","Failed formating","Return",0,0,0);
                            menu = MENU_SETTINGS;

                        } else {
                            WBFS_Open();
                            sprintf(text, "%s formated!", text);
                            WindowPrompt("Success:",text,"OK",0,0,0);
                            menu = MENU_DISCLIST;
                        }
                    }
                    }
                }
            }
		if (shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();

	    if(poweroffBtn.GetState() == STATE_CLICKED)
		{
		    choice = WindowPrompt ("Shutdown System","Are you sure?","Yes","No",0,0);
			if(choice == 1)
			{
			    WPAD_Flush(0);
                WPAD_Disconnect(0);
                WPAD_Shutdown();
				Sys_Shutdown();
			}

		} else if(exitBtn.GetState() == STATE_CLICKED)
		{
		    choice = WindowPrompt ("Return to Wii Menu","Are you sure?","Yes","No",0,0);
			if(choice == 1)
			{
                SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
                //exit(0); //zum debuggen schneller
			}
		}
	}


	HaltGui();

	#ifdef HW_RVL
	for(i=0; i < 4; i++)
	{
		delete batteryTxt[i];
		delete batteryImg[i];
		delete batteryBarImg[i];
		delete batteryBtn[i];
	}
	#endif

	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	ResumeGui();
	return menu;
}

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
static int MenuSettings()
{
	int menu = MENU_NONE;
	int ret;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];

	GuiImageData btnOutline(settings_menu_button_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);
	snprintf(imgPath, sizeof(imgPath), "%spage1.png", CFG.theme_path);
	GuiImageData page1(imgPath, page1_png);
	snprintf(imgPath, sizeof(imgPath), "%spage1d.png", CFG.theme_path);
	GuiImageData page1d(imgPath, page1d_png);
	snprintf(imgPath, sizeof(imgPath), "%spage2.png", CFG.theme_path);
	GuiImageData page2(imgPath, page2_png);
	snprintf(imgPath, sizeof(imgPath), "%spage2d.png", CFG.theme_path);
	GuiImageData page2d(imgPath, page2d_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigL;
	trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	GuiTrigger trigR;
	trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

    GuiText titleTxt("Settings", 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);

    GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage backBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	backBtnImg.SetWidescreen(CFG.widescreen);}//////
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtn.SetPosition(-180, 400);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnClick);
	backBtn.SetTrigger(&trigA);
	backBtn.SetTrigger(&trigB);
	backBtn.SetEffectGrow();
	///////////////////////////////
	GuiImage page1Img(&page1);
	GuiImage page1dImg(&page1d);
	GuiButton page1Btn(page1.GetWidth(), page1.GetHeight());
	page1Btn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	page1Btn.SetPosition(-202, 90);
	page1Btn.SetImage(&page1Img);
	page1Btn.SetSoundOver(&btnSoundOver);
	page1Btn.SetSoundClick(&btnClick);
	page1Btn.SetTrigger(&trigA);
	page1Btn.SetTrigger(&trigL);

	GuiTooltip page1BtnTT("Go to Page 1");
	if (Settings.wsprompt == yes)
		page1BtnTT.SetWidescreen(CFG.widescreen);///////////

	page1Btn.SetToolTip(&page1BtnTT,105, 15);


	GuiImage page2Img(&page2);
	GuiImage page2dImg(&page2d);
	GuiButton page2Btn(page2.GetWidth(), page2.GetHeight());
	page2Btn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	page2Btn.SetPosition(-202, 186);
	page2Btn.SetImage(&page2dImg);
	page2Btn.SetSoundOver(&btnSoundOver);
	page2Btn.SetSoundClick(&btnClick);
	page2Btn.SetTrigger(&trigA);
	page2Btn.SetTrigger(&trigR);

	GuiTooltip page2BtnTT("Go to Page 2");
	if (Settings.wsprompt == yes)
		page2BtnTT.SetWidescreen(CFG.widescreen);///////////

	page2Btn.SetToolTip(&page2BtnTT,105,0);

	////////////////////////////////


	const char * text = "Unlock";
	if (CFG.godmode == 1)
			text = "Lock";
	GuiText lockBtnTxt(text, 22, (GXColor){0, 0, 0, 255});
	lockBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage lockBtnImg(&btnOutline);
	lockBtnImg.SetWidescreen(CFG.widescreen);//////
	GuiButton lockBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	lockBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	lockBtn.SetPosition(180, 400);
	lockBtn.SetLabel(&lockBtnTxt);
	lockBtn.SetImage(&lockBtnImg);
	lockBtn.SetSoundOver(&btnSoundOver);
	lockBtn.SetSoundClick(&btnClick);
	lockBtn.SetTrigger(&trigA);
	lockBtn.SetEffectGrow();

	GuiImageData logo(credits_button_png);
	GuiImage logoImg(&logo);
	GuiImageData logoOver(credits_button_over_png);
	GuiImage logoImgOver(&logoOver);
	btnLogo = new GuiButton(logoImg.GetWidth(), logoImg.GetHeight());
	btnLogo->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	btnLogo->SetPosition(0, -35);
	btnLogo->SetImage(&logoImg);
	btnLogo->SetImageOver(&logoImgOver);
	btnLogo->SetEffectGrow();
	btnLogo->SetSoundOver(&btnSoundOver);
	btnLogo->SetSoundClick(&btnClick);
	btnLogo->SetTrigger(&trigA);
	btnLogo->SetUpdateCallback(WindowCredits);

	customOptionList options2(9);
	GuiCustomOptionBrowser optionBrowser2(396, 280, &options2, CFG.theme_path, "bg_options_settings", bg_options_settings_png, 0);
	optionBrowser2.SetPosition(0, 90);
	optionBrowser2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser2.SetCol2Position(200);
	GuiWindow w(screenwidth, screenheight);

	int pageToDisplay = 1;
	while ( pageToDisplay > 0) //set pageToDisplay to 0 to quit
	{
		menu = MENU_NONE;
		if ( pageToDisplay == 1)
		{
			sprintf(options2.name[0], "Video Mode");
			sprintf(options2.name[1], "VIDTV Patch");
			sprintf(options2.name[2], "Language");
			sprintf(options2.name[3], "Ocarina");
			sprintf(options2.name[4], "Display");
			sprintf(options2.name[5], "Clock"); //CLOCK
			sprintf(options2.name[6], "Rumble"); //RUMBLE
			sprintf(options2.name[7], "Volume");
			sprintf(options2.name[8], "Tooltips");

			HaltGui();
			w.Append(&settingsbackgroundbtn);
			w.Append(&titleTxt);
			w.Append(&backBtn);
			w.Append(&lockBtn);
			w.Append(btnLogo);

			mainWindow->Append(&w);
			mainWindow->Append(&optionBrowser2);
			mainWindow->Append(&page2Btn);
			mainWindow->Append(&page1Btn);


			ResumeGui();
		}
		else if ( pageToDisplay == 2 )
		{

			mainWindow->Append(&optionBrowser2);
			mainWindow->Append(&page1Btn);
			mainWindow->Append(&page2Btn);

			sprintf(options2.name[0], "Password");
			sprintf(options2.name[1], "Boot/Standard");
			sprintf(options2.name[2], "Flip X");
			sprintf(options2.name[3], "Quick Boot");
			sprintf(options2.name[4], "Prompts & Buttons");
			sprintf(options2.name[5], " ");
			sprintf(options2.name[6], " ");
			sprintf(options2.name[7], " ");
			sprintf(options2.name[8], " ");

		}
		while(menu == MENU_NONE)
		{
			VIDEO_WaitVSync ();

			if ( pageToDisplay == 1 )
			{
				if(Settings.video > 5)
					Settings.video = 0;
				if(Settings.language  > 10)
					Settings.language = 0;
				if(Settings.ocarina  > 1)
					Settings.ocarina = 0;
				if(Settings.vpatch  > 1)
					Settings.vpatch = 0;
				if(Settings.sinfo  > 3)
					Settings.sinfo = 0;
				if(Settings.hddinfo > 1)
					Settings.hddinfo = 0; //CLOCK
				if(Settings.rumble > 1)
					Settings.rumble = 0; //RUMBLE
				if(Settings.volume > 10)
					Settings.volume = 0;
                if (Settings.tooltips > 1 )
					Settings.tooltips = 0;

				if (Settings.video == discdefault) sprintf (options2.value[0],"Disc Default");
				else if (Settings.video == systemdefault) sprintf (options2.value[0],"System Default");
				else if (Settings.video == patch) sprintf (options2.value[0],"Auto Patch");
				else if (Settings.video == pal50) sprintf (options2.value[0],"Force PAL50");
				else if (Settings.video == pal60) sprintf (options2.value[0],"Force PAL60");
				else if (Settings.video == ntsc) sprintf (options2.value[0],"Force NTSC");

				if (Settings.vpatch == on) sprintf (options2.value[1],"On");
				else if (Settings.vpatch == off) sprintf (options2.value[1],"Off");

				if (Settings.language == ConsoleLangDefault) sprintf (options2.value[2],"Console Default");
				else if (Settings.language == jap) sprintf (options2.value[2],"Japanese");
				else if (Settings.language == ger) sprintf (options2.value[2],"German");
				else if (Settings.language == eng) sprintf (options2.value[2],"English");
				else if (Settings.language == fren) sprintf (options2.value[2],"French");
				else if (Settings.language == esp) sprintf (options2.value[2],"Spanish");
				else if (Settings.language == it) sprintf (options2.value[2],"Italian");
				else if (Settings.language == dut) sprintf (options2.value[2],"Dutch");
				else if (Settings.language == schin) sprintf (options2.value[2],"S. Chinese");
				else if (Settings.language == tchin) sprintf (options2.value[2],"T. Chinese");
				else if (Settings.language == kor) sprintf (options2.value[2],"Korean");

				if (Settings.ocarina == on) sprintf (options2.value[3],"On");
				else if (Settings.ocarina == off) sprintf (options2.value[3],"Off");

				if (Settings.sinfo == GameID) sprintf (options2.value[4],"Game ID");
				else if (Settings.sinfo == GameRegion) sprintf (options2.value[4],"Game Region");
				else if (Settings.sinfo == Both) sprintf (options2.value[4],"Both");
				else if (Settings.sinfo == Neither) sprintf (options2.value[4],"Neither");

				if (Settings.hddinfo == HDDInfo) sprintf (options2.value[5],"Off");
				else if (Settings.hddinfo == Clock) sprintf (options2.value[5],"On");

				if (Settings.rumble == RumbleOn) sprintf (options2.value[6],"On");
				else if (Settings.rumble == RumbleOff) sprintf (options2.value[6],"Off");

				if (Settings.volume == v10) sprintf (options2.value[7],"10");
				else if (Settings.volume == v20) sprintf (options2.value[7],"20");
				else if (Settings.volume == v30) sprintf (options2.value[7],"30");
				else if (Settings.volume == v40) sprintf (options2.value[7],"40");
				else if (Settings.volume == v50) sprintf (options2.value[7],"50");
				else if (Settings.volume == v60) sprintf (options2.value[7],"60");
				else if (Settings.volume == v70) sprintf (options2.value[7],"70");
				else if (Settings.volume == v80) sprintf (options2.value[7],"80");
				else if (Settings.volume == v90) sprintf (options2.value[7],"90");
				else if (Settings.volume == v100) sprintf (options2.value[7],"100");
				else if (Settings.volume == v0) sprintf (options2.value[7],"Off");


                if (Settings.tooltips == TooltipsOn) sprintf (options2.value[8],"On");
				else if (Settings.tooltips == TooltipsOff) sprintf (options2.value[8],"Off");

				ret = optionBrowser2.GetClickedOption();

				switch (ret)
				{
					case 0:
						Settings.video++;
						break;
					case 1:
						Settings.vpatch++;
						break;
					case 2:
						Settings.language++;
						break;
					case 3:
						Settings.ocarina++;
						break;
					case 4:  // Game Code and Region
						Settings.sinfo++;
						break;
					case 5:  //CLOCK
						Settings.hddinfo++;
						break;
					case 6:  //Rumble
						Settings.rumble++;
						break;
					case 7:
						Settings.volume++;
						break;
                    case 8:
						Settings.tooltips++;
						break;
					}
			}

			if ( pageToDisplay == 2 )
			{
				if ( Settings.cios > 1 )
					Settings.cios = 0;
				if ( Settings.xflip > 1 )
					Settings.xflip = 0;
				if ( Settings.qboot > 1 )
					Settings.qboot = 0;
				if ( Settings.wsprompt > 1 )
					Settings.wsprompt = 0;


				if ( CFG.godmode != 1) sprintf(options2.value[0], "********");
				else if (!strcmp("", Settings.unlockCode)) sprintf(options2.value[0], "<not set>");
				else sprintf(options2.value[0], Settings.unlockCode);

                if (Settings.cios == ios249) sprintf (options2.value[1],"cIOS 249");
				else if (Settings.cios == ios222) sprintf (options2.value[1],"cIOS 222");

				if (Settings.xflip == no) sprintf (options2.value[2],"No");
				else if (Settings.xflip == yes) sprintf (options2.value[2],"Yes");

				if (Settings.qboot == no) sprintf (options2.value[3],"No");
				else if (Settings.qboot == yes) sprintf (options2.value[3],"Yes");

				if (Settings.wsprompt == no) sprintf (options2.value[4],"Normal");
				else if (Settings.wsprompt == yes) sprintf (options2.value[4],"Widescreen Fix");

				sprintf (options2.value[5]," ");
				sprintf (options2.value[6]," ");
				sprintf (options2.value[7]," ");
				sprintf (options2.value[8]," ");


				ret = optionBrowser2.GetClickedOption();

				switch (ret)
				{

					case 0: // Modify Password
						if ( CFG.godmode == 1)
						{
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							char entered[20] = "";
							strncpy(entered, Settings.unlockCode, sizeof(entered));
							int result = OnScreenKeyboard(entered, 20);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&optionBrowser2);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							if ( result == 1 )
							{
								strncpy(Settings.unlockCode, entered, sizeof(Settings.unlockCode));
								WindowPrompt("Password Changed","Password has been changed","OK",0,0,0);
								cfg_save_global();
							}
						}
						else
						{
							WindowPrompt("Password change","Console should be unlocked to modify it.","OK",0,0,0);
						}
						break;
					case 1:
						Settings.cios++;
						break;
					case 2:
						Settings.xflip++;
						break;
					case 3:
						Settings.qboot++;
						break;
					case 4:
						Settings.wsprompt++;
						break;
					}
			}

			if(shutdown == 1)
				Sys_Shutdown();
			if(reset == 1)
			Sys_Reboot();

			if(page1Btn.GetState() == STATE_CLICKED)
			{
				pageToDisplay = 1;
				page1Btn.ResetState();
				page1Btn.SetImage(&page1Img);
				page2Btn.SetImage(&page2dImg);
				menu = MENU_NONE;
				break;
			}

			if(page2Btn.GetState() == STATE_CLICKED)
			{
				pageToDisplay = 2;
				menu = MENU_NONE;
				page2Btn.ResetState();
				page1Btn.SetImage(&page1dImg);
				page2Btn.SetImage(&page2Img);
				break;
			}

			if(backBtn.GetState() == STATE_CLICKED)
			{
				//Add the procedure call to save the global configuration
				if(isSdInserted() == 1) {
				cfg_save_global();
				}
				menu = MENU_DISCLIST;
				pageToDisplay = 0;
				break;
			}

			if(lockBtn.GetState() == STATE_CLICKED)
			{
				if (!strcmp("", Settings.unlockCode))
				{
					CFG.godmode = !CFG.godmode;
				}
				else if ( CFG.godmode == 0 )
				{
					//password check to unlock Install,Delete and Format
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
						char entered[20] = "";
					int result = OnScreenKeyboard(entered, 20);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&optionBrowser2);
							w.Append(&backBtn);
							w.Append(&lockBtn);
					if ( result == 1 )
					{
						if (!strcmp(entered, Settings.unlockCode)) //if password correct
						{
							if (CFG.godmode == 0)
							{
								WindowPrompt("Correct Password","Install, Rename, and Delete are unlocked.","OK",0,0,0);
								CFG.godmode = 1;
								__Menu_GetEntries();
								menu = MENU_DISCLIST;
							}
						}
						else
						{
							WindowPrompt("Wrong Password","USB Loader is protected.","OK",0,0,0);
						}
					}
				}
				else
				{
					int choice = WindowPrompt ("Lock Console","Are you sure?","Yes","No",0,0);
					if(choice == 1)
					{
						WindowPrompt("Console Locked","USB Loader is now protected.","OK",0,0,0);
						CFG.godmode = 0;
						__Menu_GetEntries();
						menu = MENU_DISCLIST;
					}
				}
				if ( CFG.godmode == 1)
				{
					lockBtnTxt.SetText("Lock");
				}
				else
				{
					lockBtnTxt.SetText("Unlock");
				}
				lockBtn.ResetState();
			}
			if(settingsbackgroundbtn.GetState() == STATE_CLICKED)
			{
			optionBrowser2.SetFocus(1);
			}
		}

	}
	HaltGui();
	delete btnLogo;
	btnLogo = NULL;
	mainWindow->Remove(&optionBrowser2);
	mainWindow->Remove(&w);
	ResumeGui();
	return menu;
}


/********************************************************************************
*Game specific settings
*********************************************************************************/
int GameSettings(struct discHdr * header)
{
	bool exit = false;
	int ret;
	int retVal = 0;

	char gameName[31];

	if (strlen(get_title(header)) < (27 + 3)) {
		sprintf(gameName, "%s", get_title(header));
	}
	else {
		strncpy(gameName, get_title(header),  27);
		gameName[27] = '\0';
		strncat(gameName, "...", 3);
	}

	customOptionList options3(5);
	sprintf(options3.name[0], "Video Mode");
	sprintf(options3.name[1], "VIDTV Patch");
	sprintf(options3.name[2], "Language");
	sprintf(options3.name[3], "Ocarina");
	sprintf(options3.name[4], "IOS");

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);

	char imgPath[100];

	GuiImageData btnOutline(settings_menu_button_png);
	snprintf(imgPath, sizeof(imgPath), "%sgamesettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText titleTxt(gameName, 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);

    GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText saveBtnTxt("Save", 22, (GXColor){0, 0, 0, 255});
	saveBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage saveBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	saveBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton saveBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	saveBtn.SetScale(0.9);
	saveBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	saveBtn.SetPosition(-180, 400);
	saveBtn.SetLabel(&saveBtnTxt);
	saveBtn.SetImage(&saveBtnImg);
	saveBtn.SetSoundOver(&btnSoundOver);
	saveBtn.SetTrigger(&trigA);
	saveBtn.SetEffectGrow();

    GuiText cancelBtnTxt("Back", 22, (GXColor){0, 0, 0, 255});
	cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	cancelBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetScale(0.9);
	cancelBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	cancelBtn.SetPosition(180, 400);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetTrigger(&trigB);
	cancelBtn.SetEffectGrow();

	GuiText deleteBtnTxt("Uninstall", 22, (GXColor){0, 0, 0, 255});
	deleteBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage deleteBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	deleteBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton deleteBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	deleteBtn.SetScale(0.9);
	deleteBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	deleteBtn.SetPosition(0, 400);
	deleteBtn.SetLabel(&deleteBtnTxt);
	deleteBtn.SetImage(&deleteBtnImg);
	deleteBtn.SetSoundOver(&btnSoundOver);
	deleteBtn.SetTrigger(&trigA);
	deleteBtn.SetEffectGrow();

	GuiCustomOptionBrowser optionBrowser3(396, 280, &options3, CFG.theme_path, "bg_options_gamesettings", bg_options_settings_png, 0);
	optionBrowser3.SetPosition(0, 90);
	optionBrowser3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser3.SetCol2Position(150);

    HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackgroundbtn);
    w.Append(&titleTxt);
	w.Append(&deleteBtn);
	w.Append(&saveBtn);
	w.Append(&cancelBtn);

    mainWindow->Append(&w);
    mainWindow->Append(&optionBrowser3);

	ResumeGui();

	struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);

	if (game_cfg)//if there are saved settings for this game use them
	{
		videoChoice = game_cfg->video;
		languageChoice = game_cfg->language;
		ocarinaChoice = game_cfg->ocarina;
		viChoice = game_cfg->vipatch;
		iosChoice = game_cfg->ios;
	}
	else// otherwise use the global settings
	{
		videoChoice = Settings.video;
		languageChoice = Settings.language;
		ocarinaChoice = Settings.ocarina;
		viChoice = Settings.vpatch;
		if(Settings.cios == ios222) {
        iosChoice = i222;
		} else {
		iosChoice = i249;
		}
	}

	while(!exit)
	{

		VIDEO_WaitVSync ();

		if (videoChoice == discdefault) sprintf (options3.value[0],"Disc Default");
		else if (videoChoice == systemdefault) sprintf (options3.value[0],"System Default");
		else if (videoChoice == patch) sprintf (options3.value[0],"Auto Patch");
		else if (videoChoice == pal50) sprintf (options3.value[0],"Force PAL50");
		else if (videoChoice == pal60) sprintf (options3.value[0],"Force PAL60");
		else if (videoChoice == ntsc) sprintf (options3.value[0],"Force NTSC");

        if (viChoice == on) sprintf (options3.value[1],"ON");
		else if (viChoice == off) sprintf (options3.value[1],"OFF");

		if (languageChoice == ConsoleLangDefault) sprintf (options3.value[2],"Console Default");
		else if (languageChoice == jap) sprintf (options3.value[2],"Japanese");
		else if (languageChoice == ger) sprintf (options3.value[2],"German");
		else if (languageChoice == eng) sprintf (options3.value[2],"English");
		else if (languageChoice == fren) sprintf (options3.value[2],"French");
		else if (languageChoice == esp) sprintf (options3.value[2],"Spanish");
        else if (languageChoice == it) sprintf (options3.value[2],"Italian");
		else if (languageChoice == dut) sprintf (options3.value[2],"Dutch");
		else if (languageChoice == schin) sprintf (options3.value[2],"S. Chinese");
		else if (languageChoice == tchin) sprintf (options3.value[2],"T. Chinese");
		else if (languageChoice == kor) sprintf (options3.value[2],"Korean");

        if (ocarinaChoice == on) sprintf (options3.value[3],"ON");
		else if (ocarinaChoice == off) sprintf (options3.value[3],"OFF");

		if (iosChoice == i249) sprintf (options3.value[4],"249");
		else if (iosChoice == i222) sprintf (options3.value[4],"222");

		if(shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();

		ret = optionBrowser3.GetClickedOption();

		switch (ret)
		{
			case 0:
				videoChoice = (videoChoice + 1) % CFG_VIDEO_COUNT;
				break;
			case 1:
				viChoice = (viChoice + 1) % 2;
				break;
            case 2:
				languageChoice = (languageChoice + 1) % CFG_LANG_COUNT;
				break;
            case 3:
				ocarinaChoice = (ocarinaChoice + 1) % 2;
				break;
			case 4:
				iosChoice = (iosChoice + 1) % 2;
				break;
		}

		if(saveBtn.GetState() == STATE_CLICKED)
		{
		    if(isSdInserted() == 1) {
				if (CFG_save_game_opt(header->id))
				{
					WindowPrompt("Successfully Saved", 0, "OK", 0,0,0);
				}
				else
				{
					WindowPrompt("Save Failed", 0, "OK", 0,0,0);
				}
		    } else {
                WindowPrompt("No SD-Card inserted!", "Insert a SD-Card to save.", "OK", 0,0,0);
		    }

			saveBtn.ResetState();
			optionBrowser3.SetFocus(1);
		}

		if (cancelBtn.GetState() == STATE_CLICKED)
		{
			exit = true;
			break;
		}

		if (deleteBtn.GetState() == STATE_CLICKED)
		{
			int choice = WindowPrompt(
					"Do you really want to delete:",
					gameName,
					"Yes","Cancel",0,0);

			if (choice == 1)
			{
				ret = WBFS_RemoveGame(header->id);
				if (ret < 0)
				{
					WindowPrompt(
					"Can't delete:",
					gameName,
					"OK",0,0,0);
				}
				else {
					__Menu_GetEntries();
					WindowPrompt(
					"Successfully deleted:",
					gameName,
					"OK",0,0,0);
					retVal = 1;
				}
				break;
			}
			else if (choice == 0)
			{
				deleteBtn.ResetState();
				optionBrowser3.SetFocus(1);
			}

		}
	}

	HaltGui();
	mainWindow->Remove(&optionBrowser3);
	mainWindow->Remove(&w);
	ResumeGui();
	return retVal;
}

/****************************************************************************
 * MenuCheck
 ***************************************************************************/
static int MenuCheck()
{
	int menu = MENU_NONE;
	int i = 0;
	int choice;
	s32 ret2;
	OptionList options;
	options.length = i;
	partitionEntry partitions[MAX_PARTITIONS];

		VIDEO_WaitVSync ();


        ret2 = WBFS_Init(WBFS_DEVICE_USB);
        if (ret2 < 0)
        {
			fatUnmount("SD");
			__io_wiisd.shutdown();
            ret2 = DiscWait("No USB Device:", "Waiting for USB Device", 0, 0, 1);
			PAD_Init();
            Wpad_Init();
            WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
            WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
            __io_wiisd.startup();
			fatMountSimple("SD", &__io_wiisd);
        }
        if (ret2 < 0) {
            WindowPrompt ("ERROR:","USB-Device not found!", "ok", 0,0,0);
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        } else {
            PAD_Init();
            Wpad_Init();
            WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
            WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
            __io_wiisd.startup();
			fatMountSimple("SD", &__io_wiisd);
        }

        ret2 = Disc_Init();
        if (ret2 < 0) {
            WindowPrompt ("Error","Could not initialize DIP module!", "ok", 0,0,0);
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }

        ret2 = WBFS_Open();

        if (ret2 < 0) {

            choice = WindowPrompt("No WBFS partition found!",
                                    "You need to format a partition.",
                                    "Format",
                                    "Return",0,0);
                if(choice == 0)
                {
                    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

                } else {
                    /* Get partition entries */
					u32 sector_size;
                    ret2 = Partition_GetEntries(partitions, &sector_size);
                    if (ret2 < 0) {

                            WindowPrompt ("No partitions found!",0, "Restart", 0,0,0);
                            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

                    }
                    menu = MENU_FORMAT;

                }
        }

		if(shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();
		//Spieleliste laden
		__Menu_GetEntries();


		menu = MENU_DISCLIST;



	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
int MainMenu(int menu)
{

	int currentMenu = menu;
	char imgPath[100];

	#ifdef HW_RVL
	snprintf(imgPath, sizeof(imgPath), "%splayer1_point.png", CFG.theme_path);
	pointer[0] = new GuiImageData(imgPath, player1_point_png);
	snprintf(imgPath, sizeof(imgPath), "%splayer2_point.png", CFG.theme_path);
	pointer[1] = new GuiImageData(imgPath, player2_point_png);
	snprintf(imgPath, sizeof(imgPath), "%splayer3_point.png", CFG.theme_path);
	pointer[2] = new GuiImageData(imgPath, player3_point_png);
	snprintf(imgPath, sizeof(imgPath), "%splayer4_point.png", CFG.theme_path);
	pointer[3] = new GuiImageData(imgPath, player4_point_png);
	#endif

	mainWindow = new GuiWindow(screenwidth, screenheight);

	if (CFG.widescreen)
		snprintf(imgPath, sizeof(imgPath), "%swbackground.png", CFG.theme_path);
	else
		snprintf(imgPath, sizeof(imgPath), "%sbackground.png", CFG.theme_path);

	background = new GuiImageData(imgPath, CFG.widescreen? wbackground_png : background_png);

    bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	ResumeGui();

    bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG, vol);
    bgMusic->SetVolume(vol);
	bgMusic->SetLoop(1); //loop music
	bgMusic->Play(); // startup music

	while(currentMenu != MENU_EXIT)
	{
	    bgMusic->SetVolume(vol);
		switch (currentMenu)
		{
			case MENU_CHECK:
				currentMenu = MenuCheck();
				break;
            case MENU_FORMAT:
				currentMenu = MenuFormat();
				break;
            case MENU_INSTALL:
				currentMenu = MenuInstall();
				break;
            case MENU_SETTINGS:
				currentMenu = MenuSettings();
				break;
            case MENU_DISCLIST:
				currentMenu = MenuDiscList();
				break;
			default: // unrecognized menu
				currentMenu = MenuCheck();
				break;
		}

		switch (Settings.volume)
		{
			case v10:
				vol = 10;
				break;
			case v20:
				vol = 20;
				break;
			case v30:
				vol = 30;
				break;
			case v40:
				vol = 40;
				break;
			case v50:
				vol = 50;
				break;
			case v60:
				vol = 60;
				break;
			case v70:
				vol = 70;
				break;
			case v80:
				vol = 80;
				break;
			case v90:
				vol = 90;
				break;
			case v100:
				vol = 100;
				break;
			case v0:
				vol = 0;
				break;
			default:
				vol = 80;
				break;
		}
	}


    bgMusic->Stop();
	delete bgMusic;
	delete background;
	delete bgImg;
	delete mainWindow;
	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];

	delete cover;
	delete coverImg;

	mainWindow = NULL;

    ExitApp();

    struct discHdr *header = &gameList[gameSelected];
    struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);

    if (game_cfg) {

        videoChoice = game_cfg->video;
        languageChoice = game_cfg->language;
        ocarinaChoice = game_cfg->ocarina;
        viChoice = game_cfg->vipatch;

    } else {

        videoChoice = Settings.video;
        languageChoice = Settings.language;
        ocarinaChoice = Settings.ocarina;
        viChoice = Settings.vpatch;
    }


    switch(languageChoice)
    {
                        case ConsoleLangDefault:
                                configbytes[0] = 0xCD;
                        break;

                        case jap:
                                configbytes[0] = 0x00;
                        break;

                        case eng:
                                configbytes[0] = 0x01;
                        break;

                        case ger:
                                configbytes[0] = 0x02;
                        break;

                        case fren:
                                configbytes[0] = 0x03;
                        break;

                        case esp:
                                configbytes[0] = 0x04;
                        break;

                        case it:
                                configbytes[0] = 0x05;
                        break;

                        case dut:
                                configbytes[0] = 0x06;
                        break;

                        case schin:
                                configbytes[0] = 0x07;
                        break;

                        case tchin:
                                configbytes[0] = 0x08;
                        break;

                        case kor:
                                configbytes[0] = 0x09;
                        break;
                        //wenn nicht genau klar ist welches
                        default:
                                configbytes[0] = 0xCD;
                        break;
    }

    u32 videoselected = 0;

    switch(videoChoice)
    {
                        case discdefault:
                                videoselected = 0;
                        break;

                        case pal50:
                                videoselected = 1;
                        break;

                        case pal60:
                                videoselected = 2;
                        break;

                        case ntsc:
                                videoselected = 3;

                        case systemdefault:

                                videoselected = 4;
                        break;
                        case patch:

                                videoselected = 5;
                        break;
                        default:
                                videoselected = 3;
                        break;
    }

    u32 cheat = 0;
    switch(ocarinaChoice)
    {
                        case on:
                                cheat = 1;
                        break;

                        case off:
                                cheat = 0;
                        break;

                        default:
                                cheat = 1;
                        break;
    }

    u8 vipatch = 0;
    switch(viChoice)
    {
                        case on:
                                vipatch = 1;
                        break;

                        case off:
                                vipatch = 0;
                        break;

                        default:
                                vipatch = 0;
                        break;
    }

    int ret = 0;
    ret = Disc_WiiBoot(videoselected, cheat, vipatch);
    if (ret < 0) {
        printf("    ERROR: BOOT ERROR! (ret = %d)\n", ret);
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    return 0;
}
