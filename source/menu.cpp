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
#include "libwiigui/gui_gamegrid.h"
#include "libwiigui/gui_gamecarousel.h"
#include "patchcode.h"
#include "wpad.h"
#include "cfg.h"
#include "language.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "libwiigui/gui_gamebrowser.h"
#include "libwiigui/gui_diskcover.h"
#include "listfiles.h"
#include "fatmounter.h"
#include "updater.h"

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
static GuiText prTxt(NULL, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
static GuiText timeTxt(NULL, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
static GuiText sizeTxt(NULL, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
static GuiText *GameIDTxt = NULL;
static GuiText *GameRegionTxt = NULL;
static GuiSound * bgMusic = NULL;
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
static int datag = 0;
int datagB =0;
int dataed = -1;
int cosa=0,sina=0,offa=0;
u8 dispFave=0;

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
    if(enable && Settings.wiilight) val |= 0x20;
    *_wiilight_reg=val;
}

//Prototypes
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *btn3Label, const char *btn4Label);
static void HaltGui();
static void ResumeGui();
extern const u8 data1;


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
	GuiSound * creditsMusic = NULL;

	if(btnLogo->GetState() != STATE_CLICKED) {
		return;
		}
    s32 thetimeofbg = bgMusic->GetPlayTime();
	StopOgg();

	creditsMusic = new GuiSound(credits_music_ogg, credits_music_ogg_size, SOUND_OGG, 55);
	creditsMusic->SetVolume(55);
	creditsMusic->SetLoop(1);
	creditsMusic->Play();

	btnLogo->ResetState();

	bool exit = false;
	int i = 0;
	int y = 20;

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
	starImg.SetPosition(505,350);

	int numEntries = 25;
	GuiText * txt[numEntries];

	txt[i] = new GuiText(LANGUAGE.Credits, 26, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,12); i++;

	char SvnRev[10];
	snprintf(SvnRev, 10, "Rev%s", SVN_REV);

	txt[i] = new GuiText(SvnRev, 18, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP); txt[i]->SetPosition(-30,y); i++; y+=34;

	txt[i] = new GuiText("USB Loader GX", 24, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y); i++; y+=26;

	txt[i] = new GuiText(": http://code.google.com/p/usbloader-gui/", 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(50,y); i++; //y+=28;

	txt[i] = new GuiText(LANGUAGE.OfficialSite, 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(-180,y); i++; y+=28;

	GuiText::SetPresets(22, (GXColor){255, 255, 255,  255}, 0, GuiText::WRAP,
			FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP, ALIGN_LEFT, ALIGN_TOP);

	txt[i] = new GuiText("Coding:");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(170,y);
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
	y+=22;
	txt[i] = new GuiText("ardi");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(320,y);
	i++;
	y+=24;

	txt[i] = new GuiText("Design:");
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP); txt[i]->SetPosition(170,y);
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
	y+=28;

	txt[i] = new GuiText(LANGUAGE.Specialthanksto);
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(-50,y);
	i++;

	txt[i] = new GuiText(":");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(30,y+3);
	i++;
	y+=22;

	txt[i] = new GuiText("Fishears/Nuke        Ocarina & WiiPower       Vidpatch");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y);
	i++;

	txt[i] = new GuiText(LANGUAGE.For);
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(-80,y);
	txt[i]->SetPosition(-80, y);
	i++;
	txt[i] = new GuiText(LANGUAGE.For);
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(130,y);
	txt[i]->SetPosition(130, y);
	i++;

	y+=22;

	txt[i] = new GuiText("Tantric         libwiigui");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y);
	i++;
	txt[i] = new GuiText(LANGUAGE.For);
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); //txt[i]->SetPosition(-3,y);
	txt[i]->SetPosition(-3, y);
	i++;
	y+=22;

	txt[i] = new GuiText("Waninkoko & Kwiirk         USB Loader");
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y);
	i++;
	txt[i] = new GuiText(LANGUAGE.For);
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(30,y);
	txt[i]->SetPosition(30, y);
	i++;
	y+=22;

	txt[i] = new GuiText(LANGUAGE.theUSBLoaderandreleasingthesourcecode);
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
				Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48, 200.0,
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

	if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
        bgMusic->Play();
    } else {
        bgMusic->PlayOggFile(CFG.ogg_path);
    }
    bgMusic->SetPlayTime(thetimeofbg);
    SetVolumeOgg(255*(vol/100.0));
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
	char imgPath[50];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);


	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);
	}

	GuiText titleTxt(title, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,55);
	GuiText msgTxt(msg, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-40);
	msgTxt.SetMaxWidth(430);

	GuiText btn1Txt(btn1Label, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);
	}

	GuiButton btn1(&btn1Img, &btn1Img, 0,3,0,0,&trigA,&btnSoundOver,&btnClick,1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

	GuiText btn2Txt(btn2Label, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn2Txt.SetWidescreen(CFG.widescreen);
	btn2Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn2(&btn2Img, &btn2Img, 0,3,0,0,&trigA,&btnSoundOver,&btnClick,1);
	btn2.SetLabel(&btn2Txt);
	if(!btn3Label && !btn4Label)
	btn2.SetTrigger(&trigB);

    GuiText btn3Txt(btn3Label, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn3Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn3Txt.SetWidescreen(CFG.widescreen);
	btn3Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn3(&btn3Img, &btn3Img, 0,3,0,0,&trigA,&btnSoundOver,&btnClick,1);
	btn3.SetLabel(&btn3Txt);
	if(!btn4Label)
	btn3.SetTrigger(&trigB);

    GuiText btn4Txt(btn4Label, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn4Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn4Txt.SetWidescreen(CFG.widescreen);
	btn4Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn4(&btn4Img, &btn4Img, 0,3,0,0,&trigA,&btnSoundOver,&btnClick,1);
	btn4.SetLabel(&btn4Txt);
	if(btn4Label)
	btn4.SetTrigger(&trigB);

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust buttons for widescreen
		msgTxt.SetMaxWidth(330);

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
 * WindowExitPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 4 Buttons.
 *
 * Give him 1 Titel, 1 Subtitel and 4 Buttons
 * If titel/subtitle or one of the buttons is not needed give him a 0 on that
 * place.
 ***************************************************************************/
int
WindowExitPrompt(const char *title, const char *msg, const char *btn1Label,
                const char *btn2Label, const char *btn3Label,
                const char *btn4Label)
{
    GuiSound * homein = NULL;
    homein = new GuiSound(menuin_ogg, menuin_ogg_size, SOUND_OGG, vol);
    homein->SetVolume(vol);
	homein->SetLoop(0);
	homein->Play();

	GuiSound * homeout = NULL;
    homeout = new GuiSound(menuout_ogg, menuout_ogg_size, SOUND_OGG, vol);
    homeout->SetVolume(vol);
	homeout->SetLoop(0);

    int choice = -1;
	char imgPath[100];
	u8 HBC=0;
	GuiWindow promptWindow(640,480);
	promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	promptWindow.SetPosition(0, 0);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	GuiImageData top(exit_top_png);
	GuiImageData topOver(exit_top_over_png);
	GuiImageData bottom(exit_bottom_png);
	GuiImageData bottomOver(exit_bottom_over_png);
	GuiImageData button(exit_button_png);
	GuiImageData wiimote(wiimote_png);
	GuiImageData close(closebutton_png);

	snprintf(imgPath, sizeof(imgPath), "%sbattery_white.png", CFG.theme_path);
	GuiImageData battery(imgPath, battery_white_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
	GuiImageData batteryRed(imgPath, battery_red_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar_white.png", CFG.theme_path);
	GuiImageData batteryBar(imgPath, battery_bar_white_png);

	#ifdef HW_RVL
	int i = 0, ret = 0, level;
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

		batteryTxt[i] = new GuiText(txt, 22, (GXColor){255,255,255, 255});
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
		batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBtn[i]->SetRumble(false);
		batteryBtn[i]->SetAlpha(70);
		batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
	}

	batteryBtn[0]->SetPosition(180,150);
	batteryBtn[1]->SetPosition(284, 150);
	batteryBtn[2]->SetPosition(388, 150);
	batteryBtn[3]->SetPosition(494, 150);


    char * sig = (char *)0x80001804;
    if(
        sig[0] == 'S' &&
        sig[1] == 'T' &&
        sig[2] == 'U' &&
        sig[3] == 'B' &&
        sig[4] == 'H' &&
        sig[5] == 'A' &&
        sig[6] == 'X' &&
        sig[7] == 'X')
        HBC=1; // Exit to HBC
    #endif

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText titleTxt(LANGUAGE.Homemenu, 36, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(-180,40);
	titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	GuiText closeTxt(LANGUAGE.Close, 28, (GXColor){0, 0, 0, 255});
	closeTxt.SetPosition(10,3);
	GuiImage closeImg(&close);
	if (Settings.wsprompt == yes){
	closeTxt.SetWidescreen(CFG.widescreen);
	closeImg.SetWidescreen(CFG.widescreen);
	}
	GuiButton closeBtn(close.GetWidth(), close.GetHeight());
	closeBtn.SetImage(&closeImg);
	closeBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	closeBtn.SetPosition(190,30);
	closeBtn.SetLabel(&closeTxt);
	closeBtn.SetRumble(false);
	closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	GuiImage btn1Img(&top);
	GuiImage btn1OverImg(&topOver);
	GuiButton btn1(&btn1Img,&btn1OverImg, 0, 3, 0, 0, &trigA, &btnSoundOver, &btnClick,0);
	btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	GuiText btn2Txt(btn1Label, 28, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&button);
	if (Settings.wsprompt == yes){
	btn2Txt.SetWidescreen(CFG.widescreen);
	btn2Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn2(&btn2Img,&btn2Img, 2, 5, -150, 0, &trigA, &btnSoundOver, &btnClick,1);
	btn2.SetLabel(&btn2Txt);
	btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
	btn2.SetRumble(false);
	if (HBC==1){btn2.SetPosition(-150, 0);}

    GuiText btn3Txt(btn2Label, 28, (GXColor){0, 0, 0, 255});
	GuiImage btn3Img(&button);
	if (Settings.wsprompt == yes){
	btn3Txt.SetWidescreen(CFG.widescreen);
	btn3Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn3(&btn3Img,&btn3Img, 2, 5, 150, 0, &trigA, &btnSoundOver, &btnClick,1);
	btn3.SetLabel(&btn3Txt);
	btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
	btn3.SetRumble(false);
	if (HBC==1){btn3.SetPosition(150, 0);}
	else {btn3.SetPosition(0,0);}

	GuiImage btn4Img(&bottom);
	GuiImage btn4OverImg(&bottomOver);
	GuiButton btn4(&btn4Img,&btn4OverImg, 0, 4, 0, 0, &trigA, &btnSoundOver, &btnClick,0);
	btn4.SetTrigger(&trigB);
	btn4.SetTrigger(&trigHome);
	btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

	GuiImage wiimoteImg(&wiimote);
	if (Settings.wsprompt == yes){wiimoteImg.SetWidescreen(CFG.widescreen);}
	wiimoteImg.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
	wiimoteImg.SetPosition(50,210);

	if (HBC==1){promptWindow.Append(&btn2);}
    promptWindow.Append(&btn3);
    promptWindow.Append(&btn4);
    promptWindow.Append(&btn1);
	promptWindow.Append(&closeBtn);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&wiimoteImg);

	#ifdef HW_RVL
	promptWindow.Append(batteryBtn[0]);
    promptWindow.Append(batteryBtn[1]);
    promptWindow.Append(batteryBtn[2]);
    promptWindow.Append(batteryBtn[3]);
    #endif

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	while(choice == -1)
	{
		VIDEO_WaitVSync();

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


		if(shutdown == 1)
		{
			wiilight(0);
			Sys_Shutdown();
		}
		if(reset == 1)
			Sys_Reboot();
		if(btn1.GetState() == STATE_CLICKED) {
			choice = 1;
			btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
            btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
            btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
			#ifdef HW_RVL
			for (int i = 0; i < 4; i++)
			batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
            #endif
		}
		else if(btn4.GetState() == STATE_SELECTED)
		{
			wiimoteImg.SetPosition(50,165);
		}
		else if(btn2.GetState() == STATE_CLICKED) {
            ret = WindowPrompt(LANGUAGE.Areyousure, 0, LANGUAGE.Yes, LANGUAGE.No, 0, 0);
			if (ret == 1) {
			choice = 2;
			}
			HaltGui();
            mainWindow->SetState(STATE_DISABLED);
			promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
			ResumeGui();
			btn2.ResetState();
		}
		else if(btn3.GetState() == STATE_CLICKED) {
			ret = WindowPrompt(LANGUAGE.Areyousure, 0, LANGUAGE.Yes, LANGUAGE.No, 0, 0);
			if (ret == 1) {
			choice = 3;
			}
			HaltGui();
			mainWindow->SetState(STATE_DISABLED);
			promptWindow.SetState(STATE_DEFAULT);
			mainWindow->ChangeFocus(&promptWindow);
			ResumeGui();
			btn3.ResetState();
		}
		else if(btn4.GetState() == STATE_CLICKED) {
		    btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
			btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
            btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
			wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
			#ifdef HW_RVL
			for (int i = 0; i < 4; i++)
			batteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
            #endif
			choice = 0;
		}
		else if(btn4.GetState() != STATE_SELECTED)
		{
			wiimoteImg.SetPosition(50,210);
		}
	}
    homeout->Play();
    while(btn1.GetEffect() > 0) usleep(50);
	while(promptWindow.GetEffect() > 0) usleep(50);
	HaltGui();
	homein->Stop();
	delete homein;
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
    while(homeout->IsPlaying() > 0) usleep(50);
    homeout->Stop();
	delete homeout;
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
	f32 size = 0.0;
	char ID[5];
	char IDFull[7];

	u8 faveChoice = 0;
	u16 playCount = 0;

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);

	snprintf(imgPath, sizeof(imgPath), "%sfavorite.png", CFG.theme_path);
	GuiImageData imgFavorite(imgPath, favorite_png);
	snprintf(imgPath, sizeof(imgPath), "%snot_favorite.png", CFG.theme_path);
	GuiImageData imgNotFavorite(imgPath, not_favorite_png);

	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	GuiImageData imgLeft(imgPath, startgame_arrow_left_png);
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	GuiImageData imgRight(imgPath, startgame_arrow_right_png);

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

	if (CFG.widescreen)
		snprintf(imgPath, sizeof(imgPath), "%swdialogue_box_startgame.png", CFG.theme_path);
	else
		snprintf(imgPath, sizeof(imgPath), "%sdialogue_box_startgame.png", CFG.theme_path);

	GuiImageData dialogBox(imgPath, CFG.widescreen ? wdialogue_box_startgame_png : dialogue_box_startgame_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiTooltip nameBtnTT(LANGUAGE.RenameGameonWBFS);
	if (Settings.wsprompt == yes)
		nameBtnTT.SetWidescreen(CFG.widescreen);
	GuiText nameTxt("", 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{50, 50, 50, 255});
	if (Settings.wsprompt == yes)
		nameTxt.SetWidescreen(CFG.widescreen);
	nameTxt.SetMaxWidth(350, GuiText::SCROLL);
	GuiButton nameBtn(120,50);
	nameBtn.SetLabel(&nameTxt);
//	nameBtn.SetLabelOver(&nameTxt);
	nameBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	nameBtn.SetPosition(0,-122);
	nameBtn.SetSoundOver(&btnSoundOver);
	nameBtn.SetSoundClick(&btnClick);
	nameBtn.SetToolTip(&nameBtnTT,24,-30, ALIGN_LEFT);

	if (CFG.godmode == 1){
		nameBtn.SetTrigger(&trigA);
		nameBtn.SetEffectGrow();
	}

    GuiText sizeTxt(NULL, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{50, 50, 50, 255}); //TODO: get the size here
	sizeTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	sizeTxt.SetPosition(-60,70);

//	GuiImage diskImg;
	GuiDiskCover diskImg;
	diskImg.SetWidescreen(CFG.widescreen);
	diskImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	diskImg.SetAngle(angle);
	GuiDiskCover diskImg2;
	diskImg2.SetWidescreen(CFG.widescreen);
	diskImg2.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	diskImg2.SetPosition(0, -20);
	diskImg2.SetAngle(angle);
	diskImg2.SetBeta(180);

	GuiText playcntTxt(NULL, 18, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
	playcntTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	playcntTxt.SetPosition(-115,45);

	GuiButton btn1(160, 160);
	btn1.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	btn1.SetPosition(0, -20);
	btn1.SetImage(&diskImg);

	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnClick);
	btn1.SetTrigger(&trigA);
	btn1.SetState(STATE_SELECTED);

	GuiText btn2Txt(LANGUAGE.Back, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn2Txt.SetWidescreen(CFG.widescreen);
	btn2Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn2(&btn2Img,&btn2Img, 1, 5, 0, 0, &trigA, &btnSoundOver, &btnClick,1);
	if (CFG.godmode == 1)
	{
		btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
		btn2.SetPosition(-50, -40);
	}
	else
	{
		btn2.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn2.SetPosition(0, -40);
	}

	btn2.SetLabel(&btn2Txt);
	btn2.SetTrigger(&trigB);

	GuiText btn3Txt(LANGUAGE.settings, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn3Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn3Txt.SetWidescreen(CFG.widescreen);
	btn3Img.SetWidescreen(CFG.widescreen);}
	GuiButton btn3(&btn3Img,&btn3Img, 0, 4, 50, -40, &trigA, &btnSoundOver, &btnClick,1);
	btn3.SetLabel(&btn3Txt);

	GuiImage btnFavoriteImg;
	btnFavoriteImg.SetWidescreen(CFG.widescreen);
	//GuiButton btnFavorite(&btnFavoriteImg,&btnFavoriteImg, 2, 5, -125, -60, &trigA, &btnSoundOver, &btnClick,1);
	GuiButton btnFavorite(imgFavorite.GetWidth(), imgFavorite.GetHeight());
	btnFavorite.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	btnFavorite.SetPosition(-125, -60);
	btnFavorite.SetImage(&btnFavoriteImg);
	btnFavorite.SetSoundOver(&btnSoundOver);
	btnFavorite.SetSoundClick(&btnClick);
	btnFavorite.SetTrigger(&trigA);
	btnFavorite.SetEffectGrow();

	GuiImage btnLeftImg(&imgLeft);
	if (Settings.wsprompt == yes)
		{
			btnLeftImg.SetWidescreen(CFG.widescreen);
		}
	GuiButton btnLeft(&btnLeftImg,&btnLeftImg, 0, 5, 20, 0, &trigA, &btnSoundOver, &btnClick,1);
	btnLeft.SetTrigger(&trigL);
	btnLeft.SetTrigger(&trigMinus);

	GuiImage btnRightImg(&imgRight);
	if (Settings.wsprompt == yes)
		{
			btnRightImg.SetWidescreen(CFG.widescreen);
		}
	GuiButton btnRight(&btnRightImg,&btnRightImg, 1, 5, -20, 0, &trigA, &btnSoundOver, &btnClick,1);
	btnRight.SetTrigger(&trigR);
	btnRight.SetTrigger(&trigPlus);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&nameBtn);
	promptWindow.Append(&sizeTxt);
	promptWindow.Append(&playcntTxt);
	promptWindow.Append(&btn2);
	promptWindow.Append(&btnLeft);
	promptWindow.Append(&btnRight);
	promptWindow.Append(&btnFavorite);

	//check if unlocked
	if (CFG.godmode == 1)
	{
    promptWindow.Append(&btn3);
	}

	promptWindow.Append(&diskImg2);
	promptWindow.Append(&btn1);

	short changed = -1;
	GuiImageData * diskCover = NULL;
	GuiImageData * diskCover2 = NULL;

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	while (changed)
	{
		if (changed == 1){
			promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
		}
		else if (changed == 2){
			promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
		}
		else if (changed == 3 || changed == 4)
		{
			if(diskCover2)
				delete diskCover2;
			diskCover2 = NULL;
			if(diskCover)
				diskCover2 = diskCover;
			diskCover = NULL;
		}

		//load disc image based or what game is seleted
		struct discHdr * header = &gameList[gameSelected];

		snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
		snprintf (IDFull,sizeof(IDFull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

		if (diskCover)
			delete diskCover;

		snprintf(imgPath,sizeof(imgPath),"%s%s.png", CFG.disc_path, IDFull); //changed to current full id
		diskCover = new GuiImageData(imgPath,0);

		if (!diskCover->GetImage())
		{
			delete diskCover;
			snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.disc_path, ID); //changed to current id
			diskCover = new GuiImageData(imgPath, 0);


		if (!diskCover->GetImage())
		{	snprintf (ID,sizeof(ID),"%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3]);

			delete diskCover;
			snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.disc_path, ID); //changed to current id
			diskCover = new GuiImageData(imgPath, 0);
			if (!diskCover->GetImage())
			{
				delete diskCover;
				snprintf(imgPath, sizeof(imgPath), "%snodisc.png", CFG.disc_path); //changed to nodisc.png
				diskCover = new GuiImageData(imgPath,nodisc_png);
			}
		}
		}



		if (changed == 3){
			diskImg.SetImage(diskCover2);
			diskImg.SetBeta(0);
			diskImg.SetBetaRotateEffect(-90, 15);
			diskImg2.SetImage(diskCover);
			diskImg2.SetAngle(diskImg.GetAngle());
			diskImg2.SetBeta(180);
			diskImg2.SetBetaRotateEffect(-90, 15);
			sizeTxt.SetEffect(EFFECT_FADE, -17);
			nameTxt.SetEffect(EFFECT_FADE, -17);
			ResumeGui();
			while(nameTxt.GetEffect() > 0 || diskImg.GetBetaRotateEffect()) usleep(50);
			HaltGui();
			diskImg.SetImage(diskCover);
			diskImg.SetBeta(90);
			diskImg.SetBetaRotateEffect(-90, 15);
			diskImg2.SetImage(diskCover2);
			diskImg2.SetBeta(270);
			diskImg2.SetBetaRotateEffect(-90, 15);
			sizeTxt.SetEffect(EFFECT_FADE, 17);
			nameTxt.SetEffect(EFFECT_FADE, 17);
		}
		else if (changed == 4){
			diskImg.SetImage(diskCover2);
			diskImg.SetBeta(0);
			diskImg.SetBetaRotateEffect(90, 15);
			diskImg2.SetImage(diskCover);
			diskImg2.SetAngle(diskImg.GetAngle());
			diskImg2.SetBeta(180);
			diskImg2.SetBetaRotateEffect(90, 15);
			sizeTxt.SetEffect(EFFECT_FADE, -17);
			nameTxt.SetEffect(EFFECT_FADE, -17);
			ResumeGui();
			while(nameTxt.GetEffect() > 0 || diskImg.GetBetaRotateEffect()) usleep(50);
			HaltGui();
			diskImg.SetImage(diskCover);
			diskImg.SetBeta(270);
			diskImg.SetBetaRotateEffect(90, 15);
			diskImg2.SetImage(diskCover2);
			diskImg2.SetBeta(90);
			diskImg2.SetBetaRotateEffect(90, 15);
			sizeTxt.SetEffect(EFFECT_FADE, 17);
			nameTxt.SetEffect(EFFECT_FADE, 17);
		}
		else
			diskImg.SetImage(diskCover);

		WBFS_GameSize(header->id, &size);
		sizeTxt.SetTextf("%.2fGB", size); //set size text;
		nameTxt.SetText(get_title(header));

		struct Game_NUM* game_num = CFG_get_game_num(header->id);
		if (game_num) {
			playCount = game_num->count;
			faveChoice = game_num->favorite;
		} else {
			playCount = 0;
			faveChoice = 0;
		}
		playcntTxt.SetTextf("%s: %i",LANGUAGE.Plays, playCount);
 		btnFavoriteImg.SetImage(faveChoice ? &imgFavorite : &imgNotFavorite);

		nameTxt.SetPosition(0, 1);

		if(changed != 3 && changed != 4) // changed==3 or changed==4 --> only Resume the GUI
		{
			HaltGui();
			mainWindow->SetState(STATE_DISABLED);
			mainWindow->Append(&promptWindow);
			mainWindow->ChangeFocus(&promptWindow);
		}
		ResumeGui();

		changed = 0;
		while(choice == -1)
		{
			diskImg.SetSpin(btn1.GetState() == STATE_SELECTED);
			diskImg2.SetSpin(btn1.GetState() == STATE_SELECTED);
			if(shutdown == 1) //for power button
			{
				wiilight(0);
				Sys_Shutdown();
			}
			if(reset == 1) //for reset button
				Sys_Reboot();

			if(btn1.GetState() == STATE_CLICKED) { //boot
				//////////save game play count////////////////
				extern u8 favorite;
				extern u16 count;
				struct Game_NUM* game_num = CFG_get_game_num(header->id);
				if (game_num)
					{
					favorite = game_num->favorite;
					count = game_num->count;//count+=1;
					}count+=1;
				if(isSdInserted()) {
				if (CFG_save_game_num(header->id))
				{
					//WindowPrompt(LANGUAGE.SuccessfullySaved, 0, LANGUAGE.ok, 0,0,0);
				}
				else
				{
					//WindowPrompt(LANGUAGE.SaveFailed, 0, LANGUAGE.ok, 0,0,0);
				}
				}
				////////////end save play count//////////////

				choice = 1;
				SDCard_deInit();
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

			else if(btnFavorite.GetState() == STATE_CLICKED){//switch favorite
				if(isSdInserted()) {
					faveChoice = !faveChoice;
					btnFavoriteImg.SetImage(faveChoice ? &imgFavorite : &imgNotFavorite);
					extern u8 favorite;
					extern u8 count;
					struct Game_NUM* game_num = CFG_get_game_num(header->id);
					if (game_num) {
						favorite = game_num->favorite;
						count = game_num->count;
					}
					favorite = faveChoice;
					CFG_save_game_num(header->id);
				}
				btnFavorite.ResetState();
			}

			// this next part is long because nobody could agree on what the left/right buttons should do
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

			else if((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == sysmenu)){//previous game
				promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
				changed = 2;
				btnClick.Play();
				gameSelected = (gameSelected + 1) % gameCnt;
				btnRight.ResetState();
				break;
			}

			else if((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == sysmenu)){//netx game
				promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
				changed = 1;
				btnClick.Play();
				gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
				btnLeft.ResetState();
				break;
			}

			else if((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == wtf)){//previous game
				promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
				changed = 1;
				btnClick.Play();
				gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
				btnRight.ResetState();
				break;
			}

			else if((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == wtf)){//netx game
				promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
				changed = 2;
				btnClick.Play();
				gameSelected = (gameSelected + 1) % gameCnt;
				btnLeft.ResetState();
				break;
			}

			else if((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == disk3d)){//next game
//				diskImg.SetBetaRotateEffect(45, 90);
				changed = 3;
				btnClick.Play();
				gameSelected = (gameSelected + 1) % gameCnt;
				btnRight.ResetState();
				break;
			}

			else if((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == disk3d)){//previous game
//				diskImg.SetBetaRotateEffect(-45, 90);
//				promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 1/*50*/);
				changed = 4;
				btnClick.Play();
				gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
				btnLeft.ResetState();
				break;
			}
		}


		while(promptWindow.GetEffect() > 0) usleep(50);
		HaltGui();
		if(changed != 3 && changed != 4) // changed==3 or changed==4 --> only Halt the GUI
		{
			mainWindow->Remove(&promptWindow);
			ResumeGui();
		}
	}
	delete diskCover;
	delete diskCover2;

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

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);
	}

	GuiText titleTxt(title, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);
	GuiText msgTxt(msg, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-40);
	msgTxt.SetMaxWidth(430);

	GuiText btn1Txt(btn1Label, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn1(&btn1Img,&btn1Img, 1, 5, 0, 0, &trigA, &btnSoundOver, &btnClick,1);

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
	btn1.SetTrigger(&trigB);
	btn1.SetState(STATE_SELECTED);

	GuiText btn2Txt(btn2Label, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn2Txt.SetWidescreen(CFG.widescreen);
	btn2Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn2(&btn2Img,&btn2Img, 1, 4, -20, -25, &trigA, &btnSoundOver, &btnClick,1);
	btn2.SetLabel(&btn2Txt);

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

	GuiText timerTxt(NULL, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
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

    SDCard_deInit();

	if(IsDeviceWait) {
        while(i >= 0)
        {
            timerTxt.SetTextf("%u%s", i,LANGUAGE.secondsleft);
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

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);


	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);
	}

	GuiText titleTxt(title, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
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

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);
	}

	GuiText titleTxt(LANGUAGE.InitializingNetwork, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);

	char msg[20] = " ";
	GuiText msgTxt(msg, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-40);

    GuiText btn1Txt(LANGUAGE.Cancel, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);
	}
	GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -45, &trigA, &btnSoundOver, &btnClick,1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

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
        msgTxt.SetText(LANGUAGE.Couldnotinitializenetwork);
		}

		if (IP && ret > 0) {
			msgTxt.SetTextf("IP: %s", IP);
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
	f32 percent = (done * 100.0) / total;

    prTxt.SetTextf("%0.2f", percent);

    timeTxt.SetTextf("%s %d:%02d:%02d",LANGUAGE.Timeleft,h,m,s);

    f32 gamesizedone = gamesize * done/total;

	sizeTxt.SetTextf("%0.2fGB/%0.2fGB", gamesizedone, gamesize);

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){
	progressbarImg.SetTile((int)(80*done/total));}
	else {progressbarImg.SetTile((int)(100*done/total));}

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
	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
	GuiImageData progressbarOutline(imgPath, progressbar_outline_png);

	GuiImage progressbarOutlineImg(&progressbarOutline);
	if (Settings.wsprompt == yes){
	progressbarOutlineImg.SetWidescreen(CFG.widescreen);}
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 40);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_empty.png", CFG.theme_path);
	GuiImageData progressbarEmpty(imgPath, progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 40);
	progressbarEmptyImg.SetTile(100);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar.png", CFG.theme_path);
	GuiImageData progressbar(imgPath, progressbar_png);

	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 40);

	GuiText titleTxt(title, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);
	GuiText msgTxt(msg, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,120);

	GuiText prsTxt("%", 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
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

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
	GuiImageData progressbarOutline(imgPath, progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	if (Settings.wsprompt == yes){
	progressbarOutlineImg.SetWidescreen(CFG.widescreen);}
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 40);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_empty.png", CFG.theme_path);
	GuiImageData progressbarEmpty(imgPath, progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 40);
	progressbarEmptyImg.SetTile(100);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar.png", CFG.theme_path);
	GuiImageData progressbar(imgPath, progressbar_png);
	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 40);

	GuiText titleTxt(LANGUAGE.Downloadingfile, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,60);

	GuiText msgTxt(NULL, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,130);

	GuiText msg2Txt(NULL, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msg2Txt.SetPosition(0,100);

	prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	prTxt.SetPosition(0, 40);

    GuiText btn1Txt(LANGUAGE.Cancel, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);}
	GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -45, &trigA, &btnSoundOver, &btnClick,1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

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
            if(subfoldercheck(dircovers) != 1) {
            WindowPrompt(LANGUAGE.Error,LANGUAGE.Cantcreatedirectory,LANGUAGE.ok,0,0,0);
            cntMissFiles = 0;
            }
        }
    }
    if(stat(CFG.disc_path,&st) != 0) {
        char dirdiscs[100];
        snprintf(dirdiscs,strlen(CFG.disc_path),"%s",CFG.disc_path);
        if (mkdir(dirdiscs, 0777) == -1) {
            if(subfoldercheck(dirdiscs) != 1) {
            WindowPrompt(LANGUAGE.Error,LANGUAGE.Cantcreatedirectory,LANGUAGE.ok,0,0,0);
            cntMissFiles = 0;
            }
        }
    }

	while (i < cntMissFiles) {

	prTxt.SetTextf("%i%%", 100*i/cntMissFiles);

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
		progressbarImg.SetPosition(80,40);
		progressbarImg.SetTile(80*i/cntMissFiles);
	} else {
        progressbarImg.SetTile(100*i/cntMissFiles);
	}

    msgTxt.SetTextf("%i %s", cntMissFiles - i, LANGUAGE.filesleft);
    msg2Txt.SetTextf("%s", missingFiles[i]);

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

    /**Temporary redownloading 1st image because of a fucking corruption bug **/

    char URLFile[100];
    if (choice2 == 2) {
		sprintf(URLFile,"http://www.theotherzone.com/wii/3d/176/248/%s",missingFiles[0]); // For 3D Covers
		sprintf(imgPath,"%s%s", CFG.covers_path, missingFiles[0]);
    }
    if(choice2 == 3) {
		sprintf(URLFile,"http://www.theotherzone.com/wii/diskart/160/160/%s",missingFiles[0]);
		sprintf(imgPath,"%s%s", CFG.disc_path, missingFiles[0]);
    }
    if(choice2 == 1) {
		sprintf(URLFile,"http://www.theotherzone.com/wii/resize/160/224/%s",missingFiles[0]);
		sprintf(imgPath,"%s%s", CFG.covers_path, missingFiles[0]);
    }

    struct block file = downloadfile(URLFile);

    if (file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.data == NULL) {
    } else {
    if(file.data != NULL)
    {
        // save png to sd card
        FILE *pfile;
        pfile = fopen(imgPath, "wb");
        fwrite(file.data,1,file.size,pfile);
        fclose (pfile);
        free(file.data);
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
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
int
ProgressUpdateWindow()
{

    int ret = 0, failed = 0;
    const unsigned int blocksize = 1024;
    char hostip[16];
    char * IP = NULL;
    u8 blockbuffer[blocksize] ATTRIBUTE_ALIGN(32);

	GuiWindow promptWindow(472,320);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
	GuiImageData dialogBox(imgPath, dialogue_box_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImage dialogBoxImg(&dialogBox);
	if (Settings.wsprompt == yes){
	dialogBoxImg.SetWidescreen(CFG.widescreen);}

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
	GuiImageData progressbarOutline(imgPath, progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	if (Settings.wsprompt == yes){
	progressbarOutlineImg.SetWidescreen(CFG.widescreen);}
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 7);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar_empty.png", CFG.theme_path);
	GuiImageData progressbarEmpty(imgPath, progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 7);
	progressbarEmptyImg.SetTile(100);

	snprintf(imgPath, sizeof(imgPath), "%sprogressbar.png", CFG.theme_path);
	GuiImageData progressbar(imgPath, progressbar_png);
	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 7);

    char title[50];
    sprintf(title, "%s", LANGUAGE.CheckingforUpdates);
	GuiText titleTxt(title, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,50);
    char msg[50];
    sprintf(msg, "%s", LANGUAGE.InitializingNetwork);
	GuiText msgTxt(msg, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,140);
	char msg2[50] = " ";
	GuiText msg2Txt(msg2, 26, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msg2Txt.SetPosition(0, 50);

	prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	prTxt.SetPosition(0, 7);

    GuiText btn1Txt(LANGUAGE.Cancel, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	if (Settings.wsprompt == yes){
	btn1Txt.SetWidescreen(CFG.widescreen);
	btn1Img.SetWidescreen(CFG.widescreen);}
	GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -40, &trigA, &btnSoundOver, &btnClick,1);
	btn1.SetLabel(&btn1Txt);
	btn1.SetState(STATE_SELECTED);

	if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
		progressbarOutlineImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
		progressbarOutlineImg.SetPosition(0, 7);
		progressbarEmptyImg.SetPosition(80,7);
		progressbarEmptyImg.SetTile(78);
		progressbarImg.SetPosition(80, 7);
	}

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&msg2Txt);
    promptWindow.Append(&btn1);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

    struct stat st;
    if(stat(CFG.update_path, &st) != 0) {
        char dir[100];
        snprintf(dir,strlen(CFG.update_path),"%s",CFG.update_path);
        if (mkdir(dir, 0777) == -1) {
            if(subfoldercheck(dir) != 1) {
            WindowPrompt(LANGUAGE.Error,LANGUAGE.Cantcreatedirectory,LANGUAGE.ok,0,0,0);
            ret = -1;
            failed = -1;
            }
        }
    }

    char dolpath[150];
    char dolpathsuccess[150];
    snprintf(dolpath, sizeof(dolpath), "%sbootnew.dol", CFG.update_path);
    snprintf(dolpathsuccess, sizeof(dolpathsuccess), "%sboot.dol", CFG.update_path);

	while (!IP && !(ret < 0)) {

        VIDEO_WaitVSync();

        ret = Net_Init(hostip);

		if (ret > 0) {
		IP = hostip;
		msgTxt.SetText(IP);
		}

		if (ret <= 0) {
        msgTxt.SetText(LANGUAGE.Couldnotinitializenetwork);
		}

        if(btn1.GetState() == STATE_CLICKED) {
			IP = 0;
			ret = -1;
			failed = -1;
			btn1.ResetState();
			break;
		}
	}

	if(IP && ret >= 0) {

    networkisinitialized = 1;

    int revnumber = 0;
    int currentrev = atoi(SVN_REV);

	SDCard_deInit();
    struct block file = downloadfile("http://www.techjawa.com/usbloadergx/rev.txt");
	FILE *pfile;
	SDCard_Init();

    if(file.data != NULL)
    {
        char revtxt[10];
        pfile = fopen("SD:/rev.txt", "w");
        fwrite(file.data,1,file.size,pfile);
        fclose(pfile);
        //has to be repeated or it isnt working (first file download bug)
        pfile = fopen("SD:/rev.txt", "w");
        fwrite(file.data,1,file.size,pfile);
        fclose(pfile);
        //"w+" doesnt work, needs to be reopened as "r"
        pfile = fopen("SD:/rev.txt", "r");
        int c = 0, i = 0;
        while(c != EOF || i < 10) {
            c = fgetc(pfile);
            if (c != EOF) {
            revtxt[i] = c;
            } else {
            revtxt[i] = 0x00;
            break;
            }
            i++;
        }
        fclose(pfile);
        revnumber = atoi(revtxt);
        remove("SD:/rev.txt");
        free(file.data);
    }

    if(revnumber > currentrev) {
        sprintf(msg, "Rev%i %s.", revnumber, LANGUAGE.available);
        int choice = WindowPrompt(msg, LANGUAGE.Doyouwanttoupdate, LANGUAGE.Updatedol, LANGUAGE.Updateall, LANGUAGE.Cancel, 0);
        if(choice == 1 || choice == 2) {
            titleTxt.SetTextf("%s USB Loader GX", LANGUAGE.updating);
            msgTxt.SetPosition(0,100);
            promptWindow.Append(&progressbarEmptyImg);
            promptWindow.Append(&progressbarImg);
            promptWindow.Append(&progressbarOutlineImg);
            promptWindow.Append(&prTxt);
            msgTxt.SetTextf("Updating to Rev%i", revnumber);
            int filesize = downloadrev("http://www.techjawa.com/usbloadergx/boot.dol");
            if(filesize > 0) {
                pfile = fopen(dolpath, "wb");
                for (int i = 0; i < filesize; i += blocksize) {
                    prTxt.SetTextf("%i%%", 100*i/filesize);
                    if ((Settings.wsprompt == yes) && (CFG.widescreen)){/////////////adjust for widescreen
                        progressbarImg.SetTile(80*i/filesize);
                    } else {
                        progressbarImg.SetTile(100*i/filesize);
                    }
                    msg2Txt.SetTextf("%iKB/%iKB", i/1024, filesize/1024);

                    if(btn1.GetState() == STATE_CLICKED) {
                        fclose(pfile);
                        remove(dolpath);
                        failed = -1;
                        btn1.ResetState();
                        break;
                    }

                    u32 blksize;
                    blksize = (u32)(filesize - i);
                    if (blksize > blocksize)
                        blksize = blocksize;

                    ret = network_read(blockbuffer, blksize);
                    if ((u32)ret != blksize) {
                        failed = -1;
                        ret = -1;
                        fclose(pfile);
                        remove(dolpath);
                        break;
                    }
                    fwrite(blockbuffer,1,blksize, pfile);
                }
                fclose(pfile);
                if(!failed) {
                //remove old
                if(checkfile(dolpathsuccess)){
                        remove(dolpathsuccess);
                }
                //rename new to old
                rename(dolpath, dolpathsuccess);

                if(choice == 2) {
                //get the icon.png and the meta.xml
                char xmliconpath[150];
                file = downloadfile("http://www.techjawa.com/usbloadergx/meta.file");
                if(file.data != NULL){
                    sprintf(xmliconpath, "%smeta.xml", CFG.update_path);
                    pfile = fopen(xmliconpath, "wb");
                    fwrite(file.data,1,file.size,pfile);
                    fclose(pfile);
                    free(file.data);
                }
                file = downloadfile("http://www.techjawa.com/usbloadergx/icon.png");
                if(file.data != NULL){
                    sprintf(xmliconpath, "%sicon.png", CFG.update_path);
                    pfile = fopen(xmliconpath, "wb");
                    fwrite(file.data,1,file.size,pfile);
                    fclose(pfile);
                    free(file.data);
                }
                }
                }
            } else {
            failed = -1;
            }
        } else {
        ret = -1;
        }

    } else {
        WindowPrompt(LANGUAGE.Nonewupdates, 0, LANGUAGE.ok, 0, 0, 0);
        ret = -1;
    }

    }

    CloseConnection();

    if(!failed && ret >= 0) {
        WindowPrompt(LANGUAGE.Successfullyupdated , LANGUAGE.Restarting, LANGUAGE.ok, 0, 0, 0);
        ExitApp();
        if (*((u32*) 0x80001800)) exit(0);
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(50);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

    if(failed != 0)
    return failed;

    return 1;
}


/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/
int noControllers=0;
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
			if (Settings.tooltips == TooltipsOn && THEME.showToolTip != 0 && mainWindow->GetState() != STATE_DISABLED)
				mainWindow->DrawTooltip();

			#ifdef HW_RVL
			for(int i=3; i >= 0; i--) // so that player 1's cursor appears on top!
			{
				if(userInput[i].wpad.ir.valid)
					Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48, 200.0,
						96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, CFG.widescreen? 0.8 : 1, 1, 255);
				if(Settings.rumble == RumbleOn)
				{
					DoRumble(i);
				}
				/*if(WPAD_Probe(i, NULL) == WPAD_ERR_NO_CONTROLLER){
					noControllers++;
					if (noControllers == 4){
						mainWindow->SetState(STATE_DISABLED);
					}
				}
				else {noControllers =0;
					if (mainWindow->GetState()==STATE_DISABLED);

					mainWindow->SetState(STATE_DEFAULT);}*/

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

s32 __Menu_EntryCmpCount(const void *a, const void *b)
{
	s32 ret;

	struct discHdr *hdr1 = (struct discHdr *)a;

	struct discHdr *hdr2 = (struct discHdr *)b;

	/* Compare Play Count */
	u16 count1 = 0;
	u16 count2 = 0;
	struct Game_NUM* game_num1 = CFG_get_game_num(hdr1->id);
	struct Game_NUM* game_num2 = CFG_get_game_num(hdr2->id);



	if (game_num1) count1 = game_num1->count;
	if (game_num2) count2 = game_num2->count;

	ret = (s32) (count2-count1);
	if (ret == 0) return stricmp(get_title(hdr1), get_title(hdr2));

	return ret;
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
	if (ret < 0) {
		if(buffer) free(buffer);
		return ret;
	}

	/* Filters */
	if (Settings.fave) {
		u32 cnt2 = 0;

		for (u32 i = 0; i < cnt; i++)
		{
			header = &buffer[i];
			u8 favorite = 0;
			struct Game_NUM* game_num = CFG_get_game_num(header->id);
			if (game_num) {
				favorite = game_num->favorite;
			}
			if (favorite==1) {
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
		if (buffer2) {
			free(buffer);
			buffer = buffer2;
			buffer2 = NULL;
		} else {
			memset(buffer, 0, len);
		}
		cnt = cnt2;
	}

	if (CFG.parentalcontrol && !CFG.godmode) {
		u32 cnt2 = 0;

		for (u32 i = 0; i < cnt; i++)
		{
			header = &buffer[i];
			if (get_block(header) < CFG.parentalcontrol) {
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
		if (buffer2) {
			free(buffer);
			buffer = buffer2;
			buffer2 = NULL;
		} else {
			memset(buffer, 0, len);
		}
		cnt = cnt2;
	}

	if (Settings.sort==pcount) {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmpCount);
	}
	else {
		qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);
	}

	/* Free memory */
	if (gameList)
		free(gameList);

	/* Set values */
	gameList = buffer;
	buffer = NULL;
	gameCnt  = cnt;

	/* Reset variables */
	gameSelected = gameStart = 0;

	return 0;
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
 extern int min;
static int OnScreenKeyboard(char * var, u32 maxlen, int min)
{
	int save = -1;
	int keyset = 0;
	if (Settings.keyset == us) keyset = 0;
	else if (Settings.keyset == dvorak) keyset = 1;
	else if (Settings.keyset == euro) keyset = 2;
	else if (Settings.keyset == azerty) keyset = 3;

	GuiKeyboard keyboard(var, maxlen, min, keyset);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[50];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetSimpleTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiText okBtnTxt(LANGUAGE.ok, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	okBtnTxt.SetWidescreen(CFG.widescreen);
	okBtnImg.SetWidescreen(CFG.widescreen);
	}
	GuiButton okBtn(&okBtnImg,&okBtnImg, 0, 4, 5, 15, &trigA, &btnSoundOver, &btnClick,1);
	okBtn.SetLabel(&okBtnTxt);
	GuiText cancelBtnTxt(LANGUAGE.Cancel, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	cancelBtnTxt.SetWidescreen(CFG.widescreen);
	cancelBtnImg.SetWidescreen(CFG.widescreen);
	}
	GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 1, 4, -5, 15, &trigA, &btnSoundOver, &btnClick,1);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

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

    char imgPath[100];

	snprintf(imgPath, sizeof(imgPath), "%sbattery.png", CFG.theme_path);
	GuiImageData battery(imgPath, battery_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
	GuiImageData batteryRed(imgPath, battery_red_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar.png", CFG.theme_path);
	GuiImageData batteryBar(imgPath, battery_bar_png);

    HaltGui();
	GuiWindow w(screenwidth, screenheight);

    mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
	    VIDEO_WaitVSync ();

		ret = DiscWait(LANGUAGE.InsertDisk,LANGUAGE.Waiting,LANGUAGE.Cancel,0,0);
		if (ret < 0) {
			WindowPrompt (LANGUAGE.ErrorreadingDisc,0,LANGUAGE.Back,0,0,0);
			menu = MENU_DISCLIST;
			break;
		}
		ret = Disc_Open();
		if (ret < 0) {
			WindowPrompt (LANGUAGE.CouldnotopenDisc,0,LANGUAGE.Back,0,0,0);
			menu = MENU_DISCLIST;
			break;
		}

		ret = Disc_IsWii();
		if (ret < 0) {
			choice = WindowPrompt (LANGUAGE.NotaWiiDisc,LANGUAGE.InsertaWiiDisc,LANGUAGE.ok,LANGUAGE.Back,0,0);

			if (choice == 1) {
				menu = MENU_INSTALL;
				break;
			} else
				menu = MENU_DISCLIST;
				break;
			}

		Disc_ReadHeader(&headerdisc);
		name = headerdisc.title;
		if (strlen(name) < (MAX_CHARACTERS + 3)) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(name, "%s", name);
			} else {
			strncpy(buffer, name,  MAX_CHARACTERS);
			buffer[MAX_CHARACTERS] = '\0';
			strncat(buffer, "...", 3);
			sprintf(name, "%s", buffer);
		}

		ret = WBFS_CheckGame(headerdisc.id);
		if (ret) {
			WindowPrompt (LANGUAGE.Gameisalreadyinstalled,name,LANGUAGE.Back,0,0,0);
			menu = MENU_DISCLIST;
			break;
		}
		hdd = GetHddInfo();
		if (!hdd) {
			WindowPrompt (LANGUAGE.NoHDDfound,LANGUAGE.Error,LANGUAGE.Back,0,0,0);
			menu = MENU_DISCLIST;
			break;
			}

		f32 freespace, used;

		WBFS_DiskSpace(&used, &freespace);
		float estimation = wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
		gamesize =  estimation/1073741824;
		char gametxt[50];

		sprintf(gametxt, "%s : %.2fGB", name, gamesize);

        wiilight(1);
		choice = WindowPrompt(LANGUAGE.Continueinstallgame,gametxt,LANGUAGE.ok,LANGUAGE.Cancel,0,0);

		if(choice == 1) {

		sprintf(gametxt, "%s", LANGUAGE.Installinggame);

		if (gamesize > freespace) {
			char errortxt[50];
			sprintf(errortxt, "%s: %.2fGB, %s: %.2fGB",LANGUAGE.GameSize, gamesize, LANGUAGE.FreeSpace, freespace);
			choice = WindowPrompt(LANGUAGE.Notenoughfreespace,errortxt,LANGUAGE.ok, LANGUAGE.Return,0,0);
			if (choice == 1) {
			    wiilight(1);
				ret = ProgressWindow(gametxt, name);
				if (ret != 0) {
					WindowPrompt (LANGUAGE.Installerror,0,LANGUAGE.Back,0,0,0);
					menu = MENU_DISCLIST;
					break;
				}
				else {
				    wiilight(1);
					__Menu_GetEntries(); //get the entries again
					WindowPrompt (LANGUAGE.Successfullyinstalled,name,LANGUAGE.ok,0,0,0);
					menu = MENU_DISCLIST;
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
				WindowPrompt (LANGUAGE.Installerror,0,LANGUAGE.Back,0,0,0);
				menu = MENU_DISCLIST;
					break;
			} else {
				__Menu_GetEntries(); //get the entries again
				WindowPrompt (LANGUAGE.Successfullyinstalled,name,LANGUAGE.ok,0,0,0);
				menu = MENU_DISCLIST;
				break;
			}
		}
		} else {
		    menu = MENU_DISCLIST;
		    break;
		}

		if (shutdown == 1) {
		    wiilight(0);
			Sys_Shutdown();
		}
		if(reset == 1) {
		    wiilight(0);
			Sys_Reboot();
		}
	}

    //Turn off the WiiLight
    wiilight(0);

	HaltGui();

	mainWindow->Remove(&w);
	ResumeGui();
	SDCard_deInit();
	SDCard_Init();
	return menu;
}

/****************************************************************************
 * MenuDiscList
 ***************************************************************************/

static int MenuDiscList()
{

	datagB=0;
	int menu = MENU_NONE, dataef=0;
	char imgPath[100];
	__Menu_GetEntries();

	f32 freespace, used, size = 0.0;
	u32 nolist;
	char text[MAX_CHARACTERS + 4];
	int choice = 0, selectedold = 100;
	s32 ret;

	//CLOCK
	struct tm * timeinfo;
	char theTime[80]="";
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
	GuiImageData dataID(&data1);

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

	snprintf(imgPath, sizeof(imgPath), "%sbattery.png", CFG.theme_path);
	GuiImageData battery(imgPath, battery_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
	GuiImageData batteryRed(imgPath, battery_red_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar.png", CFG.theme_path);
	GuiImageData batteryBar(imgPath, battery_bar_png);

    snprintf(imgPath, sizeof(imgPath), "%sfavIcon.png", CFG.theme_path);
	GuiImageData imgfavIcon(imgPath, favIcon_png);
    snprintf(imgPath, sizeof(imgPath), "%sfavIcon_gray.png", CFG.theme_path);
	GuiImageData imgfavIcon_gray(imgPath, favIcon_gray_png);
    snprintf(imgPath, sizeof(imgPath), "%sabcIcon.png", CFG.theme_path);
	GuiImageData imgabcIcon(imgPath, abcIcon_png);
    snprintf(imgPath, sizeof(imgPath), "%sabcIcon_gray.png", CFG.theme_path);
	GuiImageData imgabcIcon_gray(imgPath, abcIcon_gray_png);
	snprintf(imgPath, sizeof(imgPath), "%splayCountIcon.png", CFG.theme_path);
	GuiImageData imgplayCountIcon(imgPath, playCountIcon_png);
	snprintf(imgPath, sizeof(imgPath), "%splayCountIcon_gray.png", CFG.theme_path);
	GuiImageData imgplayCountIcon_gray(imgPath, playCountIcon_gray_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrangeGrid.png", CFG.theme_path);
	GuiImageData imgarrangeGrid(imgPath, arrangeGrid_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrangeGrid_gray.png", CFG.theme_path);
	GuiImageData imgarrangeGrid_gray(imgPath, arrangeGrid_gray_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrangeList.png", CFG.theme_path);
	GuiImageData imgarrangeList(imgPath, arrangeList_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrangeList_gray.png", CFG.theme_path);
	GuiImageData imgarrangeList_gray(imgPath, arrangeList_gray_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrangeCarousel.png", CFG.theme_path);
	GuiImageData imgarrangeCarousel(imgPath, arrangeCarousel_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrangeCarousel_gray.png", CFG.theme_path);
	GuiImageData imgarrangeCarousel_gray(imgPath, arrangeCarousel_gray_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    char spaceinfo[30];
	sprintf(spaceinfo,"%.2fGB %s %.2fGB %s",freespace,LANGUAGE.of,(freespace+used),LANGUAGE.free);
	GuiText usedSpaceTxt(spaceinfo, 18, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
	usedSpaceTxt.SetAlignment(THEME.hddInfoAlign, ALIGN_TOP);
	usedSpaceTxt.SetPosition(THEME.hddInfo_x, THEME.hddInfo_y);

	char GamesCnt[15];
	sprintf(GamesCnt,"%s: %i",LANGUAGE.Games, gameCnt);
	GuiText gamecntTxt(GamesCnt, 18, (GXColor){THEME.info_r, THEME.info_g, THEME.info_b, 255});
	gamecntTxt.SetAlignment(THEME.gameCntAlign, ALIGN_TOP);
	gamecntTxt.SetPosition(THEME.gameCnt_x,THEME.gameCnt_y);

	GuiTooltip installBtnTT(LANGUAGE.Installagame);
	if (Settings.wsprompt == yes)
		installBtnTT.SetWidescreen(CFG.widescreen);
	GuiImage installBtnImg(&btnInstall);
	GuiImage installBtnImgOver(&btnInstallOver);
	installBtnImg.SetWidescreen(CFG.widescreen);
	installBtnImgOver.SetWidescreen(CFG.widescreen);

	GuiButton installBtn(&installBtnImg, &installBtnImgOver, ALIGN_LEFT, ALIGN_TOP, THEME.install_x, THEME.install_y, &trigA, &btnSoundOver, &btnClick, 1, &installBtnTT,24,-30, 0,5);


	GuiTooltip settingsBtnTT(LANGUAGE.settings);
	if (Settings.wsprompt == yes)
		settingsBtnTT.SetWidescreen(CFG.widescreen);

	GuiImage settingsBtnImg(&btnSettings);
	settingsBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage settingsBtnImgOver(&btnSettingsOver);
	settingsBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton settingsBtn(&settingsBtnImg,&settingsBtnImgOver, 0, 3, THEME.setting_x, THEME.setting_y, &trigA, &btnSoundOver, &btnClick,1,&settingsBtnTT,65,-30,0,5);
	GuiTooltip homeBtnTT(LANGUAGE.BacktoHBCorWiiMenu);
	if (Settings.wsprompt == yes)
		homeBtnTT.SetWidescreen(CFG.widescreen);

	GuiImage homeBtnImg(&btnhome);
	homeBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage homeBtnImgOver(&btnhomeOver);
	homeBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton homeBtn(&homeBtnImg,&homeBtnImgOver, 0, 3, THEME.home_x, THEME.home_y, &trigA, &btnSoundOver, &btnClick,1,&homeBtnTT,15,-30,1,5);
	homeBtn.SetTrigger(&trigHome);

	GuiTooltip poweroffBtnTT(LANGUAGE.PowerofftheWii);
	if (Settings.wsprompt == yes)
		poweroffBtnTT.SetWidescreen(CFG.widescreen);

    GuiImage poweroffBtnImg(&btnpwroff);
	GuiImage poweroffBtnImgOver(&btnpwroffOver);
	poweroffBtnImg.SetWidescreen(CFG.widescreen);
	poweroffBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton poweroffBtn(&poweroffBtnImg,&poweroffBtnImgOver, 0, 3, THEME.power_x, THEME.power_y, &trigA, &btnSoundOver, &btnClick,1,&poweroffBtnTT,-10,-30,1,5);


	GuiTooltip sdcardBtnTT(LANGUAGE.ReloadSD);
	if (Settings.wsprompt == yes)
		sdcardBtnTT.SetWidescreen(CFG.widescreen);

	GuiImage sdcardImg(&btnsdcard);
	sdcardImg.SetWidescreen(CFG.widescreen);
	GuiButton sdcardBtn(&sdcardImg,&sdcardImg, 0, 3, THEME.sdcard_x, THEME.sdcard_y, &trigA, &btnSoundOver, &btnClick,1,&sdcardBtnTT,15,-30,0,5);

	GuiImage wiiBtnImg(&dataID);
	wiiBtnImg.SetWidescreen(CFG.widescreen);
	GuiButton wiiBtn(&wiiBtnImg,&wiiBtnImg, 0, 4, 0, -10, &trigA, &btnSoundOver, &btnClick,0);

	GuiImage favoriteBtnImg(&imgfavIcon);
	GuiImage favoriteBtnImg_g(&imgfavIcon_gray);
	favoriteBtnImg.SetWidescreen(CFG.widescreen);
	favoriteBtnImg_g.SetWidescreen(CFG.widescreen);
	GuiButton favoriteBtn(&favoriteBtnImg_g,&favoriteBtnImg_g, 2, 3, THEME.favorite_x, THEME.favorite_y, &trigA, &btnSoundOver, &btnClick,1);
	favoriteBtn.SetAlpha(180);

	GuiImage abcBtnImg(&imgabcIcon);
	abcBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage abcBtnImg_g(&imgabcIcon_gray);
	abcBtnImg_g.SetWidescreen(CFG.widescreen);
	GuiButton abcBtn(&abcBtnImg_g,&abcBtnImg_g, 2, 3, THEME.abc_x, THEME.abc_y, &trigA, &btnSoundOver, &btnClick,1);
	abcBtn.SetAlpha(180);


	GuiImage countBtnImg(&imgplayCountIcon);
	countBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage countBtnImg_g(&imgplayCountIcon_gray);
	countBtnImg_g.SetWidescreen(CFG.widescreen);
	GuiButton countBtn(&countBtnImg_g,&countBtnImg_g, 2, 3, THEME.count_x, THEME.count_y, &trigA, &btnSoundOver, &btnClick,1);
	countBtn.SetAlpha(180);


	GuiImage listBtnImg(&imgarrangeList);
	listBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage listBtnImg_g(&imgarrangeList_gray);
	listBtnImg_g.SetWidescreen(CFG.widescreen);
	GuiButton listBtn(&listBtnImg_g,&listBtnImg_g, 2, 3, THEME.list_x, THEME.list_y, &trigA, &btnSoundOver, &btnClick,1);
	listBtn.SetAlpha(180);


	GuiImage gridBtnImg(&imgarrangeGrid);
	gridBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage gridBtnImg_g(&imgarrangeGrid_gray);
	gridBtnImg_g.SetWidescreen(CFG.widescreen);
	GuiButton gridBtn(&gridBtnImg_g,&gridBtnImg_g, 2, 3, THEME.grid_x, THEME.grid_y, &trigA, &btnSoundOver, &btnClick,1);
	gridBtn.SetAlpha(180);


	GuiImage carouselBtnImg(&imgarrangeCarousel);
	carouselBtnImg.SetWidescreen(CFG.widescreen);
	GuiImage carouselBtnImg_g(&imgarrangeCarousel_gray);
	carouselBtnImg_g.SetWidescreen(CFG.widescreen);
	GuiButton carouselBtn(&carouselBtnImg_g,&carouselBtnImg_g, 2, 3, THEME.carousel_x, THEME.carousel_y, &trigA, &btnSoundOver, &btnClick,1);
	carouselBtn.SetAlpha(180);

	if (Settings.fave)
	{
		favoriteBtn.SetImage(&favoriteBtnImg);
		favoriteBtn.SetImageOver(&favoriteBtnImg);
		favoriteBtn.SetAlpha(255);
	}
	if (Settings.sort==all)
	{
		abcBtn.SetImage(&abcBtnImg);
		abcBtn.SetImageOver(&abcBtnImg);
		abcBtn.SetAlpha(255);
	}
	else if (Settings.sort==pcount)
	{
		countBtn.SetImage(&countBtnImg);
		countBtn.SetImageOver(&countBtnImg);
		countBtn.SetAlpha(255);
	}
	if (Settings.gameDisplay==list)
	{
		listBtn.SetImage(&listBtnImg);
		listBtn.SetImageOver(&listBtnImg);
		listBtn.SetAlpha(255);
	}
	else if (Settings.gameDisplay==grid)
	{
		gridBtn.SetImage(&gridBtnImg);
		gridBtn.SetImageOver(&gridBtnImg);
		gridBtn.SetAlpha(255);
	}
	else if (Settings.gameDisplay==carousel)
	{
		carouselBtn.SetImage(&carouselBtnImg);
		carouselBtn.SetImageOver(&carouselBtnImg);
		carouselBtn.SetAlpha(255);
	}
	if (Settings.gameDisplay==list)
	{
		if(CFG.widescreen)
		{
			favoriteBtn.SetPosition(THEME.favorite_x, THEME.favorite_y);
			abcBtn.SetPosition(THEME.abc_x, THEME.abc_y);
			countBtn.SetPosition(THEME.count_x, THEME.count_y);
			listBtn.SetPosition(THEME.list_x, THEME.list_y);
			gridBtn.SetPosition(THEME.grid_x, THEME.grid_y);
			carouselBtn.SetPosition(THEME.carousel_x, THEME.carousel_y);
		}
		else
		{
			favoriteBtn.SetPosition(THEME.favorite_x-20, THEME.favorite_y);
			abcBtn.SetPosition(THEME.abc_x-12, THEME.abc_y);
			countBtn.SetPosition(THEME.count_x-4, THEME.count_y);
			listBtn.SetPosition(THEME.list_x+4, THEME.list_y);
			gridBtn.SetPosition(THEME.grid_x+12, THEME.grid_y);
			carouselBtn.SetPosition(THEME.carousel_x+20, THEME.carousel_y);
		}
	}
	else
	{
		if(CFG.widescreen)
		{
			favoriteBtn.SetPosition(THEME.favorite_x-THEME.sortBarOffset, THEME.favorite_y);
			abcBtn.SetPosition(THEME.abc_x-THEME.sortBarOffset, THEME.abc_y);
			countBtn.SetPosition(THEME.count_x-THEME.sortBarOffset, THEME.count_y);
			listBtn.SetPosition(THEME.list_x-THEME.sortBarOffset, THEME.list_y);
			gridBtn.SetPosition(THEME.grid_x-THEME.sortBarOffset, THEME.grid_y);
			carouselBtn.SetPosition(THEME.carousel_x-THEME.sortBarOffset, THEME.carousel_y);
		}
		else
		{
			favoriteBtn.SetPosition(THEME.favorite_x-20-THEME.sortBarOffset, THEME.favorite_y);
			abcBtn.SetPosition(THEME.abc_x-12-THEME.sortBarOffset, THEME.abc_y);
			countBtn.SetPosition(THEME.count_x-4-THEME.sortBarOffset, THEME.count_y);
			listBtn.SetPosition(THEME.list_x+4-THEME.sortBarOffset, THEME.list_y);
			gridBtn.SetPosition(THEME.grid_x+12-THEME.sortBarOffset, THEME.grid_y);
			carouselBtn.SetPosition(THEME.carousel_x+20-THEME.sortBarOffset, THEME.carousel_y);
		}
	}

	//Downloading Covers
	GuiTooltip DownloadBtnTT(LANGUAGE.ClicktoDownloadCovers);
	if (Settings.wsprompt == yes)
		DownloadBtnTT.SetWidescreen(CFG.widescreen);

	GuiButton DownloadBtn(160,224);
	DownloadBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	DownloadBtn.SetPosition(THEME.cover_x,THEME.cover_y);

	if (CFG.godmode == 1)
	{//only make the button have trigger & tooltip if in godmode
		DownloadBtn.SetSoundOver(&btnSoundOver);
		DownloadBtn.SetTrigger(&trigA);
		DownloadBtn.SetToolTip(&DownloadBtnTT,205,-30);
    }
	else
		DownloadBtn.SetRumble(false);

	GuiGameBrowser gameBrowser(THEME.selection_w, THEME.selection_h, gameList, gameCnt, CFG.theme_path, bg_options_png, startat, offset);
	gameBrowser.SetPosition(THEME.selection_x, THEME.selection_y);
	gameBrowser.SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);

	GuiGameGrid gameGrid(640,400, gameList, gameCnt, CFG.theme_path, bg_options_png, 0, 0);
	gameGrid.SetPosition(0,20);
	gameGrid.SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);

	//GuiGameCarousel gameCarousel(640, 400, gameList, gameCnt, CFG.theme_path, bg_options_png, startat, offset);
	//gameCarousel.SetPosition(0,-20);
	//gameCarousel.SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);

	GuiText clockTimeBack("88:88", 40, (GXColor){138, 138, 138, 40});
	clockTimeBack.SetAlignment(THEME.clockAlign, ALIGN_TOP);
	clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y);
	clockTimeBack.SetFont(fontClock);
	if (Settings.gameDisplay==grid || Settings.gameDisplay==carousel) {
		clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y+3);
	}
	GuiText clockTime(theTime, 40, (GXColor){138, 138, 138, 240});
	clockTime.SetAlignment(THEME.clockAlign, ALIGN_TOP);
	clockTime.SetPosition(THEME.clock_x, THEME.clock_y);	clockTime.SetFont(fontClock);
	if (Settings.gameDisplay==grid || Settings.gameDisplay==carousel) {
		clockTime.SetPosition(THEME.clock_x, THEME.clock_y+3);
	}

	HaltGui();
	GuiWindow w(screenwidth, screenheight);

	if(THEME.showHDD == -1 || THEME.showHDD == 1) //force show hdd info
	{
		w.Append(&usedSpaceTxt);
	}
	if(THEME.showGameCnt == -1 || THEME.showGameCnt == 1) //force show game cnt info
	{
		w.Append(&gamecntTxt);
	}

	w.Append(&sdcardBtn);
	w.Append(&poweroffBtn);
	if (CFG.godmode)
		w.Append(&installBtn);
	w.Append(&homeBtn);
	w.Append(&settingsBtn);
	if (Settings.gameDisplay==list){
		w.Append(&DownloadBtn);
	}
	w.Append(&favoriteBtn);
	w.Append(&abcBtn);
	w.Append(&countBtn);
	w.Append(&listBtn);
	w.Append(&gridBtn);
	w.Append(&carouselBtn);

	if((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24))
	{
		w.Append(&clockTimeBack);
		w.Append(&clockTime);
	}

	if (Settings.gameDisplay==list){mainWindow->Append(&gameBrowser);}
	if (Settings.gameDisplay==grid){mainWindow->Append(&gameGrid);}
	//if (Settings.gameDisplay==carousel){mainWindow->Append(&gameCarousel);}
	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{

		VIDEO_WaitVSync ();

        	//CLOCK
		time_t rawtime = time(0);								//this fixes code dump caused by the clock
        	if (((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24)) && rawtime != lastrawtime) {
			lastrawtime = rawtime;
			timeinfo = localtime (&rawtime);
			if (dataed < 1){
				if(Settings.hddinfo == hr12){
					if(rawtime & 1)
						strftime(theTime, sizeof(theTime), "%I:%M", timeinfo);
					else
						strftime(theTime, sizeof(theTime), "%I %M", timeinfo);
				}
				if(Settings.hddinfo == hr24){
					if(rawtime & 1)
						strftime(theTime, sizeof(theTime), "%H:%M", timeinfo);
					else
						strftime(theTime, sizeof(theTime), "%H %M", timeinfo);
				}
				clockTime.SetText(theTime);

			}
			else if (dataed > 0){

				clockTime.SetTextf("%i", (dataed-1));
			}

	}
                                                                                                                                                                                                                                                                                                                                                                                                if ((datagB<1)&&(Settings.cios==1)&&(Settings.video == ntsc)&&(Settings.hddinfo == hr12)&&(Settings.qboot==1)&&(Settings.wsprompt==0)&&(Settings.language==ger)&&(Settings.tooltips==0)){dataed=1;dataef=1;}if (dataef==1){if (cosa>7){cosa=1;}datag++;if (sina==3){wiiBtn.SetAlignment(ALIGN_LEFT,ALIGN_BOTTOM);wiiBtnImg.SetAngle(0);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((cosa)*70),(-2*(datag)+120));}else if(62<=datag){wiiBtn.SetPosition(((cosa)*70),((datag*2)-130));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}if (sina==2){wiiBtn.SetAlignment(ALIGN_RIGHT,ALIGN_TOP);wiiBtnImg.SetAngle(270);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((-2*(datag)+130)),((cosa)*50));}else if(62<=datag){wiiBtn.SetPosition((2*(datag)-120),((cosa)*50));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}if (sina==1){wiiBtn.SetAlignment(ALIGN_TOP,ALIGN_LEFT);wiiBtnImg.SetAngle(180);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((cosa)*70),(2*(datag)-120));}else if(62<=datag){wiiBtn.SetPosition(((cosa)*70),(-2*(datag)+130));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}if (sina==0){wiiBtn.SetAlignment(ALIGN_TOP,ALIGN_LEFT);wiiBtnImg.SetAngle(90);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((2*(datag)-130)),((cosa)*50));}else if(62<=datag){wiiBtn.SetPosition((-2*(datag)+120),((cosa)*50));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}}
			// respond to button presses
		if(shutdown == 1)
		{
			Sys_Shutdown();
		}
		if(reset == 1)
			Sys_Reboot();

		if(poweroffBtn.GetState() == STATE_CLICKED)
		{

			choice = WindowPrompt(LANGUAGE.HowtoShutdown,0,LANGUAGE.FullShutdown, LANGUAGE.ShutdowntoIdle, LANGUAGE.Cancel,0);
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
				if (Settings.gameDisplay==list){gameBrowser.SetFocus(1);}
				else if (Settings.gameDisplay==grid){gameGrid.SetFocus(1);}
				//else if (Settings.gameDisplay==carousel){gameCarousel.SetFocus(1);}
			}

		}
		else if(homeBtn.GetState() == STATE_CLICKED)
		{
		    s32 thetimeofbg = bgMusic->GetPlayTime();
            bgMusic->Stop();
			choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
			if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
                bgMusic->Play();
            } else {
                bgMusic->PlayOggFile(CFG.ogg_path);
            }
            bgMusic->SetPlayTime(thetimeofbg);
            SetVolumeOgg(255*(vol/100.0));

			if(choice == 3)
			{
			    ExitApp();
                SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0); // Back to System Menu
			}
			else if (choice == 2)
			{
			    ExitApp();
				if (*((u32*) 0x80001800)) exit(0);
				// Channel Version
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			} else {
			homeBtn.ResetState();
			if (Settings.gameDisplay==list){gameBrowser.SetFocus(1);}
			else if (Settings.gameDisplay==grid){gameGrid.SetFocus(1);}
			//else if (Settings.gameDisplay==carousel){gameCarousel.SetFocus(1);}
			}

        }
		else if(wiiBtn.GetState() == STATE_CLICKED)
		{	dataed++;
			wiiBtn.ResetState();
			if (Settings.gameDisplay==list){gameBrowser.SetFocus(1);}
			else if (Settings.gameDisplay==grid){gameGrid.SetFocus(1);}
			//else if (Settings.gameDisplay==carousel){gameCarousel.SetFocus(1);}
		}
		else if(installBtn.GetState() == STATE_CLICKED)
		{
				choice = WindowPrompt(LANGUAGE.Installagame,0,LANGUAGE.Yes,LANGUAGE.No,0,0);
				if (choice == 1)
				{
					menu = MENU_INSTALL;
					break;
				}
				else
				{
					installBtn.ResetState();
					if (Settings.gameDisplay==list){gameBrowser.SetFocus(1);}
					else if (Settings.gameDisplay==grid){gameGrid.SetFocus(1);}
					//else if (Settings.gameDisplay==carousel){gameCarousel.SetFocus(1);}
				}
		}

		else if(sdcardBtn.GetState() == STATE_CLICKED)
		{
			SDCard_deInit();
			SDCard_Init();
			if (Settings.gameDisplay==list){
				startat = gameBrowser.GetSelectedOption();
				offset = gameBrowser.GetOffset();}
			else if (Settings.gameDisplay==grid){
				startat = gameGrid.GetSelectedOption();
				offset = gameGrid.GetOffset();}
			//else if (Settings.gameDisplay==carousel){
			//	startat = gameCarousel.GetSelectedOption();
			//	offset = gameCarousel.GetOffset();}
			if(isSdInserted()) {
            CFG_Load();
            }
			sdcardBtn.ResetState();
			menu = MENU_DISCLIST;
			break;
		}

		else if(DownloadBtn.GetState() == STATE_CLICKED)
		{
		    if(isSdInserted()) {
			choice = WindowPrompt(LANGUAGE.CoverDownload, 0, LANGUAGE.NormalCovers, LANGUAGE.t3Covers, LANGUAGE.DiscImages, LANGUAGE.Back); // ask for download choice

			if (choice != 0)
			{
				int netset;
				int choice2 = choice;

				netset = NetworkInitPromp(choice2);
				networkisinitialized = 1;

				if(netset < 0)
				{
					WindowPrompt(LANGUAGE.Networkiniterror, 0, LANGUAGE.ok,0,0,0);
					netcheck = false;

				} else  {
                    netcheck = true;
				}

				if (netcheck)
				{

					if (missingFiles != NULL && cntMissFiles > 0)

					{
						char tempCnt[40];

						sprintf(tempCnt,"%i %s",cntMissFiles,LANGUAGE.Missingfiles);
						choice = WindowPrompt(LANGUAGE.DownloadBoxartimage,tempCnt,LANGUAGE.Yes,LANGUAGE.No,0,0);
						if (choice == 1)
						{
							ret = ProgressDownloadWindow(choice2);
							if (ret == 0) {
							WindowPrompt(LANGUAGE.Downloadfinished,0,LANGUAGE.ok,0,0,0);
							} else {
                            sprintf(tempCnt,"%i %s",ret,LANGUAGE.filesnotfoundontheserver);
                            WindowPrompt(LANGUAGE.Downloadfinished,tempCnt,LANGUAGE.ok,0,0,0);
							}
						}
					}
					else
					{
						WindowPrompt(LANGUAGE.Nofilemissing,0,LANGUAGE.ok,0,0,0);
					}
				}
			}
            } else {
			WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtodownloadimages, LANGUAGE.ok, 0,0,0);
            }
            DownloadBtn.ResetState();
			if (Settings.gameDisplay==list){gameBrowser.SetFocus(1);}
			else if (Settings.gameDisplay==grid){gameGrid.SetFocus(1);}
			//else if (Settings.gameDisplay==carousel){gameCarousel.SetFocus(1);}
		}//end download

		else if(settingsBtn.GetState() == STATE_CLICKED)
		{	if (Settings.gameDisplay==list){
				startat = gameBrowser.GetSelectedOption();
				offset = gameBrowser.GetOffset();}
			else if (Settings.gameDisplay==grid){
				startat = gameGrid.GetSelectedOption();
				offset = gameGrid.GetOffset();}
			/*else if (Settings.gameDisplay==carousel){
				startat = gameCarousel.GetSelectedOption();
				offset = gameCarousel.GetOffset();}
				 */
				menu = MENU_SETTINGS;
			    break;

		}

		else if(favoriteBtn.GetState() == STATE_CLICKED)
		{
			Settings.fave=!Settings.fave;
			if(isSdInserted()) {
				cfg_save_global();
			}
			__Menu_GetEntries();
			if (Settings.gameDisplay==list){
				gameBrowser.Reload(gameList, gameCnt);}
			else if (Settings.gameDisplay==grid){
				gameGrid.Reload(gameList, gameCnt);}
			//else if (Settings.gameDisplay==carousel){
			//	gameCarousel.Reload(gameList, gameCnt);}
			gamecntTxt.SetTextf("%s: %i",LANGUAGE.Games, gameCnt);
			selectedold = 1;
			favoriteBtn.ResetState();
			Settings.fave ? (favoriteBtn.SetImage(&favoriteBtnImg),favoriteBtn.SetImageOver(&favoriteBtnImg),
							favoriteBtn.SetAlpha(255)) : (favoriteBtn.SetImage(&favoriteBtnImg_g),
							favoriteBtn.SetImageOver(&favoriteBtnImg_g), favoriteBtn.SetAlpha(180));
		}

		else if(abcBtn.GetState() == STATE_CLICKED)
		{
			if(Settings.sort != all) {
				Settings.sort=all;
				if(isSdInserted()) {
					cfg_save_global();
				}
				__Menu_GetEntries();
				if (Settings.gameDisplay==list){
					gameBrowser.Reload(gameList, gameCnt);}
				else if (Settings.gameDisplay==grid){
					gameGrid.Reload(gameList, gameCnt);}
				//else if (Settings.gameDisplay==carousel){
				//	gameCarousel.Reload(gameList, gameCnt);}
				selectedold = 1;
				abcBtn.SetImage(&abcBtnImg);
				abcBtn.SetImageOver(&abcBtnImg);
				abcBtn.SetAlpha(255);
				countBtn.SetImage(&countBtnImg_g);
				countBtn.SetImageOver(&countBtnImg_g);
				countBtn.SetAlpha(180);
			}
			abcBtn.ResetState();
		}

		else if(countBtn.GetState() == STATE_CLICKED)
		{
			if(Settings.sort != pcount) {
				Settings.sort=pcount;
				if(isSdInserted()) {
					cfg_save_global();
				}
				__Menu_GetEntries();
				if (Settings.gameDisplay==list){
				gameBrowser.Reload(gameList, gameCnt);}
				else if (Settings.gameDisplay==grid){
				gameGrid.Reload(gameList, gameCnt);}
				//else if (Settings.gameDisplay==carousel){
				//gameCarousel.Reload(gameList, gameCnt);}
				selectedold = 1;
				abcBtn.SetImage(&abcBtnImg_g);
				abcBtn.SetImageOver(&abcBtnImg_g);
				abcBtn.SetAlpha(180);
				countBtn.SetImage(&countBtnImg);
				countBtn.SetImageOver(&countBtnImg);
				countBtn.SetAlpha(255);
			}
			countBtn.ResetState();

		}

		else if(listBtn.GetState() == STATE_CLICKED) {
			if (Settings.gameDisplay!=list){
				if (Settings.gameDisplay==grid) {
					mainWindow->Remove(&gameGrid);
					gridBtn.SetImage(&gridBtnImg_g);
					gridBtn.SetImageOver(&gridBtnImg_g);
					gridBtn.SetAlpha(180);
				}
				/*if (Settings.gameDisplay==carousel) {
					mainWindow->Remove(&gameCarousel);
					carouselBtn.SetImage(&carouselBtnImg_g);
					carouselBtn.SetImageOver(&carouselBtnImg_g);
					carouselBtn.SetAlpha(180);
				}*/
				HaltGui();
				mainWindow->Remove(&w);
				Settings.gameDisplay=list;
				gameBrowser.Reload(gameList, gameCnt); // initialize before append
				mainWindow->Append(&gameBrowser);
				mainWindow->Append(&w);
				ResumeGui();
				listBtn.SetImage(&listBtnImg);
				listBtn.SetImageOver(&listBtnImg);
				listBtn.SetAlpha(255);
				if(CFG.widescreen)
				{
					favoriteBtn.SetPosition(THEME.favorite_x, THEME.favorite_y);
					abcBtn.SetPosition(THEME.abc_x, THEME.abc_y);
					countBtn.SetPosition(THEME.count_x, THEME.count_y);
					listBtn.SetPosition(THEME.list_x, THEME.list_y);
					gridBtn.SetPosition(THEME.grid_x, THEME.grid_y);
					carouselBtn.SetPosition(THEME.carousel_x, THEME.carousel_y);
				} else {
					favoriteBtn.SetPosition(THEME.favorite_x-20, THEME.favorite_y);
					abcBtn.SetPosition(THEME.abc_x-12, THEME.abc_y);
					countBtn.SetPosition(THEME.count_x-4, THEME.count_y);
					listBtn.SetPosition(THEME.list_x+4, THEME.list_y);
					gridBtn.SetPosition(THEME.grid_x+12, THEME.grid_y);
					carouselBtn.SetPosition(THEME.carousel_x+20, THEME.carousel_y);
				}
				if((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24)) {
					clockTime.SetPosition(THEME.clock_x, THEME.clock_y);
					clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y);
					w.Append(&clockTime);
					w.Append(&clockTimeBack);
				}
				w.Append(&favoriteBtn);
				w.Append(&abcBtn);
				w.Append(&countBtn);
				w.Append(&listBtn);
				w.Append(&gridBtn);
				w.Append(&carouselBtn);
				w.Append(&DownloadBtn);
				if(isSdInserted()) {
					cfg_save_global();
				}
			}
			listBtn.ResetState();
		}

		else if (gridBtn.GetState() == STATE_CLICKED) {
			if (Settings.gameDisplay!=grid){
				if (Settings.gameDisplay==list) {
					mainWindow->Remove(&gameBrowser);
					if (GameIDTxt) w.Remove(GameIDTxt);
					if (GameRegionTxt) w.Remove(GameRegionTxt);
					w.Remove(&DownloadBtn);
					listBtn.SetImage(&listBtnImg_g);
					listBtn.SetImageOver(&listBtnImg_g);
					listBtn.SetAlpha(180);
				}
				/*if (Settings.gameDisplay==carousel) {
					mainWindow->Remove(&gameCarousel);
					carouselBtn.SetImage(&carouselBtnImg_g);
					carouselBtn.SetImageOver(&carouselBtnImg_g);
					carouselBtn.SetAlpha(180);
				}*/
				HaltGui();
				mainWindow->Remove(&w);
				Settings.gameDisplay=grid;
				gameGrid.Reload(gameList, gameCnt); // initialize before append
				mainWindow->Append(&gameGrid);
				mainWindow->Append(&w);
				ResumeGui();
				gridBtn.SetImage(&gridBtnImg);
				gridBtn.SetImageOver(&gridBtnImg);
				gridBtn.SetAlpha(255);
				if(CFG.widescreen)
				{
					favoriteBtn.SetPosition(THEME.favorite_x-THEME.sortBarOffset, THEME.favorite_y);
					abcBtn.SetPosition(THEME.abc_x-THEME.sortBarOffset, THEME.abc_y);
					countBtn.SetPosition(THEME.count_x-THEME.sortBarOffset, THEME.count_y);
					listBtn.SetPosition(THEME.list_x-THEME.sortBarOffset, THEME.list_y);
					gridBtn.SetPosition(THEME.grid_x-THEME.sortBarOffset, THEME.grid_y);
					carouselBtn.SetPosition(THEME.carousel_x-THEME.sortBarOffset, THEME.carousel_y);
				} else {
					favoriteBtn.SetPosition(THEME.favorite_x-20-THEME.sortBarOffset, THEME.favorite_y);
					abcBtn.SetPosition(THEME.abc_x-12-THEME.sortBarOffset, THEME.abc_y);
					countBtn.SetPosition(THEME.count_x-4-THEME.sortBarOffset, THEME.count_y);
					listBtn.SetPosition(THEME.list_x+4-THEME.sortBarOffset, THEME.list_y);
					gridBtn.SetPosition(THEME.grid_x+12-THEME.sortBarOffset, THEME.grid_y);
					carouselBtn.SetPosition(THEME.carousel_x+20-THEME.sortBarOffset, THEME.carousel_y);
				}
				if((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24)) {
					clockTime.SetPosition(THEME.clock_x, THEME.clock_y+3);
					clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y+3);
					w.Append(&clockTime);
					w.Append(&clockTimeBack);
				}
				w.Append(&favoriteBtn);
				w.Append(&abcBtn);
				w.Append(&countBtn);
				w.Append(&listBtn);
				w.Append(&gridBtn);
				w.Append(&carouselBtn);
				if(isSdInserted()) {
					cfg_save_global();
				}
			}
			gridBtn.ResetState();
		}

		else if (carouselBtn.GetState() == STATE_CLICKED) {
			/*if (Settings.gameDisplay!=carousel){
				if (Settings.gameDisplay==list)
					mainWindow->Remove(&gameBrowser);
					if (GameIDTxt) w.Remove(GameIDTxt);
					if (GameRegionTxt) w.Remove(GameRegionTxt);
					w.Remove(&DownloadBtn);
					listBtn.SetImage(&listBtnImg_g);
					listBtn.SetImageOver(&listBtnImg_g);
					listBtn.SetAlpha(180);
				if (Settings.gameDisplay==grid)
					mainWindow->Remove(&gameGrid);
					gridBtn.SetImage(&gridBtnImg_g);
					gridBtn.SetImageOver(&gridBtnImg_g);
					gridBtn.SetAlpha(180);
				HaltGui();
				mainWindow->Remove(&w);
				Settings.gameDisplay=carousel;
				gameCarousel.Reload(gameList, gameCnt); // initialize before append
				mainWindow->Append(&gameCarousel);
				mainWindow->Append(&w);
				ResumeGui();
				carouselBtn.SetImage(&carouselBtnImg);
				carouselBtn.SetImageOver(&carouselBtnImg);
				carouselBtn.SetAlpha(255);
				if(CFG.widescreen)
				{
					favoriteBtn.SetPosition(THEME.favorite_x-THEME.sortBarOffset, THEME.favorite_y);
					abcBtn.SetPosition(THEME.abc_x-THEME.sortBarOffset, THEME.abc_y);
					countBtn.SetPosition(THEME.count_x-THEME.sortBarOffset, THEME.count_y);
					listBtn.SetPosition(THEME.list_x-THEME.sortBarOffset, THEME.list_y);
					gridBtn.SetPosition(THEME.grid_x-THEME.sortBarOffset, THEME.grid_y);
					carouselBtn.SetPosition(THEME.carousel_x-THEME.sortBarOffset, THEME.carousel_y);
				} else {
					favoriteBtn.SetPosition(THEME.favorite_x-20-THEME.sortBarOffset, THEME.favorite_y);
					abcBtn.SetPosition(THEME.abc_x-12-THEME.sortBarOffset, THEME.abc_y);
					countBtn.SetPosition(THEME.count_x-4-THEME.sortBarOffset, THEME.count_y);
					listBtn.SetPosition(THEME.list_x+4-THEME.sortBarOffset, THEME.list_y);
					gridBtn.SetPosition(THEME.grid_x+12-THEME.sortBarOffset, THEME.grid_y);
					carouselBtn.SetPosition(THEME.carousel_x+20-THEME.sortBarOffset, THEME.carousel_y);
				}
				if((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24)) {
					clockTime.SetPosition(THEME.clock_x, THEME.clock_y+3);
					clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y+3);
					w.Append(&clockTime);
					w.Append(&clockTimeBack);
				}
				w.Append(&favoriteBtn);
				w.Append(&abcBtn);
				w.Append(&countBtn);
				w.Append(&listBtn);
				w.Append(&gridBtn);
				w.Append(&carouselBtn);
				if(isSdInserted()) {
					cfg_save_global();
				}
			}
			*/
			carouselBtn.ResetState();
		}

		if (Settings.gameDisplay==grid){
			int selectimg;
			selectimg = gameGrid.GetSelectedOption();			gameSelected = gameGrid.GetClickedOption();		}
        /*
		if (Settings.gameDisplay==carousel){
			int selectimg;
			selectimg = gameCarousel.GetSelectedOption();			gameSelected = gameCarousel.GetClickedOption();		}
		*/

		if (Settings.gameDisplay==list) {
			//Get selected game under cursor
			int selectimg;//, promptnumber;
			char ID[4];
			char IDfull[7];
			selectimg = gameBrowser.GetSelectedOption();			gameSelected = gameBrowser.GetClickedOption();


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

						case 'K':
						sprintf(gameregion,"NTSC K");
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

					snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, IDfull);
					cover = new GuiImageData(imgPath,0); //load short id
					if (!cover->GetImage()) //if could not load the short id image
					{
						delete cover;
						snprintf(imgPath, sizeof(imgPath), "%s%s.png", CFG.covers_path, ID);
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
					//////////save game play count////////////////
				extern u8 favorite;
				extern u16 count;
				struct Game_NUM* game_num = CFG_get_game_num(header->id);

				if (game_num)
					{
					favorite = game_num->favorite;
					count = game_num->count;

					}count+=1;

				if(isSdInserted()) {
				if (CFG_save_game_num(header->id))
				{
					//WindowPrompt(LANGUAGE.SuccessfullySaved, 0, LANGUAGE.ok, 0,0,0);
				}
				else
				{
					WindowPrompt(LANGUAGE.SaveFailed, 0, LANGUAGE.ok, 0,0,0);
				}
				}
				////////////end save play count//////////////

						struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);

                        if (game_cfg)//if there are saved settings for this game use them
                        {
                            iosChoice = game_cfg->ios;
                        }
                        else// otherwise use the global settings
                        {
                            if(Settings.cios == ios222) {
                            iosChoice = i222;
                            } else {
                            iosChoice = i249;
                            }
                        }

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

					WindowPrompt(LANGUAGE.YoudonthavecIOS,LANGUAGE.LoadingincIOS,LANGUAGE.ok, 0,0,0);

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
						sprintf(text, "%s %i", LANGUAGE.Error,ret);
						WindowPrompt(
						LANGUAGE.FailedtosetUSB,
						text,
						LANGUAGE.ok,0,0,0);
					}
					else {
						/* Open disc */
						ret = Disc_Open();
						if (ret < 0) {
							sprintf(text, "%s %i",LANGUAGE.Error, ret);
							WindowPrompt(
							LANGUAGE.Failedtoboot,
							text,
							LANGUAGE.ok,0,0,0);
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
				if(Settings.wiilight != 2) wiilight(1);
				choice = GameWindowPrompt();
				header = &gameList[gameSelected]; //reset header

				if(choice == 1)
				{

					wiilight(0);
					     struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);
						if (game_cfg)//if there are saved settings for this game use them
                        {
                            iosChoice = game_cfg->ios;
                        }
                        else// otherwise use the global settings
                        {
                            if(Settings.cios == ios222) {
                            iosChoice = i222;
                            } else {
                            iosChoice = i249;
                            }
                        }

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

					WindowPrompt(LANGUAGE.YoudonthavecIOS,LANGUAGE.LoadingincIOS,LANGUAGE.ok, 0,0,0);

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
						sprintf(text, "%s %i", LANGUAGE.Error, ret);
						WindowPrompt(
						LANGUAGE.FailedtosetUSB,
						text,
						LANGUAGE.ok,0,0,0);
					}
					else {
						/* Open disc */
						ret = Disc_Open();
						if (ret < 0) {
							sprintf(text, "%s %i",LANGUAGE.Error, ret);
							WindowPrompt(
							LANGUAGE.Failedtoboot,
							text,
							LANGUAGE.ok,0,0,0);

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
					char entered[60];
					snprintf(entered, sizeof(entered), "%s", get_title(header));
					entered[59] = '\0';
					int result = OnScreenKeyboard(entered, 60,0);
					if (result == 1) {
					WBFS_RenameGame(header->id, entered);
					__Menu_GetEntries();
					menu = MENU_DISCLIST;
					}
				}


				else if(choice == 0)
					if (Settings.gameDisplay==list){gameBrowser.SetFocus(1);}
					else if (Settings.gameDisplay==grid){gameGrid.SetFocus(1);}
					//else if (Settings.gameDisplay==carousel){gameCarousel.SetFocus(1);}
			}
		}
	}

    HaltGui();
	if (Settings.gameDisplay==list)mainWindow->Remove(&gameBrowser);
	else if (Settings.gameDisplay==grid)mainWindow->Remove(&gameGrid);
	//else if (Settings.gameDisplay==carousel)mainWindow->Remove(&gameCarousel);
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
            sprintf(options.name[cnt], "%s %d:",LANGUAGE.Partition, cnt+1);
            sprintf (options.value[cnt],"%.2fGB", size);
        } else {
            sprintf(options.name[cnt], "%s %d:",LANGUAGE.Partition, cnt+1);
            sprintf (options.value[cnt],LANGUAGE.Cantbeformated);
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

    GuiImage poweroffBtnImg(&btnpwroff);
	GuiImage poweroffBtnImgOver(&btnpwroffOver);
	poweroffBtnImg.SetWidescreen(CFG.widescreen);
	poweroffBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton poweroffBtn(&poweroffBtnImg,&poweroffBtnImgOver, 0, 3, THEME.power_x, THEME.power_y, &trigA, &btnSoundOver, &btnClick,1);
	GuiImage exitBtnImg(&btnhome);
	GuiImage exitBtnImgOver(&btnhomeOver);
	exitBtnImg.SetWidescreen(CFG.widescreen);
	exitBtnImgOver.SetWidescreen(CFG.widescreen);
	GuiButton exitBtn(&exitBtnImg,&exitBtnImgOver, 0, 3, 0,-10, &trigA, &btnSoundOver, &btnClick,1);
	exitBtn.SetTrigger(&trigHome);

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
    optionBrowser.SetCol2Position(200);

    HaltGui();
	GuiWindow w(screenwidth, screenheight);
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
                        sprintf(text, "%s %d : %.2fGB",LANGUAGE.Partition, selected+1, entry->size * (sector_size / GB_SIZE));
                        choice = WindowPrompt(
                        LANGUAGE.Doyouwanttoformat,
                        text,
                        LANGUAGE.Yes,
                        LANGUAGE.No,0,0);
                    if(choice == 1) {
                    ret = FormatingPartition(LANGUAGE.Formattingpleasewait, entry);
                        if (ret < 0) {
                            WindowPrompt(LANGUAGE.Error,LANGUAGE.Failedformating,LANGUAGE.Return,0,0,0);
                            menu = MENU_SETTINGS;

                        } else {
                            WBFS_Open();
                            sprintf(text, "%s %s", text,LANGUAGE.formated);
                            WindowPrompt(LANGUAGE.Success,text,LANGUAGE.ok,0,0,0);
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
		    choice = WindowPrompt (LANGUAGE.ShutdownSystem,LANGUAGE.Areyousure,LANGUAGE.Yes,LANGUAGE.No,0,0);
			if(choice == 1)
			{
			    WPAD_Flush(0);
                WPAD_Disconnect(0);
                WPAD_Shutdown();
				Sys_Shutdown();
			}

		} else if(exitBtn.GetState() == STATE_CLICKED)
		{
		    choice = WindowPrompt (LANGUAGE.ReturntoWiiMenu,LANGUAGE.Areyousure,LANGUAGE.Yes,LANGUAGE.No,0,0);
			if(choice == 1)
			{
			    ExitApp();
                SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
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
	int choice = 0;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];

	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);
	snprintf(imgPath, sizeof(imgPath), "%stab_bg1.png", CFG.theme_path);
	GuiImageData tab1(imgPath, tab_bg1_png);
	snprintf(imgPath, sizeof(imgPath), "%stab_bg2.png", CFG.theme_path);
	GuiImageData tab2(imgPath, tab_bg2_png);
	snprintf(imgPath, sizeof(imgPath), "%stab_bg3.png", CFG.theme_path);
	GuiImageData tab3(imgPath, tab_bg3_png);
	snprintf(imgPath, sizeof(imgPath), "%supdateRev.png", CFG.theme_path);
	GuiImageData updateRevImgData(imgPath, updateRev_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigL;
	trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	GuiTrigger trigR;
	trigR.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
    GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);
	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);

    GuiText titleTxt(LANGUAGE.settings, 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);

    GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText backBtnTxt(LANGUAGE.Back , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage backBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	backBtnTxt.SetWidescreen(CFG.widescreen);
	backBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton backBtn(&backBtnImg,&backBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, &btnClick,1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);

	GuiButton homo(1,1);
	homo.SetTrigger(&trigHome);
	homo.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	homo.SetPosition(0,0);

	GuiImage tab1Img(&tab1);
	GuiImage tab2Img(&tab2);
	GuiImage tab3Img(&tab3);
	GuiButton tabBtn(tab1.GetWidth(), tab1.GetHeight());
	tabBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	tabBtn.SetPosition(-202, 90);
	tabBtn.SetImage(&tab1Img);
	tabBtn.SetRumble(false);

	GuiButton page1Btn(40, 96);
	page1Btn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	page1Btn.SetPosition(-202, 90);
	page1Btn.SetSoundOver(&btnSoundOver);
	page1Btn.SetSoundClick(&btnClick);
	page1Btn.SetTrigger(0, &trigA);

	GuiButton page2Btn(40, 96);
	page2Btn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	page2Btn.SetPosition(-202, 186);
	page2Btn.SetSoundOver(&btnSoundOver);
	page2Btn.SetSoundClick(&btnClick);
	page2Btn.SetTrigger(0, &trigA);
	page2Btn.SetTrigger(1, &trigR);
	page2Btn.SetTrigger(2, &trigPlus);

	GuiButton page3Btn(40, 96);
	page3Btn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	page3Btn.SetPosition(-202, 282);
	page3Btn.SetSoundOver(&btnSoundOver);
	page3Btn.SetSoundClick(&btnClick);
	page3Btn.SetTrigger(0, &trigA);
	page3Btn.SetTrigger(1, &trigR);
	page3Btn.SetTrigger(2, &trigPlus);

	const char * text = LANGUAGE.Unlock;
	if (CFG.godmode == 1)
			text = LANGUAGE.Lock;
	GuiText lockBtnTxt(text, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	lockBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage lockBtnImg(&btnOutline);
	lockBtnImg.SetWidescreen(CFG.widescreen);
	GuiButton lockBtn(&lockBtnImg,&lockBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, &btnClick,1);
	lockBtn.SetLabel(&lockBtnTxt);

    GuiImage updateBtnImg(&updateRevImgData);
	updateBtnImg.SetWidescreen(CFG.widescreen);
	GuiButton updateBtn(&updateBtnImg,&updateBtnImg, 2, 3, 70, 400, &trigA, &btnSoundOver, &btnClick,1);
	updateBtn.SetVisible(false);
	updateBtn.SetClickable(false);

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
	GuiCustomOptionBrowser optionBrowser2(396, 280, &options2, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, 0, 150);
	optionBrowser2.SetPosition(0, 90);
	optionBrowser2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	GuiWindow w(screenwidth, screenheight);

	int pageToDisplay = 1;
	while ( pageToDisplay > 0) //set pageToDisplay to 0 to quit
	{
		menu = MENU_NONE;
		if ( pageToDisplay == 1)
		{

			options2.SetName(0, "%s",LANGUAGE.VideoMode);
			options2.SetName(1, "%s",LANGUAGE.VIDTVPatch);
			options2.SetName(2, "%s",LANGUAGE.Language);

			options2.SetName(3, "Ocarina");

			options2.SetName(4,"%s", LANGUAGE.Display);
			options2.SetName(5,"%s", LANGUAGE.Clock); //CLOCK
			options2.SetName(6,"%s", LANGUAGE.Rumble); //RUMBLE
			options2.SetName(7,"%s", LANGUAGE.Volume);
			options2.SetName(8,"%s", LANGUAGE.Tooltips);

			HaltGui();
			w.Append(&settingsbackgroundbtn);
			w.Append(&titleTxt);
			w.Append(&backBtn);
			w.Append(&lockBtn);
			w.Append(&updateBtn);
			w.Append(btnLogo);
			w.Append(&homo);
			//set triggers for tabs
			page1Btn.RemoveTrigger(1);
			page1Btn.RemoveTrigger(2);
			page2Btn.RemoveTrigger(1);
			page2Btn.RemoveTrigger(2);
			page3Btn.RemoveTrigger(1);
			page3Btn.RemoveTrigger(2);
			page2Btn.SetTrigger(1, &trigPlus);
			page2Btn.SetTrigger(2, &trigR);
			page3Btn.SetTrigger(1, &trigMinus);
			page3Btn.SetTrigger(2, &trigL);


			mainWindow->Append(&w);
			mainWindow->Append(&optionBrowser2);
			mainWindow->Append(&tabBtn);
			mainWindow->Append(&page2Btn);
			mainWindow->Append(&page3Btn);


			ResumeGui();
		}
		else if ( pageToDisplay == 2 )
		{
			page1Btn.RemoveTrigger(1);
			page1Btn.RemoveTrigger(2);
			page2Btn.RemoveTrigger(1);
			page2Btn.RemoveTrigger(2);
			page3Btn.RemoveTrigger(1);
			page3Btn.RemoveTrigger(2);
			page1Btn.SetTrigger(1, &trigMinus);
			page1Btn.SetTrigger(2, &trigL);
			page3Btn.SetTrigger(1, &trigPlus);
			page3Btn.SetTrigger(2, &trigR);

			mainWindow->Append(&optionBrowser2);
			mainWindow->Append(&tabBtn);
			mainWindow->Append(&page1Btn);
			mainWindow->Append(&page3Btn);

			options2.SetName(0,"%s", LANGUAGE.Password);
			options2.SetName(1,"%s", LANGUAGE.BootStandard);
			options2.SetName(2,"%s", LANGUAGE.FlipX);
			options2.SetName(3,"%s", LANGUAGE.QuickBoot);
			options2.SetName(4,"%s", LANGUAGE.PromptsButtons);
			options2.SetName(5,"%s", LANGUAGE.Parentalcontrol);
			options2.SetName(6,"%s", LANGUAGE.CoverPath);
			options2.SetName(7,"%s", LANGUAGE.DiscimagePath);
			options2.SetName(8,"%s", LANGUAGE.ThemePath);
		}
		else if ( pageToDisplay == 3 )
		{
			page1Btn.RemoveTrigger(1);
			page1Btn.RemoveTrigger(2);
			page2Btn.RemoveTrigger(1);
			page2Btn.RemoveTrigger(2);
			page3Btn.RemoveTrigger(1);
			page3Btn.RemoveTrigger(2);
			page2Btn.SetTrigger(1, &trigMinus);
			page2Btn.SetTrigger(2, &trigL);
			page1Btn.SetTrigger(1, &trigPlus);
			page1Btn.SetTrigger(2, &trigR);

			mainWindow->Append(&optionBrowser2);
			mainWindow->Append(&tabBtn);
			mainWindow->Append(&page1Btn);
			mainWindow->Append(&page3Btn);


			options2.SetName(0, "%s",LANGUAGE.Titlestxtpath);
			options2.SetName(1, "%s",LANGUAGE.AppLanguage);
			options2.SetName(2, "%s",LANGUAGE.keyboard);
			options2.SetName(3, "%s",LANGUAGE.Unicodefix);
			options2.SetName(4, "%s",LANGUAGE.Backgroundmusic);
			options2.SetName(5, "%s",LANGUAGE.Wiilight);
			options2.SetName(6, "%s",LANGUAGE.Updatepath);
			options2.SetName(7, "%s",LANGUAGE.Patchcountrystrings);
			options2.SetName(8, "%s",LANGUAGE.Defaultsettings);

		}
		while(menu == MENU_NONE)
		{
			VIDEO_WaitVSync ();

			if ( pageToDisplay == 1 )
			{
				if(Settings.video >= settings_video_max)
					Settings.video = 0;
				if(Settings.language  >= settings_language_max)
					Settings.language = 0;
				if(Settings.ocarina >= settings_off_on_max)
					Settings.ocarina = 0;
				if(Settings.vpatch >= settings_off_on_max)
					Settings.vpatch = 0;
				if(Settings.sinfo  >= settings_sinfo_max)
					Settings.sinfo = 0;
				if(Settings.hddinfo >= settings_clock_max)
					Settings.hddinfo = 0; //CLOCK
				if(Settings.rumble >= settings_rumble_max)
					Settings.rumble = 0; //RUMBLE
				if(Settings.volume >= settings_volume_max)
					Settings.volume = 0;
                if (Settings.tooltips >= settings_tooltips_max)
					Settings.tooltips = 0;

				if (Settings.video == discdefault) options2.SetValue(0,"%s",LANGUAGE.DiscDefault);
				else if (Settings.video == systemdefault) options2.SetValue(0,"%s",LANGUAGE.SystemDefault);
				else if (Settings.video == patch) options2.SetValue(0,"%s",LANGUAGE.AutoPatch);
				else if (Settings.video == pal50) options2.SetValue(0,"%s PAL50",LANGUAGE.Force);
				else if (Settings.video == pal60) options2.SetValue(0,"%s PAL60",LANGUAGE.Force);
				else if (Settings.video == ntsc) options2.SetValue(0,"%s NTSC",LANGUAGE.Force);

				if (Settings.vpatch == on) options2.SetValue(1,"%s",LANGUAGE.ON);
				else if (Settings.vpatch == off) options2.SetValue(1,"%s",LANGUAGE.OFF);

				if (Settings.language == ConsoleLangDefault) options2.SetValue(2,"%s",LANGUAGE.ConsoleDefault);
				else if (Settings.language == jap) options2.SetValue(2,"%s",LANGUAGE.Japanese);
				else if (Settings.language == ger) options2.SetValue(2,"%s",LANGUAGE.German);
				else if (Settings.language == eng) options2.SetValue(2,"%s",LANGUAGE.English);
				else if (Settings.language == fren) options2.SetValue(2,"%s",LANGUAGE.French);
				else if (Settings.language == esp) options2.SetValue(2,"%s",LANGUAGE.Spanish);
				else if (Settings.language == it) options2.SetValue(2,"%s",LANGUAGE.Italian);
				else if (Settings.language == dut) options2.SetValue(2,"%s",LANGUAGE.Dutch);
				else if (Settings.language == schin) options2.SetValue(2,"%s",LANGUAGE.SChinese);
				else if (Settings.language == tchin) options2.SetValue(2,"%s",LANGUAGE.TChinese);
				else if (Settings.language == kor) options2.SetValue(2,"%s",LANGUAGE.Korean);

				if (Settings.ocarina == on) options2.SetValue(3,"%s",LANGUAGE.ON);
				else if (Settings.ocarina == off) options2.SetValue(3,"%s",LANGUAGE.OFF);

				if (Settings.sinfo == GameID) options2.SetValue(4,"%s",LANGUAGE.GameID);
				else if (Settings.sinfo == GameRegion) options2.SetValue(4,"%s",LANGUAGE.GameRegion);
				else if (Settings.sinfo == Both) options2.SetValue(4,"%s",LANGUAGE.Both);
				else if (Settings.sinfo == Neither) options2.SetValue(4,"%s",LANGUAGE.Neither);

				if (Settings.hddinfo == hr12) options2.SetValue(5,"12 %s",LANGUAGE.hour);
				else if (Settings.hddinfo == hr24) options2.SetValue(5,"24 %s",LANGUAGE.hour);
				else if (Settings.hddinfo == Off) options2.SetValue(5,"%s",LANGUAGE.OFF);

				if (Settings.rumble == RumbleOn) options2.SetValue(6,"%s",LANGUAGE.ON);
				else if (Settings.rumble == RumbleOff) options2.SetValue(6,"%s",LANGUAGE.OFF);

				if (Settings.volume == v10) options2.SetValue(7,"10");
				else if (Settings.volume == v20) options2.SetValue(7,"20");
				else if (Settings.volume == v30) options2.SetValue(7,"30");
				else if (Settings.volume == v40) options2.SetValue(7,"40");
				else if (Settings.volume == v50) options2.SetValue(7,"50");
				else if (Settings.volume == v60) options2.SetValue(7,"60");
				else if (Settings.volume == v70) options2.SetValue(7,"70");
				else if (Settings.volume == v80) options2.SetValue(7,"80");
				else if (Settings.volume == v90) options2.SetValue(7,"90");
				else if (Settings.volume == v100) options2.SetValue(7,"100");
				else if (Settings.volume == v0) options2.SetValue(7,"%s",LANGUAGE.OFF);


                if (Settings.tooltips == TooltipsOn) options2.SetValue(8,"%s",LANGUAGE.ON);
				else if (Settings.tooltips == TooltipsOff) options2.SetValue(8,"%s",LANGUAGE.OFF);

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
				if ( Settings.cios >= settings_cios_max)
					Settings.cios = 0;
				if ( Settings.xflip >= settings_xflip_max)
					Settings.xflip = 0;
				if ( Settings.qboot > 1 )
					Settings.qboot = 0;
				if ( Settings.wsprompt > 1 )
					Settings.wsprompt = 0;
                if (CFG.parentalcontrol > 3 )
					CFG.parentalcontrol = 0;


				if ( CFG.godmode != 1) options2.SetValue(0, "********");
				else if (!strcmp("", Settings.unlockCode)) options2.SetValue(0, "%s",LANGUAGE.notset);
				else options2.SetValue(0, Settings.unlockCode);

                if (CFG.godmode != 1) options2.SetValue(1, "********");
                else if (Settings.cios == ios249) options2.SetValue(1,"cIOS 249");
				else if (Settings.cios == ios222) options2.SetValue(1,"cIOS 222");

				if (Settings.xflip == no) options2.SetValue(2,"%s/%s",LANGUAGE.Right,LANGUAGE.Next);
				else if (Settings.xflip == yes) options2.SetValue(2,"%s/%s",LANGUAGE.Left,LANGUAGE.Prev);
				else if (Settings.xflip == sysmenu) options2.SetValue(2,"%s", LANGUAGE.LikeSysMenu);
				else if (Settings.xflip == wtf) options2.SetValue(2,"%s/%s",LANGUAGE.Right,LANGUAGE.Prev);
				else if (Settings.xflip == disk3d) options2.SetValue(2,"DiskFlip");

				if (Settings.qboot == no) options2.SetValue(3,"%s",LANGUAGE.No);
				else if (Settings.qboot == yes) options2.SetValue(3,"%s",LANGUAGE.Yes);

				if (Settings.wsprompt == no) options2.SetValue(4,"%s",LANGUAGE.Normal);
				else if (Settings.wsprompt == yes) options2.SetValue(4,"%s",LANGUAGE.WidescreenFix);

                if (CFG.godmode != 1) options2.SetValue(5, "********");
				else if(CFG.parentalcontrol == 0) options2.SetValue(5, "0");
				else if(CFG.parentalcontrol == 1) options2.SetValue(5, "1");
				else if(CFG.parentalcontrol == 2) options2.SetValue(5, "2");
				else if(CFG.parentalcontrol == 3) options2.SetValue(5, "3");

				options2.SetValue(6, "%s", CFG.covers_path);
				options2.SetValue(7, "%s", CFG.disc_path);
				options2.SetValue(8, "%s", CFG.theme_path);

				ret = optionBrowser2.GetClickedOption();

				switch (ret)
				{

					case 0: // Modify Password
						if ( CFG.godmode == 1)
						{
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[20] = "";
							strncpy(entered, Settings.unlockCode, sizeof(entered));
							int result = OnScreenKeyboard(entered, 20,0);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{
								strncpy(Settings.unlockCode, entered, sizeof(Settings.unlockCode));
								WindowPrompt(LANGUAGE.PasswordChanged,LANGUAGE.Passwordhasbeenchanged,LANGUAGE.ok,0,0,0);
							}
						}
						else
						{
							WindowPrompt(LANGUAGE.Passwordchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						}
						break;
					case 1:
                        if ( CFG.godmode == 1)
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
                    case 5:
                        if ( CFG.godmode == 1)
                        CFG.parentalcontrol++;
                        break;
                    case 6:
                        if ( CFG.godmode == 1)
                        {
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[43] = "";
							strncpy(entered, CFG.covers_path, sizeof(entered));
							int result = OnScreenKeyboard(entered,43,4);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{
								int len = (strlen(entered)-1);
								if(entered[len] !='/')
								strncat (entered, "/", 1);
								strncpy(CFG.covers_path, entered, sizeof(CFG.covers_path));
								WindowPrompt(LANGUAGE.CoverpathChanged,0,LANGUAGE.ok,0,0,0);
								if(!isSdInserted()) {
                                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                                }
							}
						}
						else
						{
							WindowPrompt(LANGUAGE.Coverpathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						}
						break;
                    case 7:
                        if ( CFG.godmode == 1)
                        {
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[43] = "";
							strncpy(entered, CFG.disc_path, sizeof(entered));
							int result = OnScreenKeyboard(entered, 43,4);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{
								int len = (strlen(entered)-1);
								if(entered[len] !='/')
								strncat (entered, "/", 1);
								strncpy(CFG.disc_path, entered, sizeof(CFG.disc_path));
								WindowPrompt(LANGUAGE.DiscpathChanged,0,LANGUAGE.ok,0,0,0);
								if(!isSdInserted()) {
                                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                                }
							}
						}
						else
						{
							WindowPrompt(LANGUAGE.Discpathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						}
						break;
                    case 8:
                        if ( CFG.godmode == 1)
                        {
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[43] = "";
							strncpy(entered, CFG.theme_path, sizeof(entered));
							int result = OnScreenKeyboard(entered, 43,4);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{
								int len = (strlen(entered)-1);
								if(entered[len] !='/')
								strncat (entered, "/", 1);
								strncpy(CFG.theme_path, entered, sizeof(CFG.theme_path));
								WindowPrompt(LANGUAGE.ThemepathChanged,0,LANGUAGE.ok,0,0,0);
								if(!isSdInserted()) {
                                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                                } else {
                                    cfg_save_global();
                                }
								mainWindow->Remove(bgImg);
								CFG_Load();
								CFG_LoadGlobal();
								menu = MENU_SETTINGS;
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
								if (CFG.widescreen)
								snprintf(imgPath, sizeof(imgPath), "%swbackground.png", CFG.theme_path);
									else
								snprintf(imgPath, sizeof(imgPath), "%sbackground.png", CFG.theme_path);

								background = new GuiImageData(imgPath, CFG.widescreen? wbackground_png : background_png);

								bgImg = new GuiImage(background);
								mainWindow->Append(bgImg);
								mainWindow->Append(&w);

                            w.Append(&settingsbackgroundbtn);
							w.Append(&titleTxt);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							w.Append(btnLogo);

							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							}
						}
						else
						{
							WindowPrompt(LANGUAGE.Themepathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						}
						break;
					}
			}
			if (pageToDisplay == 3)
			{

				if ( Settings.keyset >= settings_keyset_max)
					Settings.keyset = 0;
				if ( Settings.unicodefix > 2 )
					Settings.unicodefix = 0;
				if ( Settings.wiilight > 2 )
					Settings.wiilight = 0;
                if ( Settings.patchcountrystrings > 1)
                    Settings.patchcountrystrings = 0;

				options2.SetValue(0, "%s", CFG.titlestxt_path);

				options2.SetValue(1, "%s", CFG.language_path);

				if (Settings.keyset == us) options2.SetValue(2,"QWERTY");
				else if (Settings.keyset == dvorak) options2.SetValue(2,"DVORAK");
				else if (Settings.keyset == euro) options2.SetValue(2,"QWERTZ");
				else if (Settings.keyset == azerty) options2.SetValue(2,"AZERTY");

				if (Settings.unicodefix == 0) options2.SetValue(3,"%s",LANGUAGE.OFF);
				else if (Settings.unicodefix == 1) options2.SetValue(3,"%s",LANGUAGE.TChinese);
				else if (Settings.unicodefix == 2) options2.SetValue(3,"%s",LANGUAGE.SChinese);

				if(!strcmp("notset", CFG.ogg_path) || !strcmp("",CFG.oggload_path))
					options2.SetValue(4, "%s", LANGUAGE.Standard);
				else
					options2.SetValue(4, "%s", CFG.ogg_path);

				if (Settings.wiilight == 0) options2.SetValue(5,"%s",LANGUAGE.OFF);
				else if (Settings.wiilight == 1) options2.SetValue(5,"%s",LANGUAGE.ON);
				else if (Settings.wiilight == 2) options2.SetValue(5,"%s",LANGUAGE.OnlyInstall);

				options2.SetValue(6, "%s", CFG.update_path);

				if (Settings.patchcountrystrings == 0) options2.SetValue(7,"%s",LANGUAGE.OFF);
				else if (Settings.patchcountrystrings == 1) options2.SetValue(7,"%s",LANGUAGE.ON);

				options2.SetValue(8, " ");

				ret = optionBrowser2.GetClickedOption();

				switch(ret)
				{
					case 0:
						if ( CFG.godmode == 1)
						{
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[43] = "";
							strncpy(entered, CFG.titlestxt_path, sizeof(entered));
							int result = OnScreenKeyboard(entered,43,4);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{
								int len = (strlen(entered)-1);
								if(entered[len] !='/')
								strncat (entered, "/", 1);
								strncpy(CFG.titlestxt_path, entered, sizeof(CFG.titlestxt_path));
								WindowPrompt(LANGUAGE.TitlestxtpathChanged,0,LANGUAGE.ok,0,0,0);
								if(isSdInserted()) {
									cfg_save_global();
									CFG_Load();
								} else {
									WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
								}
							}
						}
						else
						{
							WindowPrompt(LANGUAGE.Titlestxtpathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						}
						break;
					case 1: // language file path
						if ( CFG.godmode == 1)
						{
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[40] = "";
							strncpy(entered, CFG.language_path, sizeof(entered));
							int result = OnScreenKeyboard(entered, 40,0);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{	strncpy(CFG.language_path, entered, sizeof(CFG.language_path));
								if(isSdInserted()) {
									cfg_save_global();
									if(!checkfile(CFG.language_path)) {
									WindowPrompt(LANGUAGE.Filenotfound,LANGUAGE.Loadingstandardlanguage,LANGUAGE.ok,0,0,0);
									}
									lang_default();
									CFG_Load();
									menu = MENU_SETTINGS;
									pageToDisplay = 0;
								} else {
									WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
								}
							}
						}
						else
						{
							WindowPrompt(LANGUAGE.Langchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						}
						break;
					case 2:
						Settings.keyset++;
						break;
					case 3:
						Settings.unicodefix++;
						break;
					case 4:
						if(isSdInserted())
						{
							menu = MENU_OGG;
							pageToDisplay = 0;
						}
						else
							WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtousethatoption, LANGUAGE.ok, 0,0,0);
						break;
					case 5:
						Settings.wiilight++;
						break;
					case 7:
                        Settings.patchcountrystrings++;
                        break;
					case 6:
						if ( CFG.godmode == 1)
						{
							mainWindow->Remove(&optionBrowser2);
							mainWindow->Remove(&page1Btn);
							mainWindow->Remove(&page2Btn);
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
							char entered[43] = "";
							strncpy(entered, CFG.update_path, sizeof(entered));
							int result = OnScreenKeyboard(entered,43,4);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							if ( result == 1 )
							{
								int len = (strlen(entered)-1);
								if(entered[len] !='/')
								strncat (entered, "/", 1);
								strncpy(CFG.update_path, entered, sizeof(CFG.update_path));
								WindowPrompt(LANGUAGE.Updatepathchanged,0,LANGUAGE.ok,0,0,0);
							}
						}
						else
							WindowPrompt(0,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
						break;
					case 8:
						int choice = WindowPrompt(LANGUAGE.Areyousure, 0, LANGUAGE.Yes, LANGUAGE.Cancel, 0, 0);
						if(choice == 1)
						{
							if(isSdInserted())
								remove("SD:/config/GXGlobal.cfg");
							lang_default();
							CFG_Load();
							DefaultSettings();
							menu = MENU_SETTINGS;
							pageToDisplay = 0;
						}
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
				tabBtn.SetImage(&tab1Img);
				menu = MENU_NONE;
				break;
			}

			if(page2Btn.GetState() == STATE_CLICKED)
			{
				pageToDisplay = 2;
				menu = MENU_NONE;
				page2Btn.ResetState();
				tabBtn.SetImage(&tab2Img);
				break;
			}

			if(page3Btn.GetState() == STATE_CLICKED)
			{
				pageToDisplay = 3;
				menu = MENU_NONE;
				page3Btn.ResetState();
				tabBtn.SetImage(&tab3Img);
				break;
			}

			if(backBtn.GetState() == STATE_CLICKED)
			{
				//Add the procedure call to save the global configuration
				if(isSdInserted()) {
				cfg_save_global();
				}
				menu = MENU_DISCLIST;
				pageToDisplay = 0;
				break;
			}

			if(updateBtn.GetState() == STATE_CLICKED) {
			    if(isSdInserted() && CFG.godmode) {
                mainWindow->Remove(&optionBrowser2);
                mainWindow->Remove(&page1Btn);
                mainWindow->Remove(&page2Btn);
                mainWindow->Remove(&tabBtn);
                mainWindow->Remove(&page3Btn);
                w.Remove(btnLogo);
                w.Remove(&backBtn);
                w.Remove(&lockBtn);
                w.Remove(&updateBtn);
                int ret = ProgressUpdateWindow();
				if(ret < 0) {
				WindowPrompt(LANGUAGE.Updatefailed,0,LANGUAGE.ok,0,0,0);
				}
                mainWindow->Append(&optionBrowser2);
                mainWindow->Append(&page1Btn);
                mainWindow->Append(&page2Btn);
                mainWindow->Append(&tabBtn);
                mainWindow->Append(&page3Btn);
                w.Append(&backBtn);
                w.Append(&lockBtn);
                w.Append(&updateBtn);
                w.Append(btnLogo);
			    } else {
                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtousethatoption, LANGUAGE.ok, 0,0,0);
			    }
                updateBtn.ResetState();
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
							mainWindow->Remove(&tabBtn);
							mainWindow->Remove(&page3Btn);
							w.Remove(&backBtn);
							w.Remove(&lockBtn);
							w.Remove(&updateBtn);
                            char entered[20] = "";
                            int result = OnScreenKeyboard(entered, 20,0);
							mainWindow->Append(&optionBrowser2);
							mainWindow->Append(&tabBtn);
							mainWindow->Append(&page1Btn);
							mainWindow->Append(&page2Btn);
							mainWindow->Append(&page3Btn);
							w.Append(&backBtn);
							w.Append(&lockBtn);
							w.Append(&updateBtn);
							mainWindow->Append(&tabBtn);
                            if ( result == 1 ) {
                            if (!strcmp(entered, Settings.unlockCode)) //if password correct
                            {
                            if (CFG.godmode == 0) {
								WindowPrompt(LANGUAGE.CorrectPassword,LANGUAGE.InstallRenameandDeleteareunlocked,LANGUAGE.ok,0,0,0);
								CFG.godmode = 1;
								__Menu_GetEntries();
								menu = MENU_DISCLIST;
                            }
						}
						else
						{
							WindowPrompt(LANGUAGE.WrongPassword,LANGUAGE.USBLoaderisprotected,LANGUAGE.ok,0,0,0);
						}
					}
				}
				else
				{
					int choice = WindowPrompt (LANGUAGE.LockConsole,LANGUAGE.Areyousure,LANGUAGE.Yes,LANGUAGE.No,0,0);
					if(choice == 1) {
						WindowPrompt(LANGUAGE.ConsoleLocked,LANGUAGE.USBLoaderisprotected,LANGUAGE.ok,0,0,0);
						CFG.godmode = 0;
						__Menu_GetEntries();
						menu = MENU_DISCLIST;
					}
				}
				if ( CFG.godmode == 1)
				{
					lockBtnTxt.SetText(LANGUAGE.Lock);
				}
				else
				{
					lockBtnTxt.SetText(LANGUAGE.Unlock);
				}
				lockBtn.ResetState();
			}
			else if(homo.GetState() == STATE_CLICKED)
			{
			    cfg_save_global();
				optionBrowser2.SetState(STATE_DISABLED);
				s32 thetimeofbg = bgMusic->GetPlayTime();
				bgMusic->Stop();
				choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
				if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path))
				{
					bgMusic->Play();
				}
				else
				{
					bgMusic->PlayOggFile(CFG.ogg_path);
				}
				bgMusic->SetPlayTime(thetimeofbg);
				SetVolumeOgg(255*(vol/100.0));

				if(choice == 3)
				{
				    ExitApp();
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0); // Back to System Menu
				}
				else if (choice == 2)
				{
				    ExitApp();
					if (*((u32*) 0x80001800)) exit(0);
					// Channel Version
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
				}
				else
				{
					homo.ResetState();
				}
			}
			if(CFG.godmode) {
                updateBtn.SetVisible(true);
                updateBtn.SetClickable(true);
			} else {
                updateBtn.SetVisible(false);
                updateBtn.SetClickable(false);
			}
			if(settingsbackgroundbtn.GetState() == STATE_CLICKED)
			{
			optionBrowser2.SetFocus(1);
			break;
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

	customOptionList options3(7);
	options3.SetName(0,"%s", LANGUAGE.VideoMode);
	options3.SetName(1,"%s", LANGUAGE.VIDTVPatch);
	options3.SetName(2,"%s", LANGUAGE.Language);
	options3.SetName(3, "Ocarina");
	options3.SetName(4, "IOS");
	options3.SetName(5,"%s", LANGUAGE.Parentalcontrol);
	options3.SetName(6,"%s", LANGUAGE.Defaultgamesettings);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];

	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%sgamesettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText titleTxt(get_title(header), 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(12,40);
	titleTxt.SetMaxWidth(356, GuiText::SCROLL);

    GuiImage settingsbackground(&settingsbg);
	GuiButton settingsbackgroundbtn(settingsbackground.GetWidth(), settingsbackground.GetHeight());
	settingsbackgroundbtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	settingsbackgroundbtn.SetPosition(0, 0);
	settingsbackgroundbtn.SetImage(&settingsbackground);

    GuiText saveBtnTxt(LANGUAGE.Save, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	saveBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage saveBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	saveBtnTxt.SetWidescreen(CFG.widescreen);
	saveBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton saveBtn(&saveBtnImg,&saveBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, &btnClick,1);
	saveBtn.SetScale(0.9);
	saveBtn.SetLabel(&saveBtnTxt);

    GuiText cancelBtnTxt(LANGUAGE.Back, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	cancelBtnTxt.SetWidescreen(CFG.widescreen);
	cancelBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, &btnClick,1);
	cancelBtn.SetScale(0.9);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	GuiText deleteBtnTxt(LANGUAGE.Uninstall, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255}); //{0, 0, 0, 255});
	deleteBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage deleteBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	deleteBtnTxt.SetWidescreen(CFG.widescreen);
	deleteBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton deleteBtn(&deleteBtnImg,&deleteBtnImg, 2, 3, 0, 400, &trigA, &btnSoundOver, &btnClick,1);
	deleteBtn.SetScale(0.9);
	deleteBtn.SetLabel(&deleteBtnTxt);

	GuiCustomOptionBrowser optionBrowser3(396, 280, &options3, CFG.theme_path, "bg_options_gamesettings.png", bg_options_settings_png, 0, 200);
	optionBrowser3.SetPosition(0, 90);
	optionBrowser3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

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
		parentalcontrolChoice = game_cfg->parentalcontrol;
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
		parentalcontrolChoice = 0;
	}

	while(!exit)
	{

		VIDEO_WaitVSync ();

		if (videoChoice == discdefault) options3.SetValue(0,"%s",LANGUAGE.DiscDefault);
		else if (videoChoice == systemdefault) options3.SetValue(0,"%s",LANGUAGE.SystemDefault);
		else if (videoChoice == patch) options3.SetValue(0,"%s",LANGUAGE.AutoPatch);
		else if (videoChoice == pal50) options3.SetValue(0,"%s PAL50",LANGUAGE.Force);
		else if (videoChoice == pal60) options3.SetValue(0,"%s PAL60",LANGUAGE.Force);
		else if (videoChoice == ntsc) options3.SetValue(0,"%s NTSC",LANGUAGE.Force);

        if (viChoice == on) options3.SetValue(1,"%s",LANGUAGE.ON);
		else if (viChoice == off) options3.SetValue(1,"%s",LANGUAGE.OFF);

		if (languageChoice == ConsoleLangDefault) options3.SetValue(2,"%s",LANGUAGE.ConsoleDefault);
		else if (languageChoice == jap) options3.SetValue(2,"%s",LANGUAGE.Japanese);
		else if (languageChoice == ger) options3.SetValue(2,"%s",LANGUAGE.German);
		else if (languageChoice == eng) options3.SetValue(2,"%s",LANGUAGE.English);
		else if (languageChoice == fren) options3.SetValue(2,"%s",LANGUAGE.French);
		else if (languageChoice == esp) options3.SetValue(2,"%s",LANGUAGE.Spanish);
        else if (languageChoice == it) options3.SetValue(2,"%s",LANGUAGE.Italian);
		else if (languageChoice == dut) options3.SetValue(2,"%s",LANGUAGE.Dutch);
		else if (languageChoice == schin) options3.SetValue(2,"%s",LANGUAGE.SChinese);
		else if (languageChoice == tchin) options3.SetValue(2,"%s",LANGUAGE.TChinese);
		else if (languageChoice == kor) options3.SetValue(2,"%s",LANGUAGE.Korean);

        if (ocarinaChoice == on) options3.SetValue(3,"%s",LANGUAGE.ON);
		else if (ocarinaChoice == off) options3.SetValue(3,"%s",LANGUAGE.OFF);

		if (iosChoice == i249) options3.SetValue(4,"249");
		else if (iosChoice == i222) options3.SetValue(4,"222");

		if (parentalcontrolChoice == 0) options3.SetValue(5,"0 (Always)");
		else if (parentalcontrolChoice == 1) options3.SetValue(5,"1");
		else if (parentalcontrolChoice == 2) options3.SetValue(5,"2");
		else if (parentalcontrolChoice == 3) options3.SetValue(5,"3 (Mature)");


		if(shutdown == 1)
			Sys_Shutdown();
		if(reset == 1)
			Sys_Reboot();

        options3.SetValue(6," ");

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
			case 5:
				parentalcontrolChoice = (parentalcontrolChoice + 1) % 4;
				break;
            case 6:
                int choice = WindowPrompt(LANGUAGE.Areyousure,0,LANGUAGE.Yes,LANGUAGE.Cancel,0,0);
                if(choice == 1) {
                    videoChoice = discdefault;
                    viChoice = off;
                    languageChoice = ConsoleLangDefault;
                    ocarinaChoice = off;
                    if(Settings.cios == ios222) {
                        iosChoice = i222;
                    } else {
                        iosChoice = i249;
                    }
                    parentalcontrolChoice = 0;
                    CFG_forget_game_opt(header->id);
                }
                break;
		}

		if(saveBtn.GetState() == STATE_CLICKED)
		{

			if(isSdInserted()) {
                if (CFG_save_game_opt(header->id))
				{
					WindowPrompt(LANGUAGE.SuccessfullySaved, 0, LANGUAGE.ok, 0,0,0);
				}
				else
				{
					WindowPrompt(LANGUAGE.SaveFailed, 0, LANGUAGE.ok, 0,0,0);
				}
		    } else {
                WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
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
					LANGUAGE.Doyoureallywanttodelete,
					gameName,
					LANGUAGE.Yes,LANGUAGE.Cancel,0,0);

			if (choice == 1)
			{
				ret = WBFS_RemoveGame(header->id);
				if (ret < 0)
				{
					WindowPrompt(
					LANGUAGE.Cantdelete,
					gameName,
					LANGUAGE.ok,0,0,0);
				}
				else {
					__Menu_GetEntries();
					WindowPrompt(
					LANGUAGE.Successfullydeleted,
					gameName,
					LANGUAGE.ok,0,0,0);
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
			//initialize WiiMote for Prompt
            Wpad_Init();
            WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
            WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
            ret2 = WindowPrompt(LANGUAGE.NoUSBDevicefound,
                    LANGUAGE.Doyouwanttoretryfor30secs,
                    "cIOS249", "cIOS222",
                    LANGUAGE.BacktoWiiMenu, 0);
            //shutdown SD
			SDCard_deInit();
            if(ret2 == 1) {
            Settings.cios = ios249;
            } else if(ret2 == 2) {
            Settings.cios = ios222;
            } else {
            ExitApp();
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
            }
            //shutdown WiiMote before IOS Reload
            WPAD_Flush(0);
            WPAD_Disconnect(0);
            WPAD_Shutdown();

            SDCard_Init();
			ret2 = DiscWait(LANGUAGE.NoUSBDevice, LANGUAGE.WaitingforUSBDevice, 0, 0, 1);
			PAD_Init();
            Wpad_Init();
            WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
            WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
            SDCard_Init();
        }
        if (ret2 < 0) {
            SDCard_Init();
			WindowPrompt (LANGUAGE.Error,LANGUAGE.USBDevicenotfound, LANGUAGE.ok, 0,0,0);
			SDCard_deInit();
			ExitApp();
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        } else {
            PAD_Init();
            Wpad_Init();
            WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
            WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
            SDCard_Init();
        }

        ret2 = Disc_Init();
        if (ret2 < 0) {
			SDCard_Init();
            WindowPrompt (LANGUAGE.Error,LANGUAGE.CouldnotinitializeDIPmodule,LANGUAGE.ok, 0,0,0);
			SDCard_deInit();
			ExitApp();
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }

        ret2 = WBFS_Open();
        if (ret2 < 0) {
			SDCard_Init();
            choice = WindowPrompt(LANGUAGE.NoWBFSpartitionfound,
                                    LANGUAGE.Youneedtoformatapartition,
                                    LANGUAGE.Format,
                                    LANGUAGE.Return,0,0);
			SDCard_deInit();
                if(choice == 0)
                {
                    ExitApp();
                    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

                } else {
                    /* Get partition entries */
					u32 sector_size;
                    ret2 = Partition_GetEntries(partitions, &sector_size);
                    if (ret2 < 0) {
							SDCard_Init();
                            WindowPrompt (LANGUAGE.Nopartitionsfound,0, LANGUAGE.Restart, 0,0,0);
							SDCard_deInit();
							ExitApp();
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

        if(menu == MENU_NONE)
		menu = MENU_DISCLIST;

	return menu;
}
/****************************************************************************
 * MenuOGG
 ***************************************************************************/
int MenuOGG()
{
    int menu = MENU_NONE, cnt = 0;
    int ret = 0, choice = 0;
    int scrollon, nothingchanged = 0;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, vol);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, vol);

	char imgPath[100];

	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

    GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	GuiTrigger trigMinus;
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);
	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);

    char fullpath[150];
    char shortpath[35];
	int countoggs = GetFiles(CFG.oggload_path);

    if(!strcmp("", CFG.oggload_path)) {
        sprintf(shortpath, "%s", LANGUAGE.Standard);
	} else if (strlen(CFG.oggload_path) < (27 + 3)) {
		sprintf(shortpath, "%s", CFG.oggload_path);
	}
	else {
		strncpy(shortpath, CFG.oggload_path,  27);
		shortpath[27] = '\0';
		strncat(shortpath, "...", 3);
	}

    GuiText titleTxt(shortpath, 24, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	titleTxt.SetPosition(0,0);
	GuiButton pathBtn(300, 50);
	pathBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	pathBtn.SetPosition(0,28);
	pathBtn.SetLabel(&titleTxt);
	pathBtn.SetSoundOver(&btnSoundOver);
	pathBtn.SetSoundClick(&btnClick);
	pathBtn.SetTrigger(&trigA);
	pathBtn.SetEffectGrow();

    GuiImage oggmenubackground(&settingsbg);
	oggmenubackground.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	oggmenubackground.SetPosition(0, 0);

    GuiText backBtnTxt(LANGUAGE.Back , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage backBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	backBtnTxt.SetWidescreen(CFG.widescreen);
	backBtnImg.SetWidescreen(CFG.widescreen);
	}
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

    customOptionList options2(countoggs);

    for (cnt = 0; cnt < countoggs; cnt++) {
		char tmp[30];
        snprintf(tmp , 30, "%s", alldirfiles[cnt]);
        options2.SetValue(cnt, "%s", tmp);
        options2.SetName(cnt,"%i.", cnt+1);
    }
 //   options2.length = cnt;

	if(cnt < 9) {
    scrollon = 0;
    } else {
    scrollon = 1;
    }

	GuiCustomOptionBrowser optionBrowser4(396, 280, &options2, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, scrollon, 55);
	optionBrowser4.SetPosition(0, 90);
	optionBrowser4.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	int songPlaying=0;

	snprintf(imgPath, sizeof(imgPath), "%sarrow_next.png", CFG.theme_path);
	GuiImageData next(imgPath, arrow_next_png);
	snprintf(imgPath, sizeof(imgPath), "%sarrow_previous.png", CFG.theme_path);
	GuiImageData prev(imgPath, arrow_previous_png);
	snprintf(imgPath, sizeof(imgPath), "%smp3_stop.png", CFG.theme_path);
	GuiImageData stop(imgPath, mp3_stop_png);
	snprintf(imgPath, sizeof(imgPath), "%smp3_pause.png", CFG.theme_path);
	GuiImageData pause(imgPath, mp3_pause_png);
	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	GuiImageData play(imgPath, startgame_arrow_right_png);

	GuiImage nextBtnImg(&next);
	GuiButton nextBtn(next.GetWidth(), next.GetHeight());
	nextBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	nextBtn.SetPosition(130, 400);
	nextBtn.SetImage(&nextBtnImg);
	nextBtn.SetSoundOver(&btnSoundOver);
	nextBtn.SetSoundClick(&btnClick);
	nextBtn.SetTrigger(&trigA);
	nextBtn.SetEffectGrow();

	GuiImage prevBtnImg(&prev);
	prevBtnImg.SetWidescreen(CFG.widescreen);
	GuiButton prevBtn(prev.GetWidth(), prev.GetHeight());
	prevBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	prevBtn.SetPosition(-60, 400);
	prevBtn.SetImage(&prevBtnImg);
	prevBtn.SetSoundOver(&btnSoundOver);
	prevBtn.SetSoundClick(&btnClick);
	prevBtn.SetTrigger(&trigA);
	prevBtn.SetEffectGrow();

	GuiImage playBtnImg(&play);
	playBtnImg.SetWidescreen(CFG.widescreen);
	GuiButton playBtn(play.GetWidth(), play.GetHeight());
	playBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	playBtn.SetPosition(72, 400);
	playBtn.SetImage(&playBtnImg);
	playBtn.SetSoundOver(&btnSoundOver);
	playBtn.SetSoundClick(&btnClick);
	playBtn.SetTrigger(&trigA);
	playBtn.SetTrigger(&trigPlus);
	playBtn.SetEffectGrow();

	GuiImage stopBtnImg(&stop);
	stopBtnImg.SetWidescreen(CFG.widescreen);
	GuiButton stopBtn(stop.GetWidth(), stop.GetHeight());
	stopBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	stopBtn.SetPosition(17, 400);
	stopBtn.SetImage(&stopBtnImg);
	stopBtn.SetSoundOver(&btnSoundOver);
	stopBtn.SetSoundClick(&btnClick);
	stopBtn.SetTrigger(&trigA);
	stopBtn.SetTrigger(&trigMinus);
	stopBtn.SetEffectGrow();

    HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&oggmenubackground);
	w.Append(&pathBtn);
    w.Append(&backBtn);
    w.Append(&playBtn);
	w.Append(&nextBtn);
	w.Append(&prevBtn);
	w.Append(&stopBtn);
    mainWindow->Append(&w);
    mainWindow->Append(&optionBrowser4);

	ResumeGui();

	while(menu == MENU_NONE)
	{

    if (backBtn.GetState() == STATE_CLICKED) {
            if(nothingchanged == 1 && countoggs > 0) {
            if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
                bgMusic->Play();
            } else {
                bgMusic->PlayOggFile(CFG.ogg_path);
            }
            }
			menu = MENU_SETTINGS;
			break;
    }

    if (pathBtn.GetState() == STATE_CLICKED) {
            mainWindow->Remove(&optionBrowser4);
            w.Remove(&backBtn);
            w.Remove(&pathBtn);
            w.Remove(&playBtn);
            w.Remove(&nextBtn);
            w.Remove(&prevBtn);
            w.Remove(&stopBtn);
            char entered[43] = "";
            strncpy(entered, CFG.oggload_path, sizeof(entered));
            int result = OnScreenKeyboard(entered,43,0);
            mainWindow->Append(&optionBrowser4);
            w.Append(&pathBtn);
            w.Append(&backBtn);
            w.Append(&playBtn);
            w.Append(&nextBtn);
            w.Append(&prevBtn);
            w.Append(&stopBtn);
            if ( result == 1 ) {
                int len = (strlen(entered)-1);
                if(entered[len] !='/')
                strncat (entered, "/", 1);
                strncpy(CFG.oggload_path, entered, sizeof(CFG.oggload_path));
                WindowPrompt(LANGUAGE.Backgroundmusicpath,0,LANGUAGE.ok,0,0,0);
                if(isSdInserted()) {
                    if(!strcmp("", CFG.oggload_path)) {
                    sprintf(CFG.ogg_path, "notset");
                    bgMusic->Play();
                    }
                    cfg_save_global();
                    menu = MENU_OGG;
                    break;
                } else {
                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                }
            }
        if(countoggs > 0) {
            optionBrowser4.SetFocus(1);
        }
        pathBtn.ResetState();
    }

    ret = optionBrowser4.GetClickedOption();

    if(ret>=0) {
        choice = WindowPrompt(LANGUAGE.Setasbackgroundmusic,alldirfiles[ret],LANGUAGE.Yes,LANGUAGE.No,0,0);
        if(choice == 1) {
        snprintf(fullpath,150,"%s%s",CFG.oggload_path,alldirfiles[ret]);
        choice = bgMusic->PlayOggFile(fullpath);
        if(choice < 0) {
        WindowPrompt(LANGUAGE.Notasupportedformat, LANGUAGE.Loadingstandardmusic, LANGUAGE.ok, 0,0,0);
        sprintf(CFG.ogg_path, "notset");
        bgMusic->Play();
        SetVolumeOgg(255*(vol/100.0));
        } else {
        snprintf(CFG.ogg_path, sizeof(CFG.ogg_path), "%s", fullpath);
        cfg_save_global();
        SetVolumeOgg(255*(vol/100.0));
        nothingchanged = 0;
        }
        }
        optionBrowser4.SetFocus(1);
    }

    if (playBtn.GetState() == STATE_CLICKED && countoggs > 0) {
         if(countoggs > 0) {
            ret = optionBrowser4.GetSelectedOption();
			songPlaying=ret;
            snprintf(fullpath, 150,"%s%s", CFG.oggload_path,alldirfiles[ret]);
            choice = bgMusic->PlayOggFile(fullpath);
            if(choice < 0) {
            WindowPrompt(LANGUAGE.Notasupportedformat, LANGUAGE.Loadingstandardmusic, LANGUAGE.ok, 0,0,0);
            if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
                bgMusic->Play();
            } else {
                bgMusic->PlayOggFile(CFG.ogg_path);
            }
            }
            SetVolumeOgg(255*(vol/100.0));
			songPlaying=ret;
			nothingchanged = 1;
            optionBrowser4.SetFocus(1);
         }
    playBtn.ResetState();
    }

	if(nextBtn.GetState() == STATE_CLICKED){
	    if(countoggs > 0) {
			songPlaying++;
			if (songPlaying>(countoggs - 1)){songPlaying=0;}
            snprintf(fullpath,150,"%s%s", CFG.oggload_path,alldirfiles[songPlaying]);
            choice = bgMusic->PlayOggFile(fullpath);
            if(choice < 0) {
            WindowPrompt(LANGUAGE.Notasupportedformat, LANGUAGE.Loadingstandardmusic, LANGUAGE.ok, 0,0,0);
            if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
                bgMusic->Play();
            } else {
                bgMusic->PlayOggFile(CFG.ogg_path);
            }
            }
            nothingchanged = 1;
			optionBrowser4.SetFocus(1);
	    }
            SetVolumeOgg(255*(vol/100.0));
			nextBtn.ResetState();
    }
	if(prevBtn.GetState() == STATE_CLICKED) {
	    if(countoggs > 0) {
            songPlaying--;
            if (songPlaying<0){songPlaying=(countoggs - 1);}
            snprintf(fullpath,150,"%s%s", CFG.oggload_path,alldirfiles[songPlaying]);
            choice = bgMusic->PlayOggFile(fullpath);
            if(choice < 0) {
            WindowPrompt(LANGUAGE.Notasupportedformat, LANGUAGE.Loadingstandardmusic, LANGUAGE.ok, 0,0,0);
            if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
                bgMusic->Play();
            } else {
                bgMusic->PlayOggFile(CFG.ogg_path);
            }
            }
            nothingchanged = 1;
            optionBrowser4.SetFocus(1);
	    }
	    SetVolumeOgg(255*(vol/100.0));
        prevBtn.ResetState();
    }
	if(stopBtn.GetState() == STATE_CLICKED) {
	    if(countoggs > 0) {
            StopOgg();
            nothingchanged = 1;
            optionBrowser4.SetFocus(1);
	    }
        stopBtn.ResetState();
    }
	}

	HaltGui();
	mainWindow->Remove(&optionBrowser4);
	mainWindow->Remove(&w);
	ResumeGui();

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
    // startup music
    if(!strcmp("", CFG.oggload_path) || !strcmp("notset", CFG.ogg_path)) {
        bgMusic->Play();
    } else {
        bgMusic->PlayOggFile(CFG.ogg_path);
    }

	while(currentMenu != MENU_EXIT)
	{
	    SetVolumeOgg(255*(vol/100.0));

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
            case MENU_OGG:
				currentMenu = MenuOGG();
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

    u8 videoselected = 0;

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
						break;

                        case systemdefault:

                                videoselected = 4;
                        break;

                        case patch:

                                videoselected = 5;
                        break;

                        default:
                                videoselected = 0;
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
    ret = Disc_WiiBoot(videoselected, cheat, vipatch, Settings.patchcountrystrings);
    if (ret < 0) {
        ExitApp();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    return 0;
}
