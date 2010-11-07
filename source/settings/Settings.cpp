#include <string.h>
#include <unistd.h>

#include "usbloader/wbfs.h"
#include "language/gettext.h"
#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "prompts/DiscBrowser.h"
#include "settings/SettingsPrompts.h"
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/GameTitles.h"
#include "prompts/filebrowser.h"
#include "cheats/cheatmenu.h"
#include "themes/CTheme.h"
#include "fatmounter.h"
#include "menu.h"
#include "menu/menus.h"
#include "filelist.h"
#include "FileOperations/fileops.h"
#include "sys.h"
#include "usbloader/partition_usbloader.h"
#include "usbloader/utils.h"
#include "xml/xml.h"
#include "wad/nandtitle.h"
#include "prompts/TitleBrowser.h"

#define MAXOPTIONS 13

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();
extern void titles_default();

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiBGM * bgMusic;
extern GuiImage * bgImg;
extern GuiImageData * pointer[4];
extern GuiImageData * background;
extern u8 shutdown;
extern u8 reset;
extern u8 mountMethod;
extern struct discHdr *dvdheader;
extern PartList partitions;
extern char game_partition[6];
extern u8 load_from_fs;

static const char *opts_no_yes[MAX_ON_OFF] = { trNOOP( "No" ), trNOOP( "Yes" ) };
static const char *opts_off_on[MAX_ON_OFF] = { trNOOP( "OFF" ), trNOOP( "ON" ) };
static const char *opts_videomode[VIDEO_MODE_MAX][2] = { { "", trNOOP( "Disc Default" ) }, {
        trNOOP( "System Default" ), "" }, { trNOOP( "AutoPatch" ), "" }, { trNOOP( "Force" ), " PAL50" }, {
        trNOOP( "Force" ), " PAL60" }, { trNOOP( "Force" ), " NTSC" } };
static const char *opts_language[MAX_LANGUAGE] = { trNOOP( "App Default" ), trNOOP( "Console Default" ), trNOOP( "Japanese" ),
        trNOOP( "English" ), trNOOP( "German" ), trNOOP( "French" ), trNOOP( "Spanish" ), trNOOP( "Italian" ),
        trNOOP( "Dutch" ), trNOOP( "SChinese" ), trNOOP( "TChinese" ), trNOOP( "Korean" ) };
static const char *opts_lockedgames[2] = { trNOOP( "0 (Locked and Unlocked Games)" ), trNOOP( "1 (Unlocked Games Only)" ) };
static const char *opts_parentalcontrol[5] = { trNOOP( "0 (Everyone)" ), trNOOP( "1 (Child 7+)" ),
        trNOOP( "2 (Teen 12+)" ), trNOOP( "3 (Mature 16+)" ), trNOOP( "4 (Adults Only 18+)" ) };
static const char *opts_error002[3] = { trNOOP( "No" ), trNOOP( "Yes" ), trNOOP( "Anti" ) };
static const char *opts_partitions[3] = { trNOOP( "Game partition" ), trNOOP( "All partitions" ),
        trNOOP( "Remove update" ) };
