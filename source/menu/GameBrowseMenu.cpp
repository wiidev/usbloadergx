#include <unistd.h>
#include "GameBrowseMenu.hpp"
#include "Controls/DeviceHandler.hpp"
#include "libwiigui/LoadCoverImage.h"
#include "prompts/PromptWindows.h"
#include "prompts/gameinfo.h"
#include "prompts/DiscBrowser.h"
#include "prompts/GameWindow.hpp"
#include "themes/CTheme.h"
#include "language/gettext.h"
#include "usbloader/wbfs.h"
#include "usbloader/wdvd.h"
#include "usbloader/GameList.h"
#include "network/networkops.h"
#include "network/update.h"
#include "network/CoverDownload.h"
#include "FileOperations/fileops.h"
#include "settings/Settings.h"
#include "settings/CSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/CGameSettings.h"
#include "settings/GameTitles.h"
#include "utils/StringTools.h"
#include "GameBootProcess.h"
#include "utils/rockout.h"
#include "utils/ShowError.h"
#include "utils/tools.h"
#include "utils/PasswordCheck.h"
#include "WDMMenu.hpp"
#include "gecko.h"
#include "menus.h"
#include "wpad.h"
#include "sys.h"

extern int load_from_fs;
extern u8 mountMethod;
extern bool updateavailable;
extern struct discHdr *dvdheader;
extern int cntMissFiles;

static int lastSelectedGame = 0;
static bool WiiMoteInitiated = false;
static bool Exiting = false;

