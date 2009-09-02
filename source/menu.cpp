/****************************************************************************
 * USB Loader GX Team
 *
 * libwiigui Template
 * by Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h> //CLOCK
#include <time.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_gamegrid.h"
#include "libwiigui/gui_gamecarousel.h"
#include "libwiigui/gui_gamebrowser.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "usbloader/usbstorage.h"
#include "usbloader/wbfs.h"
#include "usbloader/disc.h"
#include "usbloader/getentries.h"
#include "language/gettext.h"
#include "settings/Settings.h"
#include "homebrewboot/HomebrewBrowse.h"
#include "homebrewboot/BootHomebrew.h"
#include "prompts/PromptWindows.h"
#include "prompts/filebrowser.h"
#include "prompts/ProgressWindow.h"
#include "prompts/TitleBrowser.h"
#include "prompts/gameinfo.h"
#include "mload/mload.h"
#include "patches/patchcode.h"
#include "network/networkops.h"
#include "cheats/cheatmenu.h"
#include "menu.h"
#include "audio.h"
#include "wad/wad.h"
#include "input.h"
#include "filelist.h"
#include "sys.h"
#include "wpad.h"
#include "listfiles.h"
#include "fatmounter.h"
#include "buffer.h"
#include "xml/xml.h"
#include "wad/title.h"

#include "usbloader/wdvd.h"


#define MAX_CHARACTERS		38

/*** Variables that are also used extern ***/
GuiWindow * mainWindow = NULL;
GuiImageData * pointer[4];
GuiImage * bgImg = NULL;
GuiImageData * background = NULL;
GuiSound * bgMusic = NULL;
float gamesize;
int currentMenu;
int idiotFlag=-1;
char idiotChar[50];

/*** Variables used only in menu.cpp ***/
static GuiImage * coverImg = NULL;
static GuiImageData * cover = NULL;
static GuiText * GameIDTxt = NULL;
static GuiText * GameRegionTxt = NULL;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static int ExitRequested = 0;
static char gameregion[7];

/*** Extern variables ***/
extern FreeTypeGX *fontClock;
extern u8 shutdown;
extern u8 reset;
extern int cntMissFiles;
extern struct discHdr * gameList;
extern u32 gameCnt;
extern s32 gameSelected, gameStart;
extern const u8 data1;
extern u8 boothomebrew;
extern bool updateavailable;

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
void
ResumeGui() {
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
void
HaltGui() {
    guiHalt = true;

    // wait for thread to finish
    while (!LWP_ThreadIsSuspended(guithread))
        usleep(50);
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/
static void * UpdateGUI (void *arg) {
    while (1) {
        if (guiHalt) {
            LWP_SuspendThread(guithread);
        } else {
            if (!ExitRequested) {
                mainWindow->Draw();
                if (Settings.tooltips == TooltipsOn && THEME.showToolTip != 0 && mainWindow->GetState() != STATE_DISABLED)
                    mainWindow->DrawTooltip();

#ifdef HW_RVL
                for (int i=3; i >= 0; i--) { // so that player 1's cursor appears on top!
                    if (userInput[i].wpad.ir.valid)
                        Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48, 200.0,
                                     96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, CFG.widescreen? 0.8 : 1, 1, 255,0,0,0,0,0,0,0,0);
                    if (Settings.rumble == RumbleOn) {
                        DoRumble(i);
                    }
                }
#endif

                Menu_Render();

                for (int i=0; i < 4; i++)
                    mainWindow->Update(&userInput[i]);


            } else {
                for (int a = 5; a < 255; a += 10) {
                    mainWindow->Draw();
                    Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor) {0, 0, 0, a},1);
                    Menu_Render();
                }
                mainWindow->RemoveAll();
                ShutoffRumble();
                return 0;
            }
        }

        switch (Settings.screensaver) {
        case 1:
            WPad_SetIdleTime(180);
            break;
        case 2:
            WPad_SetIdleTime(300);
            break;
        case 3:
            WPad_SetIdleTime(600);
            break;
        case 4:
            WPad_SetIdleTime(1200);
            break;
        case 5:
            WPad_SetIdleTime(1800);
            break;
        case 6:
            WPad_SetIdleTime(3600);
            break;

        }



    }
    return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void InitGUIThreads() {
    LWP_CreateThread(&guithread, UpdateGUI, NULL, NULL, 0, 75);
    InitProgressThread();
    InitBufferThread();
    InitNetworkThread();

    if (Settings.autonetwork)
        ResumeNetworkThread();
}

void ExitGUIThreads() {
    ExitRequested = 1;
    LWP_JoinThread(guithread, NULL);
    guithread = LWP_THREAD_NULL;
}
void rockout(int f = 0) {

    HaltGui();
    int num=(f==2?-1:gameSelected);

    char imgPath[100];
#ifdef HW_RVL
    if (!(strcasestr(get_title(&gameList[num]),"guitar")||
            strcasestr(get_title(&gameList[num]),"band")||
            strcasestr(get_title(&gameList[num]),"rock")||
            f==1)) {
        for (int i = 0; i < 4; i++)
            delete pointer[i];
        snprintf(imgPath, sizeof(imgPath), "%splayer1_point.png", CFG.theme_path);
        pointer[0] = new GuiImageData(imgPath, player1_point_png);
        snprintf(imgPath, sizeof(imgPath), "%splayer2_point.png", CFG.theme_path);
        pointer[1] = new GuiImageData(imgPath, player2_point_png);
        snprintf(imgPath, sizeof(imgPath), "%splayer3_point.png", CFG.theme_path);
        pointer[2] = new GuiImageData(imgPath, player3_point_png);
        snprintf(imgPath, sizeof(imgPath), "%splayer4_point.png", CFG.theme_path);
        pointer[3] = new GuiImageData(imgPath, player4_point_png);
    } else {

        for (int i = 0; i < 4; i++)
            delete pointer[i];
        snprintf(imgPath, sizeof(imgPath), "%srplayer1_point.png", CFG.theme_path);
        pointer[0] = new GuiImageData(imgPath, rplayer1_point_png);
        snprintf(imgPath, sizeof(imgPath), "%srplayer2_point.png", CFG.theme_path);
        pointer[1] = new GuiImageData(imgPath, rplayer2_point_png);
        snprintf(imgPath, sizeof(imgPath), "%srplayer3_point.png", CFG.theme_path);
        pointer[2] = new GuiImageData(imgPath, rplayer3_point_png);
        snprintf(imgPath, sizeof(imgPath), "%srplayer4_point.png", CFG.theme_path);
        pointer[3] = new GuiImageData(imgPath, rplayer4_point_png);
    }
#endif
    ResumeGui();
}

/****************************************************************************
 * MenuDiscList
 ***************************************************************************/