static const char *opts_installdir[INSTALL_TO_MAX] = { trNOOP( "None" ), trNOOP( "GAMEID_Gamename" ),
        trNOOP( "Gamename [GAMEID]" ) };

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
    char opt_lang[100];
    strcpy(opt_lang, Settings.language_path);
    // backup title override setting
    int opt_override = Settings.titlesOverride;
    // backup partition index
    u8 settingspartitionold = Settings.partition;

    enum
    {
        FADE, LEFT, RIGHT
    };

    int slidedirection = FADE;

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
    // because destroy GuiSound must wait while sound playing is finished, we use a global sound
    if (!btnClick2) btnClick2 = new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    //  GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    GuiSound btnClick1(button_click_pcm, button_click_pcm_size, Settings.sfxvolume);

    GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));

    GuiImageData MainButtonImgData(Resources::GetFile("settings_title.png"), Resources::GetFileSize("settings_title.png"));

    GuiImageData MainButtonImgOverData(Resources::GetFile("settings_title_over.png"), Resources::GetFileSize("settings_title_over.png"));

    GuiImageData PageindicatorImgData(Resources::GetFile("pageindicator.png"), Resources::GetFileSize("pageindicator.png"));

    GuiImageData arrow_left(Resources::GetFile("startgame_arrow_left.png"), Resources::GetFileSize("startgame_arrow_left.png"));

    GuiImageData arrow_right(Resources::GetFile("startgame_arrow_right.png"), Resources::GetFileSize("startgame_arrow_right.png"));

    GuiImageData creditsImgData(Resources::GetFile("credits_button.png"), Resources::GetFileSize("credits_button.png"));

    GuiImageData creditsOver(Resources::GetFile("credits_button_over.png"), Resources::GetFileSize("credits_button_over.png"));

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

    GuiText titleTxt(tr( "Settings" ), 28, ( GXColor ) {0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0, 40);

    GuiImage settingsbackground(&settingsbg);

    GuiText backBtnTxt(tr( "Back" ), 22, Theme.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
    GuiImage backBtnImg(&btnOutline);
    if (Settings.wsprompt == ON)
    {
        backBtnTxt.SetWidescreen(Settings.widescreen);
        backBtnImg.SetWidescreen(Settings.widescreen);
    }
    GuiButton backBtn(&backBtnImg, &backBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, btnClick2, 1);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetTrigger(&trigB);

    GuiButton homo(1, 1);
    homo.SetTrigger(&trigHome);

    GuiImage PageindicatorImg1(&PageindicatorImgData);
    GuiText PageindicatorTxt1("1", 22, ( GXColor ) {0, 0, 0, 255});
    GuiButton PageIndicatorBtn1(PageindicatorImg1.GetWidth(), PageindicatorImg1.GetHeight());
    PageIndicatorBtn1.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    PageIndicatorBtn1.SetPosition(165, 400);
    PageIndicatorBtn1.SetImage(&PageindicatorImg1);
    PageIndicatorBtn1.SetLabel(&PageindicatorTxt1);
    PageIndicatorBtn1.SetSoundOver(&btnSoundOver);
    PageIndicatorBtn1.SetSoundClick(&btnClick1);
    PageIndicatorBtn1.SetTrigger(&trigA);
    PageIndicatorBtn1.SetEffectGrow();

    GuiImage PageindicatorImg2(&PageindicatorImgData);
    GuiText PageindicatorTxt2("2", 22, ( GXColor ) {0, 0, 0, 255});
    GuiButton PageIndicatorBtn2(PageindicatorImg2.GetWidth(), PageindicatorImg2.GetHeight());
    PageIndicatorBtn2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    PageIndicatorBtn2.SetPosition(200, 400);
    PageIndicatorBtn2.SetImage(&PageindicatorImg2);
    PageIndicatorBtn2.SetLabel(&PageindicatorTxt2);
    PageIndicatorBtn2.SetSoundOver(&btnSoundOver);
    PageIndicatorBtn2.SetSoundClick(&btnClick1);
    PageIndicatorBtn2.SetTrigger(&trigA);
    PageIndicatorBtn2.SetEffectGrow();

    GuiImage PageindicatorImg3(&PageindicatorImgData);
    GuiText PageindicatorTxt3("3", 22, ( GXColor ) {0, 0, 0, 255});
    GuiButton PageIndicatorBtn3(PageindicatorImg3.GetWidth(), PageindicatorImg3.GetHeight());
    PageIndicatorBtn3.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    PageIndicatorBtn3.SetPosition(235, 400);
    PageIndicatorBtn3.SetImage(&PageindicatorImg3);
    PageIndicatorBtn3.SetLabel(&PageindicatorTxt3);
    PageIndicatorBtn3.SetSoundOver(&btnSoundOver);
    PageIndicatorBtn3.SetSoundClick(&btnClick1);
    PageIndicatorBtn3.SetTrigger(&trigA);
    PageIndicatorBtn3.SetEffectGrow();

    GuiImage GoLeftImg(&arrow_left);
    GuiButton GoLeftBtn(GoLeftImg.GetWidth(), GoLeftImg.GetHeight());
    GoLeftBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    GoLeftBtn.SetPosition(25, -25);
    GoLeftBtn.SetImage(&GoLeftImg);
    GoLeftBtn.SetSoundOver(&btnSoundOver);
    GoLeftBtn.SetSoundClick(btnClick2);
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
    GoRightBtn.SetSoundClick(btnClick2);
    GoRightBtn.SetEffectGrow();
    GoRightBtn.SetTrigger(&trigA);
    GoRightBtn.SetTrigger(&trigR);
    GoRightBtn.SetTrigger(&trigPlus);

    char MainButtonText[50];
    snprintf(MainButtonText, sizeof(MainButtonText), "%s", " ");

    GuiImage MainButton1Img(&MainButtonImgData);
    GuiImage MainButton1ImgOver(&MainButtonImgOverData);
    GuiText MainButton1Txt(MainButtonText, 22, ( GXColor ) {0, 0, 0, 255});
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
    GuiText MainButton2Txt(MainButtonText, 22, ( GXColor ) {0, 0, 0, 255});
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
    GuiText MainButton3Txt(MainButtonText, 22, ( GXColor ) {0, 0, 0, 255});
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
    GuiText MainButton4Txt(MainButtonText, 22, ( GXColor ) {0, 0, 0, 255});
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

    OptionList options2;
    GuiCustomOptionBrowser optionBrowser2(396, 280, &options2, "bg_options_settings.png", 0, 150);
    optionBrowser2.SetPosition(0, 90);
    optionBrowser2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    GuiWindow w(screenwidth, screenheight);

    int pageToDisplay = 1;
    while (pageToDisplay > 0)
    {
        usleep(100);

        menu = MENU_NONE;

        if (pageToDisplay == 1)
        {
            /** Standard procedure made in all pages **/
            MainButton1.StopEffect();
            MainButton2.StopEffect();
            MainButton3.StopEffect();
            MainButton4.StopEffect();

            if (slidedirection == RIGHT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
            }
            else if (slidedirection == LEFT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
            }

            HaltGui();

            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "GUI Settings" ));
            MainButton1Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Game Load" ));
            MainButton2Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Parental Control" ));
            MainButton3Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Sound" ));
            MainButton4Txt.SetText(MainButtonText);

            mainWindow->RemoveAll();
            mainWindow->Append(&w);
            w.RemoveAll();
            w.Append(&settingsbackground);
            w.Append(&PageIndicatorBtn1);
            w.Append(&PageIndicatorBtn2);
            w.Append(&PageIndicatorBtn3);
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
            PageIndicatorBtn3.SetAlpha(50);

            /** Creditsbutton change **/
            MainButton4.SetImage(&MainButton4Img);
            MainButton4.SetImageOver(&MainButton4ImgOver);

            /** Disable ability to click through MainButtons */
            optionBrowser2.SetClickable(false);
            /** Default no scrollbar and reset position **/
            //          optionBrowser2.SetScrollbar(0);
            optionBrowser2.SetOffset(0);

            MainButton1.StopEffect();
            MainButton2.StopEffect();
            MainButton3.StopEffect();
            MainButton4.StopEffect();

            MainButton1.SetEffectGrow();
            MainButton2.SetEffectGrow();
            MainButton3.SetEffectGrow();
            MainButton4.SetEffectGrow();

            if (slidedirection == FADE)
            {
                MainButton1.SetEffect(EFFECT_FADE, 20);
                MainButton2.SetEffect(EFFECT_FADE, 20);
                MainButton3.SetEffect(EFFECT_FADE, 20);
                MainButton4.SetEffect(EFFECT_FADE, 20);
            }
            else if (slidedirection == LEFT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            }
            else if (slidedirection == RIGHT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            }

            mainWindow->Append(&w);

            ResumeGui();

            while (MainButton1.GetEffect() > 0)
                usleep(50);

        }
        else if (pageToDisplay == 2)
        {
            /** Standard procedure made in all pages **/
            MainButton1.StopEffect();
            MainButton2.StopEffect();
            MainButton3.StopEffect();
            MainButton4.StopEffect();

            if (slidedirection == RIGHT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
            }
            else if (slidedirection == LEFT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
            }

            HaltGui();

            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Custom Paths" ));
            MainButton1Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Update" ));
            MainButton2Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Default Settings" ));
            MainButton3Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Credits" ));
            MainButton4Txt.SetText(MainButtonText);

            mainWindow->RemoveAll();
            mainWindow->Append(&w);
            w.RemoveAll();
            w.Append(&settingsbackground);
            w.Append(&PageIndicatorBtn1);
            w.Append(&PageIndicatorBtn2);
            w.Append(&PageIndicatorBtn3);
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
            PageIndicatorBtn3.SetAlpha(50);

            /** Creditsbutton change **/
            MainButton4.SetImage(&creditsImg);
            MainButton4.SetImageOver(&creditsImgOver);

            /** Disable ability to click through MainButtons */
            optionBrowser2.SetClickable(false);
            /** Default no scrollbar and reset position **/
            //          optionBrowser2.SetScrollbar(0);
            optionBrowser2.SetOffset(0);

            MainButton1.StopEffect();
            MainButton2.StopEffect();
            MainButton3.StopEffect();
            MainButton4.StopEffect();

            MainButton1.SetEffectGrow();
            MainButton2.SetEffectGrow();
            MainButton3.SetEffectGrow();
            MainButton4.SetEffectGrow();

            if (slidedirection == FADE)
            {
                MainButton1.SetEffect(EFFECT_FADE, 20);
                MainButton2.SetEffect(EFFECT_FADE, 20);
                MainButton3.SetEffect(EFFECT_FADE, 20);
                MainButton4.SetEffect(EFFECT_FADE, 20);
            }
            else if (slidedirection == LEFT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            }
            else if (slidedirection == RIGHT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            }

            mainWindow->Append(&w);

            ResumeGui();

            while (MainButton1.GetEffect() > 0)
                usleep(50);

        }
        else if (pageToDisplay == 3)
        {
            /** Standard procedure made in all pages **/
            MainButton1.StopEffect();
            MainButton2.StopEffect();
            MainButton3.StopEffect();
            MainButton4.StopEffect();

            if (slidedirection == RIGHT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 35);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
            }
            else if (slidedirection == LEFT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 35);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
            }

            HaltGui();

            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Theme Downloader" ));
            MainButton1Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( " " ));
            MainButton2Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( " " ));
            MainButton3Txt.SetText(MainButtonText);
            snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( " " ));
            MainButton4Txt.SetText(MainButtonText);

            mainWindow->RemoveAll();
            mainWindow->Append(&w);
            w.RemoveAll();
            w.Append(&settingsbackground);
            w.Append(&PageIndicatorBtn1);
            w.Append(&PageIndicatorBtn2);
            w.Append(&PageIndicatorBtn3);
            w.Append(&titleTxt);
            w.Append(&backBtn);
            w.Append(&homo);
            w.Append(&GoRightBtn);
            w.Append(&GoLeftBtn);
            w.Append(&MainButton1);

            PageIndicatorBtn1.SetAlpha(50);
            PageIndicatorBtn2.SetAlpha(50);
            PageIndicatorBtn3.SetAlpha(255);

            /** Disable ability to click through MainButtons */
            optionBrowser2.SetClickable(false);
            /** Default no scrollbar and reset position **/
            //          optionBrowser2.SetScrollbar(0);
            optionBrowser2.SetOffset(0);

            MainButton1.StopEffect();
            MainButton2.StopEffect();
            MainButton3.StopEffect();
            MainButton4.StopEffect();

            MainButton1.SetEffectGrow();
            MainButton2.SetEffectGrow();
            MainButton3.SetEffectGrow();
            MainButton4.SetEffectGrow();

            if (slidedirection == FADE)
            {
                MainButton1.SetEffect(EFFECT_FADE, 20);
                MainButton2.SetEffect(EFFECT_FADE, 20);
                MainButton3.SetEffect(EFFECT_FADE, 20);
                MainButton4.SetEffect(EFFECT_FADE, 20);
            }
            else if (slidedirection == LEFT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 35);
            }
            else if (slidedirection == RIGHT)
            {
                MainButton1.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton2.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton3.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
                MainButton4.SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 35);
            }

            mainWindow->Append(&w);

            ResumeGui();

            while (MainButton1.GetEffect() > 0)
                usleep(50);
        }

        while (menu == MENU_NONE)
        {
            usleep(100);

            if (shutdown == 1) Sys_Shutdown();
            if (reset == 1) Sys_Reboot();

            if (pageToDisplay == 1)
            {
                if (MainButton1.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton1.GetEffect() > 0)
                        usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(tr( "GUI Settings" ));
                    exit = false;
                    options2.ClearList();
                    //                  optionBrowser2.SetScrollbar(1);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);

                    int returnhere = 1;
                    char * languagefile;
                    languagefile = strrchr(Settings.language_path, '/') + 1;

                    bool firstRun = true;
                    while (!exit)
                    {
                        usleep(100);

                        returnhere = 1;

                        if (shutdown == 1) Sys_Shutdown();
                        if (reset == 1)
                            Sys_Reboot();

                        else if (backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        else if (menu == MENU_DISCLIST)
                        {
                            w.Remove(&optionBrowser2);
                            w.Remove(&backBtn);
                            WindowCredits();
                            w.Append(&optionBrowser2);
                            w.Append(&backBtn);
                        }

                        else if (homo.GetState() == STATE_CLICKED)
                        {
                            Settings.Save();
                            optionBrowser2.SetState(STATE_DISABLED);
                            bgMusic->Pause();
                            choice = WindowExitPrompt();
                            bgMusic->Resume();
                            if (choice == 3)
                                Sys_LoadMenu(); // Back to System Menu
                            else if (choice == 2)
                                Sys_BackToLoader();
                            else homo.ResetState();
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        if (firstRun || ret >= 0)
                        {
                            int Idx = -1;

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "App Language" ));
                                if (ret == Idx)
                                {
                                    if (isInserted(Settings.BootDevice))
                                    {
                                        if (Settings.godmode == 1)
                                        {
                                            w.SetEffect(EFFECT_FADE, -20);
                                            while (w.GetEffect() > 0)
                                                usleep(50);
                                            mainWindow->Remove(&w);
                                            while (returnhere == 1)
                                                returnhere = MenuLanguageSelect();
                                            if (returnhere == 2)
                                            {
                                                menu = MENU_SETTINGS;
                                                pageToDisplay = 0;
                                                exit = true;
                                                mainWindow->Append(&w);
                                                break;
                                            }
                                            else
                                            {
                                                HaltGui();
                                                mainWindow->Append(&w);
                                                w.SetEffect(EFFECT_FADE, 20);
                                                ResumeGui();
                                                while (w.GetEffect() > 0)
                                                    usleep(50);
                                            }
                                        }
                                        else
                                        {
                                            WindowPrompt(tr( "Language change:" ),
                                                    tr( "Console should be unlocked to modify it." ), tr( "OK" ));
                                        }
                                    }
                                    else
                                    {
                                        WindowPrompt(tr( "No SD-Card inserted!" ),
                                                tr( "Insert an SD-Card to use this option." ), tr( "OK" ));
                                    }
                                }

                                if (!strcmp("notset", Settings.language_path))
                                    options2.SetValue(Idx, "%s", tr( "Default" ));
                                else options2.SetValue(Idx, "%s", languagefile);
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Display" ));
                                if (ret == Idx && ++Settings.sinfo >= GAMEINFO_MAX) Settings.sinfo = 0;
                                static const char *opts[GAMEINFO_MAX] = { trNOOP( "Game ID" ),
                                        trNOOP( "Game Region" ), trNOOP( "Both" ), trNOOP( "Neither" ) };
                                options2.SetValue(Idx, "%s", tr( opts[Settings.sinfo] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Clock" ));
                                if (ret == Idx && ++Settings.hddinfo >= CLOCK_MAX) Settings.hddinfo = 0; //CLOCK
                                if (Settings.hddinfo == CLOCK_HR12)
                                    options2.SetValue(Idx, "12 %s", tr( "Hour" ));
                                else if (Settings.hddinfo == CLOCK_HR24)
                                    options2.SetValue(Idx, "24 %s", tr( "Hour" ));
                                else if (Settings.hddinfo == OFF) options2.SetValue(Idx, "%s", tr( "OFF" ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Tooltips" ));
                                if (ret == Idx && ++Settings.tooltips >= MAX_ON_OFF) Settings.tooltips = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.tooltips] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Flip-X" ));
                                if (ret == Idx && ++Settings.xflip >= XFLIP_MAX) Settings.xflip = 0;
                                static const char *opts[XFLIP_MAX][3] =
                                        { { trNOOP( "Right" ), "/", trNOOP( "Next" ) }, { trNOOP( "Left" ), "/",
                                                trNOOP( "Prev" ) }, { trNOOP( "Like SysMenu" ), "", "" }, {
                                                trNOOP( "Right" ), "/", trNOOP( "Prev" ) }, { trNOOP( "DiskFlip" ),
                                                "", "" } };
                                options2.SetValue(Idx, "%s%s%s", tr( opts[Settings.xflip][0] ),
                                        opts[Settings.xflip][1], tr( opts[Settings.xflip][2] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Prompts Buttons" ));
                                if (ret == Idx && ++Settings.wsprompt >= MAX_ON_OFF) Settings.wsprompt = 0;
                                static const char *opts[MAX_ON_OFF] = { trNOOP( "Normal" ),
                                        trNOOP( "Widescreen Fix" ) };
                                options2.SetValue(Idx, "%s", tr( opts[Settings.wsprompt] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Keyboard" ));
                                if (ret == Idx && ++Settings.keyset >= KEYBOARD_MAX) Settings.keyset = 0;
                                static const char *opts[KEYBOARD_MAX] = { "QWERTY", "DVORAK",
                                        "QWERTZ", "AZERTY", "QWERTY 2" };
                                options2.SetValue(Idx, "%s", opts[Settings.keyset]);
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Disc Artwork Download" ));
                                if (ret == Idx && ++Settings.discart >= 4) Settings.discart = 0;
                                static const char *opts[4] = { trNOOP( "Only Original" ), trNOOP( "Only Customs" ),
                                        trNOOP( "Original/Customs" ), trNOOP( "Customs/Original" ) };
                                options2.SetValue(Idx, "%s", tr( opts[Settings.discart] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Wiilight" ));
                                if (ret == Idx && ++Settings.wiilight >= WIILIGHT_MAX) Settings.wiilight = 0;
                                static const char *opts[WIILIGHT_MAX] = { trNOOP( "OFF" ), trNOOP( "ON" ),
                                        trNOOP( "Only for Install" ) };
                                options2.SetValue(Idx, "%s", tr( opts[Settings.wiilight] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Rumble" ));
                                if (ret == Idx && ++Settings.rumble >= MAX_ON_OFF) Settings.rumble = 0; //RUMBLE
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.rumble] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "AutoInit Network" ));
                                if (ret == Idx && ++Settings.autonetwork >= MAX_ON_OFF) Settings.autonetwork
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.autonetwork] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "BETA revisions" ));
                                if (ret == Idx && ++Settings.beta_upgrades >= MAX_ON_OFF) Settings.beta_upgrades
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.beta_upgrades] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Titles from WiiTDB" ));
                                if (ret == Idx && ++Settings.titlesOverride >= MAX_ON_OFF) Settings.titlesOverride
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.titlesOverride] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Screensaver" ));
                                if (ret == Idx && ++Settings.screensaver >= SCREENSAVER_MAX) Settings.screensaver
                                        = 0; //RUMBLE
                                static const char *opts[SCREENSAVER_MAX] = { trNOOP( "OFF" ),
                                        trNOOP( "3 min" ), trNOOP( "5 min" ), trNOOP( "10 min" ), trNOOP( "20 min" ),
                                        trNOOP( "30 min" ), trNOOP( "1 hour" ) };
                                options2.SetValue(Idx, "%s", tr( opts[Settings.screensaver] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Mark new games" ));
                                if (ret == Idx && ++Settings.marknewtitles >= MAX_ON_OFF) Settings.marknewtitles
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.marknewtitles] ));
                            }

                            firstRun = false;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);
                    titleTxt.SetText(tr( "Settings" ));
                    slidedirection = FADE;
                    if (returnhere != 2) pageToDisplay = 1;
                    MainButton1.ResetState();
                    break;
                }

                else if (MainButton2.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton2.GetEffect() > 0)
                        usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(tr( "Game Load" ));
                    exit = false;
                    options2.ClearList();
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);

                    bool firstRun = true;
                    while (!exit)
                    {
                        usleep(100);

                        if (shutdown == 1) Sys_Shutdown();
                        if (reset == 1)
                            Sys_Reboot();

                        else if (backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        else if (homo.GetState() == STATE_CLICKED)
                        {
                            Settings.Save();
                            optionBrowser2.SetState(STATE_DISABLED);
                            bgMusic->Pause();
                            choice = WindowExitPrompt();
                            bgMusic->Resume();
                            if (choice == 3) Sys_LoadMenu(); // Back to System Menu
                            if (choice == 2)
                                Sys_BackToLoader();
                            else homo.ResetState();
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        if (firstRun || ret >= 0)
                        {
                            int Idx = -1;

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Video Mode" ));
                                if (ret == Idx && ++Settings.videomode >= VIDEO_MODE_MAX) Settings.videomode = 0;
                                options2.SetValue(Idx, "%s%s", opts_videomode[Settings.videomode][0],
                                        tr( opts_videomode[Settings.videomode][1] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "VIDTV Patch" ));
                                if (ret == Idx && ++Settings.videopatch >= MAX_ON_OFF) Settings.videopatch = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.videopatch] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Game Language" ));
                                if (ret == Idx && ++Settings.language >= MAX_LANGUAGE) Settings.language = 0;
                                options2.SetValue(Idx, "%s", tr( opts_language[Settings.language] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Patch Country Strings" ));
                                if (ret == Idx && ++Settings.patchcountrystrings >= MAX_ON_OFF) Settings.patchcountrystrings
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.patchcountrystrings] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "Ocarina");
                                if (ret == Idx && ++Settings.ocarina >= MAX_ON_OFF) Settings.ocarina = 0;
                                options2.SetValue(Idx, "%s", tr( opts_off_on[Settings.ocarina] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Boot/Standard" ));
                                if (ret == Idx && Settings.godmode == 1)
                                {
                                    char entered[4];
                                    snprintf(entered, sizeof(entered), "%i", Settings.cios);
                                    if(OnScreenKeyboard(entered, sizeof(entered), 0))
                                    {
                                        Settings.cios = atoi(entered);
                                        if(Settings.cios < 200) Settings.cios = 200;
                                        else if(Settings.cios > 255) Settings.cios = 255;

                                        if(NandTitles.IndexOf(TITLE_ID(1, Settings.cios)) < 0)
                                        {
                                            WindowPrompt(tr("Warning:"), tr("This IOS was not found on the titles list. If you are sure you have it installed than ignore this warning."), tr("OK"));
                                        }
                                        else if(Settings.cios == 254)
                                        {
                                            WindowPrompt(tr("Warning:"), tr("This IOS is the BootMii ios. If you are sure it is not BootMii and you have something else installed there than ignore this warning."), tr("OK"));
                                        }
                                    }
                                }
                                if (Settings.godmode == 1)
                                    options2.SetValue(Idx, "IOS %i", Settings.cios);
                                else options2.SetValue(Idx, "********");
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Partition" ));
                                if (ret == Idx)
                                {
                                    // Select the next valid partition, even if that's the same one
									int fs_type = partitions.pinfo[Settings.partition].fs_type;
                                    do
                                    {
                                        Settings.partition = (Settings.partition + 1) % partitions.num;
										fs_type = partitions.pinfo[Settings.partition].fs_type;
                                    } while (!(fs_type == FS_TYPE_WBFS || fs_type == FS_TYPE_FAT32 || fs_type == FS_TYPE_NTFS));
                                }

                                PartInfo pInfo = partitions.pinfo[Settings.partition];
                                f32 partition_size = partitions.pentry[Settings.partition].size
                                        * (partitions.sector_size / GB_SIZE);

                                // Get the partition name and it's size in GB's
                                options2.SetValue(Idx, "%s%d (%.2fGB)", pInfo.fs_type == FS_TYPE_FAT32 ? "FAT"
                                        : pInfo.fs_type == FS_TYPE_NTFS ? "NTFS" : "WBFS", pInfo.index, partition_size);
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "FAT: Use directories" ));
                                if (ret == Idx && ++Settings.FatInstallToDir >= INSTALL_TO_MAX) Settings.FatInstallToDir
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_installdir[Settings.FatInstallToDir] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Quick Boot" ));
                                if (ret == Idx && ++Settings.quickboot >= MAX_ON_OFF) Settings.quickboot = 0;
                                options2.SetValue(Idx, "%s", tr( opts_no_yes[Settings.quickboot] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Error 002 fix" ));
                                if (ret == Idx && ++Settings.error002 >= 3) Settings.error002 = 0;
                                options2.SetValue(Idx, "%s", tr( opts_error002[Settings.error002] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Install partitions" ));
                                if (ret == Idx && ++Settings.InstallPartitions >= 3) Settings.InstallPartitions
                                        = 0;
                                options2.SetValue(Idx, "%s", tr( opts_partitions[Settings.InstallPartitions] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Install 1:1 Copy" ));
                                if (ret == Idx)
                                {
                                    Settings.fullcopy = Settings.fullcopy == 0 ? 1 : 0;
                                }
                                options2.SetValue(Idx, "%s", tr( opts_no_yes[Settings.fullcopy] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                char* name = NULL;
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Return To" ));
                                if (ret == Idx)
                                {
                                    char tidChar[10];
                                    bool getChannel = TitleSelector(tidChar);
                                    if (getChannel)
                                    {
                                        snprintf(Settings.returnTo, sizeof(Settings.returnTo), "%s", tidChar);
                                    }
                                }
                                int haveTitle = NandTitles.FindU32(Settings.returnTo);
                                if (haveTitle >= 0)
                                {
                                    name = (char*) NandTitles.NameFromIndex(haveTitle);
				    if( name && !strlen(name) ) name = NULL;
                                }
                                options2.SetValue(Idx, "%s", name ? name : tr( opts_off_on[ 0 ] ));
                            }

                            firstRun = false;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);
                    titleTxt.SetText(tr( "Settings" ));
                    slidedirection = FADE;
                    pageToDisplay = 1;
                    MainButton2.ResetState();
                    break;
                }

                else if (MainButton3.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton3.GetEffect() > 0)
                        usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(tr( "Parental Control" ));
                    exit = false;
                    options2.ClearList();
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);

                    bool firstRun = true;
                    while (!exit)
                    {
                        usleep(100);

                        if (shutdown == 1) Sys_Shutdown();
                        if (reset == 1)
                            Sys_Reboot();

                        else if (backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        else if (homo.GetState() == STATE_CLICKED)
                        {
                            Settings.Save();
                            optionBrowser2.SetState(STATE_DISABLED);
                            bgMusic->Pause();
                            choice = WindowExitPrompt();
                            bgMusic->Resume();
                            if (choice == 3)
                                Sys_LoadMenu(); // Back to System Menu
                            else if (choice == 2)
                                Sys_BackToLoader();
                            else homo.ResetState();
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        if (firstRun || ret >= 0)
                        {

                            int Idx = -1;

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Console" ));
                                if (ret == Idx)
                                {
                                    if (!strcmp("", Settings.unlockCode) && Settings.Parental.enabled == 0)
                                    {
                                        Settings.godmode = !Settings.godmode;
                                    }
                                    else if (Settings.godmode == 0)
                                    {
                                        char entered[20];
                                        memset(entered, 0, 20);

                                        //password check to unlock Install,Delete and Format
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        int result = Settings.Parental.enabled == 0 ? OnScreenKeyboard(entered, 20, 0)
                                                : OnScreenNumpad(entered, 5);
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            if (!strcmp(entered, Settings.unlockCode) || !memcmp(entered,
                                                    Settings.Parental.pin, 4)) //if password correct
                                            {
                                                if (Settings.godmode == 0)
                                                {
                                                    WindowPrompt(
                                                            tr( "Correct Password" ),
                                                            tr( "All the features of USB Loader GX are unlocked." ),
                                                            tr( "OK" ));
                                                    Settings.godmode = 1;
                                                    menu = MENU_DISCLIST;
                                                }
                                            }
                                            else WindowPrompt(tr( "Wrong Password" ),
                                                    tr( "USB Loader GX is protected" ), tr( "OK" ));
                                        }
                                    }
                                    else
                                    {
                                        int choice = WindowPrompt(tr( "Lock Console" ), tr( "Are you sure?" ),
                                                tr( "Yes" ), tr( "No" ));
                                        if (choice == 1)
                                        {
                                            WindowPrompt(tr( "Console Locked" ), tr( "USB Loader GX is protected" ),
                                                    tr( "OK" ));
                                            Settings.godmode = 0;
                                            menu = MENU_DISCLIST;
                                        }
                                    }
                                }
                                static const char *opts[] = { trNOOP( "Locked" ), trNOOP( "Unlocked" ) };
                                options2.SetValue(Idx, "%s", tr( opts[Settings.godmode] ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Password" ));
                                if (ret == Idx)
                                {
                                    if (Settings.godmode == 1)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[20] = "";
                                        strlcpy(entered, Settings.unlockCode, sizeof(entered));
                                        int result = OnScreenKeyboard(entered, 20, 0);
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            strlcpy(Settings.unlockCode, entered, sizeof(Settings.unlockCode));
                                            WindowPrompt(tr( "Password Changed" ), tr( "Password has been changed" ),
                                                    tr( "OK" ));
                                        }
                                    }
                                    else
                                    {
                                        WindowPrompt(tr( "Password Changed" ),
                                                tr( "Console should be unlocked to modify it." ), tr( "OK" ));
                                    }
                                }
                                if (Settings.godmode != 1)
                                    options2.SetValue(Idx, "********");
                                else if (!strcmp("", Settings.unlockCode))
                                    options2.SetValue(Idx, "%s", tr( "not set" ));
                                else options2.SetValue(Idx, Settings.unlockCode);
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Controllevel" ));
                                if (ret == Idx && Settings.godmode == 1 && ++Settings.parentalcontrol >= 5) Settings.parentalcontrol
                                        = 0;
                                if (Settings.godmode == 1)
                                    options2.SetValue(Idx, "%s", tr( opts_parentalcontrol[Settings.parentalcontrol] ));
                                else options2.SetValue(Idx, "********");
                            }

                           if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "GamesLevel" ));
                                if (ret == Idx && Settings.godmode == 1 && ++Settings.lockedgames >= 2) Settings.lockedgames
                                        = 0;
                                if (Settings.godmode == 1)
                                    options2.SetValue(Idx, "%s", tr( opts_lockedgames[Settings.lockedgames] ));
                                else options2.SetValue(Idx, "********");
                            }

                            firstRun = false;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);
                    titleTxt.SetText(tr( "Settings" ));
                    slidedirection = FADE;
                    pageToDisplay = 1;
                    MainButton3.ResetState();
                    break;
                }

                else if (MainButton4.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton4.GetEffect() > 0)
                        usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(tr( "Sound" ));
                    exit = false;
                    options2.ClearList();
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);

                    bool firstRun = true;
                    while (!exit)
                    {
                        usleep(100);

                        bool returnhere = true;

                        if (shutdown == 1) Sys_Shutdown();
                        if (reset == 1)
                            Sys_Reboot();
                        else if (backBtn.GetState() == STATE_CLICKED)
                        {
                            backBtn.ResetState();
                            exit = true;
                            break;
                        }

                        else if (homo.GetState() == STATE_CLICKED)
                        {
                            Settings.Save();
                            optionBrowser2.SetState(STATE_DISABLED);
                            bgMusic->Pause();
                            choice = WindowExitPrompt();
                            bgMusic->Resume();
                            if (choice == 3)
                                Sys_LoadMenu(); // Back to System Menu
                            else if (choice == 2)
                                Sys_BackToLoader();
                            else homo.ResetState();
                            optionBrowser2.SetState(STATE_DEFAULT);
                        }

                        ret = optionBrowser2.GetClickedOption();

                        if (firstRun || ret >= 0)
                        {
                            int Idx = -1;
                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Backgroundmusic" ));
                                if (ret == Idx)
                                {
                                    if (isInserted(Settings.BootDevice))
                                    {
                                        w.SetEffect(EFFECT_FADE, -20);
                                        while (w.GetEffect() > 0)
                                            usleep(50);
                                        mainWindow->Remove(&w);

                                        returnhere = MenuBackgroundMusic();

                                        HaltGui();
                                        mainWindow->Append(&w);
                                        w.SetEffect(EFFECT_FADE, 20);
                                        ResumeGui();
                                        while (w.GetEffect() > 0)
                                            usleep(50);
                                    }
                                    else WindowPrompt(tr( "No SD-Card inserted!" ),
                                            tr( "Insert an SD-Card to use this option." ), tr( "OK" ));
                                }

                                char * filename = strrchr(Settings.ogg_path, '/');
                                if (filename)
                                {
                                    filename += 1;
                                    options2.SetValue(Idx, "%s", filename);
                                }
                                else options2.SetValue(Idx, "%s", tr( "Standard" ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Music Volume" ));
                                if (ret == Idx)
                                {
                                    Settings.volume += 10;
                                    if (Settings.volume > 100) Settings.volume = 0;
                                    bgMusic->SetVolume(Settings.volume);
                                }
                                if (Settings.volume > 0)
                                    options2.SetValue(Idx, "%i", Settings.volume);
                                else options2.SetValue(Idx, "%s", tr( "OFF" ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "SFX Volume" ));
                                if (ret == Idx)
                                {
                                    Settings.sfxvolume += 10;
                                    if (Settings.sfxvolume > 100) Settings.sfxvolume = 0;
                                    btnSoundOver.SetVolume(Settings.sfxvolume);
                                    btnClick2->SetVolume(Settings.sfxvolume);
                                    btnClick1.SetVolume(Settings.sfxvolume);
                                }
                                if (Settings.sfxvolume > 0)
                                    options2.SetValue(Idx, "%i", Settings.sfxvolume);
                                else options2.SetValue(Idx, "%s", tr( "OFF" ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Game Sound Mode" ));
                                if (ret == Idx)
                                {
                                    Settings.gamesound++;
                                    if (Settings.gamesound > 2) Settings.gamesound = 0;
                                }

                                if (Settings.gamesound == 1)
                                    options2.SetValue(Idx, "%s", tr( "Sound+BGM" ));
                                else if (Settings.gamesound == 2)
                                    options2.SetValue(Idx, "%s", tr( "Loop Sound" ));
                                else options2.SetValue(Idx, "%s", tr( "Sound+Quiet" ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Game Sound Volume" ));
                                if (ret == Idx)
                                {
                                    Settings.gamesoundvolume += 10;
                                    if (Settings.gamesoundvolume > 100) Settings.gamesoundvolume = 0;
                                }

                                if (Settings.gamesoundvolume > 0)
                                    options2.SetValue(Idx, "%i", Settings.gamesoundvolume);
                                else options2.SetValue(Idx, "%s", tr( "OFF" ));
                            }

                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Music Loop Mode" ));
                                if (ret == Idx)
                                {
                                    Settings.musicloopmode++;
                                    if (Settings.musicloopmode > 3) Settings.musicloopmode = 0;
                                    bgMusic->SetLoop(Settings.musicloopmode);
                                }

                                if (Settings.musicloopmode == ONCE)
                                    options2.SetValue(Idx, tr( "Play Once" ));
                                else if (Settings.musicloopmode == LOOP)
                                    options2.SetValue(Idx, tr( "Loop Music" ));
                                else if (Settings.musicloopmode == DIR_LOOP)
                                    options2.SetValue(Idx, tr( "Loop Directory" ));
                                else if (Settings.musicloopmode == RANDOM_BGM) options2.SetValue(Idx,
                                        tr( "Random Directory Music" ));
                            }
                            if (ret == ++Idx || firstRun)
                            {
                                if (firstRun) options2.SetName(Idx, "%s", tr( "Reset BG Music" ));
                                if (ret == Idx)
                                {
                                    int result =
                                            WindowPrompt(tr( "Reset to standard BGM?" ), 0, tr( "Yes" ), tr( "No" ));
                                    if (result)
                                    {
                                        bgMusic->LoadStandard();
                                        bgMusic->Play();
                                        options2.SetValue(Idx, "%s", tr( "Standard" ));
                                    }
                                }
                                options2.SetValue(Idx, tr( " " ));
                            }

                            firstRun = false;
                        }
                    }
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);
                    titleTxt.SetText(tr( "Settings" ));
                    slidedirection = FADE;
                    pageToDisplay = 1;
                    MainButton4.ResetState();
                    break;
                }
            }

            else if (pageToDisplay == 2)
            {
                if (MainButton1.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton1.GetEffect() > 0)
                        usleep(50);
                    HaltGui();
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    titleTxt.SetText(tr( "Custom Paths" ));
                    exit = false;
                    options2.ClearList();
                    //                  optionBrowser2.SetScrollbar(1);
                    w.Append(&optionBrowser2);
                    optionBrowser2.SetClickable(true);
                    ResumeGui();

                    optionBrowser2.SetEffect(EFFECT_FADE, 20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);

                    if (Settings.godmode)
                    {
                        bool firstRun = true;
                        while (!exit)
                        {
                            usleep(100);

                            if (shutdown == 1) Sys_Shutdown();
                            if (reset == 1)
                                Sys_Reboot();

                            else if (backBtn.GetState() == STATE_CLICKED)
                            {
                                backBtn.ResetState();
                                exit = true;
                                break;
                            }

                            else if (homo.GetState() == STATE_CLICKED)
                            {
                                Settings.Save();
                                optionBrowser2.SetState(STATE_DISABLED);
                                bgMusic->Pause();
                                choice = WindowExitPrompt();
                                bgMusic->Resume();
                                if (choice == 3)
                                    Sys_LoadMenu(); // Back to System Menu
                                else if (choice == 2)
                                    Sys_BackToLoader();
                                else homo.ResetState();
                                optionBrowser2.SetState(STATE_DEFAULT);
                            }

                            ret = optionBrowser2.GetClickedOption();

                            if (firstRun || ret >= 0)
                            {

                                int Idx = -1;
                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "3D Cover Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.covers_path, sizeof(entered));
                                        titleTxt.SetText(tr( "3D Cover Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.covers_path, entered, sizeof(Settings.covers_path));
                                            WindowPrompt(tr( "Cover Path Changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice)) WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.covers_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "2D Cover Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.covers2d_path, sizeof(entered));
                                        titleTxt.SetText(tr( "2D Cover Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.covers2d_path, entered, sizeof(Settings.covers2d_path));
                                            WindowPrompt(tr( "Cover Path Changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice)) WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.covers2d_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "Disc Artwork Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.disc_path, sizeof(entered));
                                        titleTxt.SetText(tr( "Disc Artwork Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.disc_path, entered, sizeof(Settings.disc_path));
                                            WindowPrompt(tr( "Disc Path Changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice)) WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.disc_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "Theme Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        titleTxt.SetText(tr( "Theme Path" ));
                                        strlcpy(entered, Settings.theme_path, sizeof(entered));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        HaltGui();
                                        w.RemoveAll();
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.theme_path, entered, sizeof(Settings.theme_path));
                                            WindowPrompt(tr( "Theme Path Changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice))
                                                WindowPrompt(tr( "No SD-Card inserted!" ),
                                                        tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                            else Settings.Save();
                                            mainWindow->Remove(bgImg);
                                            HaltGui();
											Theme.Load(Settings.theme_path);
                                            ResumeGui();
                                            menu = MENU_SETTINGS;
											if(pointer[0]) delete pointer[0];
											if(pointer[1]) delete pointer[1];
											if(pointer[2]) delete pointer[2];
											if(pointer[3]) delete pointer[3];
                                            pointer[0] = Resources::GetImageData("player1_point.png");
                                            pointer[1] = Resources::GetImageData("player2_point.png");
                                            pointer[2] = Resources::GetImageData("player3_point.png");
                                            pointer[3] = Resources::GetImageData("player4_point.png");
											if(background) delete background;
                                            background = Resources::GetImageData(Settings.widescreen ? "wbackground.png" : "background.png");
											if(bgImg) delete bgImg;
                                            bgImg = new GuiImage(background);
                                            mainWindow->Append(bgImg);
                                            mainWindow->Append(&w);
                                        }
                                        w.Append(&settingsbackground);
                                        w.Append(&titleTxt);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&backBtn);
                                        w.Append(&optionBrowser2);
                                        ResumeGui();
                                    }
                                    options2.SetValue(Idx, "%s", Settings.theme_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "WiiTDB Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        titleTxt.SetText(tr( "WiiTDB Path" ));
                                        strlcpy(entered, Settings.titlestxt_path, sizeof(entered));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        w.Append(&optionBrowser2);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.titlestxt_path, entered, sizeof(Settings.titlestxt_path));
                                            WindowPrompt(tr( "WiiTDB Path changed." ), 0, tr( "OK" ));
                                            if (isInserted(Settings.BootDevice))
                                            {
                                                Settings.Save();
                                                HaltGui();
                                                Settings.Load();
                                                ResumeGui();
                                            }
                                            else WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.titlestxt_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "Update Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.update_path, sizeof(entered));
                                        titleTxt.SetText(tr( "Update Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.update_path, entered, sizeof(Settings.update_path));
                                            WindowPrompt(tr( "Update Path changed." ), 0, tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.update_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "GCT Cheatcodes Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.Cheatcodespath, sizeof(entered));
                                        titleTxt.SetText(tr( "GCT Cheatcodes Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.Cheatcodespath, entered, sizeof(Settings.Cheatcodespath));
                                            WindowPrompt(tr( "GCT Cheatcodes Path changed" ), 0, tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.Cheatcodespath);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "TXT Cheatcodes Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.TxtCheatcodespath, sizeof(entered));
                                        titleTxt.SetText(tr( "TXT Cheatcodes Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.TxtCheatcodespath, entered,
                                                    sizeof(Settings.TxtCheatcodespath));
                                            WindowPrompt(tr( "TXT Cheatcodes Path changed" ), 0, tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.TxtCheatcodespath);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "DOL Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.dolpath, sizeof(entered));
                                        titleTxt.SetText(tr( "DOL Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.dolpath, entered, sizeof(Settings.dolpath));
                                            WindowPrompt(tr( "DOL path changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice))
                                            {
                                                WindowPrompt(tr( "No SD-Card inserted!" ),
                                                        tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                            }
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.dolpath);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "Homebrew Apps Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.homebrewapps_path, sizeof(entered));
                                        titleTxt.SetText(tr( "Homebrew Apps Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.homebrewapps_path, entered,
                                                    sizeof(Settings.homebrewapps_path));
                                            WindowPrompt(tr( "Homebrew Appspath changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice))
                                            {
                                                WindowPrompt(tr( "No SD-Card inserted!" ),
                                                        tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                            }
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.homebrewapps_path);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "Theme Download Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.theme_downloadpath, sizeof(entered));
                                        titleTxt.SetText(tr( "Theme Download Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.theme_downloadpath, entered,
                                                    sizeof(Settings.theme_downloadpath));
                                            WindowPrompt(tr( "Theme Download Path changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice)) WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.theme_downloadpath);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "BCA Codes Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.BcaCodepath, sizeof(entered));
                                        titleTxt.SetText(tr( "BCA Codes Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.BcaCodepath, entered, sizeof(Settings.BcaCodepath));
                                            WindowPrompt(tr( "BCA Codes Path changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice)) WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.BcaCodepath);
                                }

                                if (ret == ++Idx || firstRun)
                                {
                                    if (firstRun) options2.SetName(Idx, "%s", tr( "WIP Patches Path" ));
                                    if (ret == Idx)
                                    {
                                        w.Remove(&optionBrowser2);
                                        w.Remove(&backBtn);
                                        char entered[100] = "";
                                        strlcpy(entered, Settings.WipCodepath, sizeof(entered));
                                        titleTxt.SetText(tr( "WIP Patches Path" ));
                                        int result = BrowseDevice(entered, sizeof(entered), FB_DEFAULT, noFILES);
                                        titleTxt.SetText(tr( "Custom Paths" ));
                                        w.Append(&optionBrowser2);
                                        w.Append(&backBtn);
                                        if (result == 1)
                                        {
                                            int len = (strlen(entered) - 1);
                                            if (entered[len] != '/') strncat(entered, "/", 1);
                                            strlcpy(Settings.WipCodepath, entered, sizeof(Settings.WipCodepath));
                                            WindowPrompt(tr( "WIP Patches Path changed" ), 0, tr( "OK" ));
                                            if (!isInserted(Settings.BootDevice)) WindowPrompt(tr( "No SD-Card inserted!" ),
                                                    tr( "Insert an SD-Card to save." ), tr( "OK" ));
                                        }
                                    }
                                    options2.SetValue(Idx, "%s", Settings.WipCodepath);
                                }

                                firstRun = false;
                            }
                        }
                        /** If not godmode don't let him inside **/
                    }
                    else WindowPrompt(tr( "Console Locked" ), tr( "Unlock console to use this option." ), tr( "OK" ));
                    optionBrowser2.SetEffect(EFFECT_FADE, -20);
                    while (optionBrowser2.GetEffect() > 0)
                        usleep(50);
                    titleTxt.SetText(tr( "Settings" ));
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton1.ResetState();
                    break;
                }

                else if (MainButton2.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton2.GetEffect() > 0)
                        usleep(50);
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    if (isInserted(Settings.BootDevice) && Settings.godmode)
                    {
                        w.Remove(&optionBrowser2);
                        w.Remove(&backBtn);
                        int ret = ProgressUpdateWindow();
                        if (ret < 0) WindowPrompt(tr( "Update failed" ), 0, tr( "OK" ));
                        w.Append(&optionBrowser2);
                        w.Append(&backBtn);
                    }
                    else WindowPrompt(tr( "Console Locked" ), tr( "Unlock console to use this option." ), tr( "OK" ));
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton2.ResetState();
                    break;
                }

                else if (MainButton3.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton3.GetEffect() > 0)
                        usleep(50);
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
                    w.Remove(&GoRightBtn);
                    w.Remove(&GoLeftBtn);
                    w.Remove(&MainButton1);
                    w.Remove(&MainButton2);
                    w.Remove(&MainButton3);
                    w.Remove(&MainButton4);
                    w.Remove(&backBtn);
                    w.Remove(&optionBrowser2);
                    if (Settings.godmode)
                    {
                        int choice = WindowPrompt(tr( "Are you sure?" ), 0, tr( "Yes" ), tr( "Cancel" ));
                        if (choice == 1)
                        {
                            gettextCleanUp();
                            HaltGui();
                            Settings.Reset();
                            ResumeGui();
                            menu = MENU_SETTINGS;
                            pageToDisplay = 0;
                        }
                    }
                    else WindowPrompt(tr( "Console Locked" ), tr( "Unlock console to use this option." ), tr( "OK" ));
                    w.Append(&backBtn);
                    w.Append(&optionBrowser2);
                    slidedirection = FADE;
                    pageToDisplay = 2;
                    MainButton3.ResetState();
                    break;
                }

                else if (MainButton4.GetState() == STATE_CLICKED)
                {
                    MainButton1.SetEffect(EFFECT_FADE, -20);
                    MainButton2.SetEffect(EFFECT_FADE, -20);
                    MainButton3.SetEffect(EFFECT_FADE, -20);
                    MainButton4.SetEffect(EFFECT_FADE, -20);
                    while (MainButton4.GetEffect() > 0)
                        usleep(50);
                    w.Remove(&PageIndicatorBtn1);
                    w.Remove(&PageIndicatorBtn2);
                    w.Remove(&PageIndicatorBtn3);
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

            else if (pageToDisplay == 3)
            {
                if (MainButton1.GetState() == STATE_CLICKED)
                {
                    if (isInserted(Settings.BootDevice)) Settings.Save();
                    menu = MENU_THEMEDOWNLOADER;
                    pageToDisplay = 0;
                    break;
                }
            }

            if (backBtn.GetState() == STATE_CLICKED)
            {
                //Add the procedure call to save the global configuration
                if (isInserted(Settings.BootDevice)) Settings.Save();
                menu = MENU_DISCLIST;
                pageToDisplay = 0;
                backBtn.ResetState();
                break;
            }

            else if (GoLeftBtn.GetState() == STATE_CLICKED)
            {
                pageToDisplay--;
                /** Change direction of the flying buttons **/
                if (pageToDisplay < 1) pageToDisplay = 3;
                slidedirection = LEFT;
                GoLeftBtn.ResetState();
                break;
            }

            else if (GoRightBtn.GetState() == STATE_CLICKED)
            {
                pageToDisplay++;
                /** Change direction of the flying buttons **/
                if (pageToDisplay > 3) pageToDisplay = 1;
                slidedirection = RIGHT;
                GoRightBtn.ResetState();
                break;
            }
            else if (PageIndicatorBtn1.GetState() == STATE_CLICKED)
            {
                if (pageToDisplay > 1)
                {
                    slidedirection = LEFT;
                    pageToDisplay = 1;
                    PageIndicatorBtn1.ResetState();
                    break;
                }
                PageIndicatorBtn1.ResetState();
            }
            else if (PageIndicatorBtn2.GetState() == STATE_CLICKED)
            {
                if (pageToDisplay < 2)
                {
                    slidedirection = RIGHT;
                    pageToDisplay = 2;
                    PageIndicatorBtn2.ResetState();
                    break;
                }
                else if (pageToDisplay > 2)
                {
                    slidedirection = LEFT;
                    pageToDisplay = 2;
                    PageIndicatorBtn2.ResetState();
                    break;
                }
                else PageIndicatorBtn2.ResetState();
            }
            else if (PageIndicatorBtn3.GetState() == STATE_CLICKED)
            {
                if (pageToDisplay < 3)
                {
                    slidedirection = RIGHT;
                    pageToDisplay = 3;
                    PageIndicatorBtn3.ResetState();
                    break;
                }
                else PageIndicatorBtn3.ResetState();
            }
            else if (homo.GetState() == STATE_CLICKED)
            {
                Settings.Save();
                optionBrowser2.SetState(STATE_DISABLED);
                bgMusic->Pause();
                choice = WindowExitPrompt();
                bgMusic->Resume();

                if (choice == 3)
                    Sys_LoadMenu(); // Back to System Menu
                else if (choice == 2)
                    Sys_BackToLoader();
                else homo.ResetState();
                optionBrowser2.SetState(STATE_DEFAULT);
            }
        }
    }

    w.SetEffect(EFFECT_FADE, -20);
    while (w.GetEffect() > 0)
        usleep(50);

    // if partition has changed, Reinitialize it
    if (Settings.partition != settingspartitionold)
    {
        PartInfo pinfo = partitions.pinfo[Settings.partition];
        partitionEntry pentry = partitions.pentry[Settings.partition];
        WBFS_OpenPart(pinfo.part_fs, pinfo.index, pentry.sector, pentry.size, (char *) &game_partition);
        load_from_fs = pinfo.part_fs;
    }

    // if language has changed, reload titles
    char opt_langnew[100];
    strcpy(opt_langnew, Settings.language_path);
    int opt_overridenew = Settings.titlesOverride;
    bool reloaddatabasefile = false;
    if (strcmp(opt_lang, opt_langnew) || (opt_override != opt_overridenew && Settings.titlesOverride == 1)
            || (Settings.partition != settingspartitionold))
    {
        if (Settings.partition != settingspartitionold)
        {
            reloaddatabasefile = true;
            CloseXMLDatabase();
            GameTitles.SetDefault();
        }
        OpenXMLDatabase(Settings.titlestxt_path, Settings.db_language, Settings.db_JPtoEN, reloaddatabasefile,
                Settings.titlesOverride == 1 ? true : false, true); // open file, reload titles, keep in memory
    }
    // disable titles from database if setting has changed
    if (opt_override != opt_overridenew && Settings.titlesOverride == 0) GameTitles.SetDefault();

    HaltGui();

    mainWindow->RemoveAll();
    mainWindow->Append(bgImg);

    ResumeGui();
    return menu;
}

/********************************************************************************
 *Game specific settings
 *********************************************************************************/
int MenuGameSettings(struct discHdr * header)
{
    int menu = MENU_NONE;
    int ret;
    int choice = 0;
    bool exit = false;

    int retVal = 0;

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
    // because destroy GuiSound must wait while sound playing is finished, we use a global sound
    if (!btnClick2) btnClick2 = new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    //  GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    GuiSound btnClick1(button_click_pcm, button_click_pcm_size, Settings.sfxvolume);

    GuiImageData btnOutline(Resources::GetFile("button_dialogue_box.png"), Resources::GetFileSize("button_dialogue_box.png"));
    GuiImageData settingsbg(Resources::GetFile("settings_background.png"), Resources::GetFileSize("settings_background.png"));

    GuiImageData MainButtonImgData(Resources::GetFile("settings_title.png"), Resources::GetFileSize("settings_title.png"));

    GuiImageData MainButtonImgOverData(Resources::GetFile("settings_title_over.png"), Resources::GetFileSize("settings_title_over.png"));

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    char gameName[31];
    if (!mountMethod)
    {
        if (strlen(GameTitles.GetTitle(header)) < (27 + 3))
            sprintf(gameName, "%s", GameTitles.GetTitle(header));
        else
        {
            strncpy(gameName, GameTitles.GetTitle(header), 27);
            gameName[27] = '\0';
            strncat(gameName, "...", 3);
        }
    }
    else sprintf(gameName, "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4],
            header->id[5]);

    GuiText titleTxt(!mountMethod ? GameTitles.GetTitle(header) : gameName, 28, ( GXColor )
    {   0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(12, 40);
    titleTxt.SetMaxWidth(356, SCROLL_HORIZONTAL);

    GuiImage settingsbackground(&settingsbg);

    GuiText backBtnTxt(tr( "Back" ), 22, Theme.prompttext);
    backBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
    GuiImage backBtnImg(&btnOutline);
    if (Settings.wsprompt == ON)
    {
        backBtnTxt.SetWidescreen(Settings.widescreen);
        backBtnImg.SetWidescreen(Settings.widescreen);
    }
    GuiButton backBtn(&backBtnImg, &backBtnImg, 2, 3, -180, 400, &trigA, &btnSoundOver, btnClick2, 1);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetTrigger(&trigB);

    GuiButton homo(1, 1);
    homo.SetTrigger(&trigHome);

    GuiText saveBtnTxt(tr( "Save" ), 22, Theme.prompttext);
    saveBtnTxt.SetMaxWidth(btnOutline.GetWidth() - 30);
    GuiImage saveBtnImg(&btnOutline);
    if (Settings.wsprompt == ON)
    {
        saveBtnTxt.SetWidescreen(Settings.widescreen);
        saveBtnImg.SetWidescreen(Settings.widescreen);
    }
    GuiButton saveBtn(&saveBtnImg, &saveBtnImg, 2, 3, 180, 400, &trigA, &btnSoundOver, btnClick2, 1);
    saveBtn.SetLabel(&saveBtnTxt);

    char MainButtonText[50];
    snprintf(MainButtonText, sizeof(MainButtonText), "%s", " ");

    GuiImage MainButton1Img(&MainButtonImgData);
    GuiImage MainButton1ImgOver(&MainButtonImgOverData);
    GuiText MainButton1Txt(MainButtonText, 22, ( GXColor )
    {   0, 0, 0, 255});
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
    GuiText MainButton2Txt(MainButtonText, 22, ( GXColor )
    {   0, 0, 0, 255});
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
    GuiText MainButton3Txt(MainButtonText, 22, ( GXColor )
    {   0, 0, 0, 255});
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
    GuiText MainButton4Txt(MainButtonText, 22, ( GXColor ) {0, 0, 0, 255});
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

    OptionList options2;
    GuiCustomOptionBrowser optionBrowser2(396, 280, &options2, "bg_options_settings.png", 0, 150);
    optionBrowser2.SetPosition(0, 90);
    optionBrowser2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    GuiWindow w(screenwidth, screenheight);

	GameCFG game_cfg;

    GameCFG* existCFG = GameSettings.GetGameCFG(header->id);

	if (existCFG)
	{
	    memcpy(&game_cfg, existCFG, sizeof(GameCFG));
	}
	else
	{
        snprintf(game_cfg.id, sizeof(game_cfg.id), "%s", (char *) header->id);
        game_cfg.video = Settings.videomode;
        game_cfg.language = Settings.language;
        game_cfg.ocarina = Settings.ocarina;
        game_cfg.vipatch = Settings.videopatch;
        game_cfg.ios = Settings.cios;
        game_cfg.parentalcontrol = 0;
        game_cfg.errorfix002 = Settings.error002;
        game_cfg.patchcountrystrings = Settings.patchcountrystrings;
        game_cfg.loadalternatedol = OFF;
        game_cfg.alternatedolstart = 0;
        game_cfg.iosreloadblock = OFF;
        strcpy(game_cfg.alternatedolname, "");
        game_cfg.returnTo = 1;
        game_cfg.Locked = 0;
	}

    int pageToDisplay = 1;
    while (pageToDisplay > 0)
    {
        usleep(100);

        menu = MENU_NONE;

        /** Standard procedure made in all pages **/
        MainButton1.StopEffect();
        MainButton2.StopEffect();
        MainButton3.StopEffect();
        MainButton4.StopEffect();

        HaltGui();

        snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Game Load" ));
        MainButton1Txt.SetText(MainButtonText);
        snprintf(MainButtonText, sizeof(MainButtonText), "Ocarina");
        MainButton2Txt.SetText(MainButtonText);
        snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Uninstall Menu" ));
        MainButton3Txt.SetText(MainButtonText);
        snprintf(MainButtonText, sizeof(MainButtonText), "%s", tr( "Default Gamesettings" ));
        MainButton4Txt.SetText(MainButtonText);

        mainWindow->RemoveAll();
        mainWindow->Append(&w);
        w.RemoveAll();
        w.Append(&settingsbackground);
        w.Append(&titleTxt);
        w.Append(&backBtn);
        w.Append(&homo);
        w.Append(&MainButton1);
        w.Append(&MainButton2);
        w.Append(&MainButton3);
        w.Append(&MainButton4);

        /** Disable ability to click through MainButtons */
        optionBrowser2.SetClickable(false);
        /** Default no scrollbar and reset position **/
        //      optionBrowser2.SetScrollbar(0);
        optionBrowser2.SetOffset(0);

        MainButton1.StopEffect();
        MainButton2.StopEffect();
        MainButton3.StopEffect();
        MainButton4.StopEffect();

        MainButton1.SetEffectGrow();
        MainButton2.SetEffectGrow();
        MainButton3.SetEffectGrow();
        MainButton4.SetEffectGrow();

        MainButton1.SetEffect(EFFECT_FADE, 20);
        MainButton2.SetEffect(EFFECT_FADE, 20);
        MainButton3.SetEffect(EFFECT_FADE, 20);
        MainButton4.SetEffect(EFFECT_FADE, 20);

        mainWindow->Append(&w);

        ResumeGui();

        while (MainButton1.GetEffect() > 0)
            usleep(50);

        while (menu == MENU_NONE)
        {
            usleep(100);

            if (shutdown == 1) Sys_Shutdown();
            if (reset == 1) Sys_Reboot();

            if (MainButton1.GetState() == STATE_CLICKED)
            {
                w.Append(&saveBtn);
                MainButton1.SetEffect(EFFECT_FADE, -20);
                MainButton2.SetEffect(EFFECT_FADE, -20);
                MainButton3.SetEffect(EFFECT_FADE, -20);
                MainButton4.SetEffect(EFFECT_FADE, -20);
                while (MainButton1.GetEffect() > 0)
                    usleep(50);
                HaltGui();
                w.Remove(&MainButton1);
                w.Remove(&MainButton2);
                w.Remove(&MainButton3);
                w.Remove(&MainButton4);
                exit = false;
                options2.ClearList();
                w.Append(&optionBrowser2);
                optionBrowser2.SetClickable(true);
                ResumeGui();

                optionBrowser2.SetEffect(EFFECT_FADE, 20);
                while (optionBrowser2.GetEffect() > 0)
                    usleep(50);

                int returnhere = 1;
                char * languagefile;
                languagefile = strrchr(Settings.language_path, '/') + 1;

                bool firstRun = true;
                while (!exit)
                {
                    usleep(100);

                    returnhere = 1;

                    if (shutdown == 1) Sys_Shutdown();
                    if (reset == 1) Sys_Reboot();
                    if (backBtn.GetState() == STATE_CLICKED)
                    {
                        backBtn.ResetState();
                        exit = true;
                        break;
                    }

                    else if (menu == MENU_DISCLIST)
                    {
                        w.Remove(&optionBrowser2);
                        w.Remove(&backBtn);
                        WindowCredits();
                        w.Append(&optionBrowser2);
                        w.Append(&backBtn);
                    }

                    else if (homo.GetState() == STATE_CLICKED)
                    {
                        Settings.Save();
                        optionBrowser2.SetState(STATE_DISABLED);
                        bgMusic->Pause();
                        choice = WindowExitPrompt();
                        bgMusic->Resume();
                        if (choice == 3)
                            Sys_LoadMenu(); // Back to System Menu
                        else if (choice == 2)
                            Sys_BackToLoader();
                        else homo.ResetState();
                        optionBrowser2.SetState(STATE_DEFAULT);
                    }

                    else if (saveBtn.GetState() == STATE_CLICKED)
                    {
                        if (GameSettings.AddGame(game_cfg) && GameSettings.Save())
                        {
                            WindowPrompt(tr( "Successfully Saved" ), 0, tr( "OK" ));
                        }
                        else
                            WindowPrompt(tr( "Save Failed. No device inserted?" ), 0, tr( "OK" ));

                        saveBtn.ResetState();
                        optionBrowser2.SetFocus(1);
                    }

                    ret = optionBrowser2.GetClickedOption();

                    if (ret >= 0 || firstRun == true)
                    {
						char alternatedname[100];
                        int Idx = -1;

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Video Mode" ));
                            if (ret == Idx && ++game_cfg.video >= VIDEO_MODE_MAX) game_cfg.video = 0;
                            options2.SetValue(Idx, "%s%s", opts_videomode[game_cfg.video][0],
                                    tr( opts_videomode[game_cfg.video][1] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "VIDTV Patch" ));
                            if (ret == Idx && ++game_cfg.vipatch >= MAX_ON_OFF) game_cfg.vipatch = 0;
                            options2.SetValue(Idx, "%s", tr( opts_off_on[game_cfg.vipatch] ));

                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Game Language" ));
                            if (ret == Idx && ++game_cfg.language >= MAX_LANGUAGE) game_cfg.language = 0;
                            options2.SetValue(Idx, "%s", tr( opts_language[game_cfg.language] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "Ocarina");
                            if (ret == Idx && ++game_cfg.ocarina >= MAX_ON_OFF) game_cfg.ocarina = 0;
                            options2.SetValue(Idx, "%s", tr( opts_off_on[game_cfg.ocarina] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "IOS");
                            if (ret == Idx)
                            {
                                char entered[4];
                                snprintf(entered, sizeof(entered), "%i", Settings.cios);
                                if(OnScreenKeyboard(entered, sizeof(entered), 0))
                                {
                                    Settings.cios = atoi(entered);
                                    if(Settings.cios < 200) Settings.cios = 200;
                                    else if(Settings.cios > 255) Settings.cios = 255;

                                    if(NandTitles.IndexOf(TITLE_ID(1, Settings.cios)) < 0)
                                    {
                                        WindowPrompt(tr("Warning:"), tr("This IOS was not found on the titles list. If you are sure you have it installed than ignore this warning."), tr("OK"));
                                    }
                                    else if(Settings.cios == 254)
                                    {
                                        WindowPrompt(tr("Warning:"), tr("This IOS is the BootMii ios. If you are sure it is not BootMii and you have something else installed there than ignore this warning."), tr("OK"));
                                    }
                                }
                            }
                            options2.SetValue(Idx, "IOS %i", game_cfg.ios);
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Parental Control" ));
                            if (ret == Idx && ++game_cfg.parentalcontrol >= 5) game_cfg.parentalcontrol = 0;
                            options2.SetValue(Idx, "%s", tr( opts_parentalcontrol[game_cfg.parentalcontrol] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Error 002 fix" ));
                            if (ret == Idx && ++game_cfg.errorfix002 >= 3) game_cfg.errorfix002 = 0;
                            options2.SetValue(Idx, "%s", tr( opts_error002[game_cfg.errorfix002] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Return To" ));
                            if (ret == Idx && ++game_cfg.returnTo >= MAX_ON_OFF) game_cfg.returnTo = 0;

                            char text[IMET_MAX_NAME_LEN];
                            int channel = NandTitles.FindU32(Settings.returnTo);//is the channel set in the global settings actually installed?

                            if (!game_cfg.returnTo || channel < 0)//channel is not installed or the uer wants to not use it
                                sprintf(text, "%s", tr( opts_off_on[ 0 ] ));

                            else snprintf(text, sizeof(text), "%s", NandTitles.NameFromIndex(channel));

                            options2.SetValue(Idx, "%s", text);
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Patch Country Strings" ));
                            if (ret == Idx && ++game_cfg.patchcountrystrings >= MAX_ON_OFF) game_cfg.patchcountrystrings = 0;
                            options2.SetValue(Idx, "%s", tr( opts_off_on[game_cfg.patchcountrystrings] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Alternate DOL" ));
                            int last_alternatedol = game_cfg.loadalternatedol;
                            if (ret == Idx && (game_cfg.loadalternatedol = (game_cfg.loadalternatedol + 2) % 3) >= 3) // 0->2->1->0
                            game_cfg.loadalternatedol = 0;
                            static const char *opts[] = { trNOOP( "Default" ), trNOOP( "Load From SD/USB" ),
                                    trNOOP( "Select a DOL" ) };
                            options2.SetValue(Idx, "%s", tr( opts[game_cfg.loadalternatedol] ));
                            if (last_alternatedol != 1)
                            {
                                firstRun = true; // force re-init follow Entries
                                options2.Resize(Idx + 1);
                            }
                        }

                        if (game_cfg.loadalternatedol == 2 && (ret == ++Idx || firstRun))
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Selected DOL" ));
                            if (ret == Idx)
                            {
                                if (game_cfg.loadalternatedol == 2)
                                {
                                    char filename[10];
                                    snprintf(filename, 7, "%s", (char *) header->id);
                                    int dolchoice = 0;
                                    //alt dol menu for games that require more than a single alt dol
                                    int autodol = autoSelectDolMenu(filename, false);

                                    if (autodol > 0)
                                    {
                                        game_cfg.alternatedolstart = autodol;
                                        snprintf(alternatedname, sizeof(alternatedname), "%s <%i>", tr( "AUTO" ),
                                                autodol);
                                    }
                                    else if (autodol == 0)
                                        game_cfg.loadalternatedol = 0; // default was chosen
                                    else
                                    {
                                        //check to see if we already know the offset of the correct dol
                                        int autodol = autoSelectDol(filename, false);
                                        //if we do know that offset ask if they want to use it
                                        if (autodol > 0)
                                        {
                                            dolchoice
                                                    = WindowPrompt(
                                                            0,
                                                            tr( "Do you want to use the alternate DOL that is known to be correct?" ),
                                                            tr( "Yes" ), tr( "Pick from a list" ), tr( "Cancel" ));
                                            if (dolchoice == 0)
                                                game_cfg.loadalternatedol = 0;
                                            else if (dolchoice == 1)
                                            {
                                                game_cfg.alternatedolstart = autodol;
                                                snprintf(alternatedname, sizeof(alternatedname), "%s <%i>",
                                                        tr( "AUTO" ), autodol);
                                            }
                                            else if (dolchoice == 2) //they want to search for the correct dol themselves
                                            {
                                                int res = DiscBrowse(header, alternatedname, sizeof(alternatedname));
                                                if ((res >= 0) && (res != 696969)) //if res==696969 they pressed the back button
                                                game_cfg.alternatedolstart = res;
                                            }
                                        }
                                        else
                                        {
                                            int res = DiscBrowse(header, alternatedname, sizeof(alternatedname));
                                            if ((res >= 0) && (res != 696969))
                                            {
                                                game_cfg.alternatedolstart = res;
                                                char tmp[170];
                                                snprintf(
                                                        tmp,
                                                        sizeof(tmp),
                                                        "%s %s - %i",
                                                        tr( "It seems that you have some information that will be helpful to us. Please pass this information along to the DEV team." ),
                                                        filename, game_cfg.alternatedolstart);
                                                WindowPrompt(0, tmp, tr( "OK" ));
                                            }
                                        }
                                    }
                                }
                            }
                            if (game_cfg.loadalternatedol == 0)
                            {
                                firstRun = true; // force re-init follow Entries
                                options2.Resize(Idx--); // remove this Entry
                                options2.SetValue(Idx, "%s", tr( "Default" )); // re-set prev Entry
                            }
                            else options2.SetValue(Idx, alternatedname);
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Block IOS Reload" ));
                            if (ret == Idx && ++game_cfg.iosreloadblock >= MAX_ON_OFF) game_cfg.iosreloadblock = 0;
                            options2.SetValue(Idx, "%s", tr( opts_off_on[game_cfg.iosreloadblock] ));
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Game Locked" ));
                            if (ret == Idx && ++game_cfg.Locked >= MAX_ON_OFF) game_cfg.Locked = 0;
                            options2.SetValue(Idx, "%s", tr( opts_off_on[game_cfg.Locked] ));
                        }

                        firstRun = false;
                    }
                }

                optionBrowser2.SetEffect(EFFECT_FADE, -20);
                while (optionBrowser2.GetEffect() > 0)
                    usleep(50);
                MainButton1.ResetState();
                break;
                w.Remove(&saveBtn);
            }

            else if (MainButton2.GetState() == STATE_CLICKED)
            {
                char ID[7];
                snprintf(ID, sizeof(ID), "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3],
                        header->id[4], header->id[5]);
                CheatMenu(ID);
                MainButton2.ResetState();
                break;
            }

            else if (MainButton3.GetState() == STATE_CLICKED)
            {
                MainButton1.SetEffect(EFFECT_FADE, -20);
                MainButton2.SetEffect(EFFECT_FADE, -20);
                MainButton3.SetEffect(EFFECT_FADE, -20);
                MainButton4.SetEffect(EFFECT_FADE, -20);
                while (MainButton3.GetEffect() > 0)
                    usleep(50);
                HaltGui();
                w.Remove(&MainButton1);
                w.Remove(&MainButton2);
                w.Remove(&MainButton3);
                w.Remove(&MainButton4);
                exit = false;
                options2.ClearList();
                w.Append(&optionBrowser2);
                optionBrowser2.SetClickable(true);
                ResumeGui();

                bool firstRun = true;

                optionBrowser2.SetEffect(EFFECT_FADE, 20);
                while (optionBrowser2.GetEffect() > 0)
                    usleep(50);

                while (!exit)
                {
                    usleep(100);

                    if (shutdown == 1) Sys_Shutdown();
                    if (reset == 1) Sys_Reboot();
                    if (backBtn.GetState() == STATE_CLICKED)
                    {
                        backBtn.ResetState();
                        exit = true;
                        break;
                    }

                    else if (homo.GetState() == STATE_CLICKED)
                    {
                        Settings.Save();
                        optionBrowser2.SetState(STATE_DISABLED);
                        bgMusic->Pause();
                        choice = WindowExitPrompt();
                        bgMusic->Resume();
                        if (choice == 3)
                            Sys_LoadMenu(); // Back to System Menu
                        else if (choice == 2)
                            Sys_BackToLoader();
                        else homo.ResetState();
                        optionBrowser2.SetState(STATE_DEFAULT);
                    }

                    ret = optionBrowser2.GetClickedOption();

                    if (firstRun || ret >= 0)
                    {
                        int Idx = -1;
                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Uninstall Game" ));
                            if (ret == Idx)
                            {
                                int choice1 = WindowPrompt(tr( "Do you really want to delete:" ), gameName,
                                        tr( "Yes" ), tr( "Cancel" ));
                                if (choice1 == 1 && !mountMethod)
                                {
                                    GameSettings.Remove(header->id);
                                    GameSettings.Save();
                                    GameStatistics.Remove(header->id);
									GameStatistics.Save();
                                    ret = WBFS_RemoveGame(header->id);
                                    if (ret < 0)
                                    {
                                        WindowPrompt(tr( "Can't delete:" ), gameName, tr( "OK" ));
                                    }
                                    else
                                    {
                                        WindowPrompt(tr( "Successfully deleted:" ), gameName, tr( "OK" ));
                                        retVal = 1;
                                    }
                                }
                                else if (choice1 == 0) optionBrowser2.SetFocus(1);
                            }
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Reset Playcounter" ));
                            if (ret == Idx)
                            {
                                int result = WindowPrompt(tr( "Are you sure?" ), 0, tr( "Yes" ), tr( "Cancel" ));
                                if (result == 1)
                                {
                                    if (isInserted(Settings.BootDevice))
                                    {
                                        GameStatistics.SetPlayCount(header->id, 0);
										GameStatistics.Save();
                                    }
                                }
                            }
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Delete Cover Artwork" ));
                            if (ret == Idx)
                            {
                                char tmp[200];
                                snprintf(tmp, sizeof(tmp), "%s%c%c%c%c%c%c.png", Settings.covers_path, header->id[0],
                                        header->id[1], header->id[2], header->id[3], header->id[4], header->id[5]);

                                int choice1 = WindowPrompt(tr( "Delete" ), tmp, tr( "Yes" ), tr( "No" ));
                                if (choice1 == 1)
                                {
                                    if (CheckFile(tmp)) remove(tmp);
                                }
                            }
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Delete Disc Artwork" ));
                            if (ret == Idx)
                            {
                                char tmp[200];
                                snprintf(tmp, sizeof(tmp), "%s%c%c%c%c%c%c.png", Settings.disc_path, header->id[0],
                                        header->id[1], header->id[2], header->id[3], header->id[4], header->id[5]);

                                int choice1 = WindowPrompt(tr( "Delete" ), tmp, tr( "Yes" ), tr( "No" ));
                                if (choice1 == 1)
                                {
                                    if (CheckFile(tmp)) remove(tmp);
                                }
                            }
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Delete Cheat TXT" ));
                            if (ret == Idx)
                            {
                                char tmp[200];
                                snprintf(tmp, sizeof(tmp), "%s%c%c%c%c%c%c.txt", Settings.TxtCheatcodespath,
                                        header->id[0], header->id[1], header->id[2], header->id[3], header->id[4],
                                        header->id[5]);

                                int choice1 = WindowPrompt(tr( "Delete" ), tmp, tr( "Yes" ), tr( "No" ));
                                if (choice1 == 1)
                                {
                                    if (CheckFile(tmp)) remove(tmp);
                                }
                            }
                        }

                        if (ret == ++Idx || firstRun)
                        {
                            if (firstRun) options2.SetName(Idx, "%s", tr( "Delete Cheat GCT" ));
                            if (ret == Idx)
                            {
                                char tmp[200];
                                snprintf(tmp, sizeof(tmp), "%s%c%c%c%c%c%c.gct", Settings.Cheatcodespath,
                                        header->id[0], header->id[1], header->id[2], header->id[3], header->id[4],
                                        header->id[5]);

                                int choice1 = WindowPrompt(tr( "Delete" ), tmp, tr( "Yes" ), tr( "No" ));
                                if (choice1 == 1)
                                {
                                    if (CheckFile(tmp)) remove(tmp);
                                }
                            }
                        }

                        firstRun = false;
                    }
                }
                optionBrowser2.SetEffect(EFFECT_FADE, -20);
                while (optionBrowser2.GetEffect() > 0)
                    usleep(50);
                pageToDisplay = 1;
                MainButton3.ResetState();
                break;
            }

            else if (MainButton4.GetState() == STATE_CLICKED)
            {
                int choice1 = WindowPrompt(tr( "Are you sure?" ), 0, tr( "Yes" ), tr( "Cancel" ));
                if (choice1 == 1)
                {
                    game_cfg.video = Settings.videomode;
                    game_cfg.vipatch = Settings.videopatch;
                    game_cfg.language = Settings.language;
                    game_cfg.ocarina = Settings.ocarina;
                    game_cfg.errorfix002 = Settings.error002;
                    game_cfg.patchcountrystrings = Settings.patchcountrystrings;
                    game_cfg.loadalternatedol = OFF;
                    game_cfg.alternatedolstart = 0;
                    game_cfg.iosreloadblock = OFF;
                    game_cfg.ios = Settings.cios;
                    game_cfg.parentalcontrol = 0;
                    strcpy(game_cfg.alternatedolname, "");
                    game_cfg.returnTo = 1;
                    game_cfg.Locked = 0;
                    GameSettings.Remove(header->id);
                    GameSettings.Save();
                }

                pageToDisplay = 1;
                MainButton4.ResetState();
                break;
            }

            else if (backBtn.GetState() == STATE_CLICKED)
            {
                menu = MENU_DISCLIST;
                pageToDisplay = 0;
                backBtn.ResetState();
                break;
            }

            else if (homo.GetState() == STATE_CLICKED)
            {
                Settings.Save();
                optionBrowser2.SetState(STATE_DISABLED);
                bgMusic->Pause();
                choice = WindowExitPrompt();
                bgMusic->Resume();

                if (choice == 3)
                    Sys_LoadMenu(); // Back to System Menu
                else if (choice == 2)
                    Sys_BackToLoader();
                else homo.ResetState();
                optionBrowser2.SetState(STATE_DEFAULT);
            }
        }
    }
    w.SetEffect(EFFECT_FADE, -20);
    while (w.GetEffect() > 0)
        usleep(50);

    HaltGui();

    mainWindow->RemoveAll();
    mainWindow->Append(bgImg);

    ResumeGui();
    return retVal;
}