GameBrowseMenu::GameBrowseMenu()
    : GuiWindow(screenwidth, screenheight)
{
    returnMenu = MENU_NONE;
    gameSelectedOld = -1;
    lastrawtime = 0;
    Exiting = false;
    show_searchwindow = false;
    gameBrowser = NULL;
    gameGrid = NULL;
    gameCarousel = NULL;
    searchBar = NULL;
    gameCover = NULL;
    gameCoverImg = NULL;
    GameIDTxt = NULL;
    GameRegionTxt = NULL;
    WDVD_GetCoverStatus(&DiscDriveCoverOld);
    wString oldFilter(gameList.GetCurrentFilter());
    gameList.FilterList(oldFilter.c_str());

    btnInstall = Resources::GetImageData("button_install.png");
    btnInstallOver = Resources::GetImageData("button_install_over.png");
    btnSettings = Resources::GetImageData("settings_button.png");
    btnSettingsOver = Resources::GetImageData("settings_button_over.png");
    btnpwroff = Resources::GetImageData("wiimote_poweroff.png");
    btnpwroffOver = Resources::GetImageData("wiimote_poweroff_over.png");
    btnhome = Resources::GetImageData("menu_button.png");
    btnhomeOver = Resources::GetImageData("menu_button_over.png");
    btnsdcardOver = Resources::GetImageData("sdcard_over.png");
    btnsdcard = Resources::GetImageData("sdcard.png");

    imgfavIcon = Resources::GetImageData("favIcon.png");
    imgfavIcon_gray = Resources::GetImageData("favIcon_gray.png");
    imgsearchIcon = Resources::GetImageData("searchIcon.png");
    imgsearchIcon_gray = Resources::GetImageData("searchIcon_gray.png");
    imgabcIcon = Resources::GetImageData("abcIcon.png");
    imgrankIcon = Resources::GetImageData("rankIcon.png");
    imgplayCountIcon = Resources::GetImageData("playCountIcon.png");
    imgarrangeGrid = Resources::GetImageData("arrangeGrid.png");
    imgarrangeGrid_gray = Resources::GetImageData("arrangeGrid_gray.png");
    imgarrangeList = Resources::GetImageData("arrangeList.png");
    imgarrangeList_gray = Resources::GetImageData("arrangeList_gray.png");
    imgarrangeCarousel = Resources::GetImageData("arrangeCarousel.png");
    imgarrangeCarousel_gray = Resources::GetImageData("arrangeCarousel_gray.png");
    imgdvd = Resources::GetImageData("dvd.png");
    imgdvd_gray = Resources::GetImageData("dvd_gray.png");
    imgLock = Resources::GetImageData("lock.png");
    imgLock_gray = Resources::GetImageData("lock_gray.png");
    imgUnlock = Resources::GetImageData("unlock.png");
    imgUnlock_gray = Resources::GetImageData("unlock_gray.png");

    homebrewImgData = Resources::GetImageData("browser.png");
    homebrewImgDataOver = Resources::GetImageData("browser_over.png");

    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHome = new GuiTrigger;
    trigHome->SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
    trig2 = new GuiTrigger;
    trig2->SetButtonOnlyTrigger(-1, WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X, 0);
    trig1 = new GuiTrigger;
    trig1->SetButtonOnlyTrigger(-1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y, 0);

    usedSpaceTxt = new GuiText(" ", 18, thColor("r=55 g=190 b=237 a=255 - hdd info color"));
    usedSpaceTxt->SetAlignment(thAlign("center - hdd info align hor"), thAlign("top - hdd info align ver"));
    usedSpaceTxt->SetPosition(thInt("0 - hdd info pos x"), thInt("400 - hdd info pos y"));

    gamecntTxt = new GuiText((char *) NULL, 18, thColor("r=55 g=190 b=237 a=255 - game count color"));
    gamecntBtn = new GuiButton(100, 18);
    gamecntBtn->SetAlignment(thAlign("center - game count align hor"), thAlign("top - game count align ver"));
    gamecntBtn->SetPosition(thInt("0 - game count pos x"), thInt("420 - game count pos y"));
    gamecntBtn->SetLabel(gamecntTxt);
    gamecntBtn->SetEffectGrow();
    gamecntBtn->SetTrigger(trigA);

    installBtnTT = new GuiTooltip(tr( "Install a game" ));
    if (Settings.wsprompt) installBtnTT->SetWidescreen(Settings.widescreen);
    installBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    installBtnImg = new GuiImage(btnInstall);
    installBtnImgOver = new GuiImage(btnInstallOver);
    installBtnImg->SetWidescreen(Settings.widescreen);
    installBtnImgOver->SetWidescreen(Settings.widescreen);

    installBtn = new GuiButton(installBtnImg, installBtnImgOver, ALIGN_LEFT, ALIGN_TOP,
                               thInt("16 - install btn pos x"), thInt("355 - install btn pos y"),
                               trigA, btnSoundOver, btnSoundClick2, 1, installBtnTT, 24, -30, 0, 5);

    settingsBtnTT = new GuiTooltip(tr( "Settings" ));
    if (Settings.wsprompt) settingsBtnTT->SetWidescreen(Settings.widescreen);
    settingsBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    settingsBtnImg = new GuiImage(btnSettings);
    settingsBtnImg->SetWidescreen(Settings.widescreen);
    settingsBtnImgOver = new GuiImage(btnSettingsOver);
    settingsBtnImgOver->SetWidescreen(Settings.widescreen);
    settingsBtn = new GuiButton(settingsBtnImg, settingsBtnImgOver, 0, 3,
                                thInt("64 - settings btn pos x"), thInt("371 - settings btn pos y"),
                                trigA, btnSoundOver, btnSoundClick2, 1, settingsBtnTT, 65, -30, 0, 5);

    homeBtnTT = new GuiTooltip(tr( "Back to HBC or Wii Menu" ));
    if (Settings.wsprompt) homeBtnTT->SetWidescreen(Settings.widescreen);
    settingsBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    homeBtnImg = new GuiImage(btnhome);
    homeBtnImg->SetWidescreen(Settings.widescreen);
    homeBtnImgOver = new GuiImage(btnhomeOver);
    homeBtnImgOver->SetWidescreen(Settings.widescreen);
    homeBtn = new GuiButton(homeBtnImg, homeBtnImgOver, 0, 3,
                            thInt("489 - home menu btn pos x"), thInt("371 - home menu btn pos y"),
                            trigA, btnSoundOver, btnSoundClick2, 1, homeBtnTT, 15, -30, 1, 5);
    homeBtn->RemoveSoundClick();
    homeBtn->SetTrigger(trigHome);

    poweroffBtnTT = new GuiTooltip(tr( "Power off the Wii" ));
    if (Settings.wsprompt) poweroffBtnTT->SetWidescreen(Settings.widescreen);
    poweroffBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    poweroffBtnImg = new GuiImage(btnpwroff);
    poweroffBtnImgOver = new GuiImage(btnpwroffOver);
    poweroffBtnImg->SetWidescreen(Settings.widescreen);
    poweroffBtnImgOver->SetWidescreen(Settings.widescreen);
    poweroffBtn = new GuiButton(poweroffBtnImg, poweroffBtnImgOver, 0, 3,
                                thInt("576 - power off btn pos x"), thInt("355 - power off btn pos y"),
                                trigA, btnSoundOver, btnSoundClick2, 1, poweroffBtnTT, -10, -30, 1, 5);

    sdcardBtnTT = new GuiTooltip(tr( "Reload SD" ));
    if (Settings.wsprompt) sdcardBtnTT->SetWidescreen(Settings.widescreen);
    sdcardBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    sdcardImg = new GuiImage(btnsdcard);
    sdcardImgOver = new GuiImage(btnsdcardOver);
    sdcardImg->SetWidescreen(Settings.widescreen);
    sdcardImgOver->SetWidescreen(Settings.widescreen);
    sdcardBtn = new GuiButton(sdcardImg, sdcardImgOver, 0, 3,
                              thInt("160 - sd card btn pos x"), thInt("395 - sd card btn pos y"),
                              trigA, btnSoundOver, btnSoundClick2, 1, sdcardBtnTT, 15, -30, 0, 5);

    gameInfo = new GuiButton(0, 0);
    gameInfo->SetTrigger(trig2);
    gameInfo->SetSoundClick(btnSoundClick2);

    favoriteBtnTT = new GuiTooltip(tr( "Display favorites only" ));
    if (Settings.wsprompt) favoriteBtnTT->SetWidescreen(Settings.widescreen);
    favoriteBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    favoriteBtnImg = new GuiImage(imgfavIcon);
    favoriteBtnImg->SetWidescreen(Settings.widescreen);
    favoriteBtnImg_g = new GuiImage(imgfavIcon_gray);
    favoriteBtnImg_g->SetWidescreen(Settings.widescreen);
    favoriteBtn = new GuiButton(favoriteBtnImg_g, favoriteBtnImg_g, ALIGN_LEFT, ALIGN_TOP,
                                0, 0,
                                trigA, btnSoundOver, btnSoundClick2, 1, favoriteBtnTT, -15, 52, 0, 3);

    searchBtnTT = new GuiTooltip(tr( "Set Search-Filter" ));
    if (Settings.wsprompt) searchBtnTT->SetWidescreen(Settings.widescreen);
    searchBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    searchBtnImg = new GuiImage(imgsearchIcon);
    searchBtnImg->SetWidescreen(Settings.widescreen);
    searchBtnImg_g = new GuiImage(imgsearchIcon_gray);
    searchBtnImg_g->SetWidescreen(Settings.widescreen);
    searchBtn = new GuiButton(searchBtnImg_g, searchBtnImg_g, ALIGN_LEFT, ALIGN_TOP,
                              0, 0,
                              trigA, btnSoundOver, btnSoundClick2, 1, searchBtnTT, -15, 52, 0, 3);

    sortBtnTT = new GuiTooltip(" ");
    if (Settings.wsprompt) sortBtnTT->SetWidescreen(Settings.widescreen);
    sortBtnTT->SetAlpha(thInt("255 - tooltip alpha"));

    sortBtnImg = new GuiImage(imgabcIcon);
    sortBtnImg->SetWidescreen(Settings.widescreen);
    sortBtn = new GuiButton(sortBtnImg, sortBtnImg, ALIGN_LEFT, ALIGN_TOP, 0, 0, trigA, btnSoundOver, btnSoundClick2, 1, sortBtnTT, -15, 52, 0, 3);

    listBtnTT = new GuiTooltip(tr( "Display as a list" ));
    if (Settings.wsprompt) listBtnTT->SetWidescreen(Settings.widescreen);
    listBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    listBtnImg = new GuiImage(imgarrangeList);
    listBtnImg->SetWidescreen(Settings.widescreen);
    listBtnImg_g = new GuiImage(imgarrangeList_gray);
    listBtnImg_g->SetWidescreen(Settings.widescreen);
    listBtn = new GuiButton(listBtnImg_g, listBtnImg_g, ALIGN_LEFT, ALIGN_TOP, 0, 0, trigA, btnSoundOver, btnSoundClick2, 1, listBtnTT, 15, 52, 1, 3);

    gridBtnTT = new GuiTooltip(tr( "Display as a grid" ));
    if (Settings.wsprompt) gridBtnTT->SetWidescreen(Settings.widescreen);
    gridBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    gridBtnImg = new GuiImage(imgarrangeGrid);
    gridBtnImg->SetWidescreen(Settings.widescreen);
    gridBtnImg_g = new GuiImage(imgarrangeGrid_gray);
    gridBtnImg_g->SetWidescreen(Settings.widescreen);
    gridBtn = new GuiButton(gridBtnImg_g, gridBtnImg_g, ALIGN_LEFT, ALIGN_TOP, 0, 0, trigA, btnSoundOver, btnSoundClick2, 1, gridBtnTT, 15, 52, 1, 3);

    carouselBtnTT = new GuiTooltip(tr( "Display as a carousel" ));
    if (Settings.wsprompt) carouselBtnTT->SetWidescreen(Settings.widescreen);
    carouselBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    carouselBtnImg = new GuiImage(imgarrangeCarousel);
    carouselBtnImg->SetWidescreen(Settings.widescreen);
    carouselBtnImg_g = new GuiImage(imgarrangeCarousel_gray);
    carouselBtnImg_g->SetWidescreen(Settings.widescreen);
    carouselBtn = new GuiButton(carouselBtnImg_g, carouselBtnImg_g, ALIGN_LEFT, ALIGN_TOP, 0, 0, trigA, btnSoundOver, btnSoundClick2, 1, carouselBtnTT, 15, 52, 1, 3);

    lockBtnTT = new GuiTooltip(NULL);
    if (Settings.wsprompt) lockBtnTT->SetWidescreen(Settings.widescreen);
    lockBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    lockBtnImg = new GuiImage(imgLock);
    lockBtnImg->SetWidescreen(Settings.widescreen);
    lockBtnImg_g = new GuiImage(imgLock_gray);
    lockBtnImg_g->SetWidescreen(Settings.widescreen);
    lockBtn = new GuiButton(lockBtnImg_g, lockBtnImg_g, ALIGN_LEFT, ALIGN_TOP, 0, 0, trigA, btnSoundOver, btnSoundClick2, 1, lockBtnTT, 15, 52, 1, 3);

    unlockBtnImg = new GuiImage(imgUnlock);
    unlockBtnImg->SetWidescreen(Settings.widescreen);
    unlockBtnImg_g = new GuiImage(imgUnlock_gray);
    unlockBtnImg_g->SetWidescreen(Settings.widescreen);

    dvdBtnTT = new GuiTooltip(tr( "Mount DVD drive" ));
    if (Settings.wsprompt) dvdBtnTT->SetWidescreen(Settings.widescreen);
    dvdBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    dvdBtnImg = new GuiImage(imgdvd);
    dvdBtnImg->SetWidescreen(Settings.widescreen);
    dvdBtnImg_g = new GuiImage(imgdvd_gray);
    dvdBtnImg_g->SetWidescreen(Settings.widescreen);
    dvdBtn = new GuiButton(dvdBtnImg_g, dvdBtnImg_g, ALIGN_LEFT, ALIGN_TOP, 0, 0,
                           trigA, btnSoundOver, btnSoundClick2, 1, dvdBtnTT, 15, 52, 1, 3);

    homebrewBtnTT = new GuiTooltip(tr( "Homebrew Launcher" ));
    if (Settings.wsprompt) homebrewBtnTT->SetWidescreen(Settings.widescreen);
    homebrewBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    homebrewImg = new GuiImage(homebrewImgData);
    homebrewImgOver = new GuiImage(homebrewImgDataOver);
    homebrewImg->SetWidescreen(Settings.widescreen);
    homebrewImgOver->SetWidescreen(Settings.widescreen);
    homebrewBtn = new GuiButton(homebrewImg, homebrewImgOver, ALIGN_LEFT, ALIGN_TOP, thInt("410 - HBC btn pos x"), thInt("405 - HBC btn pos y"),
                                trigA, btnSoundOver, btnSoundClick2, 1, homebrewBtnTT, 15, -30, 1, 5);
    //Downloading Covers
    DownloadBtnTT = new GuiTooltip(tr( "Click to Download Covers" ));
    if (Settings.wsprompt) DownloadBtnTT->SetWidescreen(Settings.widescreen);
    DownloadBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    DownloadBtn = new GuiButton (0, 0);
    DownloadBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    DownloadBtn->SetPosition(thInt("26 - cover/download btn pos x"), thInt("58 - cover/download btn pos y"));
    DownloadBtn->SetSoundOver(btnSoundOver);
    DownloadBtn->SetTrigger(0, trigA);
    DownloadBtn->SetTrigger(1, trig1);
    DownloadBtn->SetToolTip(DownloadBtnTT, 205, -30);

    IDBtnTT = new GuiTooltip(tr( "Click to change game ID" ));
    if (Settings.wsprompt) IDBtnTT->SetWidescreen(Settings.widescreen);
    IDBtnTT->SetAlpha(thInt("255 - tooltip alpha"));
    idBtn = new GuiButton(60, 23);
    idBtn->SetPosition(thInt("68 - gameID btn pos x"), thInt("305 - gameID btn pos y"));
    idBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    idBtn->SetSoundOver(btnSoundOver);
    idBtn->SetTrigger(0, trigA);
    idBtn->SetToolTip(IDBtnTT, 205, -30);

    GXColor clockColor = thColor("r=138 g=138 b=138 a=240 - clock color");
    clockTimeBack = new GuiText("88:88", 40, (GXColor) {clockColor.r, clockColor.g, clockColor.b, clockColor.a / 6});
    clockTimeBack->SetAlignment(thAlign("left - clock align hor"), thAlign("top - clock align ver"));
    clockTimeBack->SetPosition(thInt("275 - clock pos x"), thInt("335 - clock pos y"));
    clockTimeBack->SetFont(Resources::GetFile("clock.ttf"), Resources::GetFileSize("clock.ttf"));

    clockTime = new GuiText("", 40, clockColor);
    clockTime->SetAlignment(thAlign("left - clock align hor"), thAlign("top - clock align ver"));
    clockTime->SetPosition(thInt("275 - clock pos x"), thInt("335 - clock pos y"));
    clockTime->SetFont(Resources::GetFile("clock.ttf"), Resources::GetFileSize("clock.ttf"));

    ToolBar.push_back(favoriteBtn);
    ToolBar.push_back(searchBtn);
    ToolBar.push_back(sortBtn);
    ToolBar.push_back(listBtn);
    ToolBar.push_back(gridBtn);
    ToolBar.push_back(carouselBtn);
    ToolBar.push_back(lockBtn);
    ToolBar.push_back(dvdBtn);
    SetUpdateCallback(UpdateCallback);

    ReloadBrowser();
}

