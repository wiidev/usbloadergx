#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include "usbloader/wbfs.h"
#include "usbloader/wdvd.h"
#include "usbloader/partition.h"
#include "usbloader/usbstorage.h"
#include "usbloader/getentries.h"
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_diskcover.h"
#include "network/networkops.h"
#include "network/http.h"
#include "prompts/PromptWindows.h"
#include "prompts/gameinfo.h"
#include "mload/mload.h"
#include "fatmounter.h"
#include "listfiles.h"
#include "menu.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "wpad.h"
#include "wad/wad.h"
#include "unzip/unzip.h"
#include "zlib.h"
#include "svnrev.h"
#include "audio.h"
#include "xml/xml.h"
#include "wad/title.h"
#include "language/UpdateLanguage.h"
#include "gecko.h"
#include "lstub.h"
#include "bannersound.h"




/*** Variables that are also used extern ***/
int cntMissFiles = 0;

/*** Variables used only in this file ***/
static char missingFiles[500][12];

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u32 gameCnt;
extern s32 gameSelected, gameStart;
extern float gamesize;
extern struct discHdr * gameList;
extern u8 shutdown;
extern u8 reset;
extern u8 mountMethod;
extern struct discHdr *dvdheader;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
int OnScreenKeyboard(char * var, u32 maxlen, int min) {

    int save = -1;
    int keyset = 0;
    if (Settings.keyset == us) keyset = 0;
    else if (Settings.keyset == dvorak) keyset = 1;
    else if (Settings.keyset == euro) keyset = 2;
    else if (Settings.keyset == azerty) keyset = 3;
    else if (Settings.keyset == qwerty) keyset = 4;

	gprintf("\nOnScreenKeyboard(%s, %i, %i) \n\tkeyset = %i",var,maxlen,min,keyset);

    GuiKeyboard keyboard(var, maxlen, min, keyset);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size,Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetSimpleTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    GuiText okBtnTxt(tr("OK"), 22, THEME.prompttext);
    GuiImage okBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        okBtnTxt.SetWidescreen(CFG.widescreen);
        okBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton okBtn(&okBtnImg,&okBtnImg, 0, 4, 5, 15, &trigA, &btnSoundOver, btnClick2,1);
    okBtn.SetLabel(&okBtnTxt);
    GuiText cancelBtnTxt(tr("Cancel"), 22, THEME.prompttext);
    GuiImage cancelBtnImg(&btnOutline);
    if (Settings.wsprompt == yes) {
        cancelBtnTxt.SetWidescreen(CFG.widescreen);
        cancelBtnImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 1, 4, -5, 15, &trigA, &btnSoundOver, btnClick2,1);
    cancelBtn.SetLabel(&cancelBtnTxt);
    cancelBtn.SetTrigger(&trigB);

    keyboard.Append(&okBtn);
    keyboard.Append(&cancelBtn);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&keyboard);
    mainWindow->ChangeFocus(&keyboard);
    ResumeGui();

    while (save == -1) {
        VIDEO_WaitVSync();

        if (okBtn.GetState() == STATE_CLICKED)
            save = 1;
        else if (cancelBtn.GetState() == STATE_CLICKED)
            save = 0;
    }

    if (save) {
        snprintf(var, maxlen, "%s", keyboard.kbtextstr);
    }

    HaltGui();
    mainWindow->Remove(&keyboard);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
	gprintf("\t%s",(save?"saved":"discarded"));
    return save;
}

/****************************************************************************
 * WindowCredits
 * Display credits
 ***************************************************************************/
