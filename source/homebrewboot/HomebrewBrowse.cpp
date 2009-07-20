/****************************************************************************
 * HomebrewBrowse
 * USB Loader GX 2009
 *
 * Homebrew launcher for USB Loader GX
 *
 * homebrewbrowse.cpp
 ***************************************************************************/
#include <string.h>
#include <unistd.h>

#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "prompts/PromptWindows.h"
#include "homebrewboot/HomebrewFiles.h"
#include "homebrewboot/HomebrewXML.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern GuiImage * bgImg;
extern u8 shutdown;
extern u8 reset;

bool boothomebrew = false;

/****************************************************************************
 * roundup Function
 ***************************************************************************/
int roundup(float number)
{
    if(number == (int) number)
        return (int) number;
    else
        return (int) (number+1);
}

/****************************************************************************
 * MenuHomebrewBrowse
 ***************************************************************************/
int MenuHomebrewBrowse()
{
	int menu = MENU_NONE;
	int choice = 0;

	HomebrewFiles HomebrewFiles(Settings.homebrewapps_path);

	u32 filecount = HomebrewFiles.GetFilecount();

	if(!filecount) {
        WindowPrompt(tr("No .dol or .elf files found."),0, tr("OK"));
        return MENU_DISCLIST;
	}

	enum {
        FADE,
        LEFT,
        RIGHT
    };

	int slidedirection = FADE;

    /*** Sound Variables ***/
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);
    GuiSound btnClick1(button_click_pcm, button_click_pcm_size, SOUND_PCM, Settings.sfxvolume);

    /*** Image Variables ***/
	char imgPath[150];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);

	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData bgData(imgPath, settings_background_png);

	snprintf(imgPath, sizeof(imgPath), "%ssettings_title.png", CFG.theme_path);
	GuiImageData MainButtonImgData(imgPath, settings_title_png);

	snprintf(imgPath, sizeof(imgPath), "%ssettings_title_over.png", CFG.theme_path);
	GuiImageData MainButtonImgOverData(imgPath, settings_title_over_png);

    snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_left.png", CFG.theme_path);
	GuiImageData arrow_left(imgPath, startgame_arrow_left_png);

	snprintf(imgPath, sizeof(imgPath), "%sstartgame_arrow_right.png", CFG.theme_path);
	GuiImageData arrow_right(imgPath, startgame_arrow_right_png);

    GuiImage background(&bgData);

    /*** Trigger Variables ***/
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

    GuiText titleTxt(tr("Homebrew Launcher"), 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,40);

    GuiImageData *IconData[4];
    GuiImage *IconImg[4];

    for(int i = 0; i < 4; i++) {
        IconData[i] = NULL;
        IconImg[i] = NULL;
    }
    /*** Buttons ***/

    GuiText backBtnTxt(tr("Back") , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
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
    GuiText MainButton1Txt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton1Txt.SetMaxWidth(MainButton1Img.GetWidth()-150, GuiText::DOTTED);
	MainButton1Txt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton1Txt.SetPosition(148, -12);
    GuiText MainButton1DescTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton1DescTxt.SetMaxWidth(MainButton1Img.GetWidth()-150, GuiText::DOTTED);
	MainButton1DescTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton1DescTxt.SetPosition(148, 15);
    GuiText MainButton1DescOverTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton1DescOverTxt.SetMaxWidth(MainButton1Img.GetWidth()-150, GuiText::SCROLL);
	MainButton1DescOverTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton1DescOverTxt.SetPosition(148, 15);
    GuiButton MainButton1(MainButton1Img.GetWidth(), MainButton1Img.GetHeight());
    MainButton1.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton1.SetPosition(0, 90);
	MainButton1.SetImage(&MainButton1Img);
	MainButton1.SetImageOver(&MainButton1ImgOver);
	MainButton1.SetLabel(&MainButton1Txt);
	MainButton1.SetLabel(&MainButton1DescTxt,1);
	MainButton1.SetLabelOver(&MainButton1DescOverTxt,1);
	MainButton1.SetSoundOver(&btnSoundOver);
	MainButton1.SetSoundClick(&btnClick1);
	MainButton1.SetEffectGrow();
	MainButton1.SetTrigger(&trigA);

    GuiImage MainButton2Img(&MainButtonImgData);
    GuiImage MainButton2ImgOver(&MainButtonImgOverData);
    GuiText MainButton2Txt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton2Txt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton2Txt.SetPosition(148, -12);
	MainButton2Txt.SetMaxWidth(MainButton2Img.GetWidth()-150, GuiText::DOTTED);
    GuiText MainButton2DescTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton2DescTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton2DescTxt.SetPosition(148, 15);
	MainButton2DescTxt.SetMaxWidth(MainButton2Img.GetWidth()-150, GuiText::DOTTED);
    GuiText MainButton2DescOverTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton2DescOverTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton2DescOverTxt.SetPosition(148, 15);
	MainButton2DescOverTxt.SetMaxWidth(MainButton2Img.GetWidth()-150, GuiText::SCROLL);
    GuiButton MainButton2(MainButton2Img.GetWidth(), MainButton2Img.GetHeight());
    MainButton2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton2.SetPosition(0, 160);
	MainButton2.SetImage(&MainButton2Img);
	MainButton2.SetImageOver(&MainButton2ImgOver);
	MainButton2.SetLabel(&MainButton2Txt);
	MainButton2.SetLabel(&MainButton2DescTxt,1);
	MainButton2.SetLabelOver(&MainButton2DescOverTxt,1);
	MainButton2.SetSoundOver(&btnSoundOver);
	MainButton2.SetSoundClick(&btnClick1);
	MainButton2.SetEffectGrow();
	MainButton2.SetTrigger(&trigA);

    GuiImage MainButton3Img(&MainButtonImgData);
    GuiImage MainButton3ImgOver(&MainButtonImgOverData);
    GuiText MainButton3Txt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton3Txt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton3Txt.SetPosition(148, -12);
	MainButton3Txt.SetMaxWidth(MainButton3Img.GetWidth()-150, GuiText::DOTTED);
    GuiText MainButton3DescTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton3DescTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton3DescTxt.SetPosition(148, 15);
	MainButton3DescTxt.SetMaxWidth(MainButton3Img.GetWidth()-150, GuiText::DOTTED);
    GuiText MainButton3DescOverTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton3DescOverTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton3DescOverTxt.SetPosition(148, 15);
	MainButton3DescOverTxt.SetMaxWidth(MainButton3Img.GetWidth()-150, GuiText::SCROLL);
    GuiButton MainButton3(MainButton3Img.GetWidth(), MainButton3Img.GetHeight());
    MainButton3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton3.SetPosition(0, 230);
	MainButton3.SetImage(&MainButton3Img);
	MainButton3.SetImageOver(&MainButton3ImgOver);
	MainButton3.SetLabel(&MainButton3Txt);
	MainButton3.SetLabel(&MainButton3DescTxt,1);
	MainButton3.SetLabelOver(&MainButton3DescOverTxt,1);
	MainButton3.SetSoundOver(&btnSoundOver);
	MainButton3.SetSoundClick(&btnClick1);
	MainButton3.SetEffectGrow();
	MainButton3.SetTrigger(&trigA);

    GuiImage MainButton4Img(&MainButtonImgData);
    GuiImage MainButton4ImgOver(&MainButtonImgOverData);
    GuiText MainButton4Txt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton4Txt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton4Txt.SetPosition(148, -12);
	MainButton4Txt.SetMaxWidth(MainButton4Img.GetWidth()-150, GuiText::DOTTED);
    GuiText MainButton4DescTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton4DescTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton4DescTxt.SetPosition(148, 15);
	MainButton4DescTxt.SetMaxWidth(MainButton4Img.GetWidth()-150, GuiText::DOTTED);
    GuiText MainButton4DescOverTxt(MainButtonText, 18, (GXColor){0, 0, 0, 255});
	MainButton4DescOverTxt.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	MainButton4DescOverTxt.SetPosition(148, 15);
	MainButton4DescOverTxt.SetMaxWidth(MainButton4Img.GetWidth()-150, GuiText::SCROLL);
    GuiButton MainButton4(MainButton4Img.GetWidth(), MainButton4Img.GetHeight());
    MainButton4.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	MainButton4.SetPosition(0, 300);
	MainButton4.SetImage(&MainButton4Img);
	MainButton4.SetImageOver(&MainButton4ImgOver);
	MainButton4.SetLabel(&MainButton4Txt);
	MainButton4.SetLabel(&MainButton4DescTxt,1);
	MainButton4.SetLabelOver(&MainButton4DescOverTxt,1);
	MainButton4.SetSoundOver(&btnSoundOver);
	MainButton4.SetSoundClick(&btnClick1);
	MainButton4.SetEffectGrow();
	MainButton4.SetTrigger(&trigA);

	GuiWindow w(screenwidth, screenheight);


	/*** XML Variables ***/
    HomebrewXML XMLInfo[4];

	int pageToDisplay = 1;
	const int pages = roundup(filecount/4.0f);

	while (menu == MENU_NONE) //set pageToDisplay to 0 to quit
	{
	    VIDEO_WaitVSync ();

		menu = MENU_NONE;
		bool changed = false;
		int fileoffset = pageToDisplay*4-4;

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

        mainWindow->RemoveAll();

        /** Set new icons **/
        for(int i = 0; i < 4; i++) {
            if(IconData[i] != NULL) {
                delete IconData[i];
                IconData[i] = NULL;
            }
            if(IconImg[i] != NULL) {
                delete IconImg[i];
                IconImg[i] = NULL;
            }
            if(fileoffset+i < (int) filecount) {
                char iconpath[200];
                snprintf(iconpath, sizeof(iconpath), "%sicon.png", HomebrewFiles.GetFilepath(fileoffset+i));
                IconData[i] = new GuiImageData(iconpath, 0);
                if(IconData[i]->GetImage()) {
                    IconImg[i] = new GuiImage(IconData[i]);
                    IconImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
                    IconImg[i]->SetPosition(12, 0);
                    IconImg[i]->SetScale(0.95);
                }
            }
        }

        if(IconImg[0] != 0)
            MainButton1.SetIcon(IconImg[0]);
        else
            MainButton1.SetIcon(NULL);
        if(IconImg[1] != 0)
            MainButton2.SetIcon(IconImg[1]);
        else
            MainButton2.SetIcon(NULL);
        if(IconImg[2] != 0)
            MainButton3.SetIcon(IconImg[2]);
        else
            MainButton3.SetIcon(NULL);
        if(IconImg[3] != 0)
            MainButton4.SetIcon(IconImg[3]);
        else
            MainButton4.SetIcon(NULL);

        mainWindow->Append(&w);
        w.RemoveAll();
        w.Append(&background);
        w.Append(&titleTxt);
        w.Append(&backBtn);
        w.Append(&homo);
        w.Append(&GoRightBtn);
        w.Append(&GoLeftBtn);

        if(pageToDisplay == pages) {
            int buttonsleft = filecount-(pages-1)*4;
            char * shortpath = NULL;
            char temp[200];

            if(buttonsleft > 0) {
					snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset));
					if(XMLInfo[0].LoadHomebrewXMLData(temp) > 0) {
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[0].GetName());
						 MainButton1Txt.SetText(MainButtonText);
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[0].GetShortDescription());
						 MainButton1DescTxt.SetText(MainButtonText);
						 MainButton1DescOverTxt.SetText(MainButtonText);
                    } else {
						snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset)), "%s", HomebrewFiles.GetFilepath(fileoffset));
						shortpath = strrchr(temp, '/');
						snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset));
						XMLInfo[0].SetName(MainButtonText);
						MainButton1Txt.SetText(MainButtonText);
						snprintf(MainButtonText, sizeof(MainButtonText), " ");
						MainButton1DescTxt.SetText(MainButtonText);
                        MainButton1DescOverTxt.SetText(MainButtonText);
                    }
                w.Append(&MainButton1);
            }
            if(buttonsleft > 1) {
                    snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+1));
					if(XMLInfo[1].LoadHomebrewXMLData(temp) > 0){
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[1].GetName());
						 MainButton2Txt.SetText(MainButtonText);
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[1].GetShortDescription());
						 MainButton2DescTxt.SetText(MainButtonText);
						 MainButton2DescOverTxt.SetText(MainButtonText);
                    } else {
						snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+1)), "%s", HomebrewFiles.GetFilepath(fileoffset+1));
						shortpath = strrchr(temp, '/');
						snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+1));
						XMLInfo[1].SetName(MainButtonText);
						MainButton2Txt.SetText(MainButtonText);
						snprintf(MainButtonText, sizeof(MainButtonText), " ");
						MainButton2DescTxt.SetText(MainButtonText);
						MainButton2DescOverTxt.SetText(MainButtonText);
                    }
                w.Append(&MainButton2);
            }
            if(buttonsleft > 2) {
                    snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+2));
					if(XMLInfo[3].LoadHomebrewXMLData(temp) > 0) {
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetName());
						 MainButton3Txt.SetText(MainButtonText);
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetShortDescription());
						 MainButton3DescTxt.SetText(MainButtonText);
						 MainButton3DescOverTxt.SetText(MainButtonText);
                    } else {
						snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+2)), "%s", HomebrewFiles.GetFilepath(fileoffset+2));
						shortpath = strrchr(temp, '/');
						snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+2));
						XMLInfo[2].SetName(MainButtonText);
						MainButton3Txt.SetText(MainButtonText);
						snprintf(MainButtonText, sizeof(MainButtonText), " ");
						MainButton3DescTxt.SetText(MainButtonText);
						MainButton3DescOverTxt.SetText(MainButtonText);
                    }
                w.Append(&MainButton3);
            }
            if(buttonsleft > 3) {
                snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+3));
					if(XMLInfo[3].LoadHomebrewXMLData(temp) > 0) {
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetName());
						 MainButton4Txt.SetText(MainButtonText);
						 snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetShortDescription());
						 MainButton4DescTxt.SetText(MainButtonText);
						 MainButton4DescOverTxt.SetText(MainButtonText);
                    } else {
						snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+3)), "%s", HomebrewFiles.GetFilepath(fileoffset+3));
						shortpath = strrchr(temp, '/');
						snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+3));
						XMLInfo[3].SetName(MainButtonText);
						MainButton4Txt.SetText(MainButtonText);
						snprintf(MainButtonText, sizeof(MainButtonText), " ");
						MainButton4DescTxt.SetText(MainButtonText);
						MainButton4DescOverTxt.SetText(MainButtonText);
                    }
                w.Append(&MainButton4);
            }
        } else {
            char temp[200];
            char *shortpath = NULL;

            snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset));
            if(XMLInfo[0].LoadHomebrewXMLData(temp) > 0) {
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[0].GetName());
                MainButton1Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[0].GetShortDescription());
                MainButton1DescTxt.SetText(MainButtonText);
                MainButton1DescOverTxt.SetText(MainButtonText);
            } else {
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset)), "%s", HomebrewFiles.GetFilepath(fileoffset));
                shortpath = strrchr(temp, '/');
                snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset));
                XMLInfo[0].SetName(MainButtonText);
                MainButton1Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), " ");
                MainButton1DescTxt.SetText(MainButtonText);
                MainButton1DescOverTxt.SetText(MainButtonText);
            }

            w.Append(&MainButton1);

            snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+1));
            if(XMLInfo[1].LoadHomebrewXMLData(temp) > 0){
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[1].GetName());
                MainButton2Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[1].GetShortDescription());
                MainButton2DescTxt.SetText(MainButtonText);
                MainButton2DescOverTxt.SetText(MainButtonText);
            } else {
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+1)), "%s", HomebrewFiles.GetFilepath(fileoffset+1));
                shortpath = strrchr(temp, '/');
                snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+1));
                XMLInfo[1].SetName(MainButtonText);
                MainButton2Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), " ");
                MainButton2DescTxt.SetText(MainButtonText);
                MainButton2DescOverTxt.SetText(MainButtonText);
            }

            w.Append(&MainButton2);

            snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+2));
            if(XMLInfo[3].LoadHomebrewXMLData(temp) > 0) {
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetName());
                MainButton3Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetShortDescription());
                MainButton3DescTxt.SetText(MainButtonText);
                MainButton3DescOverTxt.SetText(MainButtonText);
            } else {
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+2)), "%s", HomebrewFiles.GetFilepath(fileoffset+2));
                shortpath = strrchr(temp, '/');
                snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+2));
                XMLInfo[2].SetName(MainButtonText);
                MainButton3Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), " ");
                MainButton3DescTxt.SetText(MainButtonText);
                MainButton3DescOverTxt.SetText(MainButtonText);
            }
            w.Append(&MainButton3);

            snprintf(temp, sizeof(temp), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+3));
            if(XMLInfo[3].LoadHomebrewXMLData(temp) > 0) {
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetName());
                MainButton4Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), "%s", XMLInfo[3].GetShortDescription());
                MainButton4DescTxt.SetText(MainButtonText);
                MainButton4DescOverTxt.SetText(MainButtonText);
            } else {
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+3)), "%s", HomebrewFiles.GetFilepath(fileoffset+3));
                shortpath = strrchr(temp, '/');
                snprintf(MainButtonText, sizeof(MainButtonText), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+3));
                XMLInfo[3].SetName(MainButtonText);
                MainButton4Txt.SetText(MainButtonText);
                snprintf(MainButtonText, sizeof(MainButtonText), " ");
                MainButton4DescTxt.SetText(MainButtonText);
                MainButton4DescOverTxt.SetText(MainButtonText);
            }

            w.Append(&MainButton4);
        }

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

		while(!changed)
		{
			VIDEO_WaitVSync ();

			if(MainButton1.GetState() == STATE_CLICKED) {
			    char temp[200];
                char iconpath[200];
                char metapath[200];
                char * shortpath = NULL;

				 //write iconpath
			    snprintf(metapath, sizeof(metapath), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset));

				 //write iconpath
                snprintf(iconpath, sizeof(iconpath), "%sicon.png", HomebrewFiles.GetFilepath(fileoffset));

                //get filesize
                u64 filesize = HomebrewFiles.GetFilesize(fileoffset);
                //write short filename
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset)), "%s", HomebrewFiles.GetFilepath(fileoffset));
                shortpath = strrchr(temp, '/');
                snprintf(temp, sizeof(temp), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset));

                int choice = HBCWindowPrompt(XMLInfo[0].GetName(), XMLInfo[0].GetCoder(), XMLInfo[0].GetVersion(), XMLInfo[0].GetReleasedate(), XMLInfo[0].GetLongDescription(), iconpath, filesize);
			    if(choice == 1) {
			        boothomebrew = true;
			        menu = MENU_EXIT;
                    snprintf(Settings.selected_homebrew, sizeof(Settings.selected_homebrew), "%s%s",  HomebrewFiles.GetFilepath(fileoffset), HomebrewFiles.GetFilename(fileoffset));
                    break;
			    }
                MainButton1.ResetState();
            }
            else if(MainButton2.GetState() == STATE_CLICKED) {
                char temp[200];
                char iconpath[200];
                char metapath[200];
                char * shortpath = NULL;

				 //write iconpath
			    snprintf(metapath, sizeof(metapath), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+1));

				 //write iconpath
			    snprintf(iconpath, sizeof(iconpath), "%sicon.png", HomebrewFiles.GetFilepath(fileoffset+1));

				 //get filesize
                u64 filesize = HomebrewFiles.GetFilesize(fileoffset+1);
				 //write short filename
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+1)), "%s", HomebrewFiles.GetFilepath(fileoffset+1));
                shortpath = strrchr(temp, '/');
                snprintf(temp, sizeof(temp), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+1));

				int choice = HBCWindowPrompt(XMLInfo[1].GetName(), XMLInfo[1].GetCoder(), XMLInfo[1].GetVersion(), XMLInfo[1].GetReleasedate(), XMLInfo[1].GetLongDescription(), iconpath, filesize);
			    if(choice == 1) {
                    boothomebrew = true;
			        menu = MENU_EXIT;
                    snprintf(Settings.selected_homebrew, sizeof(Settings.selected_homebrew), "%s%s",  HomebrewFiles.GetFilepath(fileoffset+1), HomebrewFiles.GetFilename(fileoffset+1));
                    break;
			    }
                MainButton2.ResetState();
            }
            else if(MainButton3.GetState() == STATE_CLICKED) {
                char temp[200];
                char iconpath[200];
                char metapath[200];
                char * shortpath = NULL;

				 //write iconpath
			    snprintf(metapath, sizeof(metapath), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+2));

				 //write iconpath
			    snprintf(iconpath, sizeof(iconpath), "%sicon.png", HomebrewFiles.GetFilepath(fileoffset+2));

				 //get filesize
				u64 filesize = HomebrewFiles.GetFilesize(fileoffset+2);
				 //write short filename
				snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+2)), "%s", HomebrewFiles.GetFilepath(fileoffset+2));
                shortpath = strrchr(temp, '/');
                snprintf(temp, sizeof(temp), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+2));

				int choice = HBCWindowPrompt(XMLInfo[2].GetName(), XMLInfo[2].GetCoder(), XMLInfo[2].GetVersion(), XMLInfo[2].GetReleasedate(), XMLInfo[2].GetLongDescription(), iconpath, filesize);
			    if(choice == 1) {
                    boothomebrew = true;
			        menu = MENU_EXIT;
                    snprintf(Settings.selected_homebrew, sizeof(Settings.selected_homebrew), "%s%s",  HomebrewFiles.GetFilepath(fileoffset+2), HomebrewFiles.GetFilename(fileoffset+2));
                    break;
			    }
                MainButton3.ResetState();
            }
            else if(MainButton4.GetState() == STATE_CLICKED) {
                char temp[200];
                char iconpath[200];
                char metapath[200];
                char * shortpath = NULL;

				 //write iconpath
			    snprintf(metapath, sizeof(metapath), "%smeta.xml", HomebrewFiles.GetFilepath(fileoffset+3));

				 //write iconpath
			    snprintf(iconpath, sizeof(iconpath), "%sicon.png", HomebrewFiles.GetFilepath(fileoffset+3));

				 //get filesize
                u64 filesize = HomebrewFiles.GetFilesize(fileoffset+3);
				 //write short filename
                snprintf(temp, strlen(HomebrewFiles.GetFilepath(fileoffset+3)), "%s", HomebrewFiles.GetFilepath(fileoffset+3));
                shortpath = strrchr(temp, '/');
                snprintf(temp, sizeof(temp), "%s/%s", shortpath, HomebrewFiles.GetFilename(fileoffset+3));

				int choice = HBCWindowPrompt(XMLInfo[3].GetName(), XMLInfo[3].GetCoder(), XMLInfo[3].GetVersion(), XMLInfo[3].GetReleasedate(), XMLInfo[3].GetLongDescription(), iconpath, filesize);
			    if(choice == 1) {
                    boothomebrew = true;
			        menu = MENU_EXIT;
                    snprintf(Settings.selected_homebrew, sizeof(Settings.selected_homebrew), "%s%s",  HomebrewFiles.GetFilepath(fileoffset+3), HomebrewFiles.GetFilename(fileoffset+3));
                    break;
			    }
                MainButton4.ResetState();
            }

            else if(shutdown == 1)
                Sys_Shutdown();
            else if(reset == 1)
                Sys_Reboot();

            else if(backBtn.GetState() == STATE_CLICKED) {
                menu = MENU_DISCLIST;
                changed = true;
            }

            else if(GoLeftBtn.GetState() == STATE_CLICKED) {
                pageToDisplay--;
                /** Change direction of the flying buttons **/
                if(pageToDisplay < 1)
                    pageToDisplay = pages;
                slidedirection = LEFT;
                changed = true;
                GoLeftBtn.ResetState();
            }

            else if(GoRightBtn.GetState() == STATE_CLICKED) {
                pageToDisplay++;
                /** Change direction of the flying buttons **/
                if(pageToDisplay > pages)
                    pageToDisplay = 1;
                slidedirection = RIGHT;
                changed = true;
                GoRightBtn.ResetState();
            }

            else if(homo.GetState() == STATE_CLICKED) {
                cfg_save_global();
                s32 thetimeofbg = bgMusic->GetPlayTime();
                bgMusic->Stop();
                choice = WindowExitPrompt(tr("Exit USB Loader GX?"),0, tr("Back to Loader"),tr("Wii Menu"),tr("Back"),0);
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
            }
        }
	}

	w.SetEffect(EFFECT_FADE, -20);
	while(w.GetEffect()>0) usleep(50);

	HaltGui();

    for(int i = 0; i < 4; i++) {
        if(IconData[i] != NULL) {
            delete IconData[i];
            IconData[i] = NULL;
        }
        if(IconImg[i] != NULL) {
            delete IconImg[i];
            IconImg[i] = NULL;
        }
    }

	mainWindow->RemoveAll();
	mainWindow->Append(bgImg);

	ResumeGui();

	return menu;
}
