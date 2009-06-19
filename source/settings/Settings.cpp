#include <string.h>
#include <unistd.h>

#include "usbloader/wbfs.h"
#include "usbloader/getentries.h"
#include "language/language.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "settings/SettingsPrompts.h"
#include "fatmounter.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"

#define MAXOPTIONS 12

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern GuiImage * bgImg;
extern GuiImageData * pointer[4];
extern GuiImageData * background;
extern u8 shutdown;
extern u8 reset;

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
int MenuSettings()
{
	int menu = MENU_NONE;
	int ret;
	int choice = 0;
	bool exit = false;

	// backup game language setting
	int opt_lang = 0;
	opt_lang = Settings.language;

	enum {
    FADE,
	LEFT,
	RIGHT
    };

	int slidedirection = FADE;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);
    GuiSound btnClick1(button_click_pcm, button_click_pcm_size, SOUND_PCM, Settings.sfxvolume);

	char imgPath[100];

	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

	snprintf(imgPath, sizeof(imgPath), "%ssettings_title.png", CFG.theme_path);
	GuiImageData MainButtonImgData(imgPath, settings_title_png);

	snprintf(imgPath, sizeof(imgPath), "%ssettings_title_over.png", CFG.theme_path);
	GuiImageData MainButtonImgOverData(imgPath, settings_title_over_png);

	snprintf(imgPath, sizeof(imgPath), "%spageindicator.png", CFG.theme_path);
	GuiImageData PageindicatorImgData(imgPath, pageindicator_png);

    snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	GuiImageData arrow_left(imgPath, startgame_arrow_left_png);

	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	GuiImageData arrow_right(imgPath, startgame_arrow_right_png);

    snprintf(imgPath, sizeof(imgPath), "%scredits_button.png", CFG.theme_path);
	GuiImageData creditsImgData(imgPath, credits_button_png);

    snprintf(imgPath, sizeof(imgPath), "%scredits_button_over.png", CFG.theme_path);
	GuiImageData creditsOver(imgPath, credits_button_over_png);

	GuiImage creditsImg(&creditsImgData);
	GuiImage creditsImgOver(&creditsOver);

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

    GuiText backBtnTxt(LANGUAGE.Back , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage backBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	backBtnTxt.SetWidescreen(CFG.widescreen);
	backBtnImg.SetWidescreen(CFG.widescreen);
	}
	GuiButton backBtn(&backBtnImg,&backBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, &btnClick,1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);

	GuiButton homo(1,1);
	homo.SetTrigger(&trigHome);

	GuiImage PageindicatorImg1(&PageindicatorImgData);
	GuiText PageindicatorTxt1("1", 22, (GXColor){0, 0, 0, 255});
	GuiButton PageIndicatorBtn1(PageindicatorImg1.GetWidth(), PageindicatorImg1.GetHeight());
	PageIndicatorBtn1.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	PageIndicatorBtn1.SetPosition(200, 400);
	PageIndicatorBtn1.SetImage(&PageindicatorImg1);
	PageIndicatorBtn1.SetLabel(&PageindicatorTxt1);
	PageIndicatorBtn1.SetSoundOver(&btnSoundOver);
	PageIndicatorBtn1.SetSoundClick(&btnClick1);
	PageIndicatorBtn1.SetTrigger(&trigA);
	PageIndicatorBtn1.SetEffectGrow();

	GuiImage PageindicatorImg2(&PageindicatorImgData);
	GuiText PageindicatorTxt2("2", 22, (GXColor){0, 0, 0, 255});
	GuiButton PageIndicatorBtn2(PageindicatorImg2.GetWidth(), PageindicatorImg2.GetHeight());
	PageIndicatorBtn2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	PageIndicatorBtn2.SetPosition(235, 400);
	PageIndicatorBtn2.SetImage(&PageindicatorImg2);
	PageIndicatorBtn2.SetLabel(&PageindicatorTxt2);
	PageIndicatorBtn2.SetSoundOver(&btnSoundOver);
	PageIndicatorBtn2.SetSoundClick(&btnClick1);
	PageIndicatorBtn2.SetTrigger(&trigA);
	PageIndicatorBtn2.SetEffectGrow();

	GuiImage GoLeftImg(&arrow_left);
	GuiButton GoLeftBtn(GoLeftImg.GetWidth(), GoLeftImg.GetHeight());
	GoLeftBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	GoLeftBtn.SetPosition(25, -25);
	GoLeftBtn.SetImage(&GoLeftImg);
	GoLeftBtn.SetSoundOver(&btnSoundOver);
	GoLeftBtn.SetSoundClick(&btnClick);
	GoLeftBtn.SetEffectGrow();
	GoLeftBtn.SetTrigger(&trigA);
	GoLeftBtn.SetTrigger(&trigL);
	GoLeftBtn.SetTrigger(&trigMinus);

	GuiImage GoRightImg(&arrow_right);
	GuiButton GoRightBtn(GoRightImg.GetWidth(), GoRightImg.GetHeight());
	GoRightBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	GoRightBtn.SetPosition(-25, -25);
	GoRightBtn.SetImage(&GoRightImg);
	GoRightBtn.SetSoundOver(&btnSoundOver);
	GoRightBtn.SetSoundClick(&btnClick);
	GoRightBtn.SetEffectGrow();
	GoRightBtn.SetTrigger(&trigA);
	GoRightBtn.SetTrigger(&trigR);
	GoRightBtn.SetTrigger(&trigPlus);

    char MainButtonText[50];
    snprintf(MainButtonText, sizeof(MainButtonText), "%s", " ");

    GuiImage MainButton1Img(&MainButtonImgData);
    GuiImage MainButton1ImgOver(&MainButtonImgOverData);
    GuiText MainButton1Txt(MainButtonText, 22, (GXColor){0, 0, 0, 255});
	MainButton1Txt.SetMaxWidth(MainButton1Img.GetWidth());
    GuiButton MainButton1(MainButton1Img.GetWidth(), MainButton1Img.GetHeight());
    MainButton1.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton1.SetPosition(0, 90);
	MainButton1.SetImage(&MainButton1Img);
	MainButton1.SetImageOver(&MainButton1ImgOver);
	MainButton1.SetLabel(&MainButton1Txt);
	MainButton1.SetSoundOver(&btnSoundOver);
	MainButton1.SetSoundClick(&btnClick1);
	MainButton1.SetEffectGrow();
	MainButton1.SetTrigger(&trigA);

    GuiImage MainButton2Img(&MainButtonImgData);
    GuiImage MainButton2ImgOver(&MainButtonImgOverData);
    GuiText MainButton2Txt(MainButtonText, 22, (GXColor){0, 0, 0, 255});
	MainButton2Txt.SetMaxWidth(MainButton2Img.GetWidth());
    GuiButton MainButton2(MainButton2Img.GetWidth(), MainButton2Img.GetHeight());
    MainButton2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton2.SetPosition(0, 160);
	MainButton2.SetImage(&MainButton2Img);
	MainButton2.SetImageOver(&MainButton2ImgOver);
	MainButton2.SetLabel(&MainButton2Txt);
	MainButton2.SetSoundOver(&btnSoundOver);
	MainButton2.SetSoundClick(&btnClick1);
	MainButton2.SetEffectGrow();
	MainButton2.SetTrigger(&trigA);

    GuiImage MainButton3Img(&MainButtonImgData);
    GuiImage MainButton3ImgOver(&MainButtonImgOverData);
    GuiText MainButton3Txt(MainButtonText, 22, (GXColor){0, 0, 0, 255});
	MainButton3Txt.SetMaxWidth(MainButton3Img.GetWidth());
    GuiButton MainButton3(MainButton3Img.GetWidth(), MainButton3Img.GetHeight());
    MainButton3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton3.SetPosition(0, 230);
	MainButton3.SetImage(&MainButton3Img);
	MainButton3.SetImageOver(&MainButton3ImgOver);
	MainButton3.SetLabel(&MainButton3Txt);
	MainButton3.SetSoundOver(&btnSoundOver);
	MainButton3.SetSoundClick(&btnClick1);
	MainButton3.SetEffectGrow();
	MainButton3.SetTrigger(&trigA);

    GuiImage MainButton4Img(&MainButtonImgData);
    GuiImage MainButton4ImgOver(&MainButtonImgOverData);
    GuiText MainButton4Txt(MainButtonText, 22, (GXColor){0, 0, 0, 255});
	MainButton4Txt.SetMaxWidth(MainButton4Img.GetWidth());
    GuiButton MainButton4(MainButton4Img.GetWidth(), MainButton4Img.GetHeight());
    MainButton4.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton4.SetPosition(0, 300);
	MainButton4.SetImage(&MainButton4Img);
	MainButton4.SetImageOver(&MainButton4ImgOver);
	MainButton4.SetLabel(&MainButton4Txt);
	MainButton4.SetSoundOver(&btnSoundOver);
	MainButton4.SetSoundClick(&btnClick1);
	MainButton4.SetEffectGrow();
	MainButton4.SetTrigger(&trigA);

	customOptionList options2(MAXOPTIONS);
	GuiCustomOptionBrowser optionBrowser2(396, 280, &options2, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, 0, 150);
	optionBrowser2.SetPosition(0, 90);
	optionBrowser2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	GuiWindow w(screenwidth, screenheight);

	int pageToDisplay = 1;
	while ( pageToDisplay > 0) //set pageToDisplay to 0 to quit
	{
	    VIDEO_WaitVSync ();

		menu = MENU_NONE;

		if ( pageToDisplay == 1)
		{
		    /** Standard procedure made in all pages **/
			MainButton1.StopEffect();
			MainButton2.StopEffect();
			MainButton3.StopEffect();
			MainButton4.StopEffect();

            if(slidedirection == RIGHT) {
            MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            while (MainButton1.GetEffect()>0) usleep(50);
            }
            else if(slidedirection == LEFT) {
            MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            while (MainButton1.GetEffect()>0) usleep(50);
            }

			HaltGui();

            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.GUISettings);
            MainButton1Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Gameload);
            MainButton2Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Parentalcontrol);
            MainButton3Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Sound);
            MainButton4Txt.SetText(MainButtonText);

			mainWindow->RemoveAll();
			mainWindow->Append(&w);
			w.RemoveAll();
            w.Append(&settingsbackground);
            w.Append(&PageIndicatorBtn1);
            w.Append(&PageIndicatorBtn2);
			w.Append(&titleTxt);
			w.Append(&backBtn);
			w.Append(&homo);
			w.Append(&GoRightBtn);
			w.Append(&GoLeftBtn);
			w.Append(&MainButton1);
			w.Append(&MainButton2);
			w.Append(&MainButton3);
			w.Append(&MainButton4);

			PageIndicatorBtn1.SetAlpha(255);
			PageIndicatorBtn2.SetAlpha(50);

			/** Creditsbutton change **/
			MainButton4.SetImage(&MainButton4Img);
			MainButton4.SetImageOver(&MainButton4ImgOver);

			/** Disable ability to click through MainButtons */
			optionBrowser2.SetClickable(false);
			/** Default no scrollbar and reset position **/
			optionBrowser2.SetScrollbar(0);
			optionBrowser2.SetOffset(0);

			MainButton1.StopEffect();
			MainButton2.StopEffect();
			MainButton3.StopEffect();
			MainButton4.StopEffect();

			MainButton1.SetEffectGrow();
			MainButton2.SetEffectGrow();
			MainButton3.SetEffectGrow();
			MainButton4.SetEffectGrow();

            if(slidedirection == FADE) {
            MainButton1.SetEffect(EFFECT_FADE, 20);
			MainButton2.SetEffect(EFFECT_FADE, 20);
            MainButton3.SetEffect(EFFECT_FADE, 20);
            MainButton4.SetEffect(EFFECT_FADE, 20);
            }
            else if(slidedirection == LEFT) {
            MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
			MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            }
            else if(slidedirection == RIGHT) {
            MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
			MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            }

			mainWindow->Append(&w);

			ResumeGui();

			while(MainButton1.GetEffect() > 0) usleep(50);

		}
		else if ( pageToDisplay == 2 )
		{
			/** Standard procedure made in all pages **/
			MainButton1.StopEffect();
			MainButton2.StopEffect();
			MainButton3.StopEffect();
			MainButton4.StopEffect();

            if(slidedirection == RIGHT) {
            MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
            while (MainButton1.GetEffect()>0) usleep(50);
            }
            else if(slidedirection == LEFT) {
            MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
            while (MainButton1.GetEffect()>0) usleep(50);
            }

			HaltGui();

            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Custompaths);
            MainButton1Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Update);
            MainButton2Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Defaultsettings);
            MainButton3Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", LANGUAGE.Credits);
            MainButton4Txt.SetText(MainButtonText);

			mainWindow->RemoveAll();
			mainWindow->Append(&w);
			w.RemoveAll();
            w.Append(&settingsbackground);
            w.Append(&PageIndicatorBtn1);
            w.Append(&PageIndicatorBtn2);
			w.Append(&titleTxt);
			w.Append(&backBtn);
			w.Append(&homo);
			w.Append(&GoRightBtn);
			w.Append(&GoLeftBtn);
			w.Append(&MainButton1);
			w.Append(&MainButton2);
			w.Append(&MainButton3);
			w.Append(&MainButton4);

			PageIndicatorBtn1.SetAlpha(50);
			PageIndicatorBtn2.SetAlpha(255);

			/** Creditsbutton change **/
			MainButton4.SetImage(&creditsImg);
			MainButton4.SetImageOver(&creditsImgOver);

			/** Disable ability to click through MainButtons */
			optionBrowser2.SetClickable(false);
			/** Default no scrollbar and reset position **/
			optionBrowser2.SetScrollbar(0);
			optionBrowser2.SetOffset(0);

			MainButton1.StopEffect();
			MainButton2.StopEffect();
			MainButton3.StopEffect();
			MainButton4.StopEffect();

			MainButton1.SetEffectGrow();
			MainButton2.SetEffectGrow();
			MainButton3.SetEffectGrow();
			MainButton4.SetEffectGrow();

            if(slidedirection == FADE) {
            MainButton1.SetEffect(EFFECT_FADE, 20);
			MainButton2.SetEffect(EFFECT_FADE, 20);
            MainButton3.SetEffect(EFFECT_FADE, 20);
            MainButton4.SetEffect(EFFECT_FADE, 20);
            }
            else if(slidedirection == LEFT) {
            MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
			MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            }
            else if(slidedirection == RIGHT) {
            MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
			MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            }

			mainWindow->Append(&w);

			ResumeGui();

			while(MainButton1.GetEffect() > 0) usleep(50);

		}

		while(menu == MENU_NONE)
		{
			VIDEO_WaitVSync ();

			if ( pageToDisplay == 1 )
			{
			    if(MainButton1.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton1.GetEffect() > 0) usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(LANGUAGE.GUISettings);
                    exit = false;
					for(int i = 0; i <= MAXOPTIONS; i++) options2.SetName(i, NULL);
                    options2.SetName(0, "%s",LANGUAGE.AppLanguage);
                    options2.SetName(1, "%s",LANGUAGE.Display);
                    options2.SetName(2, "%s",LANGUAGE.Clock);
                    options2.SetName(3, "%s",LANGUAGE.Tooltips);
                    options2.SetName(4, "%s",LANGUAGE.FlipX);
                    options2.SetName(5, "%s",LANGUAGE.PromptsButtons);
                    options2.SetName(6, "%s",LANGUAGE.keyboard);
                    options2.SetName(7, "%s",LANGUAGE.Wiilight);
                    options2.SetName(8, "%s",LANGUAGE.Rumble);
                    options2.SetName(9, "%s",LANGUAGE.Unicodefix);
                    options2.SetName(10, "%s",LANGUAGE.XMLTitles);
                    options2.SetName(11, "Screensaver");
                    for(int i = 0; i <= MAXOPTIONS; i++) options2.SetValue(i, NULL);
                    optionBrowser2.SetScrollbar(1);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    VIDEO_WaitVSync ();
                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
			        while(optionBrowser2.GetEffect() > 0) usleep(50);

                    int returnhere = 1;
                    char * languagefile;
                    languagefile = strrchr(Settings.language_path, '/')+1;

                    while(!exit)
                    {
                        VIDEO_WaitVSync ();

                        returnhere = 1;

                        if(Settings.sinfo  >= settings_sinfo_max)
                            Settings.sinfo = 0;
                        if(Settings.hddinfo >= settings_clock_max)
                            Settings.hddinfo = 0; //CLOCK
                        if (Settings.tooltips >= settings_tooltips_max)
                            Settings.tooltips = 0;
                        if ( Settings.xflip >= settings_xflip_max)
                            Settings.xflip = 0;
                        if ( Settings.wsprompt > 1 )
                            Settings.wsprompt = 0;
                        if ( Settings.keyset >= settings_keyset_max)
                            Settings.keyset = 0;
                        if ( Settings.wiilight > 2 )
                            Settings.wiilight = 0;
                        if(Settings.rumble >= settings_rumble_max)
                            Settings.rumble = 0; //RUMBLE
                        if(Settings.screensaver >= settings_screensaver_max)
                            Settings.screensaver = 0; //RUMBLE
                        if ( Settings.unicodefix > 3 )
                            Settings.unicodefix = 0;
						if(Settings.titlesOverride >= 2)
							Settings.titlesOverride = 0;
                        if(!strcmp("notset", Settings.language_path))
                            options2.SetValue(0, "%s", LANGUAGE.Default);
                        else
                            options2.SetValue(0, "%s", languagefile);

                        if (Settings.sinfo == GameID) options2.SetValue(1,"%s",LANGUAGE.GameID);
                        else if (Settings.sinfo == GameRegion) options2.SetValue(1,"%s",LANGUAGE.GameRegion);
                        else if (Settings.sinfo == Both) options2.SetValue(1,"%s",LANGUAGE.Both);
                        else if (Settings.sinfo == Neither) options2.SetValue(1,"%s",LANGUAGE.Neither);

                        if (Settings.hddinfo == hr12) options2.SetValue(2,"12 %s",LANGUAGE.hour);
                        else if (Settings.hddinfo == hr24) options2.SetValue(2,"24 %s",LANGUAGE.hour);
                        else if (Settings.hddinfo == Off) options2.SetValue(2,"%s",LANGUAGE.OFF);

                        if (Settings.tooltips == TooltipsOn) options2.SetValue(3,"%s",LANGUAGE.ON);
                        else if (Settings.tooltips == TooltipsOff) options2.SetValue(3,"%s",LANGUAGE.OFF);

                        if (Settings.xflip == no) options2.SetValue(4,"%s/%s",LANGUAGE.Right,LANGUAGE.Next);
                        else if (Settings.xflip == yes) options2.SetValue(4,"%s/%s",LANGUAGE.Left,LANGUAGE.Prev);
                        else if (Settings.xflip == sysmenu) options2.SetValue(4,"%s", LANGUAGE.LikeSysMenu);
                        else if (Settings.xflip == wtf) options2.SetValue(4,"%s/%s",LANGUAGE.Right,LANGUAGE.Prev);
                        else if (Settings.xflip == disk3d) options2.SetValue(4,"DiskFlip");

                        if (Settings.wsprompt == no) options2.SetValue(5,"%s",LANGUAGE.Normal);
                        else if (Settings.wsprompt == yes) options2.SetValue(5,"%s",LANGUAGE.WidescreenFix);

                        if (Settings.keyset == us) options2.SetValue(6,"QWERTY");
                        else if (Settings.keyset == dvorak) options2.SetValue(6,"DVORAK");
                        else if (Settings.keyset == euro) options2.SetValue(6,"QWERTZ");
                        else if (Settings.keyset == azerty) options2.SetValue(6,"AZERTY");

                        if (Settings.wiilight == 0) options2.SetValue(7,"%s",LANGUAGE.OFF);
                        else if (Settings.wiilight == 1) options2.SetValue(7,"%s",LANGUAGE.ON);
                        else if (Settings.wiilight == 2) options2.SetValue(7,"%s",LANGUAGE.OnlyInstall);

                        if (Settings.rumble == RumbleOn) options2.SetValue(8,"%s",LANGUAGE.ON);
                        else if (Settings.rumble == RumbleOff) options2.SetValue(8,"%s",LANGUAGE.OFF);

                        if (Settings.unicodefix == 0) options2.SetValue(9,"%s",LANGUAGE.OFF);
                        else if (Settings.unicodefix == 1) options2.SetValue(9,"%s",LANGUAGE.TChinese);
                        else if (Settings.unicodefix == 2) options2.SetValue(9,"%s",LANGUAGE.SChinese);
                        else if (Settings.unicodefix == 3) options2.SetValue(9,"%s",LANGUAGE.Japanese);

                        if (Settings.titlesOverride == 0) options2.SetValue(10,"%s",LANGUAGE.OFF);
                        else if (Settings.titlesOverride == 1) options2.SetValue(10,"%s",LANGUAGE.ON);

						if (Settings.screensaver == 0) options2.SetValue(11,"%s",LANGUAGE.OFF);
                        else if (Settings.screensaver == 1) options2.SetValue(11,"3 min");
						else if (Settings.screensaver == 2) options2.SetValue(11,"5 min");
						else if (Settings.screensaver == 3) options2.SetValue(11,"10 min");
						else if (Settings.screensaver == 4) options2.SetValue(11,"20 min");
						else if (Settings.screensaver == 5) options2.SetValue(11,"30 min");
						else if (Settings.screensaver == 6) options2.SetValue(11,"1 hour");

						if(backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
							break;
                        }

                        if(shutdown == 1)
                            Sys_Shutdown();
                        else if(reset == 1)
                            Sys_Reboot();

                        else if(menu == MENU_DISCLIST) {
                            w.Remove(&optionBrowser2);
                            w.Remove(&backBtn);
                            WindowCredits();
                            w.Append(&optionBrowser2);
                            w.Append(&backBtn);
                        }
                        else if(homo.GetState() == STATE_CLICKED)
                        {
                            cfg_save_global();
                            optionBrowser2.SetState(STATE_DISABLED);
                            s32 thetimeofbg = bgMusic->GetPlayTime();
                            bgMusic->Stop();
                            choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
                            if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
                            {
                                bgMusic->Play();
                            } else {
                                bgMusic->PlayOggFile(Settings.ogg_path);
                            }
                            bgMusic->SetPlayTime(thetimeofbg);
                            SetVolumeOgg(255*(Settings.volume/100.0));
                            if(choice == 3) {
                                Sys_LoadMenu(); // Back to System Menu
                            } else if (choice == 2) {
                                Sys_BackToLoader();
                            } else {
                                homo.ResetState();
                            }
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        switch (ret)
                        {
                            case 0:
                                //if(isSdInserted()) {
								if(isInserted(bootDevice)) {
                                if ( Settings.godmode == 1)
                                {
                                    w.SetEffect(EFFECT_FADE, -20);
                                    while(w.GetEffect()>0) usleep(50);
                                    mainWindow->Remove(&w);
                                    while(returnhere == 1)
                                    returnhere = MenuLanguageSelect();
                                    if(returnhere == 2) {
                                        menu = MENU_SETTINGS;
                                        pageToDisplay = 0;
                                        exit = true;
                                        mainWindow->Append(&w);
                                        break;
                                    } else {
                                    HaltGui();
                                    mainWindow->Append(&w);
                                    w.SetEffect(EFFECT_FADE, 20);
                                    ResumeGui();
                                    while(w.GetEffect()>0) usleep(50);
                                    }
                                } else {
                                    WindowPrompt(LANGUAGE.Langchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                }
                                } else {
                                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtousethatoption, LANGUAGE.ok, 0,0,0);
                                }
                                break;
                            case 1:
                                Settings.sinfo++;
                                break;
                            case 2:
                                Settings.hddinfo++;
                                break;
                            case 3:
                                Settings.tooltips++;
                                break;
                            case 4:
                                Settings.xflip++;
                                break;
                            case 5:
                                Settings.wsprompt++;
                                break;
                            case 6:
                                Settings.keyset++;
                                break;
                            case 7:
                                Settings.wiilight++;
                                break;
                            case 8:
                                Settings.rumble++;
                                break;
                            case 9:
                                Settings.unicodefix++;
                                break;
                            case 10:
								//HaltGui();  this isn't done on the fly yet.  you have to restart the loader for it to take effect
                                Settings.titlesOverride++;
								//if(isInserted(bootDevice)) {
                                //cfg_save_global();
								//}
								//CFG_Load();
								//__Menu_GetEntries();
								//ResumeGui();
                                break;
							case 11:
                                Settings.screensaver++;
                                break;

                           }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while(optionBrowser2.GetEffect() > 0) usleep(50);
                    titleTxt.SetText(LANGUAGE.settings);
                    slidedirection = FADE;
                    if(returnhere != 2)
                    pageToDisplay = 1;
                    MainButton1.ResetState();
                    break;
                }

                if(MainButton2.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton2.GetEffect() > 0) usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(LANGUAGE.Gameload);
                    exit = false;
					for(int i = 0; i <= MAXOPTIONS; i++) options2.SetName(i, NULL);
                    options2.SetName(0, "%s",LANGUAGE.VideoMode);
                    options2.SetName(1, "%s",LANGUAGE.VIDTVPatch);
                    options2.SetName(2, "%s",LANGUAGE.Language);
                    options2.SetName(3, "%s",LANGUAGE.Patchcountrystrings);
                    options2.SetName(4, "Ocarina");
                    options2.SetName(5,"%s", LANGUAGE.BootStandard);
                    options2.SetName(6, "%s",LANGUAGE.QuickBoot);
                    options2.SetName(7, "%s",LANGUAGE.Error002fix);
                    options2.SetName(8, "%s",LANGUAGE.Anti002fix);
                    for(int i = 0; i <= MAXOPTIONS; i++) options2.SetValue(i, NULL);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    VIDEO_WaitVSync ();
                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
			        while(optionBrowser2.GetEffect() > 0) usleep(50);

                    while(!exit)
                    {
                        VIDEO_WaitVSync ();
                        if(Settings.video >= settings_video_max)
                            Settings.video = 0;
                        if(Settings.vpatch >= settings_off_on_max)
                            Settings.vpatch = 0;
                        if ( Settings.patchcountrystrings > 1)
                            Settings.patchcountrystrings = 0;
                        if(Settings.ocarina >= settings_off_on_max)
                            Settings.ocarina = 0;
                        if ( Settings.qboot > 1 )
                            Settings.qboot = 0;
                        if ( Settings.cios >= settings_cios_max)
                            Settings.cios = 0;
                        if ( Settings.language >= settings_language_max)
                            Settings.language = 0;
                        if(Settings.error002 >= settings_off_on_max)
                            Settings.error002 = 0;
                        if(Settings.anti002fix >= settings_off_on_max)
                            Settings.anti002fix = 0;

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

                        if (Settings.patchcountrystrings == 0) options2.SetValue(3,"%s",LANGUAGE.OFF);
                        else if (Settings.patchcountrystrings == 1) options2.SetValue(3,"%s",LANGUAGE.ON);

                        if (Settings.ocarina == on) options2.SetValue(4,"%s",LANGUAGE.ON);
                        else if (Settings.ocarina == off) options2.SetValue(4,"%s",LANGUAGE.OFF);

                       if (Settings.godmode != 1) options2.SetValue(5, "********");
                        else if (Settings.cios == ios249) options2.SetValue(5,"cIOS 249");
                        else if (Settings.cios == ios222) options2.SetValue(5,"cIOS 222");

                        if (Settings.qboot == no) options2.SetValue(6,"%s",LANGUAGE.No);
                        else if (Settings.qboot == yes) options2.SetValue(6,"%s",LANGUAGE.Yes);

                        if (Settings.error002 == no) options2.SetValue(7,"%s",LANGUAGE.No);
                        else if (Settings.error002 == yes) options2.SetValue(7,"%s",LANGUAGE.Yes);

                        if (Settings.anti002fix == no) options2.SetValue(8,"%s",LANGUAGE.No);
                        else if (Settings.anti002fix == yes) options2.SetValue(8,"%s",LANGUAGE.Yes);

                        if(backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        if(shutdown == 1)
                            Sys_Shutdown();
                        else if(reset == 1)
                            Sys_Reboot();

                        else if(homo.GetState() == STATE_CLICKED)
                        {
                            cfg_save_global();
                            optionBrowser2.SetState(STATE_DISABLED);
                            s32 thetimeofbg = bgMusic->GetPlayTime();
                            bgMusic->Stop();
                            choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
                            if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
                            {
                                bgMusic->Play();
                            } else {
                                bgMusic->PlayOggFile(Settings.ogg_path);
                            }
                            bgMusic->SetPlayTime(thetimeofbg);
                            SetVolumeOgg(255*(Settings.volume/100.0));
                            if(choice == 3) {
                                Sys_LoadMenu(); // Back to System Menu
                            } else if (choice == 2) {
                                Sys_BackToLoader();
                            } else {
                                homo.ResetState();
                            }
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

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
                                Settings.patchcountrystrings++;
                                break;
                            case 4:
                                Settings.ocarina++;
                                break;
                            case 5:
                                if(Settings.godmode)
                                Settings.cios++;
                                break;
                            case 6:
                                Settings.qboot++;
                                break;
                            case 7:
                                Settings.error002++;
                                break;
                            case 8:
                                Settings.anti002fix++;
                                break;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while(optionBrowser2.GetEffect() > 0) usleep(50);
                    titleTxt.SetText(LANGUAGE.settings);
                    slidedirection = FADE;
                    pageToDisplay = 1;
                    MainButton2.ResetState();
                    break;
                }

                if(MainButton3.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton3.GetEffect() > 0) usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(LANGUAGE.Parentalcontrol);
                    exit = false;
					for(int i = 0; i <= MAXOPTIONS; i++) options2.SetName(i, NULL);
                    options2.SetName(0, "%s",LANGUAGE.Console);
                    options2.SetName(1, "%s",LANGUAGE.Password);
                    options2.SetName(2, "%s",LANGUAGE.Controllevel);
                    for(int i = 0; i <= MAXOPTIONS; i++) options2.SetValue(i, NULL);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    VIDEO_WaitVSync ();
                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
			        while(optionBrowser2.GetEffect() > 0) usleep(50);

                    while(!exit)
                    {
                        VIDEO_WaitVSync ();

                        if (Settings.parentalcontrol > 4 )
                            Settings.parentalcontrol = 0;

                        if( Settings.godmode == 1 ) options2.SetValue(0, LANGUAGE.Unlocked);
                        else if( Settings.godmode == 0 ) options2.SetValue(0, LANGUAGE.Locked);

                        if ( Settings.godmode != 1) options2.SetValue(1, "********");
                        else if (!strcmp("", Settings.unlockCode)) options2.SetValue(1, "%s",LANGUAGE.notset);
                        else options2.SetValue(1, Settings.unlockCode);

                        if (Settings.godmode != 1) options2.SetValue(2, "********");
                        else if(Settings.parentalcontrol == 0) options2.SetValue(2, LANGUAGE.OFF);
                        else if(Settings.parentalcontrol == 1) options2.SetValue(2, LANGUAGE.Child);
                        else if(Settings.parentalcontrol == 2) options2.SetValue(2, LANGUAGE.Teen);
                        else if(Settings.parentalcontrol == 3) options2.SetValue(2, LANGUAGE.Mature);
                        else if(Settings.parentalcontrol == 4) options2.SetValue(2, LANGUAGE.Adultsonly);

                        if(backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        if(shutdown == 1)
                            Sys_Shutdown();
                        else if(reset == 1)
                            Sys_Reboot();

                        else if(homo.GetState() == STATE_CLICKED)
                        {
                            cfg_save_global();
                            optionBrowser2.SetState(STATE_DISABLED);
                            s32 thetimeofbg = bgMusic->GetPlayTime();
                            bgMusic->Stop();
                            choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
                            if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
                            {
                                bgMusic->Play();
                            } else {
                                bgMusic->PlayOggFile(Settings.ogg_path);
                            }
                            bgMusic->SetPlayTime(thetimeofbg);
                            SetVolumeOgg(255*(Settings.volume/100.0));
                            if(choice == 3) {
                                Sys_LoadMenu(); // Back to System Menu
                            } else if (choice == 2) {
                                Sys_BackToLoader();
                            } else {
                                homo.ResetState();
                            }
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        switch (ret)
                        {
                            case 0:
                                if (!strcmp("", Settings.unlockCode))
                                {
                                    Settings.godmode = !Settings.godmode;
                                    break;
                                }
                                else if ( Settings.godmode == 0 ) {
                                    //password check to unlock Install,Delete and Format
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[20] = "";
                                    int result = OnScreenKeyboard(entered, 20,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 ) {
                                        if (!strcmp(entered, Settings.unlockCode)) //if password correct
                                        {
                                            if (Settings.godmode == 0) {
                                                WindowPrompt(LANGUAGE.CorrectPassword,LANGUAGE.InstallRenameandDeleteareunlocked,LANGUAGE.ok,0,0,0);
                                                Settings.godmode = 1;
                                                //__Menu_GetEntries();
                                                menu = MENU_DISCLIST;
                                            }
                                        } else {
                                                WindowPrompt(LANGUAGE.WrongPassword,LANGUAGE.USBLoaderisprotected,LANGUAGE.ok,0,0,0);
                                        }
                                    }
                                } else {
                                    int choice = WindowPrompt (LANGUAGE.LockConsole,LANGUAGE.Areyousure,LANGUAGE.Yes,LANGUAGE.No,0,0);
                                    if(choice == 1) {
                                        WindowPrompt(LANGUAGE.ConsoleLocked,LANGUAGE.USBLoaderisprotected,LANGUAGE.ok,0,0,0);
                                        Settings.godmode = 0;
                                        //__Menu_GetEntries();
                                        menu = MENU_DISCLIST;
                                    }
                                }
                                break;
                            case 1:// Modify Password
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[20] = "";
                                    strncpy(entered, Settings.unlockCode, sizeof(entered));
                                    int result = OnScreenKeyboard(entered, 20,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 ) {
                                        strncpy(Settings.unlockCode, entered, sizeof(Settings.unlockCode));
                                        WindowPrompt(LANGUAGE.PasswordChanged,LANGUAGE.Passwordhasbeenchanged,LANGUAGE.ok,0,0,0);
                                    }
                                } else {
                                    WindowPrompt(LANGUAGE.Passwordchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                }
                                break;
                            case 2:
                                if(Settings.godmode)
                                Settings.parentalcontrol++;
                                break;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while(optionBrowser2.GetEffect() > 0) usleep(50);
                    titleTxt.SetText(LANGUAGE.settings);
                    slidedirection = FADE;
                    pageToDisplay = 1;
                    MainButton3.ResetState();
                    break;
                }

                if(MainButton4.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton4.GetEffect() > 0) usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(LANGUAGE.Sound);
                    exit = false;
					for(int i = 0; i <= MAXOPTIONS; i++) options2.SetName(i, NULL);
                    options2.SetName(0, "%s",LANGUAGE.Backgroundmusic);
                    options2.SetName(1, "%s",LANGUAGE.Volume);
                    options2.SetName(2, "%s",LANGUAGE.SFXVolume);
                    for(int i = 0; i <= MAXOPTIONS; i++) options2.SetValue(i, NULL);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    VIDEO_WaitVSync ();
                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
			        while(optionBrowser2.GetEffect() > 0) usleep(50);


                    char * oggfile;

                    while(!exit)
                    {
                        VIDEO_WaitVSync ();

                        bool returnhere = true;

                        if(!strcmp("notset", Settings.ogg_path))
                            options2.SetValue(0, "%s", LANGUAGE.Standard);
                        else {
                            oggfile = strrchr(Settings.ogg_path, '/')+1;
                            options2.SetValue(0, "%s", oggfile);
                        }

                        if(Settings.volume > 0)
                        options2.SetValue(1,"%i", Settings.volume);
                        else
                        options2.SetValue(1,"%s", LANGUAGE.OFF);
                        if(Settings.sfxvolume > 0)
                        options2.SetValue(2,"%i", Settings.sfxvolume);
                        else
                        options2.SetValue(2,"%s", LANGUAGE.OFF);

                        if(backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        if(shutdown == 1)
                            Sys_Shutdown();
                        else if(reset == 1)
                            Sys_Reboot();

                        else if(homo.GetState() == STATE_CLICKED)
                        {
                            cfg_save_global();
                            optionBrowser2.SetState(STATE_DISABLED);
                            s32 thetimeofbg = bgMusic->GetPlayTime();
                            bgMusic->Stop();
                            choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
                            if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
                            {
                                bgMusic->Play();
                            } else {
                                bgMusic->PlayOggFile(Settings.ogg_path);
                            }
                            bgMusic->SetPlayTime(thetimeofbg);
                            SetVolumeOgg(255*(Settings.volume/100.0));
                            if(choice == 3) {
                                Sys_LoadMenu(); // Back to System Menu
                            } else if (choice == 2) {
                                Sys_BackToLoader();
                            } else {
                                homo.ResetState();
                            }
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        switch (ret)
                        {
                            case 0:
                                //if(isSdInserted())
								if(isInserted(bootDevice))
                                {
                                    w.SetEffect(EFFECT_FADE, -20);
                                    while(w.GetEffect()>0) usleep(50);
                                    mainWindow->Remove(&w);
                                    while(returnhere)
                                        returnhere = MenuOGG();
                                    HaltGui();
                                    mainWindow->Append(&w);
                                    w.SetEffect(EFFECT_FADE, 20);
                                    ResumeGui();
                                    while(w.GetEffect()>0) usleep(50);
                                }
                                else
                                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtousethatoption, LANGUAGE.ok, 0,0,0);
                                break;
                            case 1:
                                Settings.volume += 10;
                                if(Settings.volume > 100)
                                    Settings.volume = 0;
                                SetVolumeOgg(255*(Settings.volume/100.0));
                                break;
                            case 2:
                                Settings.sfxvolume += 10;
                                if(Settings.sfxvolume > 100)
                                    Settings.sfxvolume = 0;
                                btnSoundOver.SetVolume(Settings.sfxvolume);
                                btnClick.SetVolume(Settings.sfxvolume);
                                btnClick1.SetVolume(Settings.sfxvolume);
                                break;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while(optionBrowser2.GetEffect() > 0) usleep(50);
                    titleTxt.SetText(LANGUAGE.settings);
                    slidedirection = FADE;
                    pageToDisplay = 1;
                    MainButton4.ResetState();
                    break;
                }
			}

			if ( pageToDisplay == 2)
			{
                if(MainButton1.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton1.GetEffect() > 0) usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(LANGUAGE.Custompaths);
                    exit = false;
					for(int i = 0; i <= MAXOPTIONS; i++) options2.SetName(i, NULL);
                    if(Settings.godmode)
                    options2.SetName(0, "%s", LANGUAGE.CoverPath);
                    options2.SetName(1, "%s", LANGUAGE.DiscimagePath);
                    options2.SetName(2, "%s", LANGUAGE.ThemePath);
                    options2.SetName(3, "%s", LANGUAGE.Titlestxtpath);
                    options2.SetName(4, "%s", LANGUAGE.Updatepath);
                    options2.SetName(5, "%s", LANGUAGE.Cheatcodespath);
                    options2.SetName(6, "%s", LANGUAGE.DolPath);
                    for(int i = 0; i <= MAXOPTIONS; i++) options2.SetValue(i, NULL);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    VIDEO_WaitVSync ();
                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
			        while(optionBrowser2.GetEffect() > 0) usleep(50);

			        if(Settings.godmode) {

                    while(!exit)
                    {
                        VIDEO_WaitVSync ();

                        options2.SetValue(0, "%s", Settings.covers_path);
                        options2.SetValue(1, "%s", Settings.disc_path);
                        options2.SetValue(2, "%s", CFG.theme_path);
                        options2.SetValue(3, "%s", Settings.titlestxt_path);
                        options2.SetValue(4, "%s", Settings.update_path);
                        options2.SetValue(5, "%s", Settings.Cheatcodespath);
						options2.SetValue(6, "%s", Settings.dolpath);

                        if(backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        if(shutdown == 1)
                            Sys_Shutdown();
                        else if(reset == 1)
                            Sys_Reboot();

                        else if(homo.GetState() == STATE_CLICKED)
                        {
                            cfg_save_global();
                            optionBrowser2.SetState(STATE_DISABLED);
                            s32 thetimeofbg = bgMusic->GetPlayTime();
                            bgMusic->Stop();
                            choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
                            if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
                            {
                                bgMusic->Play();
                            } else {
                                bgMusic->PlayOggFile(Settings.ogg_path);
                            }
                            bgMusic->SetPlayTime(thetimeofbg);
                            SetVolumeOgg(255*(Settings.volume/100.0));
                            if(choice == 3) {
                                Sys_LoadMenu(); // Back to System Menu
                            } else if (choice == 2) {
                                Sys_BackToLoader();
                            } else {
                                homo.ResetState();
                            }
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        switch (ret)
                        {
                            case 0:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, Settings.covers_path, sizeof(entered));
                                    int result = OnScreenKeyboard(entered,43,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(Settings.covers_path, entered, sizeof(Settings.covers_path));
                                        WindowPrompt(LANGUAGE.CoverpathChanged,0,LANGUAGE.ok,0,0,0);
//                                        if(!isSdInserted()) {
										if(!isInserted(bootDevice)) {
                                          WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                                        }
                                    }
                                } else {
                                    WindowPrompt(LANGUAGE.Coverpathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                }
                                break;
                            case 1:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, Settings.disc_path, sizeof(entered));
                                    int result = OnScreenKeyboard(entered, 43,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(Settings.disc_path, entered, sizeof(Settings.disc_path));
                                        WindowPrompt(LANGUAGE.DiscpathChanged,0,LANGUAGE.ok,0,0,0);
//                                        if(!isSdInserted()) {
										if(!isInserted(bootDevice)) {
                                            WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                                        }
                                    }
                                } else {
                                    WindowPrompt(LANGUAGE.Discpathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                }
                                break;
                            case 2:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, CFG.theme_path, sizeof(entered));
                                    int result = OnScreenKeyboard(entered, 43,0);
                                    HaltGui();
                                    w.RemoveAll();
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(CFG.theme_path, entered, sizeof(CFG.theme_path));
                                        WindowPrompt(LANGUAGE.ThemepathChanged,0,LANGUAGE.ok,0,0,0);
//                                        if(!isSdInserted()) {
										if(!isInserted(bootDevice)) {
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
                                    }
                                    w.Append(&settingsbackground);
                                    w.Append(&titleTxt);
                                    w.Append(&backBtn);
                                    w.Append(&optionBrowser2);
                                    ResumeGui();
                                } else {
                                    WindowPrompt(LANGUAGE.Themepathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                }
                                break;
                            case 3:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, Settings.titlestxt_path, sizeof(entered));
                                    int result = OnScreenKeyboard(entered,43,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(Settings.titlestxt_path, entered, sizeof(Settings.titlestxt_path));
                                        WindowPrompt(LANGUAGE.TitlestxtpathChanged,0,LANGUAGE.ok,0,0,0);
//                                        if(isSdInserted()) {
										if(isInserted(bootDevice)) {
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
                            case 4:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, Settings.update_path, sizeof(entered));
                                    int result = OnScreenKeyboard(entered,43,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(Settings.update_path, entered, sizeof(Settings.update_path));
                                        WindowPrompt(LANGUAGE.Updatepathchanged,0,LANGUAGE.ok,0,0,0);
                                    }
                                }
                                else
                                    WindowPrompt(0,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                break;
                            case 5:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, Settings.Cheatcodespath, sizeof(entered));
                                    int result = OnScreenKeyboard(entered,43,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(Settings.Cheatcodespath, entered, sizeof(Settings.Cheatcodespath));
                                        WindowPrompt(LANGUAGE.Cheatcodespathchanged,0,LANGUAGE.ok,0,0,0);
                                    }
                                }
                                else
                                    WindowPrompt(0,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                break;
							case 6:
                                if ( Settings.godmode == 1)
                                {
                                    w.Remove(&optionBrowser2);
                                    w.Remove(&backBtn);
                                    char entered[43] = "";
                                    strncpy(entered, Settings.dolpath, sizeof(entered));
                                    int result = OnScreenKeyboard(entered,43,0);
                                    w.Append(&optionBrowser2);
                                    w.Append(&backBtn);
                                    if ( result == 1 )
                                    {
                                        int len = (strlen(entered)-1);
                                        if(entered[len] !='/')
                                        strncat (entered, "/", 1);
                                        strncpy(Settings.dolpath, entered, sizeof(Settings.dolpath));
                                        WindowPrompt(LANGUAGE.DolpathChanged,0,LANGUAGE.ok,0,0,0);
//                                        if(!isSdInserted()) {
										if(!isInserted(bootDevice)) {
                                          WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtosave, LANGUAGE.ok, 0,0,0);
                                        }
                                    }
                                } else {
                                    WindowPrompt(LANGUAGE.Dolpathchange,LANGUAGE.Consoleshouldbeunlockedtomodifyit,LANGUAGE.ok,0,0,0);
                                }
                                break;

                        }
                    }
                    /** If not godmode don't let him inside **/
                    } else {
                        WindowPrompt(LANGUAGE.ConsoleLocked, LANGUAGE.UnlockConsoletousethisOption, LANGUAGE.ok, 0, 0, 0);
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while(optionBrowser2.GetEffect() > 0) usleep(50);
                    titleTxt.SetText(LANGUAGE.settings);
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton1.ResetState();
                    break;
                }

                if(MainButton2.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton2.GetEffect() > 0) usleep(50);
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
//                    if(isSdInserted() && Settings.godmode) {
					if(isInserted(bootDevice) && Settings.godmode) {
                    w.Remove(&optionBrowser2);
                    w.Remove(&backBtn);
                    int ret = ProgressUpdateWindow();
                    if(ret < 0) {
                        WindowPrompt(LANGUAGE.Updatefailed,0,LANGUAGE.ok,0,0,0);
                    }
                    w.Append(&optionBrowser2);
                    w.Append(&backBtn);
                    } else {
                        WindowPrompt(LANGUAGE.ConsoleLocked, LANGUAGE.UnlockConsoletousethisOption, LANGUAGE.ok, 0,0,0);
                    }
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton2.ResetState();
                    break;
                }

                if(MainButton3.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton3.GetEffect() > 0) usleep(50);
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    w.Remove(&backBtn);
                    w.Remove(&optionBrowser2);
                    if(Settings.godmode) {
                    int choice = WindowPrompt(LANGUAGE.Areyousure, 0, LANGUAGE.Yes, LANGUAGE.Cancel, 0, 0);
                    if(choice == 1) {
//							if(isSdInserted())
							if(isInserted(bootDevice))
							{
								char GXGlobal_cfg[26];
								sprintf(GXGlobal_cfg, "%s/config/GXGlobal.cfg", bootDevice);
								remove(GXGlobal_cfg);
							}
							lang_default();
							CFG_Load();
							menu = MENU_SETTINGS;
							pageToDisplay = 0;
                    }
                    } else {
                        WindowPrompt(LANGUAGE.ConsoleLocked, LANGUAGE.UnlockConsoletousethisOption, LANGUAGE.ok, 0, 0, 0);
                    }
                    w.Append(&backBtn);
                    w.Append(&optionBrowser2);
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton3.ResetState();
                    break;
                }

                if(MainButton4.GetState() == STATE_CLICKED) {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while(MainButton4.GetEffect() > 0) usleep(50);
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    WindowCredits();
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton4.ResetState();
                    break;
                }
			}

			if(shutdown == 1)
				Sys_Shutdown();
			if(reset == 1)
				Sys_Reboot();

			if(backBtn.GetState() == STATE_CLICKED)
			{
				//Add the procedure call to save the global configuration
//				if(isSdInserted()) {
				if(isInserted(bootDevice)) {
				cfg_save_global();
				}
				menu = MENU_DISCLIST;
				pageToDisplay = 0;
				break;
			}

			if(GoLeftBtn.GetState() == STATE_CLICKED)
			{
                pageToDisplay--;
                /** Change direction of the flying buttons **/
                if(pageToDisplay < 1)
                pageToDisplay = 2;
                slidedirection = LEFT;
                GoLeftBtn.ResetState();
                break;
			}

			if(GoRightBtn.GetState() == STATE_CLICKED)
			{
                pageToDisplay++;
                /** Change direction of the flying buttons **/
                if(pageToDisplay > 2)
                pageToDisplay = 1;
                slidedirection = RIGHT;
                GoRightBtn.ResetState();
                break;
			}

			if(PageIndicatorBtn1.GetState() == STATE_CLICKED)
			{
			    if(pageToDisplay == 2) {
                slidedirection = LEFT;
                pageToDisplay = 1;
                PageIndicatorBtn1.ResetState();
                break;
                }
                PageIndicatorBtn1.ResetState();
			}
			else if(PageIndicatorBtn2.GetState() == STATE_CLICKED)
			{
                if(pageToDisplay == 1) {
                slidedirection = RIGHT;
                pageToDisplay = 2;
                PageIndicatorBtn2.ResetState();
                break;
                } else
                PageIndicatorBtn2.ResetState();
			}

			if(homo.GetState() == STATE_CLICKED)
			{
			    cfg_save_global();
				optionBrowser2.SetState(STATE_DISABLED);
				s32 thetimeofbg = bgMusic->GetPlayTime();
				bgMusic->Stop();
				choice = WindowExitPrompt(LANGUAGE.ExitUSBISOLoader,0, LANGUAGE.BacktoLoader,LANGUAGE.WiiMenu,LANGUAGE.Back,0);
				if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
				{
					bgMusic->Play();
				} else {
					bgMusic->PlayOggFile(Settings.ogg_path);
				}
				bgMusic->SetPlayTime(thetimeofbg);
				SetVolumeOgg(255*(Settings.volume/100.0));

				if(choice == 3)
				{
					Sys_LoadMenu(); // Back to System Menu
				} else if (choice == 2) {
					Sys_BackToLoader();
				} else {
					homo.ResetState();
				}
                optionBrowser2.SetState(STATE_DEFAULT);
			}
		}
	}

	w.SetEffect(EFFECT_FADE, -20);
	while(w.GetEffect()>0) usleep(50);


	// if language has changed, reload titles
	int opt_langnew = 0;
	opt_langnew = Settings.language;
	if (Settings.titlesOverride==1 && opt_lang != opt_langnew) {
		CFG_LoadXml(true, true, false); // open file, reload titles, do not keep in memory
		menu = MENU_DISCLIST;
	}

	HaltGui();

	mainWindow->RemoveAll();
	mainWindow->Append(bgImg);

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

	customOptionList options3(12);
	options3.SetName(0,"%s", LANGUAGE.VideoMode);
	options3.SetName(1,"%s", LANGUAGE.VIDTVPatch);
	options3.SetName(2,"%s", LANGUAGE.Language);
	options3.SetName(3, "Ocarina");
	options3.SetName(4, "IOS");
	options3.SetName(5,"%s", LANGUAGE.Parentalcontrol);
	options3.SetName(6,"%s", LANGUAGE.Error002fix);
	options3.SetName(7,"%s", LANGUAGE.Patchcountrystrings);
	options3.SetName(8,"%s", LANGUAGE.Alternatedol);
	options3.SetName(9,"%s", LANGUAGE.Blockiosreload);
	options3.SetName(10,"%s", LANGUAGE.Defaultgamesettings);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

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

    GuiText saveBtnTxt(LANGUAGE.Save, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	saveBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage saveBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	saveBtnTxt.SetWidescreen(CFG.widescreen);
	saveBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton saveBtn(&saveBtnImg,&saveBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, &btnClick,1);
	saveBtn.SetScale(0.9);
	saveBtn.SetLabel(&saveBtnTxt);

    GuiText cancelBtnTxt(LANGUAGE.Back, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	cancelBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage cancelBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	cancelBtnTxt.SetWidescreen(CFG.widescreen);
	cancelBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton cancelBtn(&cancelBtnImg,&cancelBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, &btnClick,1);
	cancelBtn.SetScale(0.9);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetTrigger(&trigB);

	GuiText deleteBtnTxt(LANGUAGE.Uninstall, 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	deleteBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage deleteBtnImg(&btnOutline);
	if (Settings.wsprompt == yes){
	deleteBtnTxt.SetWidescreen(CFG.widescreen);
	deleteBtnImg.SetWidescreen(CFG.widescreen);}
	GuiButton deleteBtn(&deleteBtnImg,&deleteBtnImg, 2, 3, 0, 400, &trigA, &btnSoundOver, &btnClick,1);
	deleteBtn.SetScale(0.9);
	deleteBtn.SetLabel(&deleteBtnTxt);

	GuiCustomOptionBrowser optionBrowser3(396, 280, &options3, CFG.theme_path, "bg_options_gamesettings.png", bg_options_settings_png, 1, 200);
	optionBrowser3.SetPosition(0, 90);
	optionBrowser3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackgroundbtn);
    w.Append(&titleTxt);
	w.Append(&deleteBtn);
	w.Append(&saveBtn);
	w.Append(&cancelBtn);
    w.Append(&optionBrowser3);

    mainWindow->Append(&w);

	struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);

	if (game_cfg)//if there are saved settings for this game use them
	{
		videoChoice = game_cfg->video;
		languageChoice = game_cfg->language;
		ocarinaChoice = game_cfg->ocarina;
		viChoice = game_cfg->vipatch;
		iosChoice = game_cfg->ios;
		parentalcontrolChoice = game_cfg->parentalcontrol;
		fix002 = game_cfg->errorfix002;
		countrystrings = game_cfg->patchcountrystrings;
		alternatedol = game_cfg->loadalternatedol;
		reloadblock = game_cfg->iosreloadblock;
	}
	else
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
		fix002 = Settings.error002;
		countrystrings = Settings.patchcountrystrings;
		alternatedol = off;
		reloadblock = off;
	}

	int opt_lang = languageChoice; // backup language setting

	ResumeGui();

	while(!exit)
	{
		VIDEO_WaitVSync();

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
		else if (iosChoice == i223) options3.SetValue(4,"223");

		if (parentalcontrolChoice == 0) options3.SetValue(5, LANGUAGE.Everyone);
		else if (parentalcontrolChoice == 1) options3.SetValue(5, LANGUAGE.Child);
		else if (parentalcontrolChoice == 2) options3.SetValue(5, LANGUAGE.Teen);
		else if (parentalcontrolChoice == 3) options3.SetValue(5, LANGUAGE.Mature);
		else if (parentalcontrolChoice == 4) options3.SetValue(5, LANGUAGE.Adultsonly);

        if (fix002 == on) options3.SetValue(6,LANGUAGE.ON);
		else if (fix002 == off) options3.SetValue(6,LANGUAGE.OFF);

        if (countrystrings == on) options3.SetValue(7,LANGUAGE.ON);
		else if (countrystrings == off) options3.SetValue(7,LANGUAGE.OFF);

        if (alternatedol == on) options3.SetValue(8,LANGUAGE.ON);
		else if (alternatedol == off) options3.SetValue(8,LANGUAGE.OFF);

        if (reloadblock == on) options3.SetValue(9,LANGUAGE.ON);
		else if (reloadblock == off) options3.SetValue(9,LANGUAGE.OFF);

        options3.SetValue(10, NULL);

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
				iosChoice = (iosChoice + 1) % 3;
				break;
			case 5:
				parentalcontrolChoice = (parentalcontrolChoice + 1) % 5;
				break;
            case 6:
                fix002 = (fix002+1) % 2;
                break;
            case 7:
                countrystrings = (countrystrings+1) % 2;
                break;
            case 8:
                alternatedol = (alternatedol+1) % 2;
                break;
            case 9:
                reloadblock = (reloadblock+1) % 2;
                break;
            case 10:
                int choice = WindowPrompt(LANGUAGE.Areyousure,0,LANGUAGE.Yes,LANGUAGE.Cancel,0,0);
                if(choice == 1) {
                    videoChoice = Settings.video;
                    viChoice = Settings.vpatch;
                    languageChoice = Settings.language;
                    ocarinaChoice = Settings.ocarina;
                    fix002 = Settings.error002;
                    countrystrings = Settings.patchcountrystrings;
                    alternatedol = off;
                    reloadblock = off;
                    if(Settings.cios == ios222) {
                        iosChoice = i222;
                    } else {
                        iosChoice = i249;
                    }
                    parentalcontrolChoice = 0;
                    CFG_forget_game_opt(header->id);
					// if default language is different than language from main settings, reload titles
					int opt_langnew = 0;
					opt_langnew = Settings.language;
					if (Settings.titlesOverride==1 && opt_lang != opt_langnew)
						CFG_LoadXml(true, true, false); // open file, reload titles, do not keep in memory
						// titles are refreshed in menu.cpp as soon as this function returns
                }
                break;
		}

		if(saveBtn.GetState() == STATE_CLICKED)
		{
//			if(isSdInserted()) {
			if(isInserted(bootDevice)) {
                if (CFG_save_game_opt(header->id))
				{
					// if language has changed, reload titles
					int opt_langnew = 0;
					game_cfg = CFG_get_game_opt(header->id);
					if (game_cfg) opt_langnew = game_cfg->language;
					if (Settings.titlesOverride==1 && opt_lang != opt_langnew)
						CFG_LoadXml(true, true, false); // open file, reload titles, do not keep in memory
						// titles are refreshed in menu.cpp as soon as this function returns
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
				CFG_forget_game_opt(header->id);
				CFG_forget_game_num(header->id);
				ret = WBFS_RemoveGame(header->id);
				if (ret < 0)
				{
					WindowPrompt(
					LANGUAGE.Cantdelete,
					gameName,
					LANGUAGE.ok,0,0,0);
				}
				else {
					//__Menu_GetEntries();
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
	mainWindow->Remove(&w);
	ResumeGui();

	return retVal;
}