GameBrowseMenu::~GameBrowseMenu()
{
    Exiting = true;
    ResumeGui();

    SetEffect(EFFECT_FADE, -20);
    while(parentElement && this->GetEffect() > 0) usleep(100);

    HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    RemoveAll();

    delete btnInstall;
    delete btnInstallOver;
    delete btnSettings;
    delete btnSettingsOver;
    delete btnpwroff;
    delete btnpwroffOver;
    delete btnhome;
    delete btnhomeOver;
    delete btnsdcardOver;
    delete btnsdcard;
    delete imgfavIcon;
    delete imgfavIcon_gray;
    delete imgsearchIcon;
    delete imgsearchIcon_gray;
    delete imgabcIcon;
    delete imgrankIcon;
    delete imgplayCountIcon;
    delete imgarrangeGrid;
    delete imgarrangeGrid_gray;
    delete imgarrangeCarousel;
    delete imgarrangeCarousel_gray;
    delete imgarrangeList;
    delete imgarrangeList_gray;
    delete imgdvd;
    delete imgdvd_gray;
    delete imgLock;
    delete imgLock_gray;
    delete imgUnlock;
    delete imgUnlock_gray;
    delete homebrewImgData;
    delete homebrewImgDataOver;
    delete gameCover;

    delete trigA;
    delete trigHome;
    delete trig1;
    delete trig2;

    delete installBtnImg;
    delete installBtnImgOver;
    delete settingsBtnImg;
    delete settingsBtnImgOver;
    delete homeBtnImg;
    delete homeBtnImgOver;
    delete poweroffBtnImg;
    delete poweroffBtnImgOver;
    delete sdcardImg;
    delete sdcardImgOver;
    delete favoriteBtnImg;
    delete favoriteBtnImg_g;
    delete searchBtnImg;
    delete searchBtnImg_g;
    delete sortBtnImg;
    delete listBtnImg;
    delete listBtnImg_g;
    delete gridBtnImg;
    delete gridBtnImg_g;
    delete carouselBtnImg;
    delete carouselBtnImg_g;
    delete lockBtnImg;
    delete lockBtnImg_g;
    delete unlockBtnImg;
    delete unlockBtnImg_g;
    delete dvdBtnImg;
    delete dvdBtnImg_g;
    delete homebrewImg;
    delete homebrewImgOver;
    delete gameCoverImg;

    delete GameIDTxt;
    delete GameRegionTxt;
    delete usedSpaceTxt;
    delete gamecntTxt;
    delete clockTimeBack;
    delete clockTime;

    delete gamecntBtn;
    delete installBtn;
    delete settingsBtn;
    delete homeBtn;
    delete poweroffBtn;
    delete sdcardBtn;
    delete gameInfo;
    delete favoriteBtn;
    delete searchBtn;
    delete sortBtn;
    delete listBtn;
    delete gridBtn;
    delete carouselBtn;
    delete lockBtn;
    delete dvdBtn;
    delete homebrewBtn;
    delete DownloadBtn;
    delete idBtn;

    delete installBtnTT;
    delete settingsBtnTT;
    delete homeBtnTT;
    delete poweroffBtnTT;
    delete sdcardBtnTT;
    delete favoriteBtnTT;
    delete searchBtnTT;
    delete sortBtnTT;
    delete listBtnTT;
    delete gridBtnTT;
    delete carouselBtnTT;
    delete lockBtnTT;
    delete dvdBtnTT;
    delete homebrewBtnTT;
    delete DownloadBtnTT;
    delete IDBtnTT;

    lastSelectedGame = cut_bounds(GetSelectedGame(), 0, gameList.size()-1);

    delete gameBrowser;
    delete gameGrid;
    delete gameCarousel;
    mainWindow->Remove(searchBar);
    delete searchBar;

    ResumeGui();
}

