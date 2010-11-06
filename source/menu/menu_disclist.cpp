#include "menus.h"
#include "fatmounter.h"
#include "usbloader/wdvd.h"
#include "usbloader/GameList.h"
#include "usbloader/wbfs.h"
#include "patches/fst.h"
#include "network/networkops.h"
#include "prompts/gameinfo.h"
#include "prompts/DiscBrowser.h"
#include "prompts/TitleBrowser.h"
#include "settings/Settings.h"
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/GameTitles.h"
#include "themes/CTheme.h"
#include "GameBootProcess.h"
#include "wpad.h"
#include "sys.h"

#include "libwiigui/gui_gamebrowser.h"
#include "libwiigui/gui_gamegrid.h"
#include "libwiigui/gui_gamecarousel.h"
#include "libwiigui/gui_searchbar.h"

extern u8 * gameScreenTex;
extern struct discHdr *dvdheader;
extern u8 mountMethod;
extern int load_from_fs;
extern s32 gameSelected;
extern GuiText * GameIDTxt;
extern GuiText * GameRegionTxt;
extern const u8 data1;
extern FreeTypeGX *fontClock;
extern bool updateavailable;
extern int cntMissFiles;
extern GuiImageData * cover;
extern GuiImage * coverImg;
extern GuiImageData * pointer[4];
extern bool altdoldefault;
extern GuiImage * bgImg;

GuiButton *Toolbar[9];
int idiotFlag = -1;
char idiotChar[50];

void DiscListWinUpdateCallback(void * e);
void rockout(int f = 0);

static u32 startat = 0;

/****************************************************************************
 * MenuDiscList
 ***************************************************************************/
