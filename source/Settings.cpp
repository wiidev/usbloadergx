#include <string.h>

#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "wbfs.h"
#include "language.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "fatmounter.h"
#include "PromptWindows.h"
#include "getentries.h"

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
extern int vol;

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
int MenuSettings()
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

    GuiText backBtnTxt(LANGUAGE.Back , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
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
	if (Settings.godmode == 1)
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

    GuiButton btnLogo(logoImg.GetWidth(), logoImg.GetHeight());
	btnLogo.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	btnLogo.SetPosition(0, -35);
	btnLogo.SetImage(&logoImg);
	btnLogo.SetImageOver(&logoImgOver);
	btnLogo.SetEffectGrow();
	btnLogo.SetSoundOver(&btnSoundOver);
	btnLogo.SetSoundClick(&btnClick);
	btnLogo.SetTrigger(&trigA);

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
			w.Append(&btnLogo);
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
                if (Settings.parentalcontrol > 3 )
					Settings.parentalcontrol = 0;


				if ( Settings.godmode != 1) options2.SetValue(0, "********");
				else if (!strcmp("", Settings.unlockCode)) options2.SetValue(0, "%s",LANGUAGE.notset);
				else options2.SetValue(0, Settings.unlockCode);

                if (Settings.godmode != 1) options2.SetValue(1, "********");
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

                if (Settings.godmode != 1) options2.SetValue(5, "********");
				else if(Settings.parentalcontrol == 0) options2.SetValue(5, "0");
				else if(Settings.parentalcontrol == 1) options2.SetValue(5, "1");
				else if(Settings.parentalcontrol == 2) options2.SetValue(5, "2");
				else if(Settings.parentalcontrol == 3) options2.SetValue(5, "3");

				options2.SetValue(6, "%s", Settings.covers_path);
				options2.SetValue(7, "%s", Settings.disc_path);
				options2.SetValue(8, "%s", CFG.theme_path);

				ret = optionBrowser2.GetClickedOption();

				switch (ret)
				{

					case 0: // Modify Password
						if ( Settings.godmode == 1)
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
                        if ( Settings.godmode == 1)
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
                        if ( Settings.godmode == 1)
                        Settings.parentalcontrol++;
                        break;
                    case 6:
                        if ( Settings.godmode == 1)
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
							strncpy(entered, Settings.covers_path, sizeof(entered));
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
								strncpy(Settings.covers_path, entered, sizeof(Settings.covers_path));
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
                        if ( Settings.godmode == 1)
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
							strncpy(entered, Settings.disc_path, sizeof(entered));
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
								strncpy(Settings.disc_path, entered, sizeof(Settings.disc_path));
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
                        if ( Settings.godmode == 1)
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
							w.Append(&btnLogo);

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

				options2.SetValue(0, "%s", Settings.titlestxt_path);

				options2.SetValue(1, "%s", Settings.language_path);

				if (Settings.keyset == us) options2.SetValue(2,"QWERTY");
				else if (Settings.keyset == dvorak) options2.SetValue(2,"DVORAK");
				else if (Settings.keyset == euro) options2.SetValue(2,"QWERTZ");
				else if (Settings.keyset == azerty) options2.SetValue(2,"AZERTY");

				if (Settings.unicodefix == 0) options2.SetValue(3,"%s",LANGUAGE.OFF);
				else if (Settings.unicodefix == 1) options2.SetValue(3,"%s",LANGUAGE.TChinese);
				else if (Settings.unicodefix == 2) options2.SetValue(3,"%s",LANGUAGE.SChinese);

				if(!strcmp("notset", Settings.ogg_path) || !strcmp("",Settings.oggload_path))
					options2.SetValue(4, "%s", LANGUAGE.Standard);
				else
					options2.SetValue(4, "%s", Settings.ogg_path);

				if (Settings.wiilight == 0) options2.SetValue(5,"%s",LANGUAGE.OFF);
				else if (Settings.wiilight == 1) options2.SetValue(5,"%s",LANGUAGE.ON);
				else if (Settings.wiilight == 2) options2.SetValue(5,"%s",LANGUAGE.OnlyInstall);

				options2.SetValue(6, "%s", Settings.update_path);

				if (Settings.patchcountrystrings == 0) options2.SetValue(7,"%s",LANGUAGE.OFF);
				else if (Settings.patchcountrystrings == 1) options2.SetValue(7,"%s",LANGUAGE.ON);

				options2.SetValue(8, " ");

				ret = optionBrowser2.GetClickedOption();

				switch(ret)
				{
					case 0:
						if ( Settings.godmode == 1)
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
							strncpy(entered, Settings.titlestxt_path, sizeof(entered));
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
								strncpy(Settings.titlestxt_path, entered, sizeof(Settings.titlestxt_path));
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
						if ( Settings.godmode == 1)
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
							strncpy(entered, Settings.language_path, sizeof(entered));
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
							{	strncpy(Settings.language_path, entered, sizeof(Settings.language_path));
								if(isSdInserted()) {
									cfg_save_global();
									if(!checkfile(Settings.language_path)) {
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
						if ( Settings.godmode == 1)
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
							strncpy(entered, Settings.update_path, sizeof(entered));
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
								strncpy(Settings.update_path, entered, sizeof(Settings.update_path));
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
			    if(isSdInserted() && Settings.godmode) {
                mainWindow->Remove(&optionBrowser2);
                mainWindow->Remove(&page1Btn);
                mainWindow->Remove(&page2Btn);
                mainWindow->Remove(&tabBtn);
                mainWindow->Remove(&page3Btn);
                w.Remove(&btnLogo);
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
                w.Append(&btnLogo);
			    } else {
                    WindowPrompt(LANGUAGE.NoSDcardinserted, LANGUAGE.InsertaSDCardtousethatoption, LANGUAGE.ok, 0,0,0);
			    }
                updateBtn.ResetState();
			}

            if(btnLogo.GetState() == STATE_CLICKED) {
                mainWindow->Remove(&optionBrowser2);
                mainWindow->Remove(&page1Btn);
                mainWindow->Remove(&page2Btn);
                mainWindow->Remove(&tabBtn);
                mainWindow->Remove(&page3Btn);
                w.Remove(&btnLogo);
                w.Remove(&backBtn);
                w.Remove(&lockBtn);
                w.Remove(&updateBtn);
                WindowCredits();
                mainWindow->Append(&optionBrowser2);
                mainWindow->Append(&page1Btn);
                mainWindow->Append(&page2Btn);
                mainWindow->Append(&tabBtn);
                mainWindow->Append(&page3Btn);
                w.Append(&backBtn);
                w.Append(&lockBtn);
                w.Append(&updateBtn);
                w.Append(&btnLogo);
                btnLogo.ResetState();
			}

			if(lockBtn.GetState() == STATE_CLICKED)
			{
				if (!strcmp("", Settings.unlockCode))
				{
					Settings.godmode = !Settings.godmode;
				}
				else if ( Settings.godmode == 0 )
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
                            if (Settings.godmode == 0) {
								WindowPrompt(LANGUAGE.CorrectPassword,LANGUAGE.InstallRenameandDeleteareunlocked,LANGUAGE.ok,0,0,0);
								Settings.godmode = 1;
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
						Settings.godmode = 0;
						__Menu_GetEntries();
						menu = MENU_DISCLIST;
					}
				}
				if ( Settings.godmode == 1)
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
				if(!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path))
				{
					bgMusic->Play();
				}
				else
				{
					bgMusic->PlayOggFile(Settings.ogg_path);
				}
				bgMusic->SetPlayTime(thetimeofbg);
				SetVolumeOgg(255*(vol/100.0));

				if(choice == 3)
				{
					Sys_LoadMenu(); // Back to System Menu
				}
				else if (choice == 2)
				{
					Sys_BackToLoader();
				}
				else
				{
					homo.ResetState();
				}
			}
			if(Settings.godmode) {
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

        options3.SetValue(6, " ");

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