void GameBrowseMenu::ReloadBrowser()
{
    ResumeGui();

    SetEffect(EFFECT_FADE, -40);
    while(parentElement && this->GetEffect() > 0) usleep(100);

    HaltGui();
    RemoveAll();
    mainWindow->Remove(searchBar);

    gamecntTxt->SetText(fmt("%s: %i", tr( "Games" ), gameList.size()));

    const char * sortTTText = NULL;
    GuiImageData * sortImgData = NULL;

    if(Settings.GameSort & SORT_RANKING)
    {
        sortTTText = tr( "Sort by rank" );
        sortImgData = imgrankIcon;
    }
    else if(Settings.GameSort & SORT_PLAYCOUNT)
    {
        sortTTText = tr( "Sort order by most played");
        sortImgData = imgplayCountIcon;
    }
    else
    {
        sortTTText = tr("Sort alphabetically");
        sortImgData = imgabcIcon;
    }

    sortBtnTT->SetText(sortTTText);
    sortBtnImg->SetImage(sortImgData);

    if(DiscDriveCoverOld & 0x02)
        dvdBtn->SetImage(dvdBtnImg);
    else
        dvdBtn->SetImage(dvdBtnImg_g);

    if (Settings.GameSort & SORT_FAVORITE)
    {
        favoriteBtn->SetImage(favoriteBtnImg);
        favoriteBtn->SetImageOver(favoriteBtnImg);
    }
    else
    {
        favoriteBtn->SetImage(favoriteBtnImg_g);
        favoriteBtn->SetImageOver(favoriteBtnImg_g);
    }

    if (*gameList.GetCurrentFilter())
    {
        if (!show_searchwindow) searchBtn->SetEffect(EFFECT_PULSE, 10, 105);
        searchBtn->SetImage(searchBtnImg);
        searchBtn->SetImageOver(searchBtnImg);
    }
    else if(!show_searchwindow)
    {
        searchBtn->SetImage(searchBtnImg_g);
        searchBtn->SetImageOver(searchBtnImg_g);
    }

    if (Settings.godmode)
    {
        GuiImage * unlockImage = strcmp(Settings.unlockCode, "") == 0 ? unlockBtnImg_g : unlockBtnImg;
        lockBtn->SetImage(unlockImage);
        lockBtn->SetImageOver(unlockImage);
        lockBtnTT->SetText(tr( "Lock USB Loader GX" ));
    }
    else
    {
        lockBtn->SetImage(lockBtnImg);
        lockBtn->SetImageOver(lockBtnImg);
        lockBtnTT->SetText(tr( "Unlock USB Loader GX" ));
    }

    if(GetSelectedGame() >= 0)
        lastSelectedGame = cut_bounds(GetSelectedGame(), 0, gameList.size()-1);
    else
        lastSelectedGame = cut_bounds(lastSelectedGame, 0, gameList.size()-1);

    delete gameBrowser;
    delete gameGrid;
    delete gameCarousel;
    delete searchBar;
    gameBrowser = NULL;
    gameGrid = NULL;
    gameCarousel = NULL;
    searchBar = NULL;

    if (Settings.gameDisplay == LIST_MODE)
    {
        DownloadBtn->SetSize(160, 224);
        listBtn->SetImage(listBtnImg);
        listBtn->SetImageOver(listBtnImg);
        gridBtn->SetImage(gridBtnImg_g);
        gridBtn->SetImageOver(gridBtnImg_g);
        carouselBtn->SetImage(carouselBtnImg_g);
        carouselBtn->SetImageOver(carouselBtnImg_g);

        favoriteBtn->SetPosition(Settings.widescreen ? thInt("288 - list layout favorite btn pos x widescreen") : thInt("260 - list layout favorite btn pos x"),
                                thInt("13 - list layout favorite btn pos y"));
        searchBtn->SetPosition(Settings.widescreen ? thInt("320 - list layout search btn pos x widescreen") : thInt("300 - list layout search btn pos x"),
                                thInt("13 - list layout search btn pos y"));
        sortBtn->SetPosition(Settings.widescreen ? thInt("352 - list layout abc/sort btn pos x widescreen") : thInt("340 - list layout abc/sort btn pos x"),
                                thInt("13 - list layout abc/sort btn pos y"));
        listBtn->SetPosition(Settings.widescreen ? thInt("384 - list layout list btn pos x widescreen") : thInt("380 - list layout list btn pos x"),
                                thInt("13 - list layout list btn pos y"));
        gridBtn->SetPosition(Settings.widescreen ? thInt("416 - list layout grid btn pos x widescreen") : thInt("420 - list layout grid btn pos x"),
                                thInt("13 - list layout grid btn pos y"));
        carouselBtn->SetPosition(Settings.widescreen ? thInt("448 - list layout carousel btn pos x widescreen") : thInt("460 - list layout carousel btn pos x"),
                                thInt("13 - list layout carousel btn pos y"));
        lockBtn->SetPosition(Settings.widescreen ? thInt("480 - list layout lock btn pos x widescreen") : thInt("500 - list layout lock btn pos x"),
                                thInt("13 - list layout lock btn pos y"));
        dvdBtn->SetPosition(Settings.widescreen ? thInt("512 - list layout dvd btn pos x widescreen") : thInt("540 - list layout dvd btn pos x"),
                                thInt("13 - list layout dvd btn pos y"));

        gameBrowser = new GuiGameBrowser(thInt("396 - game list layout width"), thInt("280 - game list layout height"), lastSelectedGame);
        gameBrowser->SetPosition(thInt("200 - game list layout pos x"), thInt("49 - game list layout pos y"));
        gameBrowser->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }
    else if (Settings.gameDisplay == GRID_MODE)
    {
        DownloadBtn->SetImage(NULL);
        DownloadBtn->SetSize(0, 0);
        UpdateGameInfoText(NULL);
        gridBtn->SetImage(gridBtnImg);
        gridBtn->SetImageOver(gridBtnImg);
        listBtn->SetImage(listBtnImg_g);
        listBtn->SetImageOver(listBtnImg_g);
        carouselBtn->SetImage(carouselBtnImg_g);
        carouselBtn->SetImageOver(carouselBtnImg_g);

        favoriteBtn->SetPosition(Settings.widescreen ? thInt("224 - grid layout favorite btn pos x widescreen") : thInt("200 - grid layout favorite btn pos x"),
                                thInt("13 - grid layout favorite btn pos y"));
        searchBtn->SetPosition(Settings.widescreen ? thInt("256 - grid layout search btn pos x widescreen") : thInt("240 - grid layout search btn pos x"),
                                thInt("13 - grid layout search btn pos y"));
        sortBtn->SetPosition(Settings.widescreen ? thInt("288 - grid layout abc/sort btn pos x widescreen") : thInt("280 - grid layout abc/sort btn pos x"),
                                thInt("13 - grid layout abc/sort btn pos y"));
        listBtn->SetPosition(Settings.widescreen ? thInt("320 - grid layout list btn pos x widescreen") : thInt("320 - grid layout list btn pos x"),
                                thInt("13 - grid layout list btn pos y"));
        gridBtn->SetPosition(Settings.widescreen ? thInt("352 - grid layout grid btn pos x widescreen") : thInt("360 - grid layout grid btn pos x"),
                                thInt("13 - grid layout grid btn pos y"));
        carouselBtn->SetPosition(Settings.widescreen ? thInt("384 - grid layout carousel btn pos x widescreen") : thInt("400 - grid layout carousel btn pos x"),
                                thInt("13 - grid layout carousel btn pos y"));
        lockBtn->SetPosition(Settings.widescreen ? thInt("416 - grid layout lock btn pos x widescreen") : thInt("440 - grid layout lock btn pos x"),
                                thInt("13 - grid layout lock btn pos y"));
        dvdBtn->SetPosition(Settings.widescreen ? thInt("448 - grid layout dvd btn pos x widescreen") : thInt("480 - grid layout dvd btn pos x"),
                                thInt("13 - grid layout dvd btn pos y"));

        gameGrid = new GuiGameGrid(thInt("640 - game grid layout width"), thInt("400 - game grid layout height"), Settings.theme_path, lastSelectedGame);
        gameGrid->SetPosition(thInt("0 - game grid layout pos x"), thInt("20 - game grid layout pos y"));
        gameGrid->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }
    else if (Settings.gameDisplay == CAROUSEL_MODE)
    {
        DownloadBtn->SetImage(NULL);
        DownloadBtn->SetSize(0, 0);
        UpdateGameInfoText(NULL);
        carouselBtn->SetImage(carouselBtnImg);
        carouselBtn->SetImageOver(carouselBtnImg);
        listBtn->SetImage(listBtnImg_g);
        listBtn->SetImageOver(listBtnImg_g);
        gridBtn->SetImage(gridBtnImg_g);
        gridBtn->SetImageOver(gridBtnImg_g);

        favoriteBtn->SetPosition(Settings.widescreen ? thInt("224 - carousel layout favorite btn pos x widescreen") : thInt("200 - carousel layout favorite btn pos x"),
                                thInt("13 - carousel layout favorite btn pos y"));
        searchBtn->SetPosition(Settings.widescreen ? thInt("256 - carousel layout search btn pos x widescreen") : thInt("240 - carousel layout search btn pos x"),
                                thInt("13 - carousel layout search btn pos y"));
        sortBtn->SetPosition(Settings.widescreen ? thInt("288 - carousel layout abc/sort btn pos x widescreen") : thInt("280 - carousel layout abc/sort btn pos x"),
                                thInt("13 - carousel layout abc/sort btn pos y"));
        listBtn->SetPosition(Settings.widescreen ? thInt("320 - carousel layout list btn pos x widescreen") : thInt("320 - carousel layout list btn pos x"),
                                thInt("13 - carousel layout list btn pos y"));
        gridBtn->SetPosition(Settings.widescreen ? thInt("352 - carousel layout grid btn pos x widescreen") : thInt("360 - carousel layout grid btn pos x"),
                                thInt("13 - carousel layout grid btn pos y"));
        carouselBtn->SetPosition(Settings.widescreen ? thInt("384 - carousel layout carousel btn pos x widescreen") : thInt("400 - carousel layout carousel btn pos x"),
                                thInt("13 - carousel layout carousel btn pos y"));
        lockBtn->SetPosition(Settings.widescreen ? thInt("416 - carousel layout lock btn pos x widescreen") : thInt("440 - carousel layout lock btn pos x"),
                                thInt("13 - carousel layout lock btn pos y"));
        dvdBtn->SetPosition(Settings.widescreen ? thInt("448 - carousel layout dvd btn pos x widescreen") : thInt("480 - carousel layout dvd btn pos x"),
                                thInt("13 - carousel layout dvd btn pos y"));

        gameCarousel = new GuiGameCarousel(thInt("640 - game carousel layout width"), thInt("400 - game carousel layout height"), Settings.theme_path, lastSelectedGame);
        gameCarousel->SetPosition(thInt("0 - game carousel layout pos x"), thInt("-20 - game carousel layout pos y"));
        gameCarousel->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }


    if (thInt("1 - show hdd info: 1 for on and 0 for off") == 1) //force show hdd info
        Append(usedSpaceTxt);
    if (thInt("1 - show game count: 1 for on and 0 for off") == 1) //force show game cnt info
        Append(gamecntBtn);
    Append(sdcardBtn);
    Append(poweroffBtn);
    Append(gameInfo);
    Append(homeBtn);
    Append(settingsBtn);
    Append(homebrewBtn);

    if (Settings.godmode || !(Settings.ParentalBlocks & BLOCK_GAME_INSTALL))
        Append(installBtn);

    if (Settings.godmode || !(Settings.ParentalBlocks & BLOCK_COVER_DOWNLOADS))
        Append(DownloadBtn);

    if (Settings.godmode || !(Settings.ParentalBlocks & BLOCK_GAMEID_CHANGE))
        Append(idBtn);

    Append(favoriteBtn);
    Append(searchBtn);
    Append(sortBtn);
    Append(listBtn);
    Append(gridBtn);
    Append(carouselBtn);
    Append(lockBtn);
    Append(dvdBtn);

    if ((Settings.hddinfo == CLOCK_HR12) || (Settings.hddinfo == CLOCK_HR24))
    {
        Append(clockTimeBack);
        Append(clockTime);
    }

    if (gameBrowser)
        Append(gameBrowser);

    else if (gameGrid)
        Append(gameGrid);

    else if (gameCarousel)
        Append(gameCarousel);

    if (show_searchwindow)
    {
        searchBar = new GuiSearchBar(gameList.GetAvailableSearchChars());
        mainWindow->Append(searchBar);
    }

    SetEffect(EFFECT_FADE, 40);
    ResumeGui();

    while(parentElement && this->GetEffect() > 0) usleep(100);
}