int MenuDiscList()
{
    gprintf("MenuDiscList()\n");
    gameList.FilterList();
    int offset = MIN( ( int )startat, gameList.size() - 1 );
    startat = offset;
    int datagB = 0;
    int dataed = -1;
    int selectImg1 = 0;
    char ID[4];
    char IDfull[7];
    u32 covert = 0;
    if (!dvdheader) dvdheader = new struct discHdr;
    u8 mountMethodOLD = 0;

    WDVD_GetCoverStatus(&covert);
    u32 covertOld = covert;

    f32 freespace, used, size = 0.0;
    wchar_t searchChar;
    //SCREENSAVER
    int check = 0; //to skip the first cycle when wiimote isn't completely connected

    datagB = 0;
    int menu = MENU_NONE;

    u32 nolist;
    int choice = 0, selectedold = 100;
    s32 ret;

    //CLOCK
    struct tm * timeinfo;
    char theTime[80] = "";
    time_t lastrawtime = 0;

    if (mountMethod != 3 && WBFS_ShowFreeSpace())
    {
        WBFS_DiskSpace(&used, &freespace);
    }

    if (!gameList.size()) //if there is no list of games to display
    {
        nolist = 1;
    }

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
    // because destroy GuiSound must wait while sound playing is finished, we use a global sound
    if (!btnClick2) btnClick2 = new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    //  GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    GuiImageData btnInstall(Resources::GetFile("button_install.png"), Resources::GetFileSize("button_install.png"));
    GuiImageData btnInstallOver(Resources::GetFile("button_install_over.png"), Resources::GetFileSize("button_install_over.png"));

    GuiImageData btnSettings(Resources::GetFile("settings_button.png"), Resources::GetFileSize("settings_button.png"));
    GuiImageData btnSettingsOver(Resources::GetFile("settings_button_over.png"), Resources::GetFileSize("settings_button_over.png"));

    GuiImageData btnpwroff(Resources::GetFile("wiimote_poweroff.png"), Resources::GetFileSize("wiimote_poweroff.png"));
    GuiImageData btnpwroffOver(Resources::GetFile("wiimote_poweroff_over.png"), Resources::GetFileSize("wiimote_poweroff_over.png"));
    GuiImageData btnhome(Resources::GetFile("menu_button.png"), Resources::GetFileSize("menu_button.png"));
    GuiImageData btnhomeOver(Resources::GetFile("menu_button_over.png"), Resources::GetFileSize("menu_button_over.png"));
    GuiImageData btnsdcardOver(Resources::GetFile("sdcard_over.png"), Resources::GetFileSize("sdcard_over.png"));
    GuiImageData btnsdcard(Resources::GetFile("sdcard.png"), Resources::GetFileSize("sdcard.png"));

    GuiImageData imgfavIcon(Resources::GetFile("favIcon.png"), Resources::GetFileSize("favIcon.png"));
    GuiImageData imgfavIcon_gray(Resources::GetFile("favIcon_gray.png"), Resources::GetFileSize("favIcon_gray.png"));
    GuiImageData imgsearchIcon(Resources::GetFile("searchIcon.png"), Resources::GetFileSize("searchIcon.png"));
    GuiImageData imgsearchIcon_gray(Resources::GetFile("searchIcon_gray.png"), Resources::GetFileSize("searchIcon_gray.png"));
    GuiImageData imgabcIcon(Resources::GetFile("abcIcon.png"), Resources::GetFileSize("abcIcon.png"));
    GuiImageData imgabcIcon_gray(Resources::GetFile("abcIcon_gray.png"), Resources::GetFileSize("abcIcon_gray.png"));
    GuiImageData imgrankIcon(Resources::GetFile("rankIcon.png"), Resources::GetFileSize("rankIcon.png"));
    GuiImageData imgrankIcon_gray(Resources::GetFile("rankIcon_gray.png"), Resources::GetFileSize("rankIcon_gray.png"));
    GuiImageData imgplayCountIcon(Resources::GetFile("playCountIcon.png"), Resources::GetFileSize("playCountIcon.png"));
    GuiImageData imgplayCountIcon_gray(Resources::GetFile("playCountIcon_gray.png"), Resources::GetFileSize("playCountIcon_gray.png"));
    GuiImageData imgarrangeGrid(Resources::GetFile("arrangeGrid.png"), Resources::GetFileSize("arrangeGrid.png"));
    GuiImageData imgarrangeGrid_gray(Resources::GetFile("arrangeGrid_gray.png"), Resources::GetFileSize("arrangeGrid_gray.png"));
    GuiImageData imgarrangeList(Resources::GetFile("arrangeList.png"), Resources::GetFileSize("arrangeList.png"));
    GuiImageData imgarrangeList_gray(Resources::GetFile("arrangeList_gray.png"), Resources::GetFileSize("arrangeList_gray.png"));
    GuiImageData imgarrangeCarousel(Resources::GetFile("arrangeCarousel.png"), Resources::GetFileSize("arrangeCarousel.png"));
    GuiImageData imgarrangeCarousel_gray(Resources::GetFile("arrangeCarousel_gray.png"), Resources::GetFileSize("arrangeCarousel_gray.png"));

    GuiImageData imgLock(Resources::GetFile("lock.png"), Resources::GetFileSize("lock.png"));
    GuiImageData imgLock_gray(Resources::GetFile("lock_gray.png"), Resources::GetFileSize("lock_gray.png"));
    GuiImageData imgUnlock(Resources::GetFile("lock_gray.png"), Resources::GetFileSize("lock_gray.png"));
    GuiImageData imgUnlock_gray(Resources::GetFile("unlock_gray.png"), Resources::GetFileSize("unlock_gray.png"));

    GuiImageData imgdvd(Resources::GetFile("dvd.png"), Resources::GetFileSize("dvd.png"));
    GuiImageData imgdvd_gray(Resources::GetFile("dvd_gray.png"), Resources::GetFileSize("dvd_gray.png"));

    GuiImageData homebrewImgData(Resources::GetFile("browser.png"), Resources::GetFileSize("browser.png"));
    GuiImageData homebrewImgDataOver(Resources::GetFile("browser_over.png"), Resources::GetFileSize("browser_over.png"));

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
    GuiTrigger trig2;
    trig2.SetButtonOnlyTrigger(-1, WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X, 0);
    GuiTrigger trig1;
    trig1.SetButtonOnlyTrigger(-1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y, 0);
    GuiTrigger trigZ;
    trigZ.SetButtonOnlyTrigger(-1, WPAD_NUNCHUK_BUTTON_Z | WPAD_CLASSIC_BUTTON_ZL, PAD_TRIGGER_Z);

    GuiButton screenShotBtn(0, 0);
    screenShotBtn.SetPosition(0, 0);
    screenShotBtn.SetTrigger(&trigZ);

    char spaceinfo[30];
    if (load_from_fs == PART_FS_FAT)
    {
        memset(spaceinfo, 0, 30);
    }
    else
    {
        if (!strcmp(Settings.db_language, "JA"))
        {
            // needs to be "total...used" for Japanese
            sprintf(spaceinfo, (mountMethod != 3 ? "%.2fGB %s %.2fGB %s" : " "), (freespace + used), tr( "of" ),
                    freespace, tr( "free" ));
        }
        else
        {
            sprintf(spaceinfo, (mountMethod != 3 ? "%.2fGB %s %.2fGB %s" : " "), freespace, tr( "of" ), (freespace
                    + used), tr( "free" ));
        }
    }
    GuiText usedSpaceTxt(spaceinfo, 18, Theme.info);
    usedSpaceTxt.SetAlignment(Theme.hddinfo_align, ALIGN_TOP);
    usedSpaceTxt.SetPosition(Theme.hddinfo_x, Theme.hddinfo_y);

    char GamesCnt[15];
    sprintf(GamesCnt, "%s: %i", (mountMethod != 3 ? tr( "Games" ) : tr( "Channels" )), gameList.size());
    GuiText gamecntTxt(GamesCnt, 18, Theme.info);

    GuiButton gamecntBtn(100, 18);
    gamecntBtn.SetAlignment(Theme.gamecount_align, ALIGN_TOP);
    gamecntBtn.SetPosition(Theme.gamecount_x, Theme.gamecount_y);
    gamecntBtn.SetLabel(&gamecntTxt);
    gamecntBtn.SetEffectGrow();
    gamecntBtn.SetTrigger(&trigA);

    GuiTooltip installBtnTT(tr( "Install a game" ));
    if (Settings.wsprompt) installBtnTT.SetWidescreen(Settings.widescreen);
    installBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage installBtnImg(&btnInstall);
    GuiImage installBtnImgOver(&btnInstallOver);
    installBtnImg.SetWidescreen(Settings.widescreen);
    installBtnImgOver.SetWidescreen(Settings.widescreen);

    GuiButton installBtn(&installBtnImg, &installBtnImgOver, ALIGN_LEFT, ALIGN_TOP, Theme.install_x, Theme.install_y,
            &trigA, &btnSoundOver, btnClick2, 1, &installBtnTT, 24, -30, 0, 5);

    GuiTooltip settingsBtnTT(tr( "Settings" ));
    if (Settings.wsprompt) settingsBtnTT.SetWidescreen(Settings.widescreen);
    settingsBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage settingsBtnImg(&btnSettings);
    settingsBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage settingsBtnImgOver(&btnSettingsOver);
    settingsBtnImgOver.SetWidescreen(Settings.widescreen);
    GuiButton settingsBtn(&settingsBtnImg, &settingsBtnImgOver, 0, 3, Theme.setting_x, Theme.setting_y, &trigA,
            &btnSoundOver, btnClick2, 1, &settingsBtnTT, 65, -30, 0, 5);

    GuiTooltip homeBtnTT(tr( "Back to HBC or Wii Menu" ));
    if (Settings.wsprompt) homeBtnTT.SetWidescreen(Settings.widescreen);
    settingsBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage homeBtnImg(&btnhome);
    homeBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage homeBtnImgOver(&btnhomeOver);
    homeBtnImgOver.SetWidescreen(Settings.widescreen);
    GuiButton homeBtn(&homeBtnImg, &homeBtnImgOver, 0, 3, Theme.home_x, Theme.home_y, &trigA, &btnSoundOver, btnClick2,
            1, &homeBtnTT, 15, -30, 1, 5);
    homeBtn.RemoveSoundClick();
    homeBtn.SetTrigger(&trigHome);

    GuiTooltip poweroffBtnTT(tr( "Power off the Wii" ));
    if (Settings.wsprompt) poweroffBtnTT.SetWidescreen(Settings.widescreen);
    poweroffBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage poweroffBtnImg(&btnpwroff);
    GuiImage poweroffBtnImgOver(&btnpwroffOver);
    poweroffBtnImg.SetWidescreen(Settings.widescreen);
    poweroffBtnImgOver.SetWidescreen(Settings.widescreen);
    GuiButton poweroffBtn(&poweroffBtnImg, &poweroffBtnImgOver, 0, 3, Theme.power_x, Theme.power_y, &trigA,
            &btnSoundOver, btnClick2, 1, &poweroffBtnTT, -10, -30, 1, 5);

    GuiTooltip sdcardBtnTT(tr( "Reload SD" ));
    if (Settings.wsprompt) sdcardBtnTT.SetWidescreen(Settings.widescreen);
    sdcardBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage sdcardImg(&btnsdcard);
    GuiImage sdcardImgOver(&btnsdcardOver);
    sdcardImg.SetWidescreen(Settings.widescreen);
    sdcardImgOver.SetWidescreen(Settings.widescreen);
    GuiButton sdcardBtn(&sdcardImg, &sdcardImgOver, 0, 3, Theme.sdcard_x, Theme.sdcard_y, &trigA, &btnSoundOver,
            btnClick2, 1, &sdcardBtnTT, 15, -30, 0, 5);

    GuiButton gameInfo(0, 0);
    gameInfo.SetTrigger(&trig2);
    gameInfo.SetSoundClick(btnClick2);

    GuiTooltip favoriteBtnTT(tr( "Display favorites" ));
    if (Settings.wsprompt) favoriteBtnTT.SetWidescreen(Settings.widescreen);
    favoriteBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage favoriteBtnImg(&imgfavIcon);
    favoriteBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage favoriteBtnImg_g(&imgfavIcon_gray);
    favoriteBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton favoriteBtn(&favoriteBtnImg_g, &favoriteBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_favorite_x,
            Theme.gamelist_favorite_y, &trigA, &btnSoundOver, btnClick2, 1, &favoriteBtnTT, -15, 52, 0, 3);
    favoriteBtn.SetAlpha(180);

    GuiTooltip searchBtnTT(tr( "Set Search-Filter" ));
    if (Settings.wsprompt) searchBtnTT.SetWidescreen(Settings.widescreen);
    searchBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage searchBtnImg(&imgsearchIcon);
    searchBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage searchBtnImg_g(&imgsearchIcon_gray);
    searchBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton searchBtn(&searchBtnImg_g, &searchBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_search_x,
            Theme.gamelist_search_y, &trigA, &btnSoundOver, btnClick2, 1, &searchBtnTT, -15, 52, 0, 3);
    searchBtn.SetAlpha(180);

    GuiTooltip abcBtnTT(Settings.GameSort == SORT_RANKING ? tr( "Sort by rank" ) : tr( "Sort alphabetically" ));
    if (Settings.wsprompt) abcBtnTT.SetWidescreen(Settings.widescreen);
    abcBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage abcBtnImg(Settings.GameSort == SORT_RANKING ? &imgrankIcon : &imgabcIcon);
    abcBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage abcBtnImg_g(Settings.GameSort == SORT_RANKING ? &imgrankIcon_gray : &imgabcIcon_gray);
    abcBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton abcBtn(&abcBtnImg_g, &abcBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_abc_x, Theme.gamelist_abc_y,
            &trigA, &btnSoundOver, btnClick2, 1, &abcBtnTT, -15, 52, 0, 3);
    abcBtn.SetAlpha(180);

    GuiTooltip countBtnTT(tr( "Sort order by most played" ));
    if (Settings.wsprompt) countBtnTT.SetWidescreen(Settings.widescreen);
    countBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage countBtnImg(&imgplayCountIcon);
    countBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage countBtnImg_g(&imgplayCountIcon_gray);
    countBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton countBtn(&countBtnImg_g, &countBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_count_x,
            Theme.gamelist_count_y, &trigA, &btnSoundOver, btnClick2, 1, &countBtnTT, -15, 52, 0, 3);
    countBtn.SetAlpha(180);

    GuiTooltip listBtnTT(tr( "Display as a list" ));
    if (Settings.wsprompt) listBtnTT.SetWidescreen(Settings.widescreen);
    listBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage listBtnImg(&imgarrangeList);
    listBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage listBtnImg_g(&imgarrangeList_gray);
    listBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton listBtn(&listBtnImg_g, &listBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_list_x,
            Theme.gamelist_list_y, &trigA, &btnSoundOver, btnClick2, 1, &listBtnTT, 15, 52, 1, 3);
    listBtn.SetAlpha(180);

    GuiTooltip gridBtnTT(tr( "Display as a grid" ));
    if (Settings.wsprompt) gridBtnTT.SetWidescreen(Settings.widescreen);
    gridBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage gridBtnImg(&imgarrangeGrid);
    gridBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage gridBtnImg_g(&imgarrangeGrid_gray);
    gridBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton gridBtn(&gridBtnImg_g, &gridBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_grid_x,
            Theme.gamelist_grid_y, &trigA, &btnSoundOver, btnClick2, 1, &gridBtnTT, 15, 52, 1, 3);
    gridBtn.SetAlpha(180);

    GuiTooltip carouselBtnTT(tr( "Display as a carousel" ));
    if (Settings.wsprompt) carouselBtnTT.SetWidescreen(Settings.widescreen);
    carouselBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage carouselBtnImg(&imgarrangeCarousel);
    carouselBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage carouselBtnImg_g(&imgarrangeCarousel_gray);
    carouselBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton carouselBtn(&carouselBtnImg_g, &carouselBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_carousel_x,
            Theme.gamelist_carousel_y, &trigA, &btnSoundOver, btnClick2, 1, &carouselBtnTT, 15, 52, 1, 3);
    carouselBtn.SetAlpha(180);

    bool canUnlock = (Settings.parentalcontrol == 0 && Settings.Parental.enabled == 1);

    GuiTooltip lockBtnTT(canUnlock ? tr( "Unlock Parental Control" ) : tr( "Parental Control disabled" ));
    if (Settings.wsprompt) lockBtnTT.SetWidescreen(Settings.widescreen);
    lockBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage lockBtnImg(&imgLock);
    lockBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage lockBtnImg_g(&imgLock_gray);
    lockBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton lockBtn(&lockBtnImg_g, &lockBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_lock_x,
            Theme.gamelist_lock_y, &trigA, &btnSoundOver, btnClick2, 1, &lockBtnTT, 15, 52, 1, 3);
    lockBtn.SetAlpha(180);

    GuiTooltip unlockBtnTT(tr( "Enable Parental Control" ));
    if (Settings.wsprompt) unlockBtnTT.SetWidescreen(Settings.widescreen);
    unlockBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage unlockBtnImg(&imgUnlock);
    unlockBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage unlockBtnImg_g(&imgUnlock_gray);
    unlockBtnImg_g.SetWidescreen(Settings.widescreen);

    if (canUnlock && Settings.godmode)
    {
        lockBtn.SetImage(&unlockBtnImg_g);
        lockBtn.SetImageOver(&unlockBtnImg_g);
        lockBtn.SetToolTip(&unlockBtnTT, 15, 52, 1, 3);
    }

    /*
     GuiButton unlockBtn(&unlockBtnImg_g, &unlockBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_lock_x, Theme.gamelist_lock_y, &trigA, &btnSoundOver, btnClick2,1, &lockBtnTT, 15, 52, 1, 3);
     unlockBtn.SetAlpha(180);
     */

    GuiTooltip dvdBtnTT(tr( "Mount DVD drive" ));
    if (Settings.wsprompt) dvdBtnTT.SetWidescreen(Settings.widescreen);
    dvdBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage dvdBtnImg(&imgdvd);
    dvdBtnImg.SetWidescreen(Settings.widescreen);
    GuiImage dvdBtnImg_g(dvdBtnImg);
    dvdBtnImg_g.SetWidescreen(Settings.widescreen);
    GuiButton dvdBtn(&dvdBtnImg_g, &dvdBtnImg_g, ALIGN_LEFT, ALIGN_TOP, Theme.gamelist_dvd_x, Theme.gamelist_dvd_y,
            &trigA, &btnSoundOver, btnClick2, 1, &dvdBtnTT, 15, 52, 1, 3);
    dvdBtn.SetAlpha(180);

    GuiTooltip homebrewBtnTT(tr( "Homebrew Launcher" ));
    if (Settings.wsprompt) homebrewBtnTT.SetWidescreen(Settings.widescreen);
    homebrewBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiImage homebrewImg(&homebrewImgData);
    GuiImage homebrewImgOver(&homebrewImgDataOver);
    homebrewImg.SetWidescreen(Settings.widescreen);
    homebrewImgOver.SetWidescreen(Settings.widescreen);
    GuiButton homebrewBtn(&homebrewImg, &homebrewImgOver, ALIGN_LEFT, ALIGN_TOP, Theme.homebrew_x, Theme.homebrew_y,
            &trigA, &btnSoundOver, btnClick2, 1, &homebrewBtnTT, 15, -30, 1, 5);

    if (Settings.GameSort == SORT_RANKING)
    {
        favoriteBtn.SetImage(&favoriteBtnImg);
        favoriteBtn.SetImageOver(&favoriteBtnImg);
        favoriteBtn.SetAlpha(255);
    }
    static bool show_searchwindow = false;
    if (*gameList.GetCurrentFilter())
    {
        if (show_searchwindow && gameList.size() == 1) show_searchwindow = false;
        if (!show_searchwindow) searchBtn.SetEffect(EFFECT_PULSE, 10, 105);
        searchBtn.SetImage(&searchBtnImg);
        searchBtn.SetImageOver(&searchBtnImg);
        searchBtn.SetAlpha(255);
    }
    if (Settings.GameSort == SORT_ABC)
    {
        abcBtn.SetImage(&abcBtnImg);
        abcBtn.SetImageOver(&abcBtnImg);
        abcBtn.SetAlpha(255);
    }
    else if (Settings.GameSort == SORT_PLAYCOUNT)
    {
        countBtn.SetImage(&countBtnImg);
        countBtn.SetImageOver(&countBtnImg);
        countBtn.SetAlpha(255);
    }
    if (Settings.gameDisplay == LIST_MODE)
    {
        listBtn.SetImage(&listBtnImg);
        listBtn.SetImageOver(&listBtnImg);
        listBtn.SetAlpha(255);
    }
    else if (Settings.gameDisplay == GRID_MODE)
    {
        gridBtn.SetImage(&gridBtnImg);
        gridBtn.SetImageOver(&gridBtnImg);
        gridBtn.SetAlpha(255);
    }
    else if (Settings.gameDisplay == CAROUSEL_MODE)
    {
        carouselBtn.SetImage(&carouselBtnImg);
        carouselBtn.SetImageOver(&carouselBtnImg);
        carouselBtn.SetAlpha(255);
    }

    if (Settings.gameDisplay == LIST_MODE)
    {
        favoriteBtn.SetPosition(Theme.gamelist_favorite_x, Theme.gamelist_favorite_y);
        searchBtn.SetPosition(Theme.gamelist_search_x, Theme.gamelist_search_y);
        abcBtn.SetPosition(Theme.gamelist_abc_x, Theme.gamelist_abc_y);
        countBtn.SetPosition(Theme.gamelist_count_x, Theme.gamelist_count_y);
        listBtn.SetPosition(Theme.gamelist_list_x, Theme.gamelist_list_y);
        gridBtn.SetPosition(Theme.gamelist_grid_x, Theme.gamelist_grid_y);
        carouselBtn.SetPosition(Theme.gamelist_carousel_x, Theme.gamelist_carousel_y);
        lockBtn.SetPosition(Theme.gamelist_lock_x, Theme.gamelist_lock_y);
        dvdBtn.SetPosition(Theme.gamelist_dvd_x, Theme.gamelist_dvd_y);
    }
    else if (Settings.gameDisplay == GRID_MODE)
    {
        favoriteBtn.SetPosition(Theme.gamegrid_favorite_x, Theme.gamegrid_favorite_y);
        searchBtn.SetPosition(Theme.gamegrid_search_x, Theme.gamegrid_search_y);
        abcBtn.SetPosition(Theme.gamegrid_abc_x, Theme.gamegrid_abc_y);
        countBtn.SetPosition(Theme.gamegrid_count_x, Theme.gamegrid_count_y);
        listBtn.SetPosition(Theme.gamegrid_list_x, Theme.gamegrid_list_y);
        gridBtn.SetPosition(Theme.gamegrid_grid_x, Theme.gamegrid_grid_y);
        carouselBtn.SetPosition(Theme.gamegrid_carousel_x, Theme.gamegrid_carousel_y);
        lockBtn.SetPosition(Theme.gamegrid_lock_x, Theme.gamegrid_lock_y);
        dvdBtn.SetPosition(Theme.gamegrid_dvd_x, Theme.gamegrid_dvd_y);
    }
    else if (Settings.gameDisplay == CAROUSEL_MODE)
    {
        favoriteBtn.SetPosition(Theme.gamecarousel_favorite_x, Theme.gamecarousel_favorite_y);
        searchBtn.SetPosition(Theme.gamecarousel_search_x, Theme.gamecarousel_favorite_y);
        abcBtn.SetPosition(Theme.gamecarousel_abc_x, Theme.gamecarousel_abc_y);
        countBtn.SetPosition(Theme.gamecarousel_count_x, Theme.gamecarousel_count_y);
        listBtn.SetPosition(Theme.gamecarousel_list_x, Theme.gamecarousel_list_y);
        gridBtn.SetPosition(Theme.gamecarousel_grid_x, Theme.gamecarousel_grid_y);
        carouselBtn.SetPosition(Theme.gamecarousel_carousel_x, Theme.gamecarousel_carousel_y);
        lockBtn.SetPosition(Theme.gamecarousel_lock_x, Theme.gamecarousel_lock_y);
        dvdBtn.SetPosition(Theme.gamecarousel_dvd_x, Theme.gamecarousel_dvd_y);
    }

    //Downloading Covers
    GuiTooltip DownloadBtnTT(tr( "Click to Download Covers" ));
    if (Settings.wsprompt) DownloadBtnTT.SetWidescreen(Settings.widescreen);
    DownloadBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiButton DownloadBtn(0, 0);
    DownloadBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    DownloadBtn.SetPosition(Theme.covers_x, Theme.covers_y);

    GuiTooltip IDBtnTT(tr( "Click to change game ID" ));
    if (Settings.wsprompt) IDBtnTT.SetWidescreen(Settings.widescreen);
    IDBtnTT.SetAlpha(Theme.tooltipAlpha);
    GuiButton idBtn(0, 0);
    idBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    idBtn.SetPosition(Theme.id_x, Theme.id_y);

    if (Settings.godmode == 1 && mountMethod != 3) //only make the button have trigger & tooltip if in godmode
    {
        DownloadBtn.SetSoundOver(&btnSoundOver);
        DownloadBtn.SetTrigger(&trigA);
        DownloadBtn.SetTrigger(&trig1);
        DownloadBtn.SetToolTip(&DownloadBtnTT, 205, -30);

        idBtn.SetSoundOver(&btnSoundOver);
        idBtn.SetTrigger(&trigA);
        idBtn.SetToolTip(&IDBtnTT, 205, -30);

    }
    else
    {
        DownloadBtn.SetRumble(false);
        idBtn.SetRumble(false);
    }

    GuiGameBrowser * gameBrowser = NULL;
    GuiGameGrid * gameGrid = NULL;
    GuiGameCarousel * gameCarousel = NULL;
    if (Settings.gameDisplay == LIST_MODE)
    {
        gameBrowser = new GuiGameBrowser(Theme.gamelist_w, Theme.gamelist_h, startat, offset);
        gameBrowser->SetPosition(Theme.gamelist_x, Theme.gamelist_y);
        gameBrowser->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }
    else if (Settings.gameDisplay == GRID_MODE)
    {
        gameGrid = new GuiGameGrid(Theme.gamegrid_w, Theme.gamegrid_h, Settings.theme_path, bg_options_png, 0, 0);
        gameGrid->SetPosition(Theme.gamegrid_x, Theme.gamegrid_y);
        gameGrid->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }
    else if (Settings.gameDisplay == CAROUSEL_MODE)
    {
        //GuiGameCarousel gameCarousel(Theme.gamecarousel_w, Theme.gamecarousel_h, gameList, gameList.size(), Settings.theme_path, bg_options_png, startat, offset);
        gameCarousel = new GuiGameCarousel(640, 400, Settings.theme_path, bg_options_png, startat, offset);
        gameCarousel->SetPosition(Theme.gamecarousel_x, Theme.gamecarousel_y);
        gameCarousel->SetAlignment(ALIGN_LEFT, ALIGN_CENTRE);
    }

    GuiText clockTimeBack("88:88", 40, (GXColor) {Theme.clock.r, Theme.clock.g, Theme.clock.b, Theme.clock.a / 6});
    clockTimeBack.SetAlignment(Theme.clock_align, ALIGN_TOP);
    clockTimeBack.SetPosition(Theme.clock_x, Theme.clock_y);
    clockTimeBack.SetFont(clock_ttf, clock_ttf_size);
    GuiText clockTime(theTime, 40, Theme.clock);
    clockTime.SetAlignment(Theme.clock_align, ALIGN_TOP);
    clockTime.SetPosition(Theme.clock_x, Theme.clock_y);
    clockTime.SetFont(clock_ttf, clock_ttf_size);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);

    if (Theme.show_hddinfo == -1 || Theme.show_hddinfo == 1) //force show hdd info
    {
        w.Append(&usedSpaceTxt);
    }
    if (Theme.show_gamecount == -1 || Theme.show_gamecount == 1) //force show game cnt info
    {
        w.Append(&gamecntBtn);
    }
    w.Append(&sdcardBtn);
    w.Append(&poweroffBtn);
    w.Append(&gameInfo);
    if (Settings.godmode) w.Append(&installBtn);
    w.Append(&homeBtn);
    w.Append(&settingsBtn);
    w.Append(&DownloadBtn);
    w.Append(&idBtn);
    w.Append(&screenShotBtn);

    // Begin Toolbar
    w.Append(&favoriteBtn);
    Toolbar[0] = &favoriteBtn;
    w.Append(&searchBtn);
    Toolbar[1] = &searchBtn;
    w.Append(&abcBtn);
    Toolbar[2] = &abcBtn;
    w.Append(&countBtn);
    Toolbar[3] = &countBtn;
    w.Append(&listBtn);
    Toolbar[4] = &listBtn;
    w.Append(&gridBtn);
    Toolbar[5] = &gridBtn;
    w.Append(&carouselBtn);
    Toolbar[6] = &carouselBtn;
    w.Append(&lockBtn);
    Toolbar[7] = &lockBtn;
    w.Append(&dvdBtn);
    Toolbar[8] = &dvdBtn;
    w.SetUpdateCallback(DiscListWinUpdateCallback);
    // End Toolbar


    if (Settings.godmode == 1) w.Append(&homebrewBtn);

    if ((Settings.hddinfo == CLOCK_HR12) || (Settings.hddinfo == CLOCK_HR24))
    {
        w.Append(&clockTimeBack);
        w.Append(&clockTime);
    }

    if (Settings.gameDisplay == LIST_MODE)
    {
        mainWindow->Append(gameBrowser);
    }
    if (Settings.gameDisplay == GRID_MODE)
    {
        mainWindow->Append(gameGrid);
    }
    if (Settings.gameDisplay == CAROUSEL_MODE)
    {
        mainWindow->Append(gameCarousel);
    }
    mainWindow->Append(&w);

    GuiSearchBar *searchBar = NULL;
    if (show_searchwindow)
    {
        searchBar = new GuiSearchBar(gameList.GetAvailableSearchChars());
        if (searchBar) mainWindow->Append(searchBar);
    }

    ResumeGui();

    //  ShowMemInfo();

    while (menu == MENU_NONE)
    {

        if (idiotFlag == 1)
        {
            gprintf("\tIdiot flag\n");
            char idiotBuffer[200];
            snprintf(idiotBuffer, sizeof(idiotBuffer), "%s (%s). %s", tr( "You have attempted to load a bad image" ),
                    idiotChar, tr( "Most likely it has dimensions that are not evenly divisible by 4." ));

            int deleteImg = WindowPrompt(0, idiotBuffer, tr( "OK" ), tr( "Delete" ));
            if (deleteImg == 0)
            {
                snprintf(idiotBuffer, sizeof(idiotBuffer), "%s %s.", tr( "You are about to delete " ), idiotChar);
                deleteImg = WindowPrompt(tr( "Confirm" ), idiotBuffer, tr( "Delete" ), tr( "Cancel" ));
                if (deleteImg == 1)
                {
                    remove(idiotChar);
                }
            }
            idiotFlag = -1;
        }

        WDVD_GetCoverStatus(&covert);//for detecting if i disc has been inserted

        //CLOCK
        time_t rawtime = time(0);
        if (((Settings.hddinfo == CLOCK_HR12) || (Settings.hddinfo == CLOCK_HR24)) && rawtime != lastrawtime)
        {
            lastrawtime = rawtime;
            timeinfo = localtime(&rawtime);
            if (dataed < 1)
            {
                if (Settings.hddinfo == CLOCK_HR12)
                {
                    if (rawtime & 1)
                        strftime(theTime, sizeof(theTime), "%I:%M", timeinfo);
                    else strftime(theTime, sizeof(theTime), "%I %M", timeinfo);
                }
                if (Settings.hddinfo == CLOCK_HR24)
                {
                    if (rawtime & 1)
                        strftime(theTime, sizeof(theTime), "%H:%M", timeinfo);
                    else strftime(theTime, sizeof(theTime), "%H %M", timeinfo);
                }
                clockTime.SetText(theTime);

            }
            else if (dataed > 0)
            {

                clockTime.SetTextf("%i", (dataed - 1));
            }

        }

        // respond to button presses
        if (shutdown == 1)
        {
            gprintf("\n\tshutdown");
            Sys_Shutdown();
        }
        if (reset == 1) Sys_Reboot();

        if (updateavailable == true)
        {
            gprintf("\tUpdate Available\n");
            HaltGui();
            GuiWindow ww(640, 480);
            w.SetState(STATE_DISABLED);
            mainWindow->Append(&ww);
            ResumeGui();
            ProgressUpdateWindow();
            updateavailable = false;
            mainWindow->Remove(&ww);
            w.SetState(STATE_DEFAULT);
            menu = MENU_DISCLIST;
        }

        if (poweroffBtn.GetState() == STATE_CLICKED)
        {

            gprintf("\tpoweroffBtn clicked\n");
            choice = WindowPrompt(tr( "How to Shutdown?" ), 0, tr( "Full Shutdown" ), tr( "Shutdown to Idle" ),
                    tr( "Cancel" ));
            if (choice == 2)
            {
                Sys_ShutdownToIdel();
            }
            else if (choice == 1)
            {
                Sys_ShutdownToStandby();
            }
            else
            {
                poweroffBtn.ResetState();
                if (Settings.gameDisplay == LIST_MODE)
                {
                    gameBrowser->SetFocus(1);
                }
                else if (Settings.gameDisplay == GRID_MODE)
                {
                    gameGrid->SetFocus(1);
                }
                else if (Settings.gameDisplay == CAROUSEL_MODE)
                {
                    gameCarousel->SetFocus(1);
                }
            }

        }
        else if (gamecntBtn.GetState() == STATE_CLICKED && mountMethod != 3)
        {
            gprintf("\tgameCntBtn clicked\n");
            gamecntBtn.ResetState();
            char linebuf[150];
            snprintf(linebuf, sizeof(linebuf), "%s %sGameList ?", tr( "Save Game List to" ), Settings.update_path);
            choice = WindowPrompt(0, linebuf, "TXT", "CSV", tr( "Back" ));
            if (choice)
            {
                if (save_gamelist(choice - 1))
                    WindowPrompt(0, tr( "Saved" ), tr( "OK" ));
                else WindowPrompt(tr( "Error" ), tr( "Could not save." ), tr( "OK" ));
            }
            menu = MENU_DISCLIST;
            break;

        }
        else if (screenShotBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tscreenShotBtn clicked\n");
            screenShotBtn.ResetState();
            ScreenShot();
        }
        else if (homeBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\thomeBtn clicked\n");
            bgMusic->Pause();
            choice = WindowExitPrompt();
            bgMusic->Resume();

            if (choice == 3)
            {
                Sys_LoadMenu(); // Back to System Menu
            }
            else if (choice == 2)
            {
                Sys_BackToLoader();
            }
            else
            {
                homeBtn.ResetState();
                if (Settings.gameDisplay == LIST_MODE)
                {
                    gameBrowser->SetFocus(1);
                }
                else if (Settings.gameDisplay == GRID_MODE)
                {
                    gameGrid->SetFocus(1);
                }
                else if (Settings.gameDisplay == CAROUSEL_MODE)
                {
                    gameCarousel->SetFocus(1);
                }
            }

        }
        else if (installBtn.GetState() == STATE_CLICKED)
        {
            choice = WindowPrompt(tr( "Install a game" ), 0, tr( "Yes" ), tr( "No" ));
            if (choice == 1)
            {
                menu = MENU_INSTALL;
                break;
            }
            else
            {
                installBtn.ResetState();
                if (Settings.gameDisplay == LIST_MODE)
                {
                    gameBrowser->SetFocus(1);
                }
                else if (Settings.gameDisplay == GRID_MODE)
                {
                    gameGrid->SetFocus(1);
                }
                else if (Settings.gameDisplay == CAROUSEL_MODE)
                {
                    gameCarousel->SetFocus(1);
                }
            }
        }
        else if ((covert & 0x2) && (covert != covertOld))
        {
            gprintf("\tNew Disc Detected\n");
            choice
                    = WindowPrompt(tr( "New Disc Detected" ), 0, tr( "Install" ), tr( "Mount DVD drive" ),
                            tr( "Cancel" ));
            if (choice == 1)
            {
                menu = MENU_INSTALL;
                break;
            }
            else if (choice == 2)
            {
                dvdBtn.SetState(STATE_CLICKED);
            }
            else
            {
                if (Settings.gameDisplay == LIST_MODE)
                {
                    gameBrowser->SetFocus(1);
                }
                else if (Settings.gameDisplay == GRID_MODE)
                {
                    gameGrid->SetFocus(1);
                }
                else if (Settings.gameDisplay == CAROUSEL_MODE)
                {
                    gameCarousel->SetFocus(1);
                }
            }
        }

        else if (sdcardBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tsdCardBtn Clicked\n");
            SDCard_deInit();
            SDCard_Init();
            if (Settings.gameDisplay == LIST_MODE)
            {
                startat = gameBrowser->GetSelectedOption();
                offset = gameBrowser->GetOffset();
            }
            else if (Settings.gameDisplay == GRID_MODE)
            {
                startat = gameGrid->GetSelectedOption();
                offset = gameGrid->GetOffset();
            }
            else if (Settings.gameDisplay == CAROUSEL_MODE)
            {
                startat = gameCarousel->GetSelectedOption();
                offset = gameCarousel->GetOffset();
            }
            if (isInserted(Settings.BootDevice))
            {
                HaltGui(); // to fix endless rumble when clicking on the SD icon when rumble is disabled because rumble is set to on in Global_Default()
                Settings.Load();
                ResumeGui();
            }
            sdcardBtn.ResetState();
            menu = MENU_DISCLIST;
            break;
        }

        else if (DownloadBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tDownloadBtn Clicked\n");
            if (isInserted(Settings.BootDevice))
            {
                choice = WindowPrompt(tr( "Cover Download" ), 0, tr( "Normal Covers" ), tr( "3D Covers" ),
                        tr( "Disc Images" ), tr( "Back" )); // ask for download choice
                if (choice != 0)
                {
                    int choice2 = choice;
                    bool missing;
                    missing = SearchMissingImages(choice2);
                    if (IsNetworkInit() == false && missing == true)
                    {
                        WindowPrompt(tr( "Network init error" ), 0, tr( "OK" ));
                    }
                    else
                    {
                        if (GetMissingFiles() != NULL && cntMissFiles > 0)
                        {
                            char tempCnt[40];
                            sprintf(tempCnt, "%i %s", cntMissFiles, tr( "Missing files" ));
                            if (choice != 3)
                                choice = WindowPrompt(tr( "Download Boxart image?" ), tempCnt, tr( "Yes" ), tr( "No" ));
                            else if (choice == 3) choice = WindowPrompt(tr( "Download Discart image?" ), tempCnt,
                                    tr( "Yes" ), tr( "No" ));
                            if (choice == 1)
                            {
                                ret = ProgressDownloadWindow(choice2);
                                if (ret == 0)
                                {
                                    WindowPrompt(tr( "Download finished" ), 0, tr( "OK" ));
                                }
                                else
                                {
                                    sprintf(tempCnt, "%i %s", ret, tr( "files not found on the server!" ));
                                    WindowPrompt(tr( "Download finished" ), tempCnt, tr( "OK" ));
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                WindowPrompt(tr( "No SD-Card inserted!" ), tr( "Insert an SD-Card to download images." ), tr( "OK" ));
            }
            menu = MENU_DISCLIST;
            DownloadBtn.ResetState();
            if (Settings.gameDisplay == LIST_MODE)
            {
                gameBrowser->SetFocus(1);
            }
            else if (Settings.gameDisplay == GRID_MODE)
            {
                gameGrid->SetFocus(1);
            }
            else if (Settings.gameDisplay == CAROUSEL_MODE)
            {
                gameCarousel->SetFocus(1);
            }
        }//end download

        else if (settingsBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tsettingsBtn Clicked\n");
            if (Settings.gameDisplay == LIST_MODE)
            {
                startat = gameBrowser->GetSelectedOption();
                offset = gameBrowser->GetOffset();
            }
            else if (Settings.gameDisplay == GRID_MODE)
            {
                startat = gameGrid->GetSelectedOption();
                offset = gameGrid->GetOffset();
            }
            else if (Settings.gameDisplay == CAROUSEL_MODE)
            {
                startat = gameCarousel->GetSelectedOption();
                offset = gameCarousel->GetOffset();
            }
            menu = MENU_SETTINGS;
            break;

        }

        else if (favoriteBtn.GetState() == STATE_CLICKED)
        {
			if(Settings.GameSort != SORT_RANKING)
			{
				gprintf("\tfavoriteBtn Clicked\n");
				Settings.GameSort = SORT_RANKING;
				if (isInserted(Settings.BootDevice))
				{
					Settings.Save();
				}
				gameList.FilterList();
				menu = MENU_DISCLIST;
				break;
			}
			else
			favoriteBtn.ResetState();
        }

        else if (searchBtn.GetState() == STATE_CLICKED && mountMethod != 3)
        {

            gprintf("\tsearchBtn Clicked\n");
            show_searchwindow = !show_searchwindow;
            HaltGui();
            if (searchBar)
            {
                mainWindow->Remove(searchBar);
                delete searchBar;
                searchBar = NULL;
            }
            if (show_searchwindow)
            {
                if (*gameList.GetCurrentFilter())
                {
                    searchBtn.StopEffect();
                    searchBtn.SetEffectGrow();
                }
                searchBar = new GuiSearchBar(gameList.GetAvailableSearchChars());
                if (searchBar) mainWindow->Append(searchBar);
            }
            else
            {
                if (*gameList.GetCurrentFilter()) searchBtn.SetEffect(EFFECT_PULSE, 10, 105);
            }
            searchBtn.ResetState();
            ResumeGui();
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
                menu = MENU_DISCLIST;
                break;
            }
            else if (searchChar == 7) // Close
            {
                show_searchwindow = false;
                HaltGui();
                if (searchBar)
                {
                    mainWindow->Remove(searchBar);
                    delete searchBar;
                    searchBar = NULL;
                }
                if (*gameList.GetCurrentFilter())
                {
                    searchBtn.SetEffect(EFFECT_PULSE, 10, 105);
                    searchBtn.SetImage(&searchBtnImg);
                    searchBtn.SetImageOver(&searchBtnImg);
                    searchBtn.SetAlpha(255);
                }
                else
                {
                    searchBtn.StopEffect();
                    searchBtn.SetEffectGrow();
                    searchBtn.SetImage(&searchBtnImg_g);
                    searchBtn.SetImageOver(&searchBtnImg_g);
                    searchBtn.SetAlpha(180);
                }

                ResumeGui();
            }
            else if (searchChar == 8) // Backspace
            {
                int len = wcslen(gameList.GetCurrentFilter());
                wchar_t newFilter[len + 1];
                if (gameList.GetCurrentFilter()) wcscpy(newFilter, gameList.GetCurrentFilter());
                newFilter[len > 0 ? len - 1 : 0] = 0;
                gameList.FilterList(newFilter);
                menu = MENU_DISCLIST;
                break;
            }

        }

        else if (abcBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tabcBtn clicked\n");
            if (Settings.GameSort != SORT_ABC)
            {
                Settings.GameSort = SORT_ABC;
                if (isInserted(Settings.BootDevice))
                {
                    Settings.Save();
                }
                gameList.FilterList();

                menu = MENU_DISCLIST;
                break;
            }
            abcBtn.ResetState();
        }

        else if (countBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tcountBtn Clicked\n");
            if (Settings.GameSort != SORT_PLAYCOUNT)
            {
                Settings.GameSort = SORT_PLAYCOUNT;
                if (isInserted(Settings.BootDevice))
                {
                    Settings.Save();
                }
                gameList.FilterList();

                menu = MENU_DISCLIST;
                break;
            }
            countBtn.ResetState();

        }

        else if (listBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tlistBtn Clicked\n");
            if (Settings.gameDisplay != LIST_MODE)
            {
                Settings.gameDisplay = LIST_MODE;
                menu = MENU_DISCLIST;
                if (isInserted(Settings.BootDevice))
                {
                    Settings.Save();
                }
                listBtn.ResetState();
                break;
            }
            else
            {
                listBtn.ResetState();
            }
        }

        else if (gridBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tgridBtn Clicked\n");
            if (Settings.gameDisplay != GRID_MODE)
            {

                Settings.gameDisplay = GRID_MODE;
                menu = MENU_DISCLIST;
                if (isInserted(Settings.BootDevice))
                {
                    Settings.Save();
                }
                gridBtn.ResetState();
                break;
            }
            else
            {
                gridBtn.ResetState();
            }
        }

        else if (carouselBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tcarouselBtn Clicked\n");
            if (Settings.gameDisplay != CAROUSEL_MODE)
            {
                Settings.gameDisplay = CAROUSEL_MODE;
                menu = MENU_DISCLIST;
                if (isInserted(Settings.BootDevice))
                {
                    Settings.Save();
                }
                carouselBtn.ResetState();
                break;
            }
            else
            {
                carouselBtn.ResetState();
            }
        }

        else if (homebrewBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\thomebrewBtn Clicked\n");
            menu = MENU_HOMEBREWBROWSE;
            break;
        }

        else if (gameInfo.GetState() == STATE_CLICKED && mountMethod != 3)
        {
            gprintf("\tgameinfo Clicked\n");
            gameInfo.ResetState();
            if (selectImg1 >= 0 && selectImg1 < (s32) gameList.size())
            {
                gameSelected = selectImg1;
                rockout();
                struct discHdr *header = gameList[selectImg1];
                snprintf(IDfull, sizeof(IDfull), "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],
                        header->id[3], header->id[4], header->id[5]);
                choice = showGameInfo(IDfull);
                rockout(2);
                if (choice == 2) homeBtn.SetState(STATE_CLICKED);
                if (choice == 3)
                {
                    menu = MENU_DISCLIST;
                    break;
                }
            }
        }
        else if (lockBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tlockBtn clicked\n");
            lockBtn.ResetState();
            if (!canUnlock)
            {
                WindowPrompt(
                        tr( "Parental Control" ),
                        tr( "You don't have Parental Control enabled. If you wish to use Parental Control, enable it in the Wii Settings." ),
                        tr( "OK" ));
            }
            else
            {
                if (Settings.godmode)
                {
                    if (WindowPrompt(tr( "Parental Control" ), tr( "Are you sure you want to enable Parent Control?" ),
                            tr( "Yes" ), tr( "No" )) == 1)
                    {
                        Settings.godmode = 0;
                        lockBtn.SetImage(&lockBtnImg_g);
                        lockBtn.SetImageOver(&lockBtnImg_g);
                        lockBtn.SetToolTip(&lockBtnTT, 15, 52, 1, 3);

                        // Retrieve the gamelist again
                        menu = MENU_DISCLIST;
                        break;
                    }
                }
                else
                {
                    // Require the user to enter the PIN code
                    char pin[5];
                    memset(&pin, 0, 5);
                    int ret = OnScreenNumpad((char *) &pin, 5);

                    if (ret == 1)
                    {
                        if (memcmp(pin, Settings.Parental.pin, 4) == 0)
                        {
                            Settings.godmode = 1;
                            lockBtn.SetImage(&unlockBtnImg_g);
                            lockBtn.SetImageOver(&unlockBtnImg_g);
                            lockBtn.SetToolTip(&unlockBtnTT, 15, 52, 1, 3);

                            // Retrieve the gamelist again
                            menu = MENU_DISCLIST;
                            break;
                        }
                        else
                        {
                            WindowPrompt(tr( "Parental Control" ), tr( "Invalid PIN code" ), tr( "OK" ));
                        }
                    }
                }
            }
        }
        else if (dvdBtn.GetState() == STATE_CLICKED)
        {
            gprintf("\tdvdBtn Clicked\n");
            mountMethodOLD = (mountMethod == 3 ? mountMethod : 0);

            mountMethod = DiscMount(dvdheader);
            dvdBtn.ResetState();

            rockout();
            //break;
        }
        if (Settings.gameDisplay == GRID_MODE)
        {
            int selectimg;
            DownloadBtn.SetSize(0, 0);
            selectimg = gameGrid->GetSelectedOption();
            gameSelected = gameGrid->GetClickedOption();
            selectImg1 = selectimg;
        }

        if (Settings.gameDisplay == CAROUSEL_MODE)
        {
            int selectimg;
            DownloadBtn.SetSize(0, 0);
            selectimg = gameCarousel->GetSelectedOption();
            gameSelected = gameCarousel->GetClickedOption();
            selectImg1 = selectimg;
        }
        if (Settings.gameDisplay == LIST_MODE)
        {
            //Get selected game under cursor
            int selectimg;
            DownloadBtn.SetSize(160, 224);
            idBtn.SetSize(100, 40);

            selectimg = gameBrowser->GetSelectedOption();
            gameSelected = gameBrowser->GetClickedOption();
            selectImg1 = selectimg;

            if (gameSelected > 0) //if click occured
            selectimg = gameSelected;

            char gameregion[7];
            if ((selectimg >= 0) && (selectimg < (s32) gameList.size()))
            {
                if (selectimg != selectedold)
                {
                    selectedold = selectimg;//update displayed cover, game ID, and region if the selected game changes
                    struct discHdr *header = gameList[selectimg];
                    snprintf(ID, sizeof(ID), "%c%c%c", header->id[0], header->id[1], header->id[2]);
                    snprintf(IDfull, sizeof(IDfull), "%s%c%c%c", ID, header->id[3], header->id[4], header->id[5]);
                    w.Remove(&DownloadBtn);

                    if (GameIDTxt)
                    {
                        w.Remove(&idBtn);
                        delete GameIDTxt;
                        GameIDTxt = NULL;
                    }
                    if (GameRegionTxt)
                    {
                        w.Remove(GameRegionTxt);
                        delete GameRegionTxt;
                        GameRegionTxt = NULL;
                    }

                    switch (header->id[3])
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

                    //load game cover
                    if (cover)
                    {
                        delete cover;
                        cover = NULL;
                    }

                    cover = LoadCoverImage(header);

                    if (coverImg)
                    {
                        delete coverImg;
                        coverImg = NULL;
                    }
                    coverImg = new GuiImage(cover);
                    coverImg->SetWidescreen(Settings.widescreen);

                    DownloadBtn.SetImage(coverImg);// put the new image on the download button
                    w.Append(&DownloadBtn);

                    if ((Settings.sinfo == GAMEINFO_ID) || (Settings.sinfo == GAMEINFO_BOTH))
                    {
                        GameIDTxt = new GuiText(IDfull, 22, Theme.info);
                        GameIDTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                        //GameIDTxt->SetPosition(Theme.id_x,Theme.id_y);
                        idBtn.SetEffect(EFFECT_FADE, 20);
                        idBtn.SetLabel(GameIDTxt);
                        w.Append(&idBtn);
                    }
                    //don't try to show region for channels because all the custom channels wont follow the rules
                    if (((Settings.sinfo == GAMEINFO_REGION) || (Settings.sinfo == GAMEINFO_BOTH)) && mountMethod != 3)
                    {
                        GameRegionTxt = new GuiText(gameregion, 22, Theme.info);
                        GameRegionTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                        GameRegionTxt->SetPosition(Theme.region_x, Theme.region_y);
                        GameRegionTxt->SetEffect(EFFECT_FADE, 20);
                        w.Append(GameRegionTxt);
                    }
                }
            }

            if (idBtn.GetState() == STATE_CLICKED && mountMethod != 3)
            {
		gprintf("\tidBtn Clicked\n");
                struct discHdr * header = gameList[gameBrowser->GetSelectedOption()];
                //enter new game ID
                char entered[10];
                snprintf(entered, sizeof(entered), "%s", IDfull);
                //entered[9] = '\0';
                int result = OnScreenKeyboard(entered, 7, 0);
                if (result == 1)
                {
                    WBFS_ReIDGame(header->id, entered);
                    //__Menu_GetEntries();
                    menu = MENU_DISCLIST;
                }

                idBtn.ResetState();
            }
            startat = gameBrowser->GetOffset(), offset = startat;
        }

        if (((gameSelected >= 0) && (gameSelected < (s32) gameList.size())) || mountMethod == 1 || mountMethod == 2)
        {
            if (searchBar)
            {
                HaltGui();
                mainWindow->Remove(searchBar);
                ResumeGui();
            }
            rockout();
            struct discHdr *header = (mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected]);
            //  struct discHdr *header = dvdheader:gameList[gameSelected]);
            if (!mountMethod)//only get this stuff it we are booting a game from USB
            {
                WBFS_GameSize(header->id, &size);
            }

            //check if alt Dol and gct file is present
            FILE *exeFile = NULL;
            char nipple[100];
            header = (mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected]); //reset header
            snprintf(IDfull, sizeof(IDfull), "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],
                    header->id[3], header->id[4], header->id[5]);
            GameCFG* game_cfg = GameSettings.GetGameCFG(header->id);
			u8 alternatedol;
			u8 ocarinaChoice;

            if (game_cfg)
            {
                alternatedol = game_cfg->loadalternatedol;
                ocarinaChoice = game_cfg->ocarina;
            }
            else
            {
                alternatedol = OFF;
                ocarinaChoice = Settings.ocarina;
            }

            if (Settings.quickboot == ON) //quickboot game
            {
                if (alternatedol == ON)
                {
                    /* Open dol File and check exist */
                    sprintf(nipple, "%s%s.dol", Settings.dolpath, IDfull);
                    exeFile = fopen(nipple, "rb");
                    if (exeFile == NULL)
                    {
                        sprintf(nipple, "%s %s", nipple, tr( "does not exist!" ));
                        WindowPrompt(tr( "Error" ), nipple, tr( "OK" ));
                        menu = MENU_DISCLIST;
                        wiilight(0);
                        break;
                    }
                    else
                    {
                        fclose(exeFile);
                    }
                }
                if (ocarinaChoice != OFF)
                {
                    /* Open gct File and check exist */
                    sprintf(nipple, "%s%s.gct", Settings.Cheatcodespath, IDfull);
                    exeFile = fopen(nipple, "rb");
                    if (exeFile == NULL)
                    {
                        gprintf("\ttried to load missing gct.\n");
                        sprintf(nipple, "%s %s", nipple, tr( "does not exist!  Loading game without cheats." ));
                        WindowPrompt(tr( "Error" ), nipple, NULL, NULL, NULL, NULL, 170);
                    }
                    else
                    {
                        fclose(exeFile);
                    }

                }

                wiilight(0);
                if (isInserted(Settings.BootDevice))
                {
                    //////////save game play count////////////////
					GameStatistics.SetPlayCount(header->id, GameStatistics.GetPlayCount(header->id)+1);
					GameStatistics.Save();

		    gprintf("\tplaycount for %c%c%c%c%c%c raised to %i\n", header->id[0], header->id[1], header->id[2],
                            header->id[3], header->id[4], header->id[5],  GameStatistics.GetPlayCount(header->id));

                }
                //Just calling that shuts down everything and starts game
                BootGame((const char *) header->id);
            }
            bool returnHere = true;// prompt to start game
            while (returnHere)
            {
                returnHere = false;
                if (Settings.wiilight != WIILIGHT_INSTALL) wiilight(1);
                choice = GameWindowPrompt();
                // header = gameList[gameSelected]; //reset header

                if (choice == 1)
                {
                    if (alternatedol == ON)
                    {
                        /* Open dol File and check exist */
                        sprintf(nipple, "%s%s.dol", Settings.dolpath, IDfull);
                        exeFile = fopen(nipple, "rb");
                        if (exeFile == NULL)
                        {
                            gprintf("\n\tTried to load alt dol that isn't there");
                            sprintf(nipple, "%s %s", nipple, tr( "does not exist!  You messed something up." ));
                            WindowPrompt(tr( "Error" ), nipple, tr( "OK" ));
                            menu = MENU_DISCLIST;
                            wiilight(0);
                            break;
                        }
                        else
                        {
                            fclose(exeFile);
                        }
                    }
                    if (ocarinaChoice != OFF)
                    {
                        /* Open gct File and check exist */
                        sprintf(nipple, "%s%s.gct", Settings.Cheatcodespath, IDfull);
                        exeFile = fopen(nipple, "rb");
                        if (exeFile == NULL)
                        {
                            gprintf("\ttried to load gct file that isn't there\n");
                            sprintf(nipple, "%s %s", nipple, tr( "does not exist!  Loading game without cheats." ));
                            WindowPrompt(tr( "Error" ), nipple, NULL, NULL, NULL, NULL, 170);
                        }
                        else
                        {
                            fclose(exeFile);
                        }

                    }
                    wiilight(0);
                    returnHere = false;
                    //Just calling that shuts down everything and starts game
                    BootGame((const char *) gameList[gameSelected]->id);
                }
                else if (choice == 2)
                {
                    wiilight(0);
                    HaltGui();
                    if (Settings.gameDisplay == LIST_MODE)
                        mainWindow->Remove(gameBrowser);
                    else if (Settings.gameDisplay == GRID_MODE)
                        mainWindow->Remove(gameGrid);
                    else if (Settings.gameDisplay == CAROUSEL_MODE) mainWindow->Remove(gameCarousel);
                    mainWindow->Remove(&w);
                    ResumeGui();

                    //re-evaluate header now in case they changed games while on the game prompt
                    header = (mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected]);
                    int settret = MenuGameSettings(header);
                    /* unneeded for now, kept in case database gets a separate language setting
                     //menu = MENU_DISCLIST; // refresh titles (needed if the language setting has changed)
                     */
                    HaltGui();
                    if (Settings.gameDisplay == LIST_MODE)
                        mainWindow->Append(gameBrowser);
                    else if (Settings.gameDisplay == GRID_MODE)
                        mainWindow->Append(gameGrid);
                    else if (Settings.gameDisplay == CAROUSEL_MODE) mainWindow->Append(gameCarousel);
                    mainWindow->Append(&w);
                    ResumeGui();
                    if (settret == 1) //if deleted
                    {
                        menu = MENU_DISCLIST;
                        break;
                    }
                    returnHere = true;
                    rockout(2);
                }

                else if (choice == 3 && !mountMethod) //WBFS renaming
                {
                    wiilight(0);
                    //re-evaluate header now in case they changed games while on the game prompt
                    header = gameList[gameSelected];

                    //enter new game title
                    char entered[60];
                    snprintf(entered, sizeof(entered), "%s", GameTitles.GetTitle(header));
                    entered[59] = '\0';
                    int result = OnScreenKeyboard(entered, 60, 0);
                    if (result == 1)
                    {
                        WBFS_RenameGame(header->id, entered);
                        gameList.ReadGameList();
                        gameList.FilterList();
                        menu = MENU_DISCLIST;
                    }
                }
                else if (choice == 0)
                {
                    rockout(2);
                    if (mountMethod == 1 || mountMethod == 2) mountMethod = mountMethodOLD;
                    if (Settings.gameDisplay == LIST_MODE)
                    {
                        gameBrowser->SetFocus(1);
                    }
                    else if (Settings.gameDisplay == GRID_MODE)
                    {
                        gameGrid->SetFocus(1);
                    }
                    else if (Settings.gameDisplay == CAROUSEL_MODE)
                    {
                        gameCarousel->SetFocus(1);
                    }
                }

            }
            if (searchBar)
            {
                HaltGui();
                mainWindow->Append(searchBar);
                ResumeGui();
            }
        }
        // to skip the first call of windowScreensaver at startup when wiimote is not connected
        if (IsWpadConnected())
        {
            check = 1;
        }

        // screensaver is called when wiimote shuts down, depending on the wiimotet idletime
        if (!IsWpadConnected() && check != 0 && Settings.screensaver != 0)
        {
            check++;
            int screensaverIsOn = 0;
            if (check == 11500) //to allow time for the wii to turn off and not show the screensaver
            {
                screensaverIsOn = WindowScreensaver();
            }
            if (screensaverIsOn == 1) check = 0;
        }
        covertOld = covert;
    }

    HaltGui();
    mainWindow->RemoveAll();
    mainWindow->Append(bgImg);
    delete searchBar;
    searchBar = NULL;
    delete gameBrowser;
    gameBrowser = NULL;
    delete gameGrid;
    gameGrid = NULL;
    delete gameCarousel;
    gameCarousel = NULL;
    ResumeGui();
    return menu;
}