int MenuDiscList() {

    int startat = 0;
    int offset = 0;
    int datag = 0;
    int datagB =0;
    int dataed = -1;
    int cosa=0,sina=0;
    int selectImg1 = 0;
    char ID[4];
    char IDfull[7];
    u32 covert = 0;
    char imgPath[100];

    WDVD_GetCoverStatus(&covert);
    u32 covertOld=covert;


    //SCREENSAVER
    //WPad_SetIdleTime(300); //needs the time in seconds
    int check = 0; //to skip the first cycle when wiimote isn't completely connected

    datagB=0;
    int menu = MENU_NONE, dataef=0;
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


    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
    GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

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
    snprintf(imgPath, sizeof(imgPath), "%sSDcard_over.png", CFG.theme_path);
    GuiImageData btnsdcardOver(imgPath, sdcard_over_png);
    snprintf(imgPath, sizeof(imgPath), "%sSDcard.png", CFG.theme_path);
    GuiImageData btnsdcard(imgPath, sdcard_png);


    snprintf(imgPath, sizeof(imgPath), "%sbattery.png", CFG.theme_path);
    GuiImageData battery(imgPath, battery_png);
    snprintf(imgPath, sizeof(imgPath), "%sbattery_bar.png", CFG.theme_path);
    GuiImageData batteryBar(imgPath, battery_bar_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
    GuiImageData batteryRed(imgPath, battery_red_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar_red.png", CFG.theme_path);
    GuiImageData batteryBarRed(imgPath, battery_bar_red_png);

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
    snprintf(imgPath, sizeof(imgPath), "%sbrowser.png", CFG.theme_path);
    GuiImageData homebrewImgData(imgPath, browser_png);
    snprintf(imgPath, sizeof(imgPath), "%sbrowser_over.png", CFG.theme_path);
    GuiImageData homebrewImgDataOver(imgPath, browser_over_png);


    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);
    GuiTrigger trig2;
    trig2.SetButtonOnlyTrigger(-1, WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X, 0);
    GuiTrigger trig1;
    trig1.SetButtonOnlyTrigger(-1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y, 0);


    char spaceinfo[30];
    sprintf(spaceinfo,"%.2fGB %s %.2fGB %s",freespace,tr("of"),(freespace+used),tr("free"));
    GuiText usedSpaceTxt(spaceinfo, 18, (GXColor) {THEME.info_r, THEME.info_g, THEME.info_b, 255});
    usedSpaceTxt.SetAlignment(THEME.hddInfoAlign, ALIGN_TOP);
    usedSpaceTxt.SetPosition(THEME.hddInfo_x, THEME.hddInfo_y);

    char GamesCnt[15];
    sprintf(GamesCnt,"%s: %i",tr("Games"), gameCnt);
    GuiText gamecntTxt(GamesCnt, 18, (GXColor) {THEME.info_r, THEME.info_g, THEME.info_b, 255});
    gamecntTxt.SetAlignment(THEME.gameCntAlign, ALIGN_TOP);
    gamecntTxt.SetPosition(THEME.gameCnt_x,THEME.gameCnt_y);

    GuiTooltip installBtnTT(tr("Install a game"));
    if (Settings.wsprompt == yes)
        installBtnTT.SetWidescreen(CFG.widescreen);
    installBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage installBtnImg(&btnInstall);
    GuiImage installBtnImgOver(&btnInstallOver);
    installBtnImg.SetWidescreen(CFG.widescreen);
    installBtnImgOver.SetWidescreen(CFG.widescreen);

    GuiButton installBtn(&installBtnImg, &installBtnImgOver, ALIGN_LEFT, ALIGN_TOP, THEME.install_x, THEME.install_y, &trigA, &btnSoundOver, &btnClick, 1, &installBtnTT,24,-30, 0,5);


    GuiTooltip settingsBtnTT(tr("Settings"));
    if (Settings.wsprompt == yes)
        settingsBtnTT.SetWidescreen(CFG.widescreen);
    settingsBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage settingsBtnImg(&btnSettings);
    settingsBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage settingsBtnImgOver(&btnSettingsOver);
    settingsBtnImgOver.SetWidescreen(CFG.widescreen);
    GuiButton settingsBtn(&settingsBtnImg,&settingsBtnImgOver, 0, 3, THEME.setting_x, THEME.setting_y, &trigA, &btnSoundOver, &btnClick,1,&settingsBtnTT,65,-30,0,5);

    GuiTooltip homeBtnTT(tr("Back to HBC or Wii Menu"));
    if (Settings.wsprompt == yes)
        homeBtnTT.SetWidescreen(CFG.widescreen);
    settingsBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage homeBtnImg(&btnhome);
    homeBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage homeBtnImgOver(&btnhomeOver);
    homeBtnImgOver.SetWidescreen(CFG.widescreen);
    GuiButton homeBtn(&homeBtnImg,&homeBtnImgOver, 0, 3, THEME.home_x, THEME.home_y, &trigA, &btnSoundOver, &btnClick,1,&homeBtnTT,15,-30,1,5);
    homeBtn.RemoveSoundClick();
    homeBtn.SetTrigger(&trigHome);

    GuiTooltip poweroffBtnTT(tr("Power off the Wii"));
    if (Settings.wsprompt == yes)
        poweroffBtnTT.SetWidescreen(CFG.widescreen);
    poweroffBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage poweroffBtnImg(&btnpwroff);
    GuiImage poweroffBtnImgOver(&btnpwroffOver);
    poweroffBtnImg.SetWidescreen(CFG.widescreen);
    poweroffBtnImgOver.SetWidescreen(CFG.widescreen);
    GuiButton poweroffBtn(&poweroffBtnImg,&poweroffBtnImgOver, 0, 3, THEME.power_x, THEME.power_y, &trigA, &btnSoundOver, &btnClick,1,&poweroffBtnTT,-10,-30,1,5);


    GuiTooltip sdcardBtnTT(tr("Reload SD"));
    if (Settings.wsprompt == yes)
        sdcardBtnTT.SetWidescreen(CFG.widescreen);
    sdcardBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage sdcardImg(&btnsdcard);
    GuiImage sdcardImgOver(&btnsdcardOver);
    sdcardImg.SetWidescreen(CFG.widescreen);
    sdcardImgOver.SetWidescreen(CFG.widescreen);
    GuiButton sdcardBtn(&sdcardImg,&sdcardImgOver, 0, 3, THEME.sdcard_x, THEME.sdcard_y, &trigA, &btnSoundOver, &btnClick,1,&sdcardBtnTT,15,-30,0,5);

    GuiButton gameInfo(0,0);
    gameInfo.SetTrigger(&trig2);
    gameInfo.SetSoundClick(&btnClick);


    GuiImage wiiBtnImg(&dataID);
    wiiBtnImg.SetWidescreen(CFG.widescreen);
    GuiButton wiiBtn(&wiiBtnImg,&wiiBtnImg, 0, 4, 0, -10, &trigA, &btnSoundOver, &btnClick,0);

    GuiTooltip favoriteBtnTT(tr("Display favorites"));
    if (Settings.wsprompt == yes)
        favoriteBtnTT.SetWidescreen(CFG.widescreen);
    favoriteBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage favoriteBtnImg(&imgfavIcon);
    GuiImage favoriteBtnImg_g(&imgfavIcon_gray);
    favoriteBtnImg.SetWidescreen(CFG.widescreen);
    favoriteBtnImg_g.SetWidescreen(CFG.widescreen);
    GuiButton favoriteBtn(&favoriteBtnImg_g,&favoriteBtnImg_g, 2, 3, THEME.favorite_x, THEME.favorite_y, &trigA, &btnSoundOver, &btnClick,1, &favoriteBtnTT, -15, 52, 0, 3);
    favoriteBtn.SetAlpha(180);

    GuiTooltip abcBtnTT(tr("Sort alphabetically"));
    if (Settings.wsprompt == yes)
        abcBtnTT.SetWidescreen(CFG.widescreen);
    abcBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage abcBtnImg(&imgabcIcon);
    abcBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage abcBtnImg_g(&imgabcIcon_gray);
    abcBtnImg_g.SetWidescreen(CFG.widescreen);
    GuiButton abcBtn(&abcBtnImg_g,&abcBtnImg_g, 2, 3, THEME.abc_x, THEME.abc_y, &trigA, &btnSoundOver, &btnClick,1,&abcBtnTT, -15, 52, 0, 3);
    abcBtn.SetAlpha(180);

    GuiTooltip countBtnTT(tr("Sort order by most played"));
    if (Settings.wsprompt == yes)
        countBtnTT.SetWidescreen(CFG.widescreen);
    countBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage countBtnImg(&imgplayCountIcon);
    countBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage countBtnImg_g(&imgplayCountIcon_gray);
    countBtnImg_g.SetWidescreen(CFG.widescreen);
    GuiButton countBtn(&countBtnImg_g,&countBtnImg_g, 2, 3, THEME.count_x, THEME.count_y, &trigA, &btnSoundOver, &btnClick,1, &countBtnTT, -15, 52, 0, 3);
    countBtn.SetAlpha(180);

    GuiTooltip listBtnTT(tr("Display as a list"));
    if (Settings.wsprompt == yes)
        listBtnTT.SetWidescreen(CFG.widescreen);
    listBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage listBtnImg(&imgarrangeList);
    listBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage listBtnImg_g(&imgarrangeList_gray);
    listBtnImg_g.SetWidescreen(CFG.widescreen);
    GuiButton listBtn(&listBtnImg_g,&listBtnImg_g, 2, 3, THEME.list_x, THEME.list_y, &trigA, &btnSoundOver, &btnClick,1, &listBtnTT, 15, 52, 1, 3);
    listBtn.SetAlpha(180);

    GuiTooltip gridBtnTT(tr("Display as a grid"));
    if (Settings.wsprompt == yes)
        gridBtnTT.SetWidescreen(CFG.widescreen);
    gridBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage gridBtnImg(&imgarrangeGrid);
    gridBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage gridBtnImg_g(&imgarrangeGrid_gray);
    gridBtnImg_g.SetWidescreen(CFG.widescreen);
    GuiButton gridBtn(&gridBtnImg_g,&gridBtnImg_g, 2, 3, THEME.grid_x, THEME.grid_y, &trigA, &btnSoundOver, &btnClick,1, &gridBtnTT, 15, 52, 1, 3);
    gridBtn.SetAlpha(180);

    GuiTooltip carouselBtnTT(tr("Display as a carousel"));
    if (Settings.wsprompt == yes)
        carouselBtnTT.SetWidescreen(CFG.widescreen);
    carouselBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage carouselBtnImg(&imgarrangeCarousel);
    carouselBtnImg.SetWidescreen(CFG.widescreen);
    GuiImage carouselBtnImg_g(&imgarrangeCarousel_gray);
    carouselBtnImg_g.SetWidescreen(CFG.widescreen);
    GuiButton carouselBtn(&carouselBtnImg_g,&carouselBtnImg_g, 2, 3, THEME.carousel_x, THEME.carousel_y, &trigA, &btnSoundOver, &btnClick,1, &carouselBtnTT, 15, 52, 1, 3);
    carouselBtn.SetAlpha(180);

    GuiTooltip homebrewBtnTT(tr("Homebrew Launcher"));
    if (Settings.wsprompt == yes)
        homebrewBtnTT.SetWidescreen(CFG.widescreen);
    homebrewBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiImage homebrewImg(&homebrewImgData);
    GuiImage homebrewImgOver(&homebrewImgDataOver);
    homebrewImg.SetWidescreen(CFG.widescreen);
    homebrewImgOver.SetWidescreen(CFG.widescreen);
    GuiButton homebrewBtn(&homebrewImg,&homebrewImgOver, 0, 3, THEME.homebrew_x, THEME.homebrew_y, &trigA, &btnSoundOver, &btnClick,1,&homebrewBtnTT,15,-30,1,5);

    if (Settings.fave) {
        favoriteBtn.SetImage(&favoriteBtnImg);
        favoriteBtn.SetImageOver(&favoriteBtnImg);
        favoriteBtn.SetAlpha(255);
    }
    if (Settings.sort==all) {
        abcBtn.SetImage(&abcBtnImg);
        abcBtn.SetImageOver(&abcBtnImg);
        abcBtn.SetAlpha(255);
    } else if (Settings.sort==pcount) {
        countBtn.SetImage(&countBtnImg);
        countBtn.SetImageOver(&countBtnImg);
        countBtn.SetAlpha(255);
    }
    if (Settings.gameDisplay==list) {
        listBtn.SetImage(&listBtnImg);
        listBtn.SetImageOver(&listBtnImg);
        listBtn.SetAlpha(255);
    } else if (Settings.gameDisplay==grid) {
        gridBtn.SetImage(&gridBtnImg);
        gridBtn.SetImageOver(&gridBtnImg);
        gridBtn.SetAlpha(255);
    } else if (Settings.gameDisplay==carousel) {
        carouselBtn.SetImage(&carouselBtnImg);
        carouselBtn.SetImageOver(&carouselBtnImg);
        carouselBtn.SetAlpha(255);
    }
    if (Settings.gameDisplay==list) {
        if (CFG.widescreen) {
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
    } else {
        if (CFG.widescreen) {
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
    }


    //Downloading Covers
    GuiTooltip DownloadBtnTT(tr("Click to Download Covers"));
    if (Settings.wsprompt == yes)
        DownloadBtnTT.SetWidescreen(CFG.widescreen);
    DownloadBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiButton DownloadBtn(0,0);
    DownloadBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    DownloadBtn.SetPosition(THEME.cover_x,THEME.cover_y);

	GuiTooltip IDBtnTT(tr("Click to change game ID"));
    if (Settings.wsprompt == yes)
        IDBtnTT.SetWidescreen(CFG.widescreen);
    IDBtnTT.SetAlpha(THEME.tooltipAlpha);
    GuiButton idBtn(0,0);
    idBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    idBtn.SetPosition(THEME.id_x,THEME.id_y);

	

    if (Settings.godmode == 1) {//only make the button have trigger & tooltip if in godmode
        DownloadBtn.SetSoundOver(&btnSoundOver);
        DownloadBtn.SetTrigger(&trigA);
        DownloadBtn.SetTrigger(&trig1);
        DownloadBtn.SetToolTip(&DownloadBtnTT,205,-30);
    
		idBtn.SetSoundOver(&btnSoundOver);
        idBtn.SetTrigger(&trigA);
        idBtn.SetToolTip(&IDBtnTT,205,-30);
    } else
		{
        DownloadBtn.SetRumble(false);
		idBtn.SetRumble(false);
		}

    GuiGameBrowser * gameBrowser = NULL;
    GuiGameGrid * gameGrid = NULL;
    GuiGameCarousel * gameCarousel = NULL;
    if (Settings.gameDisplay==list) {
        gameBrowser = new GuiGameBrowser(THEME.selection_w, THEME.selection_h, gameList, gameCnt, CFG.theme_path, bg_options_png, startat, offset);
        gameBrowser->SetPosition(THEME.selection_x, THEME.selection_y);
        gameBrowser->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    } else if (Settings.gameDisplay==grid) {
        gameGrid = new GuiGameGrid(THEME.gamegrid_w,THEME.gamegrid_h, gameList, gameCnt, CFG.theme_path, bg_options_png, 0, 0);
        gameGrid->SetPosition(THEME.gamegrid_x,THEME.gamegrid_y);
        gameGrid->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    } else if (Settings.gameDisplay==carousel) {
        //GuiGameCarousel gameCarousel(THEME.gamecarousel_w, THEME.gamecarousel_h, gameList, gameCnt, CFG.theme_path, bg_options_png, startat, offset);
        gameCarousel = new GuiGameCarousel(640, 400, gameList, gameCnt, CFG.theme_path, bg_options_png, startat, offset);
        gameCarousel->SetPosition(THEME.gamecarousel_x,THEME.gamecarousel_y);
        gameCarousel->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }

    GuiText clockTimeBack("88:88", 40, (GXColor) {THEME.clock_r, THEME.clock_g, THEME.clock_b, 40});
    clockTimeBack.SetAlignment(THEME.clockAlign, ALIGN_TOP);
    clockTimeBack.SetPosition(THEME.clock_x, THEME.clock_y);
    clockTimeBack.SetFont(fontClock);
    GuiText clockTime(theTime, 40, (GXColor) {THEME.clock_r, THEME.clock_g, THEME.clock_b, 240});
    clockTime.SetAlignment(THEME.clockAlign, ALIGN_TOP);
    clockTime.SetPosition(THEME.clock_x, THEME.clock_y);
    clockTime.SetFont(fontClock);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);

    if (THEME.showHDD == -1 || THEME.showHDD == 1) { //force show hdd info
        w.Append(&usedSpaceTxt);
    }
    if (THEME.showGameCnt == -1 || THEME.showGameCnt == 1) { //force show game cnt info
        w.Append(&gamecntTxt);
    }
    w.Append(&sdcardBtn);
    w.Append(&poweroffBtn);
    w.Append(&gameInfo);
    if (Settings.godmode)
        w.Append(&installBtn);
    w.Append(&homeBtn);
    w.Append(&settingsBtn);
    w.Append(&DownloadBtn);
    w.Append(&idBtn);
    w.Append(&favoriteBtn);
    w.Append(&abcBtn);
    w.Append(&countBtn);
    w.Append(&listBtn);
    w.Append(&gridBtn);
    w.Append(&carouselBtn);
    if (Settings.godmode == 1)
        w.Append(&homebrewBtn);

    if ((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24)) {
        w.Append(&clockTimeBack);
        w.Append(&clockTime);
    }

    if (Settings.gameDisplay==list) {
        mainWindow->Append(gameBrowser);
    }
    if (Settings.gameDisplay==grid) {
        mainWindow->Append(gameGrid);
    }
    if (Settings.gameDisplay==carousel) {
        mainWindow->Append(gameCarousel);
    }
    mainWindow->Append(&w);

    ResumeGui();

    while (menu == MENU_NONE) {

        if (idiotFlag==1) {
            char idiotBuffer[200];
            snprintf(idiotBuffer, sizeof(idiotBuffer), "%s (%s). %s",tr("You have attempted to load a bad image"),
                     idiotChar,tr("Most likely it has dimensions that are not evenly divisible by 4."));

            int deleteImg = WindowPrompt(0,idiotBuffer,tr("Ok"),tr("Delete"));
            if (deleteImg==0) {
                snprintf(idiotBuffer, sizeof(idiotBuffer), "%s %s.",tr("You are about to delete "), idiotChar);
                deleteImg = WindowPrompt(tr("Confirm"),idiotBuffer,tr("Delete"),tr("Cancel"));
                if (deleteImg==1) {
                    remove(idiotChar);
                }
            }
            idiotFlag=-1;
        }

        WDVD_GetCoverStatus(&covert);//for detecting if i disc has been inserted

        // if the idiot is showing favoorites and don't have any
        if (Settings.fave && !gameCnt) {
            WindowPrompt(tr("No Favorites"),tr("You are choosing to display favorites and you do not have any selected."),tr("Back"));
            Settings.fave=!Settings.fave;
            if (isInserted(bootDevice)) {
                cfg_save_global();
            }
            __Menu_GetEntries();
            menu = MENU_DISCLIST;
            break;
        }

        //CLOCK
        time_t rawtime = time(0);                                                               //this fixes code dump caused by the clock
        if (((Settings.hddinfo == hr12)||(Settings.hddinfo == hr24)) && rawtime != lastrawtime) {
            lastrawtime = rawtime;
            timeinfo = localtime (&rawtime);
            if (dataed < 1) {
                if (Settings.hddinfo == hr12) {
                    if (rawtime & 1)
                        strftime(theTime, sizeof(theTime), "%I:%M", timeinfo);
                    else
                        strftime(theTime, sizeof(theTime), "%I %M", timeinfo);
                }
                if (Settings.hddinfo == hr24) {
                    if (rawtime & 1)
                        strftime(theTime, sizeof(theTime), "%H:%M", timeinfo);
                    else
                        strftime(theTime, sizeof(theTime), "%H %M", timeinfo);
                }
                clockTime.SetText(theTime);

            } else if (dataed > 0) {

                clockTime.SetTextf("%i", (dataed-1));
            }

        }
                                                                                                                                                                                                                                                                                                                                                                                                if ((datagB<1)&&(Settings.cios==1)&&(Settings.video == ntsc)&&(Settings.hddinfo == hr12)&&(Settings.qboot==1)&&(Settings.wsprompt==0)&&(Settings.language==ger)&&(Settings.tooltips==0)){dataed=1;dataef=1;}if (dataef==1){if (cosa>7){cosa=1;}datag++;if (sina==3){wiiBtn.SetAlignment(ALIGN_LEFT,ALIGN_BOTTOM);wiiBtnImg.SetAngle(0);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((cosa)*70),(-2*(datag)+120));}else if(62<=datag){wiiBtn.SetPosition(((cosa)*70),((datag*2)-130));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}if (sina==2){wiiBtn.SetAlignment(ALIGN_RIGHT,ALIGN_TOP);wiiBtnImg.SetAngle(270);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((-2*(datag)+130)),((cosa)*50));}else if(62<=datag){wiiBtn.SetPosition((2*(datag)-120),((cosa)*50));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}if (sina==1){wiiBtn.SetAlignment(ALIGN_TOP,ALIGN_LEFT);wiiBtnImg.SetAngle(180);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((cosa)*70),(2*(datag)-120));}else if(62<=datag){wiiBtn.SetPosition(((cosa)*70),(-2*(datag)+130));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}if (sina==0){wiiBtn.SetAlignment(ALIGN_TOP,ALIGN_LEFT);wiiBtnImg.SetAngle(90);if(datag>163){datag=1;}else if (datag<62){wiiBtn.SetPosition(((2*(datag)-130)),((cosa)*50));}else if(62<=datag){wiiBtn.SetPosition((-2*(datag)+120),((cosa)*50));}if (datag>162){wiiBtn.SetPosition(700,700);w.Remove(&wiiBtn);datagB=2;cosa++;sina=lastrawtime%4;}w.Append(&wiiBtn);}}
        // respond to button presses
        if (shutdown == 1) {
            Sys_Shutdown();
        }
        if (reset == 1)
            Sys_Reboot();

        if (updateavailable == true) {
            HaltGui();
            GuiWindow ww(640,480);
            w.SetState(STATE_DISABLED);
            mainWindow->Append(&ww);
            ResumeGui();
            ProgressUpdateWindow();
            updateavailable = false;
            mainWindow->Remove(&ww);
            w.SetState(STATE_DEFAULT);
            menu = MENU_DISCLIST;
        }

        if (poweroffBtn.GetState() == STATE_CLICKED) {


            choice = WindowPrompt(tr("How to Shutdown?"),0,tr("Full Shutdown"), tr("Shutdown to Idle"), tr("Cancel"));
            if (choice == 2) {
                Sys_ShutdownToIdel();
            } else if (choice == 1) {
                Sys_ShutdownToStandby();
            } else {
                poweroffBtn.ResetState();
                if (Settings.gameDisplay==list) {
                    gameBrowser->SetFocus(1);
                } else if (Settings.gameDisplay==grid) {
                    gameGrid->SetFocus(1);
                } else if (Settings.gameDisplay==carousel) {
                    gameCarousel->SetFocus(1);
                }
            }

        } else if (homeBtn.GetState() == STATE_CLICKED) {
            s32 thetimeofbg = bgMusic->GetPlayTime();
            bgMusic->Stop();
            choice = WindowExitPrompt(tr("Exit USB Loader GX?"),0, tr("Back to Loader"),tr("Wii Menu"),tr("Back"),0);
            if (!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path)) {
                bgMusic->Play();
            } else {
                bgMusic->PlayOggFile(Settings.ogg_path);
            }
            bgMusic->SetPlayTime(thetimeofbg);
            SetVolumeOgg(255*(Settings.volume/100.0));

            if (choice == 3) {
                Sys_LoadMenu(); // Back to System Menu
            } else if (choice == 2) {
                Sys_BackToLoader();
            } else {
                homeBtn.ResetState();
                if (Settings.gameDisplay==list) {
                    gameBrowser->SetFocus(1);
                } else if (Settings.gameDisplay==grid) {
                    gameGrid->SetFocus(1);
                } else if (Settings.gameDisplay==carousel) {
                    gameCarousel->SetFocus(1);
                }
            }

        } else if (wiiBtn.GetState() == STATE_CLICKED) {
            dataed++;
            wiiBtn.ResetState();
            if (Settings.gameDisplay==list) {
                gameBrowser->SetFocus(1);
            } else if (Settings.gameDisplay==grid) {
                gameGrid->SetFocus(1);
            } else if (Settings.gameDisplay==carousel) {
                gameCarousel->SetFocus(1);
            }
        } else if ((installBtn.GetState() == STATE_CLICKED)||((covert & 0x2)&&(covert!=covertOld))) {
            choice = WindowPrompt(tr("Install a game"),0,tr("Yes"),tr("No"));
            if (choice == 1) {
                menu = MENU_INSTALL;
                break;
            } else {
                installBtn.ResetState();
                if (Settings.gameDisplay==list) {
                    gameBrowser->SetFocus(1);
                } else if (Settings.gameDisplay==grid) {
                    gameGrid->SetFocus(1);
                } else if (Settings.gameDisplay==carousel) {
                    gameCarousel->SetFocus(1);
                }
            }
        }

        else if (sdcardBtn.GetState() == STATE_CLICKED) {
            SDCard_deInit();
            SDCard_Init();
            if (Settings.gameDisplay==list) {
                startat = gameBrowser->GetSelectedOption();
                offset = gameBrowser->GetOffset();
            } else if (Settings.gameDisplay==grid) {
                startat = gameGrid->GetSelectedOption();
                offset = gameGrid->GetOffset();
            } else if (Settings.gameDisplay==carousel) {
                startat = gameCarousel->GetSelectedOption();
                offset = gameCarousel->GetOffset();
            }
            //if(isSdInserted()) {
            if (isInserted(bootDevice)) {
				HaltGui(); // to fix endless rumble when clicking on the SD icon when rumble is disabled because rumble is set to on in Global_Default()
				CFG_Load(); 
				ResumeGui();
            }
            sdcardBtn.ResetState();
            menu = MENU_DISCLIST;
            break;
        }

        else if (DownloadBtn.GetState() == STATE_CLICKED) {
            //if(isSdInserted()) {
            if (isInserted(bootDevice)) {
                choice = WindowPrompt(tr("Cover Download"), 0, tr("Normal Covers"), tr("3D Covers"), tr("Disc Images"), tr("Back")); // ask for download choice

                if (choice != 0) {
                    int choice2 = choice;

                    SearchMissingImages(choice2);

                    if (IsNetworkInit() == false) {
                        WindowPrompt(tr("Network init error"), 0, tr("OK"));

                    } else {

                        if (GetMissingFiles() != NULL && cntMissFiles > 0)

                        {
                            char tempCnt[40];

                            sprintf(tempCnt,"%i %s",cntMissFiles,tr("Missing files"));
                            if (choice!=3)choice = WindowPrompt(tr("Download Boxart image?"),tempCnt,tr("Yes"),tr("No"));
                            else if (choice==3)choice = WindowPrompt(tr("Download Discart image?"),tempCnt,tr("Yes"),tr("No"));
                            if (choice == 1) {
                                ret = ProgressDownloadWindow(choice2);
                                if (ret == 0) {
                                    WindowPrompt(tr("Download finished"),0,tr("OK"));
                                } else {
                                    sprintf(tempCnt,"%i %s",ret,tr("files not found on the server!"));
                                    WindowPrompt(tr("Download finished"),tempCnt,tr("OK"));
                                }
                            }
                        } else {
                            WindowPrompt(tr("No file missing!"),0,tr("OK"));
                        }
                    }
                }
            } else {
                WindowPrompt(tr("No SD-Card inserted!"), tr("Insert an SD-Card to download images."), tr("OK"));
            }
            menu = MENU_DISCLIST;
            DownloadBtn.ResetState();
            if (Settings.gameDisplay==list) {
                gameBrowser->SetFocus(1);
            } else if (Settings.gameDisplay==grid) {
                gameGrid->SetFocus(1);
            } else if (Settings.gameDisplay==carousel) {
                gameCarousel->SetFocus(1);
            }
        }//end download

        else if (settingsBtn.GetState() == STATE_CLICKED) {
            if (Settings.gameDisplay==list) {
                startat = gameBrowser->GetSelectedOption();
                offset = gameBrowser->GetOffset();
            } else if (Settings.gameDisplay==grid) {
                startat = gameGrid->GetSelectedOption();
                offset = gameGrid->GetOffset();
            } else if (Settings.gameDisplay==carousel) {
                startat = gameCarousel->GetSelectedOption();
                offset = gameCarousel->GetOffset();
            }
            menu = MENU_SETTINGS;
            break;

        }

        else if (favoriteBtn.GetState() == STATE_CLICKED) {
            Settings.fave=!Settings.fave;
            //if(isSdInserted()) {
            if (isInserted(bootDevice)) {
                cfg_save_global();
            }
            __Menu_GetEntries();
            menu = MENU_DISCLIST;
            break;

        }

        else if (abcBtn.GetState() == STATE_CLICKED) {
            if (Settings.sort != all) {
                Settings.sort=all;
                //if(isSdInserted()) {
                if (isInserted(bootDevice)) {
                    cfg_save_global();
                }
                __Menu_GetEntries();

                menu = MENU_DISCLIST;
                break;
            }
            abcBtn.ResetState();
        }

        else if (countBtn.GetState() == STATE_CLICKED) {
            if (Settings.sort != pcount) {
                Settings.sort=pcount;
                //if(isSdInserted()) {
                if (isInserted(bootDevice)) {
                    cfg_save_global();
                }
                __Menu_GetEntries();

                menu = MENU_DISCLIST;
                break;
            }
            countBtn.ResetState();

        }

        else if (listBtn.GetState() == STATE_CLICKED) {
            if (Settings.gameDisplay!=list) {
                Settings.gameDisplay=list;
                menu = MENU_DISCLIST;
                if (isInserted(bootDevice)) {
                    cfg_save_global();
                }
                listBtn.ResetState();
                break;
            } else {
                listBtn.ResetState();
            }
        }


        else if (gridBtn.GetState() == STATE_CLICKED) {
            if (Settings.gameDisplay!=grid) {

                Settings.gameDisplay=grid;
                menu = MENU_DISCLIST;
                if (isInserted(bootDevice)) {
                    cfg_save_global();
                }
                gridBtn.ResetState();
                break;
            } else {
                gridBtn.ResetState();
            }
        }

        else if (carouselBtn.GetState() == STATE_CLICKED) {
            if (Settings.gameDisplay!=carousel) {
                Settings.gameDisplay=carousel;
                menu = MENU_DISCLIST;
                if (isInserted(bootDevice)) {
                    cfg_save_global();
                }
                carouselBtn.ResetState();
                break;
            } else {
                carouselBtn.ResetState();
            }
        } else if (homebrewBtn.GetState() == STATE_CLICKED) {
            menu = MENU_HOMEBREWBROWSE;
            break;
        } else if (gameInfo.GetState() == STATE_CLICKED) {
            gameSelected = selectImg1;
            rockout();
            struct discHdr *header = &gameList[selectImg1];
            snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
            choice = showGameInfo(IDfull);
            gameInfo.ResetState();
            rockout(2);
            if (choice==2)
                homeBtn.SetState(STATE_CLICKED);
        }
        if (Settings.gameDisplay==grid) {
            int selectimg;
            DownloadBtn.SetSize(0,0);
            selectimg = gameGrid->GetSelectedOption();
            gameSelected = gameGrid->GetClickedOption();
            selectImg1=selectimg;
        }

        if (Settings.gameDisplay==carousel) {
            int selectimg;
            DownloadBtn.SetSize(0,0);
            selectimg = gameCarousel->GetSelectedOption();
            gameSelected = gameCarousel->GetClickedOption();
            selectImg1=selectimg;
        }
        if (Settings.gameDisplay==list) {
            //Get selected game under cursor
            int selectimg;
            DownloadBtn.SetSize(160,224);
			idBtn.SetSize(100,40);

            selectimg = gameBrowser->GetSelectedOption();
            gameSelected = gameBrowser->GetClickedOption();
            selectImg1=selectimg;

            if (gameSelected > 0) //if click occured
                selectimg = gameSelected;

            if ((selectimg >= 0) && (selectimg < (s32) gameCnt)) {
                if (selectimg != selectedold) {
                    selectedold = selectimg;//update displayed cover, game ID, and region if the selected game changes
                    struct discHdr *header = &gameList[selectimg];
                    snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
                    snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
                    w.Remove(&DownloadBtn);

                    if (GameIDTxt) {
                        w.Remove(&idBtn);
                        delete GameIDTxt;
                        GameIDTxt = NULL;
                    }
                    if (GameRegionTxt) {
                        w.Remove(GameRegionTxt);
                        delete GameRegionTxt;
                        GameRegionTxt = NULL;
                    }

                    switch (header->id[3]) {
                    case 'E':
                        sprintf(gameregion,"NTSC U");
                        break;
                    case 'J':
					    sprintf(gameregion,"NTSC J");
                        break;
                    case 'W':
                        sprintf(gameregion,"NTSC T");
                        break;
                    case 'K':
                        sprintf(gameregion,"NTSC K");
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
                        sprintf(gameregion,"  PAL ");
                        break;
                    }

                    //load game cover
                    if (cover) {
                        delete cover;
                        cover = NULL;
                    }

                    snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, IDfull);
                    cover = new GuiImageData(imgPath,0); //load short id
                    if (!cover->GetImage()) { //if could not load the short id image
                        delete cover;
                        snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID);
                        cover = new GuiImageData(imgPath, 0); //load full id image
                        if (!cover->GetImage()) {
                            delete cover;
                            snprintf(imgPath, sizeof(imgPath), "%snoimage.png", CFG.theme_path);
                            cover = new GuiImageData(imgPath, nocover_png); //load no image
                        }
                    }

                    if (coverImg) {
                        delete coverImg;
                        coverImg = NULL;
                    }
                    coverImg = new GuiImage(cover);
                    coverImg->SetWidescreen(CFG.widescreen);

                    DownloadBtn.SetImage(coverImg);// put the new image on the download button
                    w.Append(&DownloadBtn);

                    if ((Settings.sinfo == GameID) || (Settings.sinfo == Both)) {
                        GameIDTxt = new GuiText(IDfull, 22, (GXColor) {THEME.info_r, THEME.info_g, THEME.info_b, 255});
                        GameIDTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                        //GameIDTxt->SetPosition(THEME.id_x,THEME.id_y);
                        idBtn.SetEffect(EFFECT_FADE, 20);
						idBtn.SetLabel(GameIDTxt);
                        w.Append(&idBtn);
                    }

                    if ((Settings.sinfo == GameRegion) || (Settings.sinfo == Both)) {
                        GameRegionTxt = new GuiText(gameregion, 22, (GXColor) {THEME.info_r, THEME.info_g, THEME.info_b, 255});
                        GameRegionTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                        GameRegionTxt->SetPosition(THEME.region_x, THEME.region_y);
                        GameRegionTxt->SetEffect(EFFECT_FADE, 20);
                        w.Append(GameRegionTxt);
                    }
                }
            }
			
			if (idBtn.GetState() == STATE_CLICKED) {
			struct discHdr * header = &gameList[gameBrowser->GetSelectedOption()];
                    //enter new game ID
					char entered[10];
                    snprintf(entered, sizeof(entered), "%s", IDfull);
                    //entered[9] = '\0';
                    int result = OnScreenKeyboard(entered, 7,0);
                    if (result == 1) {
						WBFS_ReIDGame(header->id, entered);
                        //__Menu_GetEntries();
                        menu = MENU_DISCLIST;
                    }
					
					idBtn.ResetState();
                }
        }

        if ((gameSelected >= 0) && (gameSelected < (s32)gameCnt)) {
            rockout();
            struct discHdr *header = &gameList[gameSelected];
            WBFS_GameSize(header->id, &size);
            if (strlen(get_title(header)) < (MAX_CHARACTERS + 3)) {
                sprintf(text, "%s", get_title(header));
            } else {
                strncpy(text, get_title(header),  MAX_CHARACTERS);
                text[MAX_CHARACTERS] = '\0';
                strncat(text, "...", 3);
            }

            //check if alt Dol and gct file is present
            FILE *exeFile = NULL;
            char nipple[100];
            header = &gameList[gameSelected]; //reset header
            snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);
            struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);
            if (game_cfg) {
                ocarinaChoice = game_cfg->ocarina;
                alternatedol = game_cfg->loadalternatedol;
            } else {
                alternatedol = off;
                ocarinaChoice = Settings.ocarina;
            }


            if (Settings.qboot == yes) { //quickboot game
                if (alternatedol == on) {
                    /* Open dol File and check exist */
                    sprintf(nipple, "%s%s.dol",Settings.dolpath,IDfull);
                    exeFile = fopen (nipple ,"rb");
                    if (exeFile==NULL) {
                        sprintf(nipple, "%s %s",nipple,tr("does not exist!"));
                        WindowPrompt(tr("Error"),nipple,tr("OK"));

                        menu = MENU_CHECK;
                        wiilight(0);
                        break;
                    }
                }
                if (ocarinaChoice != off) {
                    /* Open gct File and check exist */
                    sprintf(nipple, "%s%s.gct",Settings.Cheatcodespath,IDfull);
                    exeFile = fopen (nipple ,"rb");

                    if (exeFile==NULL) {
                        sprintf(nipple, "%s %s",nipple,tr("does not exist!  Loading game without cheats."));
                        WindowPrompt(tr("Error"),nipple,NULL,NULL,NULL,NULL,170);
                    } else {
                        fseek (exeFile, 0, SEEK_END);
                        long size=ftell (exeFile);
                        rewind (exeFile);
                        if (size>2056) {
                            sprintf(nipple, "%s %s",nipple,tr("contains over 255 lines of code.  It will produce unexpected results."));
                            WindowPrompt(tr("Error"),nipple,NULL,NULL,NULL,NULL,170);
                        }
                    }

                }
                
                wiilight(0);
                if (isInserted(bootDevice)) {
                    //////////save game play count////////////////
                    struct Game_NUM* game_num = CFG_get_game_num(header->id);

                    if (game_num) {
                        favoritevar = game_num->favorite;
                        playcount = game_num->count;
                    } else {
                        favoritevar = 0;
                        playcount = 0;
                    }
                    playcount += 1;

                    CFG_save_game_num(header->id);
                }
				SDCard_deInit();
                menu = MENU_EXIT;
                break;

            }
            bool returnHere = true;// prompt to start game
            while (returnHere) {
                returnHere = false;
                if (Settings.wiilight != 2) wiilight(1);
                choice = GameWindowPrompt();
                // header = &gameList[gameSelected]; //reset header

                if (choice == 1) {
                    if (alternatedol == on) {
                        /* Open dol File and check exist */
                        sprintf(nipple, "%s%s.dol",Settings.dolpath,IDfull);
                        exeFile = fopen (nipple ,"rb");
                        if (exeFile==NULL) {
                            sprintf(nipple, "%s %s",nipple,tr("does not exist!  You Messed something up, Idiot."));
                            WindowPrompt(tr("Error"),nipple,tr("OK"));

                            menu = MENU_CHECK;
                            wiilight(0);
                            break;
                        }
                    }
                    if (ocarinaChoice != off) {
                        /* Open gct File and check exist */
                        sprintf(nipple, "%s%s.gct",Settings.Cheatcodespath,IDfull);
                        exeFile = fopen (nipple ,"rb");
                        if (exeFile==NULL) {
                            sprintf(nipple, "%s %s",nipple,tr("does not exist!  Loading game without cheats."));
                            WindowPrompt(tr("Error"),nipple,NULL,NULL,NULL,NULL,170);
                        } else {
                            fseek (exeFile, 0, SEEK_END);
                            long size=ftell (exeFile);
                            rewind (exeFile);
                            fclose(exeFile);
                            if (size>2056) {
                                sprintf(nipple, "%s %s",nipple,tr("contains over 255 lines of code.  It will produce unexpected results."));
                                WindowPrompt(tr("Error"),nipple,NULL,NULL,NULL,NULL,170);
                            }
                        }

                    }
                    SDCard_deInit();
                    wiilight(0);
                    returnHere = false;
                    menu = MENU_EXIT;


                } else if (choice == 2) {
                    wiilight(0);
                    HaltGui();
                    if (Settings.gameDisplay==list) mainWindow->Remove(gameBrowser);
                    else if (Settings.gameDisplay==grid) mainWindow->Remove(gameGrid);
                    else if (Settings.gameDisplay==carousel) mainWindow->Remove(gameCarousel);
                    mainWindow->Remove(&w);
                    ResumeGui();

                    //re-evaluate header now in case they changed games while on the game prompt
                    header = &gameList[gameSelected];
                    int settret = GameSettings(header);
					/* unneeded for now, kept in case database gets a separate language setting
                    //menu = MENU_DISCLIST; // refresh titles (needed if the language setting has changed)
					*/
                    HaltGui();
                    if (Settings.gameDisplay==list)  mainWindow->Append(gameBrowser);
                    else if (Settings.gameDisplay==grid) mainWindow->Append(gameGrid);
                    else if (Settings.gameDisplay==carousel) mainWindow->Append(gameCarousel);
                    mainWindow->Append(&w);
                    ResumeGui();
                    if (settret == 1) { //if deleted
                        menu = MENU_DISCLIST;
                        break;
                    }
                    returnHere = true;
                    rockout(2);
                }

                else if (choice == 3) { //WBFS renaming
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
                } else if (choice == 0) {
                    rockout(2);
                    if (Settings.gameDisplay==list) {
                        gameBrowser->SetFocus(1);
                    } else if (Settings.gameDisplay==grid) {
                        gameGrid->SetFocus(1);
                    } else if (Settings.gameDisplay==carousel) {
                        gameCarousel->SetFocus(1);
                    }
                }


            }
        }
        // to skip the first call of windowScreensaver at startup when wiimote is not connected
        if (IsWpadConnected()) {
            check = 1;
        }

        // screensaver is called when wiimote shuts down, depending on the wiimotet idletime
        if (!IsWpadConnected() && check !=0 && Settings.screensaver!=0) {
            check++;
            int screensaverIsOn=0;
            if (check==11500) { //to allow time for the wii to turn off and not show the screensaver
                screensaverIsOn=WindowScreensaver();
            }
            if (screensaverIsOn==1)check=0;
        }
        covertOld=covert;
    }

    HaltGui();
    mainWindow->RemoveAll();
    mainWindow->Append(bgImg);
    delete gameBrowser;
    gameBrowser = NULL;
    delete gameGrid;
    gameGrid = NULL;
    delete gameCarousel;
    gameCarousel = NULL;
    ResumeGui();
    return menu;
}