int GameBrowseMenu::Show()
{
    int menu = MENU_NONE;

    if(Settings.ShowFreeSpace)
    {
        HDDSizeCallback.SetCallback(this, &GameBrowseMenu::UpdateFreeSpace);
        ThreadedTask::Instance()->AddCallback(&HDDSizeCallback);
        ThreadedTask::Instance()->Execute();
    }

    while(menu == MENU_NONE)
    {
        usleep(100);

        if (shutdown)
            Sys_Shutdown();
        if (reset)
            Sys_Reboot();

        menu = MainLoop();
    }

    return menu;
}


int GameBrowseMenu::MainLoop()
{
    time_t rawtime = time(0);
    if(rawtime != lastrawtime) //! Only update every 1 second
    {
        UpdateClock(rawtime);
        CheckDiscSlotUpdate();
    }

    if (updateavailable == true)
    {
        gprintf("\tUpdate Available\n");
        SetState(STATE_DISABLED);
        UpdateApp();
        updateavailable = false;
        SetState(STATE_DEFAULT);
    }

    else if (poweroffBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tpoweroffBtn clicked\n");
        int choice = WindowPrompt(tr( "How to Shutdown?" ), 0, tr( "Full shutdown" ), tr( "Standby" ), tr( "Cancel" ));
        if (choice == 2)
            Sys_ShutdownToIdle();
        else if (choice == 1)
            Sys_ShutdownToStandby();

        poweroffBtn->ResetState();
    }
    else if (gamecntBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tgameCntBtn clicked\n");
        gamecntBtn->ResetState();

        int choice = WindowPrompt(0, fmt("%s %sGameList ?", tr( "Save Game List to" ), Settings.update_path), "TXT", "CSV", tr( "Back" ));
        if (choice)
        {
            if (save_gamelist(choice - 1))
                WindowPrompt(0, tr( "Saved" ), tr( "OK" ));
            else
                WindowPrompt(tr( "Error" ), tr( "Could not save." ), tr( "OK" ));
        }
    }
    else if (homeBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\thomeBtn clicked\n");
        WindowExitPrompt();

        homeBtn->ResetState();
    }
    else if (installBtn->GetState() == STATE_CLICKED)
    {
        int choice = WindowPrompt(tr( "Install a game" ), 0, tr( "Yes" ), tr( "No" ));
        if (choice == 1)
            return MENU_INSTALL;

        installBtn->ResetState();
    }
    else if (sdcardBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tsdCardBtn Clicked\n");
        HaltGui();
        bgMusic->Pause();
        Settings.Save();
        DeviceHandler::Instance()->MountSD();
        gprintf("\tLoading config...%s\n", Settings.Load() ? "done" : "failed");
        gprintf("\tLoading language...%s\n", Settings.LoadLanguage(Settings.language_path, CONSOLE_DEFAULT) ? "done" : "failed");
        gprintf("\tLoading game settings...%s\n", GameSettings.Load(Settings.ConfigPath) ? "done" : "failed");
        gprintf("\tLoading game statistics...%s\n", GameStatistics.Load(Settings.ConfigPath) ? "done" : "failed");
        gprintf("\tLoading font...%s\n", Theme::LoadFont(Settings.theme_path) ? "done" : "failed (using default)");
        gprintf("\tLoading theme...%s\n", Theme::Load(Settings.theme) ? "done" : "failed (using default)");
        bgMusic->Resume();
        wString oldFilter(gameList.GetCurrentFilter());
        gameList.FilterList(oldFilter.c_str());
        ReloadBrowser();
        ResumeGui();
        sdcardBtn->ResetState();
    }

    else if (DownloadBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tDownloadBtn Clicked\n");
        CoverDownload();
        ReloadBrowser();
        DownloadBtn->ResetState();
    }

    else if (settingsBtn->GetState() == STATE_CLICKED)
    {
        return MENU_SETTINGS;
    }

    else if (favoriteBtn->GetState() == STATE_CLICKED)
    {
        favoriteBtn->ResetState();
        gprintf("\tfavoriteBtn Clicked\n");

        if(Settings.GameSort & SORT_FAVORITE)
            Settings.GameSort &= ~SORT_FAVORITE;
        else
            Settings.GameSort |= SORT_FAVORITE;

        wString oldFilter(gameList.GetCurrentFilter());
        gameList.FilterList(oldFilter.c_str());

        if(Settings.GameSort & SORT_FAVORITE && gameList.size() == 0)
        {
            Settings.GameSort &= ~SORT_FAVORITE;
            gameList.FilterList(oldFilter.c_str());
            ShowError(tr("No favorites selected."));
        }
        else
            ReloadBrowser();
    }

    else if (searchBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tsearchBtn Clicked\n");
        show_searchwindow = !show_searchwindow;
        wString oldFilter(gameList.GetCurrentFilter());
        gameList.FilterList(oldFilter.c_str());
        ReloadBrowser();
        searchBtn->ResetState();
    }

    else if (searchBar && (searchChar = searchBar->GetClicked()))
    {
        if (searchChar > 27)
        {
            int len = gameList.GetCurrentFilter() ? wcslen(gameList.GetCurrentFilter()) : 0;
            wchar_t newFilter[len + 2];
            if (gameList.GetCurrentFilter()) wcscpy(newFilter, gameList.GetCurrentFilter());
            newFilter[len] = searchChar;
            newFilter[len + 1] = 0;

            gameList.FilterList(newFilter);
        }
        else if (searchChar == 7) // Close
        {
            show_searchwindow = false;
            searchBtn->StopEffect();
        }
        else if (searchChar == 8) // Backspace
        {
            int len = wcslen(gameList.GetCurrentFilter());
            wchar_t newFilter[len + 1];
            if (gameList.GetCurrentFilter()) wcscpy(newFilter, gameList.GetCurrentFilter());
            newFilter[len > 0 ? len - 1 : 0] = 0;
            gameList.FilterList(newFilter);
        }
        ReloadBrowser();
        return MENU_NONE;
    }

    else if (sortBtn->GetState() == STATE_CLICKED)
    {
        sortBtn->ResetState();
        gprintf("\tsortBtn clicked\n");
        if(Settings.GameSort & SORT_ABC)
        {
            Settings.GameSort &= ~SORT_ABC;
            Settings.GameSort |= SORT_RANKING;
        }
        else if(Settings.GameSort & SORT_RANKING)
        {
            Settings.GameSort &= ~SORT_RANKING;
            Settings.GameSort |= SORT_PLAYCOUNT;
        }
        else if(Settings.GameSort & SORT_PLAYCOUNT)
        {
            Settings.GameSort &= ~SORT_PLAYCOUNT;
            Settings.GameSort |= SORT_ABC;
        }

        wString oldFilter(gameList.GetCurrentFilter());
        gameList.FilterList(oldFilter.c_str());
        ReloadBrowser();
    }

    else if (listBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tlistBtn Clicked\n");
        if (Settings.gameDisplay != LIST_MODE)
        {
            Settings.gameDisplay = LIST_MODE;
            ReloadBrowser();
        }
        listBtn->ResetState();
    }

    else if (gridBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tgridBtn Clicked\n");
        if (Settings.gameDisplay != GRID_MODE)
        {
            Settings.gameDisplay = GRID_MODE;
            ReloadBrowser();
        }
        gridBtn->ResetState();
    }

    else if (carouselBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tcarouselBtn Clicked\n");
        if (Settings.gameDisplay != CAROUSEL_MODE)
        {
            Settings.gameDisplay = CAROUSEL_MODE;
            ReloadBrowser();
        }
        carouselBtn->ResetState();
    }

    else if (homebrewBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\thomebrewBtn Clicked\n");
        return MENU_HOMEBREWBROWSE;
    }

    else if (gameInfo->GetState() == STATE_CLICKED)
    {
        gprintf("\tgameinfo Clicked\n");
        int SelectedGame = GetSelectedGame();
        gameInfo->ResetState();
        if (SelectedGame >= 0 && SelectedGame < (s32) gameList.size())
        {
            rockout(SelectedGame);
            struct discHdr *header = gameList[SelectedGame];
            char IDfull[7];
            snprintf(IDfull, sizeof(IDfull), "%s", (char *) header->id);
            SetState(STATE_DISABLED);
            int choice = showGameInfo(IDfull);
            SetState(STATE_DEFAULT);
            rockout(SelectedGame, 2);
            if (choice == 2)
                homeBtn->SetState(STATE_CLICKED);
        }
    }
    else if (lockBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tlockBtn clicked\n");
        lockBtn->ResetState();
        if (Settings.godmode)
        {
            if(WindowPrompt(tr( "Parental Control" ), tr( "Are you sure you want to lock USB Loader GX?" ), tr( "Yes" ), tr( "No" )) == 1)
            {
                Settings.godmode = 0;
                wString oldFilter(gameList.GetCurrentFilter());
                gameList.FilterList(oldFilter.c_str());
                ReloadBrowser();
            }
        }
        else
        {
            //password check to unlock Install,Delete and Format
            SetState(STATE_DISABLED);
            int result = PasswordCheck(Settings.unlockCode);
            SetState(STATE_DEFAULT);
            if (result > 0)
            {
                if(result == 1)
                    WindowPrompt( tr( "Correct Password" ), tr( "All the features of USB Loader GX are unlocked." ), tr( "OK" ));
                Settings.godmode = 1;
                wString oldFilter(gameList.GetCurrentFilter());
                gameList.FilterList(oldFilter.c_str());
                ReloadBrowser();
            }
            else if(result < 0)
                WindowPrompt(tr( "Wrong Password" ), tr( "USB Loader GX is protected" ), tr( "OK" ));
        }
    }

    else if (Settings.gameDisplay == LIST_MODE && idBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tidBtn Clicked\n");
        struct discHdr * header = gameList[GetSelectedGame()];
        //enter new game ID
        char entered[7];
        snprintf(entered, sizeof(entered), "%s", (char *) header->id);
        int result = OnScreenKeyboard(entered, sizeof(entered), 0);
        if (result == 1)
        {
            WBFS_ReIDGame(header->id, entered);
            wString oldFilter(gameList.GetCurrentFilter());
            gameList.ReadGameList();
            gameList.FilterList(oldFilter.c_str());
            ReloadBrowser();
        }
        idBtn->ResetState();
    }

    else if (Settings.gameDisplay == LIST_MODE && GetSelectedGame() != gameSelectedOld)
    {
		gameSelectedOld = GetSelectedGame();
		int gameSelected = gameSelectedOld;
        if(gameSelected >= 0 && gameSelected < (s32) gameList.size())
        {
            struct discHdr *header = gameList[gameSelected];
            LoadCover(header);
            UpdateGameInfoText(header->id);
        }
    }

	gameClicked = GetClickedGame();
    if ((gameClicked >= 0 && gameClicked < (s32) gameList.size()) || mountMethod != 0)
    {
        OpenClickedGame();
    }

    if (!IsWpadConnected())
    {
        if(Settings.screensaver != 0 && WiiMoteInitiated)
            WindowScreensaver();
    }
    else if(!WiiMoteInitiated)
        WiiMoteInitiated = true;

	return returnMenu;
}