void DiscListWinUpdateCallback(void * e)
{
    GuiWindow *w = (GuiWindow *) e;
    for (int i = 0; i < 8; ++i)
    {
        if (Toolbar[i]->GetState() == STATE_SELECTED)
        {
            w->Remove(Toolbar[i]);
            w->Append(Toolbar[i]); // draw the selected Icon allways on top
            break;
        }
    }
}

void rockout(int f)
{
    HaltGui();

    if (gameSelected >= 0 && gameSelected < gameList.size() && (strcasestr(GameTitles.GetTitle(gameList[gameSelected]), "guitar")
            || strcasestr(GameTitles.GetTitle(gameList[gameSelected]), "band") || strcasestr(GameTitles.GetTitle(gameList[gameSelected]),
            "rock")))
    {
        for (int i = 0; i < 4; i++)
            delete pointer[i];
        pointer[0] = Resources::GetImageData("rplayer1_point.png");
        pointer[1] = Resources::GetImageData("rplayer2_point.png");
        pointer[2] = Resources::GetImageData("rplayer3_point.png");
        pointer[3] = Resources::GetImageData("rplayer4_point.png");
    }
    else
    {

        for (int i = 0; i < 4; i++)
            delete pointer[i];
        pointer[0] = Resources::GetImageData("player1_point.png");
        pointer[1] = Resources::GetImageData("player2_point.png");
        pointer[2] = Resources::GetImageData("player3_point.png");
        pointer[3] = Resources::GetImageData("player4_point.png");
    }
    ResumeGui();
}