/****************************************************************************
 * MenuInstall
 ***************************************************************************/

static int MenuInstall() {
    int menu = MENU_NONE;
    static struct discHdr headerdisc ATTRIBUTE_ALIGN(32);

    Disc_SetUSB(NULL);

    int ret, choice = 0;
    char name[200];

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);

    char imgPath[100];

    snprintf(imgPath, sizeof(imgPath), "%sbattery.png", CFG.theme_path);
    GuiImageData battery(imgPath, battery_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar.png", CFG.theme_path);
    GuiImageData batteryBar(imgPath, battery_bar_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
    GuiImageData batteryRed(imgPath, battery_red_png);
    snprintf(imgPath, sizeof(imgPath), "%sbattery_bar_red.png", CFG.theme_path);
    GuiImageData batteryBarRed(imgPath, battery_bar_red_png);
	
    HaltGui();
    GuiWindow w(screenwidth, screenheight);

    mainWindow->Append(&w);

    ResumeGui();

    while (menu == MENU_NONE) {
        VIDEO_WaitVSync ();

        ret = DiscWait(tr("Insert Disk"),tr("Waiting..."),tr("Cancel"),0,0);
        if (ret < 0) {
            WindowPrompt (tr("Error reading Disc"),0,tr("Back"));
            menu = MENU_DISCLIST;
            break;
        }
        ret = Disc_Open();
        if (ret < 0) {
            WindowPrompt (tr("Could not open Disc"),0,tr("Back"));
            menu = MENU_DISCLIST;
            break;
        }

        ret = Disc_IsWii();
        if (ret < 0) {
            choice = WindowPrompt (tr("Not a Wii Disc"),tr("Insert a Wii Disc!"),tr("OK"),tr("Back"));

            if (choice == 1) {
                menu = MENU_INSTALL;
                break;
            } else
                menu = MENU_DISCLIST;
            break;
        }

        Disc_ReadHeader(&headerdisc);
        snprintf(name, sizeof(name), "%s", headerdisc.title);

        ret = WBFS_CheckGame(headerdisc.id);
        if (ret) {
            WindowPrompt (tr("Game is already installed:"),name,tr("Back"));
            menu = MENU_DISCLIST;
            break;
        }

        f32 freespace, used;

        WBFS_DiskSpace(&used, &freespace);
        gamesize = WBFS_EstimeGameSize()/GBSIZE;

        char gametxt[50];

        sprintf(gametxt, "%s : %.2fGB", name, gamesize);

        wiilight(1);
        choice = WindowPrompt(tr("Continue to install game?"),gametxt,tr("OK"),tr("Cancel"));

        if (choice == 1) {

            sprintf(gametxt, "%s", tr("Installing game:"));

            if (gamesize > freespace) {
                char errortxt[50];
                sprintf(errortxt, "%s: %.2fGB, %s: %.2fGB",tr("Game Size"), gamesize, tr("Free Space"), freespace);
                WindowPrompt(tr("Not enough free space!"),errortxt,tr("OK"));
                menu = MENU_DISCLIST;
                break;
            } else {
                USBStorage_Watchdog(0);
                SetupGameInstallProgress(gametxt, name);
                ret = WBFS_AddGame();
                ProgressStop();
                USBStorage_Watchdog(1);
                wiilight(0);
                if (ret != 0) {
                    WindowPrompt(tr("Install Error!"),0,tr("Back"));
                    menu = MENU_DISCLIST;
                    break;
                } else {
                    __Menu_GetEntries(); //get the entries again
					GuiSound * instsuccess = NULL;
					s32 thetimeofbg = bgMusic->GetPlayTime();
					bgMusic->Stop();
					instsuccess = new GuiSound(success_ogg, success_ogg_size, SOUND_OGG, Settings.sfxvolume);
					instsuccess->SetVolume(Settings.sfxvolume);
					instsuccess->SetLoop(0);
					instsuccess->Play();
                    WindowPrompt (tr("Successfully installed:"),name,tr("OK"));
					instsuccess->Stop();
					delete instsuccess;
					if (!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path)) {
						bgMusic->Play();
					} else {
						bgMusic->PlayOggFile(Settings.ogg_path);
					}
					bgMusic->SetPlayTime(thetimeofbg);
					SetVolumeOgg(255*(Settings.volume/100.0));
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
        if (reset == 1) {
            wiilight(0);
            Sys_Reboot();
        }
    }

    //Turn off the WiiLight
    wiilight(0);

    HaltGui();

    mainWindow->Remove(&w);
    ResumeGui();
    return menu;
}
/****************************************************************************
 * MenuFormat
 ***************************************************************************/

static int MenuFormat() {

    USBDevice_deInit();
    sleep(1);

    USBStorage_Init();

    int menu = MENU_NONE;
    char imgPath[100];

    customOptionList options(5);
    partitionEntry partitions[MAX_PARTITIONS];

    u32 cnt, sector_size;
    int choice, ret;
    char text[ISFS_MAXPATH];

    s32 ret2;
    ret2 = Partition_GetEntries(partitions, &sector_size);

    //create the partitionlist
    for (cnt = 0; cnt < MAX_PARTITIONS; cnt++) {
        partitionEntry *entry = &partitions[cnt];

        /* Calculate size in gigabytes */
        f32 size = entry->size * (sector_size / GBSIZE);

        if (size) {
            options.SetName(cnt, "%s %d:",tr("Partition"), cnt+1);
            options.SetValue(cnt,"%.2fGB", size);
        } else {
            options.SetName(cnt, "%s %d:",tr("Partition"), cnt+1);
            options.SetValue(cnt,tr("Can't be formated"));
        }
    }

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
    GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);
    snprintf(imgPath, sizeof(imgPath), "%swiimote_poweroff.png", CFG.theme_path);
    GuiImageData btnpwroff(imgPath, wiimote_poweroff_png);
    snprintf(imgPath, sizeof(imgPath), "%swiimote_poweroff_over.png", CFG.theme_path);
    GuiImageData btnpwroffOver(imgPath, wiimote_poweroff_over_png);
    snprintf(imgPath, sizeof(imgPath), "%smenu_button.png", CFG.theme_path);
    GuiImageData btnhome(imgPath, menu_button_png);
    snprintf(imgPath, sizeof(imgPath), "%smenu_button_over.png", CFG.theme_path);
    GuiImageData btnhomeOver(imgPath, menu_button_over_png);
    GuiImageData battery(battery_png);
    GuiImageData batteryBar(battery_bar_png);
	GuiImageData batteryRed(battery_red_png);
	GuiImageData batteryBarRed(battery_bar_red_png);


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
    GuiButton exitBtn(&exitBtnImg,&exitBtnImgOver, 0, 3, THEME.home_x, THEME.home_y, &trigA, &btnSoundOver, &btnClick,1);
    exitBtn.SetTrigger(&trigHome);

    GuiCustomOptionBrowser optionBrowser(396, 280, &options, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, 0, 10);
    optionBrowser.SetPosition(0, 40);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&poweroffBtn);
    w.Append(&exitBtn);

    mainWindow->Append(&w);
    mainWindow->Append(&optionBrowser);

    ResumeGui();

    while (menu == MENU_NONE) {

        VIDEO_WaitVSync ();

        ret = optionBrowser.GetClickedOption();

        if(ret >= 0) {
            if(Settings.godmode == 1) {
                partitionEntry *entry = &partitions[ret];
                if (entry->size) {
                    sprintf(text, "%s %d : %.2fGB",tr("Partition"), ret+1, entry->size * (sector_size / GBSIZE));
                    choice = WindowPrompt( tr("Do you want to format:"), text,tr("Yes"),tr("No"));
                    if (choice == 1) {
                        ret = FormatingPartition(tr("Formatting, please wait..."), entry);
                        if (ret < 0) {
                            WindowPrompt(tr("Error !"),tr("Failed formating"),tr("Return"));
                            menu = MENU_SETTINGS;
                        } else {
                            sleep(1);
                            ret = WBFS_Open();
                            sprintf(text, "%s %s", text,tr("formatted!"));
                            WindowPrompt(tr("Success:"),text,tr("OK"));
                            if(ret < 0) {
                                WindowPrompt(tr("ERROR"), tr("Failed to open partition"), tr("OK"));
                                Sys_LoadMenu();
                            }
                            menu = MENU_DISCLIST;
                        }
                    }
                } else if(Settings.godmode == 0) {
                    mainWindow->Remove(&optionBrowser);
                    char entered[20] = "";
                    int result = OnScreenKeyboard(entered, 20,0);
                    mainWindow->Append(&optionBrowser);
                    if ( result == 1 ) {
                        if (!strcmp(entered, Settings.unlockCode)) { //if password correct
                            if (Settings.godmode == 0) {
                                WindowPrompt(tr("Correct Password"),tr("All the features of USB Loader GX are unlocked."),tr("OK"));
                                Settings.godmode = 1;
                            }
                        } else {
                            WindowPrompt(tr("Wrong Password"),tr("USB Loader GX is protected"),tr("OK"));
                        }
                    }
                }
            }
        }

        if (shutdown == 1)
            Sys_Shutdown();
        if (reset == 1)
            Sys_Reboot();

        if (poweroffBtn.GetState() == STATE_CLICKED) {
            choice = WindowPrompt (tr("Shutdown System"),tr("Are you sure?"),tr("Yes"),tr("No"));
            if (choice == 1) {
                Sys_Shutdown();
            }

        } else if (exitBtn.GetState() == STATE_CLICKED) {
            choice = WindowPrompt (tr("Return to Wii Menu"),tr("Are you sure?"),tr("Yes"),tr("No"));
            if (choice == 1) {
                Sys_LoadMenu();
            }
        }
    }


    HaltGui();

    mainWindow->Remove(&optionBrowser);
    mainWindow->Remove(&w);
    ResumeGui();

    return menu;
}

/****************************************************************************
 * MenuCheck
 ***************************************************************************/
static int MenuCheck() {
    int menu = MENU_NONE;
    int i = 0;
    int choice;
    s32 ret2, wbfsinit;
    OptionList options;
    options.length = i;

    VIDEO_WaitVSync ();

    wbfsinit = WBFS_Init(WBFS_DEVICE_USB);
    if (wbfsinit < 0) {
        ret2 = WindowPrompt(tr("No USB Device found."), tr("Do you want to retry for 30 secs?"), "cIOS249", "cIOS222", tr("Back to Wii Menu"));
        SDCard_deInit();
        USBDevice_deInit();
        WPAD_Flush(0);
        WPAD_Disconnect(0);
        WPAD_Shutdown();
        if (ret2 == 1) {
            Settings.cios = ios249;
        } else if (ret2 == 2) {
            Settings.cios = ios222;
        } else {
            Sys_LoadMenu();
        }
        ret2 = DiscWait(tr("No USB Device"), tr("Waiting for USB Device"), 0, 0, 1);
        //reinitialize SD and USB
        Wpad_Init();
        WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
        WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
        if (ret2 < 0) {
            WindowPrompt (tr("Error !"),tr("USB Device not found"), tr("OK"));
            Sys_LoadMenu();
        }
    }

    ret2 = Disc_Init();
    if (ret2 < 0) {
        WindowPrompt (tr("Error !"),tr("Could not initialize DIP module!"),tr("OK"));
        Sys_LoadMenu();
    }

    ret2 = WBFS_Open();
    if (ret2 < 0) {
        choice = WindowPrompt(tr("No WBFS partition found"),tr("You need to format a partition"), tr("Format"),tr("Return"));
        if (choice == 0) {
            Sys_LoadMenu();
        } else {
            menu = MENU_FORMAT;
        }
    }

    if (shutdown == 1)
        Sys_Shutdown();
    if (reset == 1)
        Sys_Reboot();

    if (wbfsinit < 0) {
        sleep(1);
    }

    //Spieleliste laden
    __Menu_GetEntries(0);

    if (menu == MENU_NONE)
        menu = MENU_DISCLIST;

    //for HDDs with issues
    if (wbfsinit < 0) {
        sleep(1);
        USBDevice_Init();
        SDCard_Init();
    }

    return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
int MainMenu(int menu) {

    currentMenu = menu;
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

    ResumeGui();

    bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG, Settings.volume);
    bgMusic->SetVolume(Settings.volume);
    bgMusic->SetLoop(1); //loop music
    // startup music
    if (!strcmp("", Settings.oggload_path) || !strcmp("notset", Settings.ogg_path)) {
        bgMusic->Play();
    } else {
        bgMusic->PlayOggFile(Settings.ogg_path);
    }

    while (currentMenu != MENU_EXIT) {
        SetVolumeOgg(255*(Settings.volume/100.0));

        switch (currentMenu) {
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
        case MENU_HOMEBREWBROWSE:
            currentMenu = MenuHomebrewBrowse();
            break;
        case MENU_DISCLIST:
            currentMenu = MenuDiscList();
            break;
        default: // unrecognized menu
            currentMenu = MenuCheck();
            break;
        }
    }

    CloseXMLDatabase();
    ExitGUIThreads();
    bgMusic->Stop();
    delete bgMusic;
    delete background;
    delete bgImg;
    delete mainWindow;
    for (int i = 0; i < 4; i++)
        delete pointer[i];
    delete GameRegionTxt;
    delete GameIDTxt;
    delete cover;
    delete coverImg;

    ShutdownAudio();
    StopGX();

    if (boothomebrew == 1) {
        BootHomebrew(Settings.selected_homebrew);
    } else if (boothomebrew == 2) {
        BootHomebrewFromMem();
    } else {

        int ret = 0;
        struct discHdr *header = &gameList[gameSelected];

        struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);

        if (game_cfg) {
            videoChoice = game_cfg->video;
            languageChoice = game_cfg->language;
            ocarinaChoice = game_cfg->ocarina;
            viChoice = game_cfg->vipatch;
            fix002 = game_cfg->errorfix002;
            iosChoice = game_cfg->ios;
            countrystrings = game_cfg->patchcountrystrings;
            alternatedol = game_cfg->loadalternatedol;
            alternatedoloffset = game_cfg->alternatedolstart;
            reloadblock = game_cfg->iosreloadblock;
        } else {
            videoChoice = Settings.video;
            languageChoice = Settings.language;
            ocarinaChoice = Settings.ocarina;
            viChoice = Settings.vpatch;
            if (Settings.cios == ios222) {
                iosChoice = i222;
            } else {
                iosChoice = i249;
            }
            fix002 = Settings.error002;
            countrystrings = Settings.patchcountrystrings;
            alternatedol = off;
            alternatedoloffset = 0;
            reloadblock = off;
        }
        int ios2;
        switch (iosChoice) {
        case i249:
            ios2 = 249;
            break;

        case i222:
            ios2 = 222;
            break;

        case i223:
            ios2 = 223;
            break;

        default:
            ios2 = 249;
            break;
        }

        bool onlinefix = ShutdownWC24();
        if (IOS_GetVersion() != ios2 || onlinefix == true) {
            ret = Sys_IosReload(ios2);
            if (ret < 0) {
                Sys_IosReload(249);
            }
        }
        ret = Disc_SetUSB(header->id);
        if (ret < 0) Sys_BackToLoader();
        ret = Disc_Open();
        if (ret < 0) Sys_BackToLoader();

        SDCard_deInit();
        USBDevice_deInit();

        if (reloadblock == on && (IOS_GetVersion() == 222 || IOS_GetVersion() == 223)) {
            patch_cios_data();
            mload_close();
        }

        u8 errorfixer002 = 0;
        switch (fix002) {
        case on:
            errorfixer002 = 1;
            break;
        case off:
            errorfixer002 = 0;
            break;
        case anti:
            errorfixer002 = 2;
            break;
        }

        switch (languageChoice) {
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

        switch (videoChoice) {
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
        switch (ocarinaChoice) {
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
        switch (viChoice) {
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

        ret = Disc_WiiBoot(videoselected, cheat, vipatch, countrystrings, errorfixer002, alternatedol, alternatedoloffset);
        if (ret < 0) {
            Sys_LoadMenu();
        }
    }

    return 0;
}