void GameBrowseMenu::CheckDiscSlotUpdate()
{
    u32 DiscDriveCover;
    WDVD_GetCoverStatus(&DiscDriveCover);//for detecting if i disc has been inserted

    if ((DiscDriveCover & 0x02) && (DiscDriveCover != DiscDriveCoverOld))
    {
        gprintf("\tNew Disc Detected\n");
        int choice = WindowPrompt(tr( "New Disc Detected" ), 0, tr( "Install" ), tr( "Mount DVD drive" ), tr( "Cancel" ));
        if (choice == 1)
        {
            if(!Settings.godmode && (Settings.ParentalBlocks & BLOCK_GAME_INSTALL))
            {
                WindowPrompt(tr( "Permission denied." ), tr( "Console must be unlocked for this option." ), tr( "OK" ));
                return;
            }

            returnMenu = MENU_INSTALL;
        }
        else if (choice == 2)
            dvdBtn->SetState(STATE_CLICKED);
    }
    else if (dvdBtn->GetState() == STATE_CLICKED)
    {
        gprintf("\tdvdBtn Clicked\n");
        if(!dvdheader)
            dvdheader = new struct discHdr;
        mountMethod = DiscMount(dvdheader);
        dvdBtn->ResetState();

        rockout(GetSelectedGame());
    }

    if(DiscDriveCoverOld != DiscDriveCover)
    {
        if(DiscDriveCover & 0x02)
            dvdBtn->SetImage(dvdBtnImg);
        else
            dvdBtn->SetImage(dvdBtnImg_g);

        DiscDriveCoverOld = DiscDriveCover;
    }
}