void WindowCredits() {
	gprintf("\nWindowCredits()");

    int angle = 0;
    GuiSound * creditsMusic = NULL;

    bgMusic->Pause();

    creditsMusic = new GuiSound(credits_music_ogg, credits_music_ogg_size, 55);
    creditsMusic->SetVolume(60);
    creditsMusic->SetLoop(1);
    creditsMusic->Play();

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

    int numEntries = 19;
    GuiText * txt[numEntries];

    txt[i] = new GuiText(tr("Credits"), 26, (GXColor) {255, 255, 255, 255});
    txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    txt[i]->SetPosition(0,12);
    i++;


#ifdef NOTFULLCHANNEL
    char SvnRev[30];
    snprintf(SvnRev,sizeof(SvnRev), "Rev%s   IOS%u (Rev %u)", GetRev(), IOS_GetVersion(), IOS_GetRevision());
#else
    char svnTmp[4];//did this to hide the M after the rev# that is made by altering it
    //to be ready to be in a full channel
    snprintf(svnTmp,sizeof(svnTmp), "%s", GetRev());
    char SvnRev[30];
    snprintf(SvnRev,sizeof(SvnRev), "Rev%sc   IOS%u (Rev %u)", svnTmp, IOS_GetVersion(), IOS_GetRevision());
#endif




    txt[i] = new GuiText(SvnRev, 16, (GXColor) { 255, 255, 255, 255});
    txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    txt[i]->SetPosition(0,y);
    i++;
    y+=34;

    txt[i] = new GuiText("USB Loader GX", 24, (GXColor) {255, 255, 255, 255});
    txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    txt[i]->SetPosition(0,y);
    i++;
    y+=26;

    txt[i] = new GuiText(" http://code.google.com/p/usbloader-gui/", 20, (GXColor) {255, 255, 255, 255});
    txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    txt[i]->SetPosition(50,y);
    i++; //y+=28;

    txt[i] = new GuiText(tr("Official Site:"), 20, (GXColor) {255, 255, 255, 255});
    txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    txt[i]->SetPosition(-180,y);
    i++;
    y+=28;

    GuiText::SetPresets(22, (GXColor) {255, 255, 255,  255}, 0, GuiText::WRAP,FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP, ALIGN_LEFT, ALIGN_TOP);

    txt[i] = new GuiText(tr("Coding:"));
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(10,y);
    i++;

    txt[i] = new GuiText("dimok / nIxx / giantpune / ardi");
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=22;

    txt[i] = new GuiText("hungyip84 / DrayX7 / lustar / r-win");
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=34;

    char text[100];

    txt[i] = new GuiText(tr("Design:"));
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(10,y);
    i++;

    txt[i] = new GuiText("cyrex / NeoRame");
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=34;

    txt[i] = new GuiText(tr("Big thanks to:"));
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(10,y);
    i++;

    sprintf(text, "lustar %s", tr("for WiiTDB and hosting covers / disc images"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=22;

    sprintf(text, "CorneliousJD %s", tr("for hosting the update files"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=22;

    sprintf(text, "Kinyo %s", tr("and translaters for language files updates"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=22;

    sprintf(text, "Deak Phreak %s", tr("for hosting the themes"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(160,y);
    i++;
    y+=22;

    txt[i] = new GuiText(tr("Special thanks to:"));
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(10,y);
    i++;
    y+=22;

    sprintf(text, "Waninkoko, Kwiirk & Hermes %s", tr("for the USB Loader source"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(60,y);
    i++;
    y+=22;

    sprintf(text, "Tantric %s", tr("for his awesome tool LibWiiGui"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(60,y);
    i++;
    y+=22;

    sprintf(text, "Fishears/Nuke %s", tr("for Ocarina"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(60,y);
    i++;
    y+=22;

    sprintf(text, "WiiPower %s", tr("for diverse patches"));
    txt[i] = new GuiText(text);
    txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    txt[i]->SetPosition(60,y);
    i++;
    y+=22;

    for (i=0; i < numEntries; i++)
        creditsWindowBox.Append(txt[i]);


    creditsWindow.Append(&creditsWindowBox);
    creditsWindow.Append(&starImg);

    creditsWindow.SetEffect(EFFECT_FADE, 30);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&creditsWindow);
    ResumeGui();

    while (!exit) {
        angle++;
        if (angle > 360)
            angle = 0;
        usleep(12000);
        starImg.SetAngle(angle);
        if (ButtonsPressed() != 0)
            exit = true;

    }

    creditsMusic->Stop();

    delete creditsMusic;

    creditsWindow.SetEffect(EFFECT_FADE, -30);
    while (creditsWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&creditsWindow);
    mainWindow->SetState(STATE_DEFAULT);
    for (i=0; i < numEntries; i++) {
        delete txt[i];
        txt[i] = NULL;
    }
    ResumeGui();

    bgMusic->Resume();
}

/****************************************************************************
 * WindowScreensaver
 * Display screensaver
 ***************************************************************************/
int WindowScreensaver() {
	gprintf("\nWindowScreenSaver()");
	int i = 0;
	bool exit = false;
	char imgPath[100];//uncomment for themable screensaver

	/* initialize random seed: */
	srand ( time(NULL) );

	snprintf(imgPath, sizeof(imgPath), "%sscreensaver.png", CFG.theme_path);//uncomment for themable screensaver
	GuiImageData GXlogo(imgPath, gxlogo_png);//uncomment for themable screensaver
	//GuiImageData GXlogo(gxlogo_png);//comment for themable screensaver
	GuiImage GXlogoImg(&GXlogo);
	GXlogoImg.SetPosition(172,152);
	GXlogoImg.SetAlignment(ALIGN_LEFT,ALIGN_TOP);

	GuiImage BackgroundImg(640,480,(GXColor) {0, 0, 0, 255});
	BackgroundImg.SetPosition(0,0);
	BackgroundImg.SetAlignment(ALIGN_LEFT,ALIGN_TOP);

	GuiWindow screensaverWindow(screenwidth,screenheight);
	screensaverWindow.Append(&BackgroundImg);
	screensaverWindow.Append(&GXlogoImg);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&screensaverWindow);
	ResumeGui();

    while (!exit) {
        i++;
        if (IsWpadConnected()) {
            exit = true;
        }
        /* Set position only every 400000th loop */
        if ((i % 8000000) == 0) {
            /* Set random position */
            GXlogoImg.SetPosition((rand() % 345), (rand() % 305));
        }

    }

    HaltGui();
    mainWindow->Remove(&screensaverWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return 1;
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 4 Buttons.
 *
 * Give him 1 Title, 1 Subtitle and 4 Buttons
 * If title/subtitle or one of the buttons is not needed give him a 0 on that
 * place.
 ***************************************************************************/
int WindowPrompt(const char *title, const char *msg, const char *btn1Label,
             const char *btn2Label, const char *btn3Label,
             const char *btn4Label, int wait) {
    int choice = -1;
    int count = wait;
	gprintf("\nWindowPrompt(%s, %s, %s, %s, %s, %s, %i)",title,msg,btn1Label,btn2Label, btn3Label,btn4Label,wait);


    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size,  Settings.sfxvolume);
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
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    GuiText titleTxt(title, 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,55);
    GuiText msgTxt(msg, 22, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);
    msgTxt.SetMaxWidth(430);

    GuiText btn1Txt(btn1Label, 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }

    GuiButton btn1(&btn1Img, &btn1Img, 0,3,0,0,&trigA,&btnSoundOver,btnClick2,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

    GuiText btn2Txt(btn2Label, 22, THEME.prompttext);
    GuiImage btn2Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn2Txt.SetWidescreen(CFG.widescreen);
        btn2Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn2(&btn2Img, &btn2Img, 0,3,0,0,&trigA,&btnSoundOver,btnClick2,1);
    btn2.SetLabel(&btn2Txt);
    if (!btn3Label && !btn4Label)
        btn2.SetTrigger(&trigB);

    GuiText btn3Txt(btn3Label, 22, THEME.prompttext);
    GuiImage btn3Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn3Txt.SetWidescreen(CFG.widescreen);
        btn3Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn3(&btn3Img, &btn3Img, 0,3,0,0,&trigA,&btnSoundOver,btnClick2,1);
    btn3.SetLabel(&btn3Txt);
    if (!btn4Label)
        btn3.SetTrigger(&trigB);

    GuiText btn4Txt(btn4Label, 22, THEME.prompttext);
    GuiImage btn4Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn4Txt.SetWidescreen(CFG.widescreen);
        btn4Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn4(&btn4Img, &btn4Img, 0,3,0,0,&trigA,&btnSoundOver,btnClick2,1);
    btn4.SetLabel(&btn4Txt);
    if (btn4Label)
        btn4.SetTrigger(&trigB);

    if ((Settings.wsprompt == yes) && (CFG.widescreen)) {/////////////adjust buttons for widescreen
        msgTxt.SetMaxWidth(330);

        if (btn2Label && !btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(70, -80);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -80);
            btn3.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn3.SetPosition(-70, -55);
            btn4.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn4.SetPosition(70, -55);
        } else if (btn2Label && btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(70, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -120);
            btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn3.SetPosition(0, -55);
            btn4.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn4.SetPosition(70, -55);
        } else if (btn2Label && btn3Label && btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(70, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(70, -55);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-70, -55);
        }   else if (!btn2Label && btn3Label && btn4Label) {
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

        if (btn2Label && !btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(40, -45);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-40, -45);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(50, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
        } else if (btn2Label && btn3Label && !btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(50, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-50, -120);
            btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn3.SetPosition(0, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
        } else if (btn2Label && btn3Label && btn4Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn1.SetPosition(50, -120);
            btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn2.SetPosition(-50, -120);
            btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn3.SetPosition(50, -65);
            btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
            btn4.SetPosition(-50, -65);
        } else if (!btn2Label && btn3Label && btn4Label) {
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

    if (btn1Label)
        promptWindow.Append(&btn1);
    if (btn2Label)
        promptWindow.Append(&btn2);
    if (btn3Label)
        promptWindow.Append(&btn3);
    if (btn4Label)
        promptWindow.Append(&btn4);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while (choice == -1) {
        VIDEO_WaitVSync();
        if (shutdown == 1) {
            wiilight(0);
            Sys_Shutdown();
        }
        if (reset == 1)
            Sys_Reboot();
        if (btn1.GetState() == STATE_CLICKED) {
            choice = 1;
        } else if (btn2.GetState() == STATE_CLICKED) {
            if (!btn3Label)
                choice = 0;
            else
                choice = 2;
        } else if (btn3.GetState() == STATE_CLICKED) {
            if (!btn4Label)
                choice = 0;
            else
                choice = 3;
        } else if (btn4.GetState() == STATE_CLICKED) {
            choice = 0;
        }
        if (count>0)count--;
        if (count==0) choice = 1;
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
	gprintf(" = %i",choice);

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
                 const char *btn4Label) {
	gprintf("\nWindowExitPrompt()");

    GuiSound * homein = NULL;
    homein = new GuiSound(menuin_ogg, menuin_ogg_size, Settings.sfxvolume);
    homein->SetVolume(Settings.sfxvolume);
    homein->SetLoop(0);
    homein->Play();

    GuiSound * homeout = NULL;
    homeout = new GuiSound(menuout_ogg, menuout_ogg_size, Settings.sfxvolume);
    homeout->SetVolume(Settings.sfxvolume);
    homeout->SetLoop(0);

    int choice = -1;
    char imgPath[100];

	u64 oldstub = getStubDest();
	loadStub();
	if (oldstub != 0x00010001554c4e52ll && oldstub != 0x00010001554e454fll)
		Set_Stub(oldstub);

	GuiWindow promptWindow(640,480);
    promptWindow.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    promptWindow.SetPosition(0, 0);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    GuiImageData top(exit_top_png);
    GuiImageData topOver(exit_top_over_png);
    GuiImageData bottom(exit_bottom_png);
    GuiImageData bottomOver(exit_bottom_over_png);
    GuiImageData button(exit_button_png);
    GuiImageData wiimote(wiimote_png);
    GuiImageData close(closebutton_png);

    snprintf(imgPath, sizeof(imgPath), "%sbattery_white.png", CFG.theme_path);
    GuiImageData battery(imgPath, battery_white_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar_white.png", CFG.theme_path);
    GuiImageData batteryBar(imgPath, battery_bar_white_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
    GuiImageData batteryRed(imgPath, battery_red_png);
    snprintf(imgPath, sizeof(imgPath), "%sbattery_bar_red.png", CFG.theme_path);
    GuiImageData batteryBarRed(imgPath, battery_bar_red_png);


#ifdef HW_RVL
    int i = 0, ret = 0, level;
    char txt[3];
    GuiText * batteryTxt[4];
    GuiImage * batteryImg[4];
    GuiImage * batteryBarImg[4];
    GuiButton * batteryBtn[4];

    for (i=0; i < 4; i++) {

        if (i == 0)
            sprintf(txt, "P%d", i+1);
        else
            sprintf(txt, "P%d", i+1);

        batteryTxt[i] = new GuiText(txt, 22, (GXColor) {255,255,255, 255});
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



#endif
	GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    GuiText titleTxt(tr("HOME Menu"), 36, (GXColor) {255, 255, 255, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(-180,40);
    titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    GuiText closeTxt(tr("Close"), 28, (GXColor) {0, 0, 0, 255});
    closeTxt.SetPosition(10,3);
    GuiImage closeImg(&close);
    if (Settings.wsprompt == yes) {
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
    GuiButton btn1(&btn1Img,&btn1OverImg, 0, 3, 0, 0, &trigA, &btnSoundOver, btnClick2,0);
    btn1.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    GuiText btn2Txt(btn1Label, 28, (GXColor) {0, 0, 0, 255});
    GuiImage btn2Img(&button);
    if (Settings.wsprompt == yes) {
        btn2Txt.SetWidescreen(CFG.widescreen);
        btn2Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn2(&btn2Img,&btn2Img, 2, 5, -150, 0, &trigA, &btnSoundOver, btnClick2,1);
    btn2.SetLabel(&btn2Txt);
    btn2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
    btn2.SetRumble(false);
    btn2.SetPosition(-150, 0);


    GuiText btn3Txt(btn2Label, 28, (GXColor) {0, 0, 0, 255});
    GuiImage btn3Img(&button);
    if (Settings.wsprompt == yes) {
        btn3Txt.SetWidescreen(CFG.widescreen);
        btn3Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn3(&btn3Img,&btn3Img, 2, 5, 150, 0, &trigA, &btnSoundOver, btnClick2,1);
    btn3.SetLabel(&btn3Txt);
    btn3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
    btn3.SetRumble(false);
    btn3.SetPosition(150, 0);

    GuiImage btn4Img(&bottom);
    GuiImage btn4OverImg(&bottomOver);
    GuiButton btn4(&btn4Img,&btn4OverImg, 0, 4, 0, 0, &trigA, &btnSoundOver, btnClick2,0);
    btn4.SetTrigger(&trigB);
    btn4.SetTrigger(&trigHome);
    btn4.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

    GuiImage wiimoteImg(&wiimote);
    if (Settings.wsprompt == yes) {
        wiimoteImg.SetWidescreen(CFG.widescreen);
    }
    wiimoteImg.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    wiimoteImg.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
    wiimoteImg.SetPosition(50,210);

    promptWindow.Append(&btn2);
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

    while (choice == -1) {
        VIDEO_WaitVSync();

#ifdef HW_RVL
        for (i=0; i < 4; i++) {
            if (WPAD_Probe(i, NULL) == WPAD_ERR_NONE) { // controller connected
                level = (userInput[i].wpad.battery_level / 100.0) * 4;
                if (level > 4) level = 4;

                if (level <= 1) {
                    batteryBarImg[i]->SetImage(&batteryBarRed);
                    batteryImg[i]->SetImage(&batteryRed);
				} else {
                    batteryBarImg[i]->SetImage(&batteryBar);
				}

				batteryImg[i]->SetTile(level);

                batteryBtn[i]->SetAlpha(255);
            } else { // controller not connected
                batteryImg[i]->SetTile(0);
                batteryImg[i]->SetImage(&battery);
                batteryBtn[i]->SetAlpha(70);
            }
        }
#endif


        if (shutdown == 1) {
            wiilight(0);
            Sys_Shutdown();
        }
        if (reset == 1)
            Sys_Reboot();
        if (btn1.GetState() == STATE_CLICKED) {
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
        } else if (btn4.GetState() == STATE_SELECTED) {
            wiimoteImg.SetPosition(50,165);
        } else if (btn2.GetState() == STATE_CLICKED) {
            ret = WindowPrompt(tr("Are you sure?"), 0, tr("Yes"), tr("No"));
            if (ret == 1) {

                choice = 2;
            }
            HaltGui();
            mainWindow->SetState(STATE_DISABLED);
            promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
            ResumeGui();
            btn2.ResetState();
        } else if (btn3.GetState() == STATE_CLICKED) {
            ret = WindowPrompt(tr("Are you sure?"), 0, tr("Yes"), tr("No"));
            if (ret == 1) {
                choice = 3;
            }
            HaltGui();
            mainWindow->SetState(STATE_DISABLED);
            promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
            ResumeGui();
            btn3.ResetState();
        } else if (btn4.GetState() == STATE_CLICKED) {
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
        } else if (btn4.GetState() != STATE_SELECTED) {
            wiimoteImg.SetPosition(50,210);
        }
    }
    homeout->Play();
    while (btn1.GetEffect() > 0) usleep(50);
    while (promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    homein->Stop();
    delete homein;
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    while (homeout->IsPlaying() > 0) usleep(50);
    homeout->Stop();
    delete homeout;
    ResumeGui();
    return choice;
}

void SetupFavoriteButton(GuiButton *btnFavorite, int xPos, GuiImage *img, GuiSound *sndOver, GuiSound *sndClick, GuiTrigger *trig)
{
    btnFavorite->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    btnFavorite->SetPosition(xPos, -60);
    btnFavorite->SetImage(img);
    btnFavorite->SetSoundOver(sndOver);
    btnFavorite->SetSoundClick(sndClick);
    btnFavorite->SetTrigger(trig);
    btnFavorite->SetEffectGrow();
}

u8 SetFavorite(GuiButton *fav1, GuiButton *fav2, GuiButton *fav3, GuiButton *fav4, GuiButton *fav5, u8* gameId, u8 favorite)
{
	struct Game_NUM * game_num = CFG_get_game_num(gameId);
	if (game_num) {
		favoritevar = game_num->favorite;
		playcount = game_num->count;
	} else {
		favoritevar = 0;
		playcount = 0;
	}
	favoritevar = (favorite == favoritevar) ? 0 : favorite; // Press the current rank to reset the rank
	CFG_save_game_num(gameId);
	return favoritevar;
}

void SetFavoriteImages(GuiImage *b1, GuiImage *b2, GuiImage *b3, GuiImage *b4, GuiImage *b5, GuiImageData *on, GuiImageData *off)
{
	b1->SetImage(favoritevar >= 1 ? on : off);
	b2->SetImage(favoritevar >= 2 ? on : off);
	b3->SetImage(favoritevar >= 3 ? on : off);
	b4->SetImage(favoritevar >= 4 ? on : off);
	b5->SetImage(favoritevar >= 5 ? on : off);
}

/****************************************************************************
 * GameWindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int GameWindowPrompt() {
    int choice = -1, angle = 0;
    f32 size = 0.0;
    char ID[5];
    char IDFull[7];

    GuiSound * gameSound = NULL;

    gprintf("\nGameWindowPrompt()");
    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

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

    GuiTooltip nameBtnTT(tr("Rename Game on WBFS"));
    if (Settings.wsprompt == yes)
        nameBtnTT.SetWidescreen(CFG.widescreen);
    GuiText nameTxt("", 22, THEME.prompttext);
    if (Settings.wsprompt == yes)
        nameTxt.SetWidescreen(CFG.widescreen);
    nameTxt.SetMaxWidth(350, GuiText::SCROLL);
    GuiButton nameBtn(120,50);
    nameBtn.SetLabel(&nameTxt);
//	nameBtn.SetLabelOver(&nameTxt);
    nameBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    nameBtn.SetPosition(0,-122);
    nameBtn.SetSoundOver(&btnSoundOver);
    nameBtn.SetSoundClick(btnClick2);
    if (!mountMethod) nameBtn.SetToolTip(&nameBtnTT,24,-30, ALIGN_LEFT);

    if (Settings.godmode == 1 && !mountMethod) {
        nameBtn.SetTrigger(&trigA);
        nameBtn.SetEffectGrow();
    }

    GuiText sizeTxt(NULL, 22, THEME.prompttext); //TODO: get the size here
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

    GuiText playcntTxt(NULL, 18, THEME.info);
    playcntTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    playcntTxt.SetPosition(-115,45);

    GuiButton btn1(160, 160);
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    btn1.SetPosition(0, -20);
    btn1.SetImage(&diskImg);

    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetSoundClick(btnClick2);
    btn1.SetTrigger(&trigA);
    btn1.SetState(STATE_SELECTED);

    GuiText btn2Txt(tr("Back"), 22, THEME.prompttext);
    GuiImage btn2Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn2Txt.SetWidescreen(CFG.widescreen);
        btn2Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn2(&btn2Img,&btn2Img, 1, 5, 0, 0, &trigA, &btnSoundOver, btnClick2,1);
    if (Settings.godmode == 1 && mountMethod!=2 && mountMethod!=3) {
        btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
        btn2.SetPosition(-50, -40);
    } else {
        btn2.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn2.SetPosition(0, -40);
    }

    btn2.SetLabel(&btn2Txt);
    btn2.SetTrigger(&trigB);

    GuiText btn3Txt(tr("Settings"), 22, THEME.prompttext);
    GuiImage btn3Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn3Txt.SetWidescreen(CFG.widescreen);
        btn3Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn3(&btn3Img,&btn3Img, 0, 4, 50, -40, &trigA, &btnSoundOver, btnClick2,1);
    btn3.SetLabel(&btn3Txt);

    GuiImage btnFavoriteImg1;
    btnFavoriteImg1.SetWidescreen(CFG.widescreen);
    GuiImage btnFavoriteImg2;
    btnFavoriteImg2.SetWidescreen(CFG.widescreen);
    GuiImage btnFavoriteImg3;
    btnFavoriteImg3.SetWidescreen(CFG.widescreen);
    GuiImage btnFavoriteImg4;
    btnFavoriteImg4.SetWidescreen(CFG.widescreen);
    GuiImage btnFavoriteImg5;
    btnFavoriteImg5.SetWidescreen(CFG.widescreen);

    //GuiButton btnFavorite(&btnFavoriteImg,&btnFavoriteImg, 2, 5, -125, -60, &trigA, &btnSoundOver, &btnClick,1);
    GuiButton btnFavorite1(imgFavorite.GetWidth(), imgFavorite.GetHeight());
    GuiButton btnFavorite2(imgFavorite.GetWidth(), imgFavorite.GetHeight());
    GuiButton btnFavorite3(imgFavorite.GetWidth(), imgFavorite.GetHeight());
    GuiButton btnFavorite4(imgFavorite.GetWidth(), imgFavorite.GetHeight());
    GuiButton btnFavorite5(imgFavorite.GetWidth(), imgFavorite.GetHeight());

	SetupFavoriteButton(&btnFavorite1, -198, &btnFavoriteImg1, &btnSoundOver, btnClick2, &trigA);
	SetupFavoriteButton(&btnFavorite2, -171, &btnFavoriteImg2, &btnSoundOver, btnClick2, &trigA);
	SetupFavoriteButton(&btnFavorite3, -144, &btnFavoriteImg3, &btnSoundOver, btnClick2, &trigA);
	SetupFavoriteButton(&btnFavorite4, -117, &btnFavoriteImg4, &btnSoundOver, btnClick2, &trigA);
	SetupFavoriteButton(&btnFavorite5,  -90, &btnFavoriteImg5, &btnSoundOver, btnClick2, &trigA);

    GuiImage btnLeftImg(&imgLeft);
    if (Settings.wsprompt == yes) {
        btnLeftImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton btnLeft(&btnLeftImg,&btnLeftImg, 0, 5, 20, 0, &trigA, &btnSoundOver, btnClick2,1);
    btnLeft.SetTrigger(&trigL);
    btnLeft.SetTrigger(&trigMinus);

    GuiImage btnRightImg(&imgRight);
    if (Settings.wsprompt == yes) {
        btnRightImg.SetWidescreen(CFG.widescreen);
    }
    GuiButton btnRight(&btnRightImg,&btnRightImg, 1, 5, -20, 0, &trigA, &btnSoundOver, btnClick2,1);
    btnRight.SetTrigger(&trigR);
    btnRight.SetTrigger(&trigPlus);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&nameBtn);
    promptWindow.Append(&playcntTxt);
    promptWindow.Append(&btn2);
	if (!mountMethod)//stuff we don't show if it is a DVD mounted
	{
		promptWindow.Append(&sizeTxt);
		promptWindow.Append(&btnLeft);
		promptWindow.Append(&btnRight);
		promptWindow.Append(&btnFavorite1);
		promptWindow.Append(&btnFavorite2);
		promptWindow.Append(&btnFavorite3);
		promptWindow.Append(&btnFavorite4);
		promptWindow.Append(&btnFavorite5);
	}

    //check if unlocked
    if (Settings.godmode == 1 && mountMethod!=2 && mountMethod!=3) {
        promptWindow.Append(&btn3);
    }

    promptWindow.Append(&diskImg2);
    promptWindow.Append(&btn1);

    short changed = -1;
    GuiImageData * diskCover = NULL;
    GuiImageData * diskCover2 = NULL;

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

    while (changed) {
        if (changed == 1) {
            promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
        } else if (changed == 2) {
            promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
        } else if (changed == 3 || changed == 4) {
            if (diskCover2)
                delete diskCover2;
            diskCover2 = NULL;
            if (diskCover)
                diskCover2 = diskCover;
            diskCover = NULL;
        }

        //load disc image based or what game is seleted
        struct discHdr * header = (mountMethod==1||mountMethod==2?dvdheader:&gameList[gameSelected]);
        if(Settings.gamesoundvolume > 0)
        {
            if(gameSound)
			{
				gameSound->Stop();
				delete gameSound;
				gameSound = NULL;
			}
			u32 gameSoundDataLen;
            const u8 *gameSoundData = LoadBannerSound(header->id, &gameSoundDataLen);
			if(gameSoundData)
			{
				gameSound = new GuiSound(gameSoundData, gameSoundDataLen, Settings.gamesoundvolume, false, true);
				bgMusic->SetVolume(0);
                if(Settings.gamesound == 2)
                    gameSound->SetLoop(1);
				gameSound->Play();
			}
        }
        snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
        snprintf (IDFull,sizeof(IDFull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

        gprintf("\n\t%s",IDFull);
		if (diskCover)
            delete diskCover;



        snprintf(imgPath,sizeof(imgPath),"%s%s.png", Settings.disc_path, IDFull); //changed to current full id
        diskCover = new GuiImageData(imgPath,0);

        if (!diskCover->GetImage()) {
            delete diskCover;
            snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.disc_path, ID); //changed to current id
            diskCover = new GuiImageData(imgPath, 0);


            if (!diskCover->GetImage()) {
                snprintf (ID,sizeof(ID),"%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3]);

                delete diskCover;
                snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.disc_path, ID); //changed to current id
                diskCover = new GuiImageData(imgPath, 0);
                if (!diskCover->GetImage()) {
                    delete diskCover;
                    snprintf(imgPath, sizeof(imgPath), "%snodisc.png", CFG.theme_path); //changed to nodisc.png
                    diskCover = new GuiImageData(imgPath,nodisc_png);
                }
            }
        }



        if (changed == 3) {
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
            while (nameTxt.GetEffect() > 0 || diskImg.GetBetaRotateEffect()) usleep(50);
            HaltGui();
            diskImg.SetImage(diskCover);
            diskImg.SetBeta(90);
            diskImg.SetBetaRotateEffect(-90, 15);
            diskImg2.SetImage(diskCover2);
            diskImg2.SetBeta(270);
            diskImg2.SetBetaRotateEffect(-90, 15);
            sizeTxt.SetEffect(EFFECT_FADE, 17);
            nameTxt.SetEffect(EFFECT_FADE, 17);
        } else if (changed == 4) {
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
            while (nameTxt.GetEffect() > 0 || diskImg.GetBetaRotateEffect()) usleep(50);
            HaltGui();
            diskImg.SetImage(diskCover);
            diskImg.SetBeta(270);
            diskImg.SetBetaRotateEffect(90, 15);
            diskImg2.SetImage(diskCover2);
            diskImg2.SetBeta(90);
            diskImg2.SetBetaRotateEffect(90, 15);
            sizeTxt.SetEffect(EFFECT_FADE, 17);
            nameTxt.SetEffect(EFFECT_FADE, 17);
        } else
            diskImg.SetImage(diskCover);

		if (!mountMethod)
		{
			WBFS_GameSize(header->id, &size);
			sizeTxt.SetTextf("%.2fGB", size); //set size text;
		}

		nameTxt.SetText(get_title(header));

        struct Game_NUM* game_num = CFG_get_game_num(header->id);
        if (game_num) {
            playcount = game_num->count;
            favoritevar = game_num->favorite;
        } else {
            playcount = 0;
            favoritevar = 0;
        }
        playcntTxt.SetTextf("%s: %i",tr("Play Count"), playcount);
		SetFavoriteImages(&btnFavoriteImg1, &btnFavoriteImg2, &btnFavoriteImg3, &btnFavoriteImg4, &btnFavoriteImg5, &imgFavorite, &imgNotFavorite);

        nameTxt.SetPosition(0, 1);

        if (changed != 3 && changed != 4) { // changed==3 or changed==4 --> only Resume the GUI
            HaltGui();
            mainWindow->SetState(STATE_DISABLED);
            mainWindow->Append(&promptWindow);
            mainWindow->ChangeFocus(&promptWindow);
        }
        ResumeGui();

        changed = 0;
        while (choice == -1)
        {
            VIDEO_WaitVSync ();

            diskImg.SetSpin(btn1.GetState() == STATE_SELECTED);
            diskImg2.SetSpin(btn1.GetState() == STATE_SELECTED);
            if (shutdown == 1) { //for power button
				promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
                mainWindow->SetState(STATE_DEFAULT);
				while (promptWindow.GetEffect() > 0) usleep(50);
				HaltGui();
				mainWindow->Remove(&promptWindow);
				ResumeGui();
                wiilight(0);
                Sys_Shutdown();
            }

            if (reset == 1) //for reset button
                Sys_Reboot();

            if(gameSound)
            {
                if(!gameSound->IsPlaying())
                {
                    if(Settings.gamesound == 1)
                        bgMusic->SetVolume(Settings.volume);
                }
            }

            if (btn1.GetState() == STATE_CLICKED) {
                //playcounter
                struct Game_NUM* game_num = CFG_get_game_num(header->id);
                if (game_num) {
                    favoritevar = game_num->favorite;
                    playcount = game_num->count;
                } else {
                    favoritevar = 0;
                    playcount = 0;
                }
                playcount += 1;
                if (isInserted(bootDevice)) {
                    CFG_save_game_num(header->id);
                }

                choice = 1;
            }

            else if (btn2.GetState() == STATE_CLICKED) { //back
                choice = 0;
                promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
                mainWindow->SetState(STATE_DEFAULT);
                wiilight(0);
            }

            else if (btn3.GetState() == STATE_CLICKED) { //settings
                choice = 2;
                promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            }

            else if (nameBtn.GetState() == STATE_CLICKED) { //rename
                choice = 3;
                promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
            }
            else if (btnFavorite1.GetState() == STATE_CLICKED) {//switch favorite
                if (isInserted(bootDevice)) {
					SetFavorite(&btnFavorite1, &btnFavorite2, &btnFavorite3, &btnFavorite4, &btnFavorite5, header->id, 1);
					SetFavoriteImages(&btnFavoriteImg1, &btnFavoriteImg2, &btnFavoriteImg3, &btnFavoriteImg4, &btnFavoriteImg5, &imgFavorite, &imgNotFavorite);
                }
                btnFavorite1.ResetState();
            }
            else if (btnFavorite2.GetState() == STATE_CLICKED) {//switch favorite
                if (isInserted(bootDevice)) {
					SetFavorite(&btnFavorite1, &btnFavorite2, &btnFavorite3, &btnFavorite4, &btnFavorite5, header->id, 2);
					SetFavoriteImages(&btnFavoriteImg1, &btnFavoriteImg2, &btnFavoriteImg3, &btnFavoriteImg4, &btnFavoriteImg5, &imgFavorite, &imgNotFavorite);
                }
                btnFavorite2.ResetState();
            }
            else if (btnFavorite3.GetState() == STATE_CLICKED) {//switch favorite
                if (isInserted(bootDevice)) {
					SetFavorite(&btnFavorite1, &btnFavorite2, &btnFavorite3, &btnFavorite4, &btnFavorite5, header->id, 3);
					SetFavoriteImages(&btnFavoriteImg1, &btnFavoriteImg2, &btnFavoriteImg3, &btnFavoriteImg4, &btnFavoriteImg5, &imgFavorite, &imgNotFavorite);
                }
                btnFavorite3.ResetState();
            }
            else if (btnFavorite4.GetState() == STATE_CLICKED) {//switch favorite
                if (isInserted(bootDevice)) {
					SetFavorite(&btnFavorite1, &btnFavorite2, &btnFavorite3, &btnFavorite4, &btnFavorite5, header->id, 4);
					SetFavoriteImages(&btnFavoriteImg1, &btnFavoriteImg2, &btnFavoriteImg3, &btnFavoriteImg4, &btnFavoriteImg5, &imgFavorite, &imgNotFavorite);
                }
                btnFavorite4.ResetState();
            }
            else if (btnFavorite5.GetState() == STATE_CLICKED) {//switch favorite
                if (isInserted(bootDevice)) {
					SetFavorite(&btnFavorite1, &btnFavorite2, &btnFavorite3, &btnFavorite4, &btnFavorite5, header->id, 5);
					SetFavoriteImages(&btnFavoriteImg1, &btnFavoriteImg2, &btnFavoriteImg3, &btnFavoriteImg4, &btnFavoriteImg5, &imgFavorite, &imgNotFavorite);
                }
                btnFavorite5.ResetState();
            }
            // this next part is long because nobody could agree on what the left/right buttons should do
            else if ((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == no)) {//next game
                promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
                changed = 1;
                btnClick2->Play();
                gameSelected = (gameSelected + 1) % gameCnt;
                btnRight.ResetState();
                break;
            }

            else if ((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == no)) {//previous game
                promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
                changed = 2;
                btnClick2->Play();
                gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
                btnLeft.ResetState();
                break;
            }

            else if ((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == yes)) {//previous game
                promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
                changed = 2;
                btnClick2->Play();
                gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
                btnRight.ResetState();
                break;
            }

            else if ((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == yes)) {//next game
                promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
                changed = 1;
                btnClick2->Play();
                gameSelected = (gameSelected + 1) % gameCnt;
                btnLeft.ResetState();
                break;
            }

            else if ((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == sysmenu)) {//previous game
                promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
                changed = 2;
                btnClick2->Play();
                gameSelected = (gameSelected + 1) % gameCnt;
                btnRight.ResetState();
                break;
            }

            else if ((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == sysmenu)) {//next game
                promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
                changed = 1;
                btnClick2->Play();
                gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
                btnLeft.ResetState();
                break;
            }

            else if ((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == wtf)) {//previous game
                promptWindow.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
                changed = 1;
                btnClick2->Play();
                gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
                btnRight.ResetState();
                break;
            }

            else if ((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == wtf)) {//next game
                promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
                changed = 2;
                btnClick2->Play();
                gameSelected = (gameSelected + 1) % gameCnt;
                btnLeft.ResetState();
                break;
            }

            else if ((btnRight.GetState() == STATE_CLICKED) && (Settings.xflip == disk3d)) {//next game
//				diskImg.SetBetaRotateEffect(45, 90);
                changed = 3;
                btnClick2->Play();
                gameSelected = (gameSelected + 1) % gameCnt;
                btnRight.ResetState();
                break;
            }

            else if ((btnLeft.GetState() == STATE_CLICKED) && (Settings.xflip == disk3d)) {//previous game
//				diskImg.SetBetaRotateEffect(-45, 90);
//				promptWindow.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 1/*50*/);
                changed = 4;
                btnClick2->Play();
                gameSelected = (gameSelected - 1 + gameCnt) % gameCnt;
                btnLeft.ResetState();
                break;
            }
        }


        while (promptWindow.GetEffect() > 0) usleep(50);
        HaltGui();
        if (changed != 3 && changed != 4) { // changed==3 or changed==4 --> only Halt the GUI
            mainWindow->Remove(&promptWindow);
            ResumeGui();
        }
    }
    delete diskCover;
    delete diskCover2;

    if(gameSound)
    {
        gameSound->Stop();
		delete gameSound;
        gameSound = NULL;
    }
    bgMusic->SetVolume(Settings.volume);

    gprintf("\n\treturn %i",choice);
    return choice;
}

/****************************************************************************
 * DiscWait
 ***************************************************************************/
int
DiscWait(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, int IsDeviceWait) {
    int i = 30, ret = 0;
    u32 cover = 0;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

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
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    GuiText titleTxt(title, 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,60);
    GuiText msgTxt(msg, 22, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);
    msgTxt.SetMaxWidth(430);

    GuiText btn1Txt(btn1Label, 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn1(&btn1Img,&btn1Img, 1, 5, 0, 0, &trigA, &btnSoundOver, btnClick2,1);

    if (btn2Label) {
        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1.SetPosition(40, -45);
    } else {
        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1.SetPosition(0, -45);
    }

    btn1.SetLabel(&btn1Txt);
    btn1.SetTrigger(&trigB);
    btn1.SetState(STATE_SELECTED);

    GuiText btn2Txt(btn2Label, 22, THEME.prompttext);
    GuiImage btn2Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn2Txt.SetWidescreen(CFG.widescreen);
        btn2Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn2(&btn2Img,&btn2Img, 1, 4, -20, -25, &trigA, &btnSoundOver, btnClick2,1);
    btn2.SetLabel(&btn2Txt);

    if ((Settings.wsprompt == yes) && (CFG.widescreen)) {/////////////adjust buttons for widescreen
        msgTxt.SetMaxWidth(380);
        if (btn2Label) {
            btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
            btn2.SetPosition(-70, -80);
            btn1.SetPosition(70, -80);
        } else {
            btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            btn1.SetPosition(0, -80);
        }
    }

    GuiText timerTxt(NULL, 26, THEME.prompttext);
    timerTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    timerTxt.SetPosition(0,160);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);

    if (btn1Label)
        promptWindow.Append(&btn1);
    if (btn2Label)
        promptWindow.Append(&btn2);
    if (IsDeviceWait)
        promptWindow.Append(&timerTxt);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    if (IsDeviceWait) {
        while (i >= 0) {
            VIDEO_WaitVSync();
            timerTxt.SetTextf("%u %s", i,tr("seconds left"));
            HaltGui();
            if (Settings.cios == ios222) {
                ret = IOS_ReloadIOS(222);
                load_ehc_module();
            } else {
                ret = IOS_ReloadIOS(249);
            }
            ResumeGui();
            sleep(1);
            ret = WBFS_Init(WBFS_DEVICE_USB);
            if (ret>=0)
                break;

            i--;
        }
    } else {
        while (!(cover & 0x2)) {
            VIDEO_WaitVSync();
            if (btn1.GetState() == STATE_CLICKED) {
                btn1.ResetState();
                break;
            }
            ret = WDVD_GetCoverStatus(&cover);
            if (ret < 0)
                break;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);
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
FormatingPartition(const char *title, partitionEntry *entry) {
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
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    GuiText titleTxt(title, 26, THEME.prompttext);
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
    while (promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return ret;
}


/****************************************************************************
 * SearchMissingImages
 ***************************************************************************/
bool SearchMissingImages(int choice2) {

	gprintf("\nSearchMissingImages(%i)",choice2);
    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);

    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    GuiText titleTxt(tr("Checking existing artwork"), 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,60);

    char msg[20] = " ";
    GuiText msgTxt(msg, 22, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    //make sure that all games are added to the gamelist
    __Menu_GetEntries(1);

	cntMissFiles = 0;
	u32 i = 0;
	char filename[11];

	//add IDs of games that are missing covers to cntMissFiles
	bool found1 = false;
	bool found2 = false;
	bool found3 = false;
	for (i = 0; i < gameCnt && cntMissFiles < 500; i++) {
		struct discHdr* header = &gameList[i];
		if (choice2 != 3) {

			char *covers_path = choice2==1 ? Settings.covers2d_path : Settings.covers_path;

			snprintf (filename,sizeof(filename),"%c%c%c.png", header->id[0], header->id[1], header->id[2]);
			found2 = findfile(filename, covers_path);

			snprintf (filename,sizeof(filename),"%c%c%c%c.png", header->id[0], header->id[1], header->id[2], header->id[3]);
			found3 = findfile(filename, covers_path);

			snprintf(filename,sizeof(filename),"%c%c%c%c%c%c.png",header->id[0], header->id[1], header->id[2],
					 header->id[3], header->id[4], header->id[5]); //full id
			found1 = findfile(filename, covers_path);
			if (!found1 && !found2 && !found3) { //if could not find any image
				snprintf(missingFiles[cntMissFiles],11,"%s",filename);
				cntMissFiles++;
			}
		} else if (choice2 == 3) {
			snprintf (filename,sizeof(filename),"%c%c%c.png", header->id[0], header->id[1], header->id[2]);
			found2 = findfile(filename, Settings.disc_path);
			snprintf(filename,sizeof(filename),"%c%c%c%c%c%c.png",header->id[0], header->id[1], header->id[2],
					 header->id[3], header->id[4], header->id[5]); //full id
			found1 = findfile(filename,Settings.disc_path);
			if (!found1 && !found2) {
				snprintf(missingFiles[cntMissFiles],11,"%s",filename);
				cntMissFiles++;
			}
		}
	}
	if (cntMissFiles == 0) {
		msgTxt.SetText(tr("No file missing!"));
        sleep(1);
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
	__Menu_GetEntries();
    ResumeGui();

	gprintf(" = %i",cntMissFiles);
    if (cntMissFiles > 0) { //&& !IsNetworkInit()) {
		NetworkInitPrompt();
	}

	if (cntMissFiles == 0) {
		return false;
	} else {
		return true;
	}
}
/****************************************************************************
 * NetworkInitPrompt
 ***************************************************************************/
bool NetworkInitPrompt() {

	gprintf("\nNetworkinitPrompt()");
    if (IsNetworkInit())
		return true;

    bool success = true;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);

    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    GuiText titleTxt(tr("Initializing Network"), 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,60);

    char msg[20] = " ";
    GuiText msgTxt(msg, 22, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);

    GuiText btn1Txt(tr("Cancel"), 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -45, &trigA, &btnSoundOver, btnClick2,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

    if ((Settings.wsprompt == yes) && (CFG.widescreen)) {/////////////adjust buttons for widescreen
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

    while (!IsNetworkInit()) {

        VIDEO_WaitVSync();

        Initialize_Network();

        if (!IsNetworkInit()) {
            msgTxt.SetText(tr("Could not initialize network!"));
            sleep(3);
            success = false;
            break;
        }

        if (btn1.GetState() == STATE_CLICKED) {
            btn1.ResetState();
            success = false;
            break;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    return success;
}

/****************************************************************************
 * ProgressDownloadWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
int
ProgressDownloadWindow(int choice2) {

    int i = 0, cntNotFound = 0;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
    GuiImageData progressbarOutline(imgPath, progressbar_outline_png);
    GuiImage progressbarOutlineImg(&progressbarOutline);
    if (Settings.wsprompt == yes) {
        progressbarOutlineImg.SetWidescreen(CFG.widescreen);
    }
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
    GuiImage progressbarImg(&progressbar);
    progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    progressbarImg.SetPosition(25, 40);

    GuiText titleTxt(tr("Downloading file"), 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,60);

    GuiText msgTxt(NULL, 20, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msgTxt.SetPosition(0,130);

    GuiText msg2Txt(NULL, 26, THEME.prompttext);
    msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msg2Txt.SetPosition(0,100);

    GuiText prTxt(NULL, 26, THEME.prompttext);
    prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    prTxt.SetPosition(0, 40);

    GuiText btn1Txt(tr("Cancel"), 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -45, &trigA, &btnSoundOver, btnClick2,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

    if ((Settings.wsprompt == yes) && (CFG.widescreen)) {/////////////adjust for widescreen
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

    int offset=0, tries=0;
    int serverCnt3d = 1,serverCnt2d = 1,serverCntDisc = 2;

    char server3d[100];
    char serverDisc[100];
    char serverDiscCustom[100];
    char server2d[100];

    snprintf(server3d, sizeof(server3d), "http://wiitdb.com/wiitdb/artwork/cover3D/");
    snprintf(serverDisc, sizeof(serverDisc), "http://wiitdb.com/wiitdb/artwork/disc/");
    snprintf(serverDiscCustom, sizeof(serverDiscCustom), "http://wiitdb.com/wiitdb/artwork/disccustom/");
    snprintf(server2d, sizeof(server2d), "http://wiitdb.com/wiitdb/artwork/cover/");

    //check if directory exist and if not create one
    struct stat st;
    if (stat(Settings.covers_path, &st) != 0) {
        if (subfoldercreate(Settings.covers_path) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            cntMissFiles = 0;
        }
    }
    if (stat(Settings.covers2d_path, &st) != 0) {
        if (subfoldercreate(Settings.covers2d_path) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            cntMissFiles = 0;
        }
    }
    if (stat(Settings.disc_path,&st) != 0) {
        if (subfoldercreate(Settings.disc_path) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            cntMissFiles = 0;
        }
    }

    //int server = 1;
    while (i < cntMissFiles) {
        tries=0;
        prTxt.SetTextf("%i%%", 100*i/cntMissFiles);

        if ((Settings.wsprompt == yes) && (CFG.widescreen)) {
            //adjust for widescreen
            progressbarImg.SetPosition(80,40);
            progressbarImg.SetTile(80*i/cntMissFiles);
        } else {
            progressbarImg.SetTile(100*i/cntMissFiles);
        }

        if (cntMissFiles - i>1)msgTxt.SetTextf("%i %s", cntMissFiles - i, tr("files left"));
        else msgTxt.SetTextf("%i %s", cntMissFiles - i, tr("file left"));
        msg2Txt.SetTextf("http://wiitdb.com : %s", missingFiles[i]);


        //download cover
        char imgPath[100];
        char URLFile[100];
        char tmp[75];
        sprintf(tmp,tr("Not Found"));
        struct block file = downloadfile(URLFile);
        if (choice2 == 2) {
            while (tries<serverCnt3d) {

                //Creates URL depending from which Country the game is
                switch (missingFiles[i][3]) {
                case 'J':
					sprintf(URLFile,"%sJA/%s",server3d,missingFiles[i]);
                    break;
                case 'W':
                    sprintf(URLFile,"%sZH/%s",server3d,missingFiles[i]);
                    break;
                case 'K':
                    sprintf(URLFile,"%sKO/%s",server3d,missingFiles[i]);
                    break;
                case 'P':
                case 'D':
                case 'F':
                case 'I':
                case 'S':
                case 'H':
                case 'U':
                case 'X':
                case 'Y':
                case 'Z':
                    sprintf(URLFile,"%s%s/%s",server3d,Settings.db_language,missingFiles[i]);
                    break;
                case 'E':
                    sprintf(URLFile,"%sUS/%s",server3d,missingFiles[i]);
                    break;
                }

                sprintf(imgPath,"%s%s", Settings.covers_path, missingFiles[i]);
                file = downloadfile(URLFile);

                if (!(file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL)) {
                    break;
                } else {
                    sprintf(URLFile,"%sEN/%s",server3d,missingFiles[i]);
                    file = downloadfile(URLFile);
                    if (!(file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL)) {
                        break;
                    }
                }
                tries++;
            }

        }
        if (choice2 == 3) {
            while (tries<serverCntDisc) {

                //Creates URL depending from which Country the game is
                switch (missingFiles[i][3]) {
                case 'J':
                    if(Settings.discart == 0) {
                        sprintf(URLFile,"%sJA/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 1) {
                        sprintf(URLFile,"%sJA/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 0) {
                        sprintf(URLFile,"%sJA/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 1) {
                        sprintf(URLFile,"%sJA/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 0) {
                        sprintf(URLFile,"%sJA/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 1) {
                        sprintf(URLFile,"%sJA/%s",serverDisc,missingFiles[i]);
                    }
                    break;
                case 'W':
                    if(Settings.discart == 0) {
                        sprintf(URLFile,"%sZH/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 1) {
                        sprintf(URLFile,"%sZH/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 0) {
                        sprintf(URLFile,"%sZH/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 1) {
                        sprintf(URLFile,"%sZH/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 0) {
                        sprintf(URLFile,"%sZH/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 1) {
                        sprintf(URLFile,"%sZH/%s",serverDisc,missingFiles[i]);
                    }
                    break;
                case 'K':
                    if(Settings.discart == 0) {
                        sprintf(URLFile,"%sKO/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 1) {
                        sprintf(URLFile,"%sKO/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 0) {
                        sprintf(URLFile,"%sKO/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 1) {
                        sprintf(URLFile,"%sKO/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 0) {
                        sprintf(URLFile,"%sKO/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 1) {
                        sprintf(URLFile,"%sKO/%s",serverDisc,missingFiles[i]);
                    }
                    break;
                case 'P':
                case 'D':
                case 'F':
                case 'I':
                case 'S':
                case 'H':
                case 'U':
                case 'X':
                case 'Y':
                case 'Z':
                    if(Settings.discart == 0) {
                        sprintf(URLFile,"%s%s/%s",serverDisc,Settings.db_language,missingFiles[i]);
                    } else if(Settings.discart == 1) {
                        sprintf(URLFile,"%s%s/%s",serverDiscCustom,Settings.db_language,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 0) {
                        sprintf(URLFile,"%s%s/%s",serverDisc,Settings.db_language,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 1) {
                        sprintf(URLFile,"%s%s/%s",serverDiscCustom,Settings.db_language,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 0) {
                        sprintf(URLFile,"%s%s/%s",serverDiscCustom,Settings.db_language,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 1) {
                        sprintf(URLFile,"%s%s/%s",serverDisc,Settings.db_language,missingFiles[i]);
                    }
                    break;
                case 'E':
                    if(Settings.discart == 0) {
                        sprintf(URLFile,"%sUS/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 1) {
                        sprintf(URLFile,"%sUS/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 0) {
                        sprintf(URLFile,"%sUS/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 1) {
                        sprintf(URLFile,"%sUS/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 0) {
                        sprintf(URLFile,"%sUS/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 1) {
                        sprintf(URLFile,"%sUS/%s",serverDisc,missingFiles[i]);
                    }
                    break;
                }

                sprintf(imgPath,"%s%s", Settings.disc_path, missingFiles[i]);
                file = downloadfile(URLFile);
                if (!(file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL)) {
                    break;
                } else {
                    if(Settings.discart == 0) {
                        sprintf(URLFile,"%sEN/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 1) {
                        sprintf(URLFile,"%sEN/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 0) {
                        sprintf(URLFile,"%sEN/%s",serverDisc,missingFiles[i]);
                    } else if(Settings.discart == 2 && tries == 1) {
                        sprintf(URLFile,"%sEN/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 0) {
                        sprintf(URLFile,"%sEN/%s",serverDiscCustom,missingFiles[i]);
                    } else if(Settings.discart == 3 && tries == 1) {
                        sprintf(URLFile,"%sEN/%s",serverDisc,missingFiles[i]);
                    }
                    file = downloadfile(URLFile);
                    if (!(file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL)) {
                        break;
                    }
                }
                tries++;
            }
        }
        if (choice2 == 1) {
            while (tries<serverCnt2d) {

                //Creates URL depending from which Country the game is
                switch (missingFiles[i][3]) {
                case 'J':
				    sprintf(URLFile,"%sJA/%s",server2d,missingFiles[i]);
                    break;
                case 'W':
                    sprintf(URLFile,"%sZH/%s",server2d,missingFiles[i]);
                    break;
                case 'K':
                    sprintf(URLFile,"%sKO/%s",server2d,missingFiles[i]);
                    break;
                case 'P':
                case 'D':
                case 'F':
                case 'I':
                case 'S':
                case 'H':
                case 'U':
                case 'X':
                case 'Y':
                case 'Z':
                    sprintf(URLFile,"%s%s/%s",server2d,Settings.db_language,missingFiles[i]);
                    break;
                case 'E':
                    sprintf(URLFile,"%sUS/%s",server2d,missingFiles[i]);
                    break;
                }

                sprintf(imgPath,"%s%s", Settings.covers2d_path, missingFiles[i]);
                file = downloadfile(URLFile);

                if (!(file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL)) {
                    break;
                } else {
                    sprintf(URLFile,"%sEN/%s",server2d,missingFiles[i]);
                    file = downloadfile(URLFile);
                    if (!(file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL)) {
                        break;
                    }
                }
                tries++;
            }
        }


        offset++;

        if (file.size == 36864 || file.size <= 1024 || file.size <= 1174 || file.size == 7386 || file.size == 4446 || file.data == NULL) {
            cntNotFound++;
            i++;
        } else {
            if (file.data != NULL) {
                // save png to sd card
                FILE *pfile=NULL;
                if ((pfile = fopen(imgPath, "wb"))!=NULL) {
                    fwrite(file.data,1,file.size,pfile);
                    fclose (pfile);
                }
                free(file.data);
            }
            i++;

        }

        if (btn1.GetState() == STATE_CLICKED) {
            cntNotFound = cntMissFiles-i+cntNotFound;
            break;
        }
    }

    /**Temporary redownloading 1st image because of a fucking corruption bug **/
#if 0 // is no longer necessary, since libfat is fixed
    char URLFile[100];
    struct block file = downloadfile(URLFile);
    if (choice2 == 2) {
        while (tries<serverCnt3d) {
            sprintf(URLFile,"%s%s",server3d,missingFiles[0]);
            sprintf(imgPath,"%s%s", Settings.covers_path, missingFiles[0]);
            file = downloadfile(URLFile);
            if (!(file.size == 36864 || file.size <= 1024 || file.size <= 1174 || file.size == 7386 || file.size == 4446 || file.data == NULL))break;
            tries++;
        }

    }
    if (choice2 == 3) {
        while (tries<serverCntDisc) {
            sprintf(URLFile,"%s%s",serverDisc,missingFiles[0]);
            sprintf(imgPath,"%s%s", Settings.disc_path, missingFiles[0]);
            file = downloadfile(URLFile);
            if (!(file.size == 36864 || file.size <= 1024 || file.size <= 1174 || file.size == 7386 || file.size == 4446 || file.data == NULL))break;
            tries++;
        }
    }
    if (choice2 == 1) {
        while (tries<serverCnt2d) {
            sprintf(URLFile,"%s%s",server2d,missingFiles[0]);
            sprintf(imgPath,"%s%s", Settings.covers2d_path, missingFiles[0]);
            file = downloadfile(URLFile);
            if (!(file.size == 36864 || file.size <= 1024 || file.size <= 1174 || file.size == 7386 || file.size == 4446 || file.data == NULL))break;
            tries++;
        }
    }
    if (file.size == 36864 || file.size <= 1024 || file.size == 7386 || file.size <= 1174 || file.size == 4446 || file.data == NULL) {
    } else {
        if (file.data != NULL) {
            // save png to sd card
            FILE *pfile;
            pfile = fopen(imgPath, "wb");
            fwrite(file.data,1,file.size,pfile);
            fclose (pfile);
            free(file.data);
        }
    }
#endif
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
#define BLOCKSIZE    1024
/*bool unzipArchive(char * zipfilepath, char * unzipfolderpath)
{
   unzFile uf = unzOpen(zipfilepath);
   if (uf==NULL)
   {
   // printf("Cannot open %s, aborting\n",zipfilepath);
      return false;
   }
   //printf("%s opened\n",zipfilepath);
   if(chdir(unzipfolderpath)) // can't access dir
   {
   	makedir(unzipfolderpath); // attempt to make dir
   	if(chdir(unzipfolderpath)) // still can't access dir
   	  {
   	//printf("Error changing into %s, aborting\n", unzipfolderpath);
   	return false;
   	}
   }
   extractZip(uf,0,1,0);
   unzCloseCurrentFile(uf);
   return true
}
*/

#ifdef NOTFULLCHANNEL

int ProgressUpdateWindow() {

    gprintf("\nProgressUpdateWindow(not full channel)");
    int ret = 0, failed = 0, updatemode = -1;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
    GuiImageData progressbarOutline(imgPath, progressbar_outline_png);
    GuiImage progressbarOutlineImg(&progressbarOutline);
    if (Settings.wsprompt == yes) {
        progressbarOutlineImg.SetWidescreen(CFG.widescreen);
    }
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
    GuiImage progressbarImg(&progressbar);
    progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    progressbarImg.SetPosition(25, 7);

    char title[50];
    sprintf(title, "%s", tr("Checking for Updates"));
    GuiText titleTxt(title, 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,50);
    char msg[50];
    sprintf(msg, "%s", tr("Initializing Network"));
    GuiText msgTxt(msg, 26, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msgTxt.SetPosition(0,140);
    char msg2[50] = " ";
    GuiText msg2Txt(msg2, 26, THEME.prompttext);
    msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msg2Txt.SetPosition(0, 50);

    GuiText prTxt(NULL, 26, THEME.prompttext);
    prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    prTxt.SetPosition(0, 7);

    GuiText btn1Txt(tr("Cancel"), 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -40, &trigA, &btnSoundOver, btnClick2,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

    if ((Settings.wsprompt == yes) && (CFG.widescreen)) {/////////////adjust for widescreen
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
    if (stat(Settings.update_path, &st) != 0) {
        if (subfoldercreate(Settings.update_path) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            ret = -1;
            failed = -1;
        }
    }

    char dolpath[150];
    char dolpathsuccess[150];
    snprintf(dolpath, sizeof(dolpath), "%sbootnew.dol", Settings.update_path);
    snprintf(dolpathsuccess, sizeof(dolpathsuccess), "%sboot.dol", Settings.update_path);

    while (!IsNetworkInit()) {

        VIDEO_WaitVSync();

        Initialize_Network();

        if (IsNetworkInit()) {
            msgTxt.SetText(GetNetworkIP());
        } else {
            msgTxt.SetText(tr("Could not initialize network!"));
        }

        if (btn1.GetState() == STATE_CLICKED) {
            ret = -1;
            failed = -1;
            btn1.ResetState();
            break;
        }
    }

	//make the URL to get XML based on our games
	char XMLurl[3540];
	build_XML_URL(XMLurl,sizeof(XMLurl));

	if (IsNetworkInit() && ret >= 0) {

		updatemode = WindowPrompt(tr("What do you want to update?"), 0, "USB Loader GX", tr("WiiTDB Files"), tr("Language File"), tr("Cancel"));
		mainWindow->SetState(STATE_DISABLED);
		promptWindow.SetState(STATE_DEFAULT);
		mainWindow->ChangeFocus(&promptWindow);

        if(updatemode == 1) {

        int newrev = CheckUpdate();
        if (newrev > 0) {

            sprintf(msg, "Rev%i %s.", newrev, tr("available"));
            int choice = WindowPrompt(msg, tr("How do you want to update?"), tr("Update DOL"), tr("Update All"), tr("Cancel"));
            mainWindow->SetState(STATE_DISABLED);
            promptWindow.SetState(STATE_DEFAULT);
            mainWindow->ChangeFocus(&promptWindow);
            if (choice == 1 || choice == 2) {
                titleTxt.SetTextf("%s USB Loader GX", tr("Updating"));
                msgTxt.SetPosition(0,100);
                promptWindow.Append(&progressbarEmptyImg);
                promptWindow.Append(&progressbarImg);
                promptWindow.Append(&progressbarOutlineImg);
                promptWindow.Append(&prTxt);
                msgTxt.SetTextf("%s Rev%i", tr("Update to"), newrev);
                s32 filesize = download_request("http://www.techjawa.com/usbloadergx/boot.dol");
                if (filesize > 0) {
                    FILE * pfile;
                    pfile = fopen(dolpath, "wb");
                    u8 * blockbuffer = new unsigned char[BLOCKSIZE];
                    for (s32 i = 0; i < filesize; i += BLOCKSIZE) {
                        usleep(100);
                        prTxt.SetTextf("%i%%", (100*i/filesize)+1);
                        if ((Settings.wsprompt == yes) && (CFG.widescreen)) {
                            progressbarImg.SetTile(80*i/filesize);
                        } else {
                            progressbarImg.SetTile(100*i/filesize);
                        }
                        msg2Txt.SetTextf("%iKB/%iKB", i/1024, filesize/1024);

                        if (btn1.GetState() == STATE_CLICKED) {
                            fclose(pfile);
                            remove(dolpath);
                            failed = -1;
                            btn1.ResetState();
                            break;
                        }

                        u32 blksize;
                        blksize = (u32)(filesize - i);
                        if (blksize > BLOCKSIZE)
                            blksize = BLOCKSIZE;

                        ret = network_read(blockbuffer, blksize);
                        if (ret != (s32) blksize) {
                            failed = -1;
                            ret = -1;
                            fclose(pfile);
                            remove(dolpath);
                            break;
                        }
                        fwrite(blockbuffer,1,blksize, pfile);
                    }
                    fclose(pfile);
                    delete blockbuffer;
                    if (!failed) {
                        //remove old
                        if (checkfile(dolpathsuccess)) {
                            remove(dolpathsuccess);
                        }
                        //rename new to old
                        rename(dolpath, dolpathsuccess);

                        if (choice == 2) {
                            //get the icon.png and the meta.xml
                            char xmliconpath[150];
                            struct block file = downloadfile("http://www.techjawa.com/usbloadergx/meta.file");
                            if (file.data != NULL) {
                                sprintf(xmliconpath, "%smeta.xml", Settings.update_path);
                                pfile = fopen(xmliconpath, "wb");
                                fwrite(file.data,1,file.size,pfile);
                                fclose(pfile);
                                free(file.data);
                            }
                            file = downloadfile("http://www.techjawa.com/usbloadergx/icon.png");
                            if (file.data != NULL) {
                                sprintf(xmliconpath, "%sicon.png", Settings.update_path);
                                pfile = fopen(xmliconpath, "wb");
                                fwrite(file.data,1,file.size,pfile);
                                fclose(pfile);
                                free(file.data);
                            }
                            msgTxt.SetTextf("%s", tr("Updating WiiTDB.zip"));
							char wiitdbpath[200];
				            char wiitdbpathtmp[200];
                            file = downloadfile(XMLurl);
                            if (file.data != NULL) {
								subfoldercreate(Settings.titlestxt_path);
								snprintf(wiitdbpath, sizeof(wiitdbpath), "%swiitdb.zip", Settings.titlestxt_path);
								snprintf(wiitdbpathtmp, sizeof(wiitdbpathtmp), "%swiitmp.zip", Settings.titlestxt_path);
								rename(wiitdbpath,wiitdbpathtmp);
								pfile = fopen(wiitdbpath, "wb");
								fwrite(file.data,1,file.size,pfile);
								fclose(pfile);
								free(file.data);
								CloseXMLDatabase();
								if (OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true)) { // open file, reload titles, keep in memory
									remove(wiitdbpathtmp);
								} else {
									remove(wiitdbpath);
									rename(wiitdbpathtmp,wiitdbpath);
									OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true); // open file, reload titles, keep in memory
								}
                            }
                            msgTxt.SetTextf("%s", tr("Updating Language Files:"));
                            updateLanguageFiles();
                        }
                    }
                } else {
                    failed = -1;
                }
            } else {
                ret = -1;
            }
        } else {
            WindowPrompt(tr("No new updates."), 0, tr("OK"));
            ret = -1;
        }

        } else if(updatemode == 2) {
            msgTxt.SetTextf("%s", tr("Updating WiiTDB.zip"));
            char wiitdbpath[200];
            char wiitdbpathtmp[200];
            struct block file = downloadfile(XMLurl);
            if (file.data != NULL) {
				subfoldercreate(Settings.titlestxt_path);
				snprintf(wiitdbpath, sizeof(wiitdbpath), "%swiitdb.zip", Settings.titlestxt_path);
				snprintf(wiitdbpathtmp, sizeof(wiitdbpathtmp), "%swiitmp.zip", Settings.titlestxt_path);
				rename(wiitdbpath,wiitdbpathtmp);
				FILE *pfile = fopen(wiitdbpath, "wb");
				fwrite(file.data,1,file.size,pfile);
				fclose(pfile);
				free(file.data);
				CloseXMLDatabase();
				if (OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true)) { // open file, reload titles, keep in memory
					remove(wiitdbpathtmp);
				} else {
					remove(wiitdbpath);
					rename(wiitdbpathtmp,wiitdbpath);
					OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true); // open file, reload titles, keep in memory
				}
            }
            ret = 1;
        } else if(updatemode == 3) {

            msgTxt.SetTextf("%s", tr("Updating Language Files..."));
            updateLanguageFiles();
            ret = 1;
        } else {
            ret = -1;
        }
    }

    CloseConnection();

    if (!failed && ret >= 0 && updatemode == 1) {
        WindowPrompt(tr("Successfully Updated") , tr("Restarting..."), tr("OK"));

		loadStub();
		Set_Stub_Split(0x00010001,"UNEO");
        Sys_BackToLoader();
    } else if(updatemode > 0 && ret > 0) {
        WindowPrompt(tr("Successfully Updated") , 0, tr("OK"));
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if (failed != 0)
        return failed;

    return 1;
}



#else  ///////////////////this is only used if  the dol is being compiled for a full channel
int ProgressUpdateWindow() {
    int ret = 0, failed = 0;

    gprintf("\nProgressUpdateWindow(full channel)");
    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }

    snprintf(imgPath, sizeof(imgPath), "%sprogressbar_outline.png", CFG.theme_path);
    GuiImageData progressbarOutline(imgPath, progressbar_outline_png);
    GuiImage progressbarOutlineImg(&progressbarOutline);
    if (Settings.wsprompt == yes) {
        progressbarOutlineImg.SetWidescreen(CFG.widescreen);
    }
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
    GuiImage progressbarImg(&progressbar);
    progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    progressbarImg.SetPosition(25, 7);

    char title[50];
    sprintf(title, "%s", tr("Checking for Updates"));
    GuiText titleTxt(title, 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,50);
    char msg[50];
    sprintf(msg, "%s", tr("Initializing Network"));
    GuiText msgTxt(msg, 26, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msgTxt.SetPosition(0,140);
    char msg2[50] = " ";
    GuiText msg2Txt(msg2, 26, THEME.prompttext);
    msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msg2Txt.SetPosition(0, 50);

    GuiText prTxt(NULL, 26, THEME.prompttext);
    prTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    prTxt.SetPosition(0, 7);

    GuiText btn1Txt(tr("Cancel"), 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -40, &trigA, &btnSoundOver, &btnClick,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

    if ((Settings.wsprompt == yes) && (CFG.widescreen)) {/////////////adjust for widescreen
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
    if (stat(Settings.update_path, &st) != 0) {
        if (subfoldercreate(Settings.covers_path) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            ret = -1;
            failed = -1;
        }
    }

    if (stat(Settings.titlestxt_path, &st) != 0) {
        if (subfoldercreate(Settings.titlestxt_path) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            ret = -1;
            failed = -1;
        }
    }

    //make the URL to get XML based on our games
	char XMLurl[3540];
	build_XML_URL(XMLurl,sizeof(XMLurl));

    char dolpath[150];
//    char dolpathsuccess[150];//use coverspath as a folder for the update wad so we dont make a new folder and have to delete it
    snprintf(dolpath, sizeof(dolpath), "%sULNR.wad", Settings.covers_path);
    //snprintf(dolpathsuccess, sizeof(dolpathsuccess), "%sUNEO.wad", Settings.covers_path);
    Initialize_Network();
    while (!IsNetworkInit()) {

        VIDEO_WaitVSync();

        Initialize_Network();

        if (IsNetworkInit()) {
            msgTxt.SetText(GetNetworkIP());
        } else {
            msgTxt.SetText(tr("Could not initialize network!"));
        }

        if (btn1.GetState() == STATE_CLICKED) {
            ret = -1;
            failed = -1;
            btn1.ResetState();
            break;
        }
    }

    if (IsNetworkInit() && ret >= 0) {

        int newrev = CheckUpdate();

        if (newrev > 0) {
            FILE * pfile;
            sprintf(msg, "Rev%i %s.", newrev, tr("available"));
            int choice = WindowPrompt(msg, 0, tr("Update"),tr("Cancel"));
            if (choice == 1) {
                titleTxt.SetTextf("%s USB Loader GX", tr("Updating"));
                msgTxt.SetPosition(0,100);
                msgTxt.SetTextf("%s", tr("Updating WiiTDB.zip"));

				char wiitdbpath[200];
				char wiitdbpathtmp[200];
                struct block file = downloadfile(XMLurl);
                if (file.data != NULL) {
					snprintf(wiitdbpath, sizeof(wiitdbpath), "%swiitdb.zip", Settings.titlestxt_path);
					snprintf(wiitdbpathtmp, sizeof(wiitdbpathtmp), "%swiitmp.zip", Settings.titlestxt_path);
					rename(wiitdbpath,wiitdbpathtmp);
					pfile = fopen(wiitdbpath, "wb");
					fwrite(file.data,1,file.size,pfile);
					fclose(pfile);
					free(file.data);
					CloseXMLDatabase();
					if (OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true)) { // open file, reload titles, keep in memory
						remove(wiitdbpathtmp);
					} else {
						remove(wiitdbpath);
						rename(wiitdbpathtmp,wiitdbpath);
						OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, true, Settings.titlesOverride==1?true:false, true); // open file, reload titles, keep in memory
					}
				}

                msgTxt.SetTextf("%s", tr("Updating Language Files:"));
                updateLanguageFiles();
                promptWindow.Append(&progressbarEmptyImg);
                promptWindow.Append(&progressbarImg);
                promptWindow.Append(&progressbarOutlineImg);
                promptWindow.Append(&prTxt);
                msgTxt.SetTextf("%s Rev%i wad.", tr("Downloading"), newrev);
                s32 filesize = download_request("http://www.techjawa.com/usbloadergx/ULNR.file");//for some reason it didn't download completely when saved as a wad.
                if (filesize > 0) {

                    pfile = fopen(dolpath, "wb");//here we save the txt as a wad
                    u8 * blockbuffer = new unsigned char[BLOCKSIZE];
                    for (s32 i = 0; i < filesize; i += BLOCKSIZE) {
                        usleep(100);
                        prTxt.SetTextf("%i%%", (100*i/filesize)+1);
                        if ((Settings.wsprompt == yes) && (CFG.widescreen)) {
                            progressbarImg.SetTile(80*i/filesize);
                        } else {
                            progressbarImg.SetTile(100*i/filesize);
                        }
                        msg2Txt.SetTextf("%iKB/%iKB", i/1024, filesize/1024);

                        if (btn1.GetState() == STATE_CLICKED) {
                            fclose(pfile);
                            remove(dolpath);
                            failed = -1;
                            btn1.ResetState();
                            break;
                        }

                        u32 blksize;
                        blksize = (u32)(filesize - i);
                        if (blksize > BLOCKSIZE)
                            blksize = BLOCKSIZE;

                        ret = network_read(blockbuffer, blksize);
                        if (ret != (s32) blksize) {
                            failed = -1;
                            ret = -1;
                            fclose(pfile);
                            remove(dolpath);
                            break;
                        }
                        fwrite(blockbuffer,1,blksize, pfile);
                    }
                    fclose(pfile);
                    delete blockbuffer;
                    if (!failed) {
                    }
                } else {
                    failed = -1;
                }
            } else {
                ret = -1;
            }

        } else {
            WindowPrompt(tr("No new updates."), 0, tr("OK"));
            ret = -1;
        }

    }
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    //mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    CloseConnection();
    sleep(1);//sleep 1 because it froze without this for some reason

    if (!failed && ret >= 0) {

        FILE *wadFile = NULL;
        s32 error = 1;
        int diarhea = 0;
        char nipple[100];
        wadFile = fopen (dolpath ,"rb");
        if (wadFile==NULL) { //we can't open the file wad we just downloaded
            sprintf(nipple, tr("Unable to open the wad that was just downloaded (%s)."),dolpath);
            WindowPrompt(tr("Error !"), nipple, tr("OK"));
            failed = -1;
        } else {
            //sprintf(nipple, tr("The update wad has been saved as %s.  Now let's try to install it."),dolpath);
            //WindowPrompt(0,nipple, tr("OK"));
            gprintf("\n\tinstall wad");
			error = Wad_Install(wadFile);
            fclose(wadFile);
            if (error==0) {
                diarhea = remove(dolpath);
                if (diarhea)
                    WindowPrompt(tr("Success"),tr("The wad file was installed.  But It could not be deleted from the SD card."),tr("OK"));
            } else {
                gprintf(" -> failed");
				sprintf(nipple, tr("The wad installation failed with error %ld"),error);
                WindowPrompt(tr("Error"),nipple,tr("OK"));
            }
        }

        if (error)
            WindowPrompt(tr("ERROR") , tr("An Error occured"), tr("OK"));
        else
            WindowPrompt(tr("Successfully Updated") , tr("Restarting..."), 0,0,0,0,150);
        CloseXMLDatabase();
        ExitGUIThreads();
        ShutdownAudio();
        StopGX();
        gprintf("\nRebooting");
		WII_Initialize();
        WII_LaunchTitle(TITLE_ID(0x00010001,0x554c4e52));
    }

    // promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    //while(promptWindow.GetEffect() > 0) usleep(50);

    HaltGui();
    //mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    if (failed != 0)
        return failed;

    return 1;
}
#endif

int CodeDownload(const char *id) {
    int ret = 0;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiImage dialogBoxImg(&dialogBox);
    if (Settings.wsprompt == yes) {
        dialogBoxImg.SetWidescreen(CFG.widescreen);
    }



    char title[50];
    sprintf(title, "%s", tr("Code Download"));
    GuiText titleTxt(title, 26, THEME.prompttext);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,50);
    char msg[50];
    sprintf(msg, "%s", tr("Initializing Network"));
    GuiText msgTxt(msg, 26, THEME.prompttext);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    msgTxt.SetPosition(0,140);
    char msg2[50] = " ";
    GuiText msg2Txt(msg2, 26, THEME.prompttext);
    msg2Txt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msg2Txt.SetPosition(0, 50);

    GuiText btn1Txt(tr("Cancel"), 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn1(&btn1Img,&btn1Img, 2, 4, 0, -40, &trigA, &btnSoundOver, btnClick2,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

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
    if (stat(Settings.TxtCheatcodespath, &st) != 0) {
        if (subfoldercreate(Settings.TxtCheatcodespath) != 1) {
            WindowPrompt(tr("Error !"),tr("Can't create directory"),tr("OK"));
            ret = -1;
            goto exit;
        }
    }

    while (!IsNetworkInit()) {

        VIDEO_WaitVSync();

        Initialize_Network();

        if (IsNetworkInit()) {
            msgTxt.SetText(GetNetworkIP());
        } else {
            msgTxt.SetText(tr("Could not initialize network!"));
        }
        if (btn1.GetState() == STATE_CLICKED) {
            ret = -1;
            btn1.ResetState();
            goto exit;
        }
    }

    if (IsNetworkInit() && ret >= 0) {

        char txtpath[150];
        snprintf(txtpath, sizeof(txtpath), "%s%s.txt", Settings.TxtCheatcodespath,id);

        char codeurl[150];
        snprintf(codeurl, sizeof(codeurl), "http://geckocodes.org/codes/R/%s.txt",id);

        struct block  file = downloadfile(codeurl);

        if (file.size == 333 || file.size == 216 || file.size == 284) {
            strcat(codeurl, tr(" is not on the server."));

            WindowPrompt(tr("Error"),codeurl,tr("OK"));
            ret =-1;
            goto exit;
        }

        if (file.data != NULL) {

            FILE * pfile;
            pfile = fopen(txtpath, "wb");
            fwrite(file.data,1,file.size,pfile);
            fclose(pfile);
            free(file.data);
            ret = 1;
            strcat(txtpath, tr(" has been Saved.  The text has not been verified.  Some of the code may not work right with each other.  If you experience trouble, open the text in a real text editor for more information."));

            WindowPrompt(0,txtpath,tr("OK"));
        } else {
            strcat(codeurl, tr(" could not be downloaded."));

            WindowPrompt(tr("Error"),codeurl,tr("OK"));
            ret =-1;
        }

        CloseConnection();
    }
exit:
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();

    return ret;
}

char * GetMissingFiles() {
    return (char *) missingFiles;
}




/****************************************************************************
 * HBCWindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice of up to 2 Buttons.
 *
 ***************************************************************************/
/* <name>
 <coder>
   <version>
   <release_date>
   <short_description>
   <long_description>
	SD:/APPS/FTPII/ICON.PNG*/

int
HBCWindowPrompt(const char *name, const char *coder, const char *version,
                const char *release_date, const char *long_description, const char *iconPath, u64 filesize) {
    int choice = -1;


    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, 6);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
    GuiTrigger trigU;
    trigU.SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_UP);
    GuiTrigger trigD;
    trigD.SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN, PAD_BUTTON_DOWN);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
	// because destroy GuiSound must wait while sound playing is finished, we use a global sound
	if(!btnClick2) btnClick2=new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
	//	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    char imgPath[100];
    snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
    GuiImageData btnOutline(imgPath, button_dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sdialogue_box.png", CFG.theme_path);
    GuiImageData dialogBox(imgPath, dialogue_box_png);
    snprintf(imgPath, sizeof(imgPath), "%sbg_options.png", CFG.theme_path);
    GuiImageData whiteBox(imgPath, bg_options_png);

    snprintf(imgPath, sizeof(imgPath), "%sscrollbar.png", CFG.theme_path);
    GuiImageData scrollbar(imgPath, scrollbar_png);
    GuiImage scrollbarImg(&scrollbar);
    scrollbarImg.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    scrollbarImg.SetPosition(-40, 114);
    scrollbarImg.SetSkew(0,0,0,0,0,-120,0,-120);

    snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowdown.png", CFG.theme_path);
    GuiImageData arrowDown(imgPath, scrollbar_arrowdown_png);
    GuiImage arrowDownImg(&arrowDown);
    arrowDownImg.SetScale(.8);

    snprintf(imgPath, sizeof(imgPath), "%sscrollbar_arrowup.png", CFG.theme_path);
    GuiImageData arrowUp(imgPath, scrollbar_arrowup_png);
    GuiImage arrowUpImg (&arrowUp);
    arrowUpImg.SetScale(.8);

    GuiButton arrowUpBtn(arrowUpImg.GetWidth(), arrowUpImg.GetHeight());
    arrowUpBtn.SetImage(&arrowUpImg);
    arrowUpBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    arrowUpBtn.SetPosition(-25,91);
    arrowUpBtn.SetTrigger(&trigA);
    arrowUpBtn.SetTrigger(&trigU);
    arrowUpBtn.SetEffectOnOver(EFFECT_SCALE, 50, 130);
    arrowUpBtn.SetSoundClick(btnClick2);

    GuiButton arrowDownBtn(arrowDownImg.GetWidth(), arrowDownImg.GetHeight());
    arrowDownBtn.SetImage(&arrowDownImg);
    arrowDownBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    arrowDownBtn.SetPosition(-25,-27);
    arrowDownBtn.SetTrigger(&trigA);
    arrowDownBtn.SetTrigger(&trigD);
    arrowDownBtn.SetEffectOnOver(EFFECT_SCALE, 50, 130);
    arrowDownBtn.SetSoundClick(btnClick2);

    GuiImageData *iconData =NULL;
    GuiImage *iconImg =NULL;
    snprintf(imgPath, sizeof(imgPath), "%s", iconPath);

    bool iconExist = checkfile(imgPath);
    if (iconExist) {
        iconData = new GuiImageData (iconPath, dialogue_box_png);
        iconImg = new GuiImage (iconData);
        iconImg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        iconImg->SetPosition(45,10);
    }

    GuiImage dialogBoxImg(&dialogBox);
    dialogBoxImg.SetSkew(0,-80,0,-80,0,50,0,50);

    GuiImage whiteBoxImg(&whiteBox);
    whiteBoxImg.SetPosition(0,110);
    whiteBoxImg.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    whiteBoxImg.SetSkew(0,0,0,0,0,-120,0,-120);
    /*if (Settings.wsprompt == yes){
    dialogBoxImg.SetWidescreen(CFG.widescreen);
    }*/

    char tmp[510];

    GuiText nameTxt(name,30 , THEME.prompttext);
    nameTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    nameTxt.SetPosition(0,-15);
    nameTxt.SetMaxWidth(430, GuiText::SCROLL);


    if (strcmp(coder,""))
        snprintf(tmp, sizeof(tmp), tr("Coded by: %s"),coder);
    GuiText coderTxt(tmp, 16, THEME.prompttext );
    coderTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    coderTxt.SetPosition(180,30);
    coderTxt.SetMaxWidth(280);

    if (strcmp(version,""))
        snprintf(tmp, sizeof(tmp), tr("Version: %s"),version);
    GuiText versionTxt(tmp,16 , THEME.prompttext);
    versionTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    versionTxt.SetPosition(40,65);
    versionTxt.SetMaxWidth(430);

    //if (release_date)
    //snprintf(tmp, sizeof(tmp), tr("Released: %s"),release_date);
    GuiText release_dateTxt(release_date,16 , THEME.prompttext);
    release_dateTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    release_dateTxt.SetPosition(40,85);
    release_dateTxt.SetMaxWidth(430);

    int pagesize = 6;
    GuiText long_descriptionTxt(long_description, 20, THEME.prompttext);
    long_descriptionTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    long_descriptionTxt.SetPosition(46,117);
    long_descriptionTxt.SetMaxWidth(360);
    long_descriptionTxt.SetNumLines(pagesize);

    //convert filesize from u64 to char and put unit of measurement after it
    char temp2[7];
    char filesizeCH[15];
    f32 sizeAdjusted;
    if (filesize<=1024.0) {
        sizeAdjusted = filesize;
        snprintf(temp2, sizeof(temp2), "%.2f",sizeAdjusted);
        snprintf(filesizeCH, sizeof(filesizeCH), "%s B",temp2);

    }
    if (filesize>1024.0) {
        sizeAdjusted = filesize/1024.0;
        snprintf(temp2, sizeof(temp2), "%.2f",sizeAdjusted);
        snprintf(filesizeCH, sizeof(filesizeCH), "%s KB",temp2);

    }
    if (filesize>1048576.0) {
        sizeAdjusted = filesize/1048576.0;
        snprintf(temp2, sizeof(temp2), "%.2f",sizeAdjusted);
        snprintf(filesizeCH, sizeof(filesizeCH), "%s MB",temp2);

    }
    GuiText filesizeTxt(filesizeCH, 16, THEME.prompttext);
    filesizeTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    filesizeTxt.SetPosition(-40,12);

    GuiText btn1Txt(tr("Load"), 22, THEME.prompttext);
    GuiImage btn1Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn1Txt.SetWidescreen(CFG.widescreen);
        btn1Img.SetWidescreen(CFG.widescreen);
    }

    GuiButton btn1(&btn1Img, &btn1Img, 0,3,0,0,&trigA,&btnSoundOver,btnClick2,1);
    btn1.SetLabel(&btn1Txt);
    btn1.SetState(STATE_SELECTED);

    GuiText btn2Txt(tr("Back"), 22, THEME.prompttext);
    GuiImage btn2Img(&btnOutline);
    if (Settings.wsprompt == yes) {
        btn2Txt.SetWidescreen(CFG.widescreen);
        btn2Img.SetWidescreen(CFG.widescreen);
    }
    GuiButton btn2(&btn2Img, &btn2Img, 0,3,0,0,&trigA,&btnSoundOver,btnClick2,1);
    btn2.SetLabel(&btn2Txt);
    btn2.SetTrigger(&trigB);


    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(40, 2);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-40, 2);

    promptWindow.Append(&dialogBoxImg);
    if (strcmp(long_description,""))promptWindow.Append(&whiteBoxImg);
    if (strcmp(long_description,""))promptWindow.Append(&scrollbarImg);
    if (strcmp(long_description,""))promptWindow.Append(&arrowDownBtn);
    if (strcmp(long_description,""))promptWindow.Append(&arrowUpBtn);

    if (strcmp(name,""))promptWindow.Append(&nameTxt);
    if (strcmp(version,""))promptWindow.Append(&versionTxt);
    if (strcmp(coder,""))promptWindow.Append(&coderTxt);
    if (strcmp(release_date,""))promptWindow.Append(&release_dateTxt);
    if (strcmp(long_description,""))promptWindow.Append(&long_descriptionTxt);
    promptWindow.Append(&filesizeTxt);
    if (iconExist)promptWindow.Append(iconImg);
    promptWindow.Append(&btn1);
    promptWindow.Append(&btn2);


    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while (choice == -1) {
        VIDEO_WaitVSync();
        if (shutdown == 1) {
            wiilight(0);
            Sys_Shutdown();
        }
        if (reset == 1)
            Sys_Reboot();
        if (btn1.GetState() == STATE_CLICKED) {
            choice = 1;
        } else if (btn2.GetState() == STATE_CLICKED) {
            choice = 0;
        } else if ((arrowUpBtn.GetState()==STATE_CLICKED||arrowUpBtn.GetState()==STATE_HELD) ) {
            if (long_descriptionTxt.GetFirstLine()>1)
                long_descriptionTxt.SetFirstLine(long_descriptionTxt.GetFirstLine()-1);
            usleep(60000);
            if (!((ButtonsHold() & WPAD_BUTTON_UP)||(ButtonsHold() & PAD_BUTTON_UP)))
                arrowUpBtn.ResetState();
        } else if ((arrowDownBtn.GetState()==STATE_CLICKED||arrowDownBtn.GetState()==STATE_HELD)
                   &&long_descriptionTxt.GetTotalLines()>pagesize
                   &&long_descriptionTxt.GetFirstLine()-1<long_descriptionTxt.GetTotalLines()-pagesize) {
            int l=0;
            l=long_descriptionTxt.GetFirstLine()+1;

            long_descriptionTxt.SetFirstLine(l);
            usleep(60000);
            if (!((ButtonsHold() & WPAD_BUTTON_DOWN)||(ButtonsHold() & PAD_BUTTON_DOWN)))
                arrowDownBtn.ResetState();
        }

    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return choice;
}