void GameBrowseMenu::UpdateClock(time_t &rawtime)
{
    lastrawtime = rawtime;

    if(Settings.hddinfo != CLOCK_HR12 && Settings.hddinfo != CLOCK_HR24)
        return;

    char theTime[50];
    theTime[0] = 0;

    struct tm * timeinfo = localtime(&rawtime);
    if (Settings.hddinfo == CLOCK_HR12)
    {
        if (rawtime & 1)
            strftime(theTime, sizeof(theTime), "%I:%M", timeinfo);
        else
            strftime(theTime, sizeof(theTime), "%I %M", timeinfo);
    }
    if (Settings.hddinfo == CLOCK_HR24)
    {
        if (rawtime & 1)
            strftime(theTime, sizeof(theTime), "%H:%M", timeinfo);
        else
            strftime(theTime, sizeof(theTime), "%H %M", timeinfo);
    }
    clockTime->SetText(theTime);
}

int GameBrowseMenu::GetSelectedGame()
{
    if(gameBrowser)
        return gameBrowser->GetSelectedOption();

    else if(gameCarousel)
        return gameCarousel->GetSelectedOption();

    else if(gameGrid)
        return gameGrid->GetSelectedOption();

    return -1;
}

int GameBrowseMenu::GetClickedGame()
{
    if(gameBrowser)
        return gameBrowser->GetClickedOption();

    else if(gameCarousel)
        return gameCarousel->GetClickedOption();

    else if(gameGrid)
        return gameGrid->GetClickedOption();

    return -1;
}

void GameBrowseMenu::UpdateGameInfoText(const u8 * gameId)
{
    if(!gameId)
    {
        Remove(GameRegionTxt);
        delete GameRegionTxt;
        GameRegionTxt = NULL;
        Remove(idBtn);
        idBtn->SetLabel(NULL);
        delete GameIDTxt;
        GameIDTxt = NULL;
        return;
    }

    char gameregion[10];
    char IDfull[7];
    memset(IDfull, 0, sizeof(IDfull));
    snprintf(IDfull, sizeof(IDfull), (char *) gameId);

    switch (IDfull[3])
    {
        case 'E':
            sprintf(gameregion, "NTSC U");
            break;
        case 'J':
            sprintf(gameregion, "NTSC J");
            break;
        case 'W':
            sprintf(gameregion, "NTSC T");
            break;
        default:
        case 'K':
            sprintf(gameregion, "NTSC K");
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
            sprintf(gameregion, "  PAL ");
            break;
    }

    HaltGui();
    if (Settings.sinfo == GAMEINFO_ID || Settings.sinfo == GAMEINFO_BOTH)
    {
        Remove(GameIDTxt);
        delete GameIDTxt;
        GameIDTxt = new GuiText(IDfull, 22, thColor("r=55 g=190 b=237 a=255 - game id text color"));
        GameIDTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        if(Settings.godmode || !(Settings.ParentalBlocks & BLOCK_GAMEID_CHANGE))
        {
            idBtn->SetEffect(EFFECT_FADE, 20);
            idBtn->SetLabel(GameIDTxt);
            Append(idBtn);
        }
        else
        {
            GameIDTxt->SetPosition(thInt("68 - gameID btn pos x"), thInt("305 - gameID btn pos y"));
            GameIDTxt->SetEffect(EFFECT_FADE, 20);
            Append(GameIDTxt);
        }
    }
    //don't try to show region for channels because all the custom channels wont follow the rules
    if ((Settings.sinfo == GAMEINFO_REGION) || (Settings.sinfo == GAMEINFO_BOTH))
    {
        Remove(GameRegionTxt);
        delete GameRegionTxt;
        GameRegionTxt = new GuiText(gameregion, 22, thColor("r=55 g=190 b=237 a=255 - region info text color"));
        GameRegionTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
        GameRegionTxt->SetPosition(thInt("68 - region info text pos x"), thInt("30 - region info text pos y"));
        GameRegionTxt->SetEffect(EFFECT_FADE, 20);
        Append(GameRegionTxt);
    }
    ResumeGui();
}

int GameBrowseMenu::OpenClickedGame()
{
	int gameSelected = GetSelectedGame();
	if(gameSelected < 0 || gameSelected >= gameList.size())
		return -1;

    if (searchBar)
    {
        HaltGui();
        mainWindow->Remove(searchBar);
        ResumeGui();
    }

    rockout(gameSelected);

    struct discHdr *header = (mountMethod ? dvdheader : gameList[gameSelected]);

    char IDfull[7];
    snprintf(IDfull, sizeof(IDfull), "%s", (char *) header->id);

    u8 alternatedol = OFF;
    u8 ocarinaChoice = Settings.ocarina;

    bool returnHere = true;// prompt to start game
    int choice = -1;

    while (returnHere)
    {
        returnHere = false;

        if (Settings.wiilight == ON)
            wiilight(1);

        if (Settings.quickboot) //quickboot game
            choice = 1;
        else
        {
            SetState(STATE_DISABLED);
			GameWindow * GamePrompt = new GameWindow(gameSelected);
			mainWindow->Append(GamePrompt);
            choice = GamePrompt->Show();
			gameSelected = GamePrompt->GetSelectedGame();
			delete GamePrompt;
            SetState(STATE_DEFAULT);
            //update header and id if it was changed
            header = (mountMethod ? dvdheader : gameList[gameSelected]);
            snprintf(IDfull, sizeof(IDfull), "%s", (char *) header->id);
        }

        if (choice == 1)
        {
            bool RunGame = true;
            wiilight(0);

            GameCFG* game_cfg = GameSettings.GetGameCFG(header->id);
            if (game_cfg)
            {
                alternatedol = game_cfg->loadalternatedol;
                ocarinaChoice = game_cfg->ocarina;
            }

            if(alternatedol == 3)
                if(WDMMenu::Show(header) == 0)
                {
                    RunGame = false;
                    returnHere = true;
                }

            if (alternatedol == 2)
                CheckAlternativeDOL(IDfull);

            if (RunGame && ocarinaChoice != OFF)
                CheckOcarina(IDfull);

            if(RunGame)
            {
                GameStatistics.SetPlayCount(header->id, GameStatistics.GetPlayCount(header->id)+1);
                GameStatistics.Save();

                //Just calling that shuts down everything and starts game
                BootGame(IDfull);
            }
        }
        else if (choice == 2)
        {
            ReloadBrowser();
        }
    }

    rockout(-1, -1);
    mountMethod = 0;

    if (searchBar)
    {
        HaltGui();
        mainWindow->Append(searchBar);
        ResumeGui();
    }

	return 0;
}

void GameBrowseMenu::LoadCover(struct discHdr *header)
{
    DownloadBtn->SetImage(NULL);
    if(gameCover)
        delete gameCover;

    gameCover = LoadCoverImage(header);

    if (gameCoverImg)
        delete gameCoverImg;

    gameCoverImg = new GuiImage(gameCover);
    gameCoverImg->SetWidescreen(Settings.widescreen);

    DownloadBtn->SetImage(gameCoverImg);// put the new image on the download button
}

void GameBrowseMenu::CheckOcarina(const char * IDfull)
{
    char filepath[200];
    snprintf(filepath, sizeof(filepath), "%s%s.gct", Settings.Cheatcodespath, IDfull);
    if (CheckFile(filepath) == false)
    {
        gprintf("\ttried to load missing gct.\n");
        sprintf(filepath, "%s %s", filepath, tr( "does not exist!  Loading game without cheats." ));
        WindowPrompt(tr( "Error" ), filepath, NULL, NULL, NULL, NULL, 170);
    }
}

void GameBrowseMenu::CheckAlternativeDOL(const char * IDfull)
{
    char filepath[200];
    snprintf(filepath, sizeof(filepath), "%s%s.dol", Settings.dolpath, IDfull);
    if (CheckFile(filepath) == false)
    {
        sprintf(filepath, "%s %s", filepath, tr( "does not exist!" ));
        WindowPrompt(tr( "Error" ), filepath, tr( "OK" ));
    }
}

void GameBrowseMenu::UpdateCallback(void * e)
{
    //! Draw the selected Icon allways on top
    GameBrowseMenu * w = (GameBrowseMenu *) e;

    for(u32 i = 0; i < w->ToolBar.size(); ++i)
    {
        if(w->ToolBar[i]->GetState() == STATE_SELECTED)
        {
            w->Remove(w->ToolBar[i]);
            w->Append(w->ToolBar[i]);
            break;
        }
    }
}

void GameBrowseMenu::UpdateFreeSpace(void * arg)
{
    char spaceinfo[30];
    memset(spaceinfo, 0, 30);

    if(Settings.ShowFreeSpace)
    {
        float freespace = 0.0, used = 0.0;
        WBFS_DiskSpace(&used, &freespace);
        if (strcmp(Settings.db_language, "JA") == 0)
        {
            // needs to be "total...used" for Japanese
            sprintf(spaceinfo, "%.2fGB %s %.2fGB %s", (freespace + used), tr( "of" ), freespace, tr( "free" ));
        }
        else
        {
            sprintf(spaceinfo, "%.2fGB %s %.2fGB %s", freespace, tr( "of" ), (freespace + used), tr( "free" ));
        }
    }

    if(Exiting)
        return;

    usedSpaceTxt->SetText(spaceinfo);
}
