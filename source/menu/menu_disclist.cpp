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
#include "wpad.h"
#include "sys.h"

#include "libwiigui/gui_gamebrowser.h"
#include "libwiigui/gui_gamegrid.h"
#include "libwiigui/gui_gamecarousel.h"
#include "libwiigui/gui_searchbar.h"

#define MAX_CHARACTERS      38
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

void DiscListWinUpdateCallback( void * e );
void rockout( int f = 0 );

static u32 startat = 0;

/****************************************************************************
 * MenuDiscList
 ***************************************************************************/
int MenuDiscList()
{

    gprintf( "MenuDiscList()\n" );
    gameList.FilterList();
    int offset = MIN( ( int )startat, gameList.size() - 1 );
    startat = offset;
    int datag = 0;
    int datagB = 0;
    int dataed = -1;
    int cosa = 0, sina = 0;
    int selectImg1 = 0;
    char ID[4];
    char IDfull[7];
    u32 covert = 0;
    char imgPath[100];
    if ( !dvdheader )
        dvdheader = new struct discHdr;
    u8 mountMethodOLD = 0;

    WDVD_GetCoverStatus( &covert );
    u32 covertOld = covert;

    f32 freespace, used, size = 0.0;
    wchar_t searchChar;
    //SCREENSAVER
    int check = 0; //to skip the first cycle when wiimote isn't completely connected

    datagB = 0;
    int menu = MENU_NONE, dataef = 0;


    u32 nolist;
    char text[MAX_CHARACTERS + 4];
    int choice = 0, selectedold = 100;
    s32 ret;

    //CLOCK
    struct tm * timeinfo;
    char theTime[80] = "";
    time_t lastrawtime = 0;

    if ( mountMethod != 3 && WBFS_ShowFreeSpace() )
    {
        WBFS_DiskSpace( &used, &freespace );
    }

    if ( !gameList.size() ) //if there is no list of games to display
    {
        nolist = 1;
    }

    GuiSound btnSoundOver( button_over_pcm, button_over_pcm_size, Settings.sfxvolume );
    // because destroy GuiSound must wait while sound playing is finished, we use a global sound
    if ( !btnClick2 ) btnClick2 = new GuiSound( button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume );
    //  GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);

    snprintf( imgPath, sizeof( imgPath ), "%sbutton_install.png", CFG.theme_path );
    GuiImageData btnInstall( imgPath, button_install_png );
    snprintf( imgPath, sizeof( imgPath ), "%sbutton_install_over.png", CFG.theme_path );
    GuiImageData btnInstallOver( imgPath, button_install_over_png );

    snprintf( imgPath, sizeof( imgPath ), "%ssettings_button.png", CFG.theme_path );
    GuiImageData btnSettings( imgPath, settings_button_png );
    snprintf( imgPath, sizeof( imgPath ), "%ssettings_button_over.png", CFG.theme_path );
    GuiImageData btnSettingsOver( imgPath, settings_button_over_png );
    GuiImageData dataID( &data1 );

    snprintf( imgPath, sizeof( imgPath ), "%swiimote_poweroff.png", CFG.theme_path );
    GuiImageData btnpwroff( imgPath, wiimote_poweroff_png );
    snprintf( imgPath, sizeof( imgPath ), "%swiimote_poweroff_over.png", CFG.theme_path );
    GuiImageData btnpwroffOver( imgPath, wiimote_poweroff_over_png );
    snprintf( imgPath, sizeof( imgPath ), "%smenu_button.png", CFG.theme_path );
    GuiImageData btnhome( imgPath, menu_button_png );
    snprintf( imgPath, sizeof( imgPath ), "%smenu_button_over.png", CFG.theme_path );
    GuiImageData btnhomeOver( imgPath, menu_button_over_png );
    snprintf( imgPath, sizeof( imgPath ), "%sSDcard_over.png", CFG.theme_path );
    GuiImageData btnsdcardOver( imgPath, sdcard_over_png );
    snprintf( imgPath, sizeof( imgPath ), "%sSDcard.png", CFG.theme_path );
    GuiImageData btnsdcard( imgPath, sdcard_png );


    snprintf( imgPath, sizeof( imgPath ), "%sfavIcon.png", CFG.theme_path );
    GuiImageData imgfavIcon( imgPath, favIcon_png );
    snprintf( imgPath, sizeof( imgPath ), "%sfavIcon_gray.png", CFG.theme_path );
    GuiImageData imgfavIcon_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%ssearchIcon.png", CFG.theme_path );
    GuiImageData imgsearchIcon( imgPath, searchIcon_png );
    snprintf( imgPath, sizeof( imgPath ), "%ssearchIcon_gray.png", CFG.theme_path );
    GuiImageData imgsearchIcon_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%sabcIcon.png", CFG.theme_path );
    GuiImageData imgabcIcon( imgPath, abcIcon_png );
    snprintf( imgPath, sizeof( imgPath ), "%sabcIcon_gray.png", CFG.theme_path );
    GuiImageData imgabcIcon_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%srankIcon.png", CFG.theme_path );
    GuiImageData imgrankIcon( imgPath, rankIcon_png );
    snprintf( imgPath, sizeof( imgPath ), "%srankIcon_gray.png", CFG.theme_path );
    GuiImageData imgrankIcon_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%splayCountIcon.png", CFG.theme_path );
    GuiImageData imgplayCountIcon( imgPath, playCountIcon_png );
    snprintf( imgPath, sizeof( imgPath ), "%splayCountIcon_gray.png", CFG.theme_path );
    GuiImageData imgplayCountIcon_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%sarrangeGrid.png", CFG.theme_path );
    GuiImageData imgarrangeGrid( imgPath, arrangeGrid_png );
    snprintf( imgPath, sizeof( imgPath ), "%sarrangeGrid_gray.png", CFG.theme_path );
    GuiImageData imgarrangeGrid_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%sarrangeList.png", CFG.theme_path );
    GuiImageData imgarrangeList( imgPath, arrangeList_png );
    snprintf( imgPath, sizeof( imgPath ), "%sarrangeList_gray.png", CFG.theme_path );
    GuiImageData imgarrangeList_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%sarrangeCarousel.png", CFG.theme_path );
    GuiImageData imgarrangeCarousel( imgPath, arrangeCarousel_png );
    snprintf( imgPath, sizeof( imgPath ), "%sarrangeCarousel_gray.png", CFG.theme_path );
    GuiImageData imgarrangeCarousel_gray( imgPath, NULL );

    snprintf( imgPath, sizeof( imgPath ), "%slock.png", CFG.theme_path );
    GuiImageData imgLock( imgPath, lock_png );
    snprintf( imgPath, sizeof( imgPath ), "%slock_gray.png", CFG.theme_path );
    GuiImageData imgLock_gray( imgPath, NULL );
    snprintf( imgPath, sizeof( imgPath ), "%sunlock.png", CFG.theme_path );
    GuiImageData imgUnlock( imgPath, unlock_png );
    snprintf( imgPath, sizeof( imgPath ), "%sunlock_gray.png", CFG.theme_path );
    GuiImageData imgUnlock_gray( imgPath, NULL );

    snprintf( imgPath, sizeof( imgPath ), "%sdvd.png", CFG.theme_path );
    GuiImageData imgdvd( imgPath, dvd_png );
    snprintf( imgPath, sizeof( imgPath ), "%sdvd_gray.png", CFG.theme_path );
    GuiImageData imgdvd_gray( imgPath, NULL );

    snprintf( imgPath, sizeof( imgPath ), "%sbrowser.png", CFG.theme_path );
    GuiImageData homebrewImgData( imgPath, browser_png );
    snprintf( imgPath, sizeof( imgPath ), "%sbrowser_over.png", CFG.theme_path );
    GuiImageData homebrewImgDataOver( imgPath, browser_over_png );


    GuiTrigger trigA;
    trigA.SetSimpleTrigger( -1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A );
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger( -1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START );
    GuiTrigger trig2;
    trig2.SetButtonOnlyTrigger( -1, WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X, 0 );
    GuiTrigger trig1;
    trig1.SetButtonOnlyTrigger( -1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y, 0 );
    GuiTrigger trigZ;
    trigZ.SetButtonOnlyTrigger( -1, WPAD_NUNCHUK_BUTTON_Z | WPAD_CLASSIC_BUTTON_ZL, PAD_TRIGGER_Z );

    GuiButton screenShotBtn( 0, 0 );
    screenShotBtn.SetPosition( 0, 0 );
    screenShotBtn.SetTrigger( &trigZ );

    char spaceinfo[30];
    if ( load_from_fs == PART_FS_FAT )
    {
        memset( spaceinfo, 0, 30 );
    }
    else
    {
        if ( !strcmp( Settings.db_language, "JA" ) )
        {
            // needs to be "total...used" for Japanese
            sprintf( spaceinfo, ( mountMethod != 3 ? "%.2fGB %s %.2fGB %s" : " " ), ( freespace + used ), tr( "of" ), freespace, tr( "free" ) );
        }
        else
        {
            sprintf( spaceinfo, ( mountMethod != 3 ? "%.2fGB %s %.2fGB %s" : " " ), freespace, tr( "of" ), ( freespace + used ), tr( "free" ) );
        }
    }
    GuiText usedSpaceTxt( spaceinfo, 18, THEME.info );
    usedSpaceTxt.SetAlignment( THEME.hddinfo_align, ALIGN_TOP );
    usedSpaceTxt.SetPosition( THEME.hddinfo_x, THEME.hddinfo_y );

    char GamesCnt[15];
    sprintf( GamesCnt, "%s: %i", ( mountMethod != 3 ? tr( "Games" ) : tr( "Channels" ) ), gameList.size() );
    GuiText gamecntTxt( GamesCnt, 18, THEME.info );

    GuiButton gamecntBtn( 100, 18 );
    gamecntBtn.SetAlignment( THEME.gamecount_align, ALIGN_TOP );
    gamecntBtn.SetPosition( THEME.gamecount_x, THEME.gamecount_y );
    gamecntBtn.SetLabel( &gamecntTxt );
    gamecntBtn.SetEffectGrow();
    gamecntBtn.SetTrigger( &trigA );

    GuiTooltip installBtnTT( tr( "Install a game" ) );
    if ( Settings.wsprompt == yes )
        installBtnTT.SetWidescreen( CFG.widescreen );
    installBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage installBtnImg( &btnInstall );
    GuiImage installBtnImgOver( &btnInstallOver );
    installBtnImg.SetWidescreen( CFG.widescreen );
    installBtnImgOver.SetWidescreen( CFG.widescreen );

    GuiButton installBtn( &installBtnImg, &installBtnImgOver, ALIGN_LEFT, ALIGN_TOP, THEME.install_x, THEME.install_y, &trigA, &btnSoundOver, btnClick2, 1, &installBtnTT, 24, -30, 0, 5 );


    GuiTooltip settingsBtnTT( tr( "Settings" ) );
    if ( Settings.wsprompt == yes )
        settingsBtnTT.SetWidescreen( CFG.widescreen );
    settingsBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage settingsBtnImg( &btnSettings );
    settingsBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage settingsBtnImgOver( &btnSettingsOver );
    settingsBtnImgOver.SetWidescreen( CFG.widescreen );
    GuiButton settingsBtn( &settingsBtnImg, &settingsBtnImgOver, 0, 3, THEME.setting_x, THEME.setting_y, &trigA, &btnSoundOver, btnClick2, 1, &settingsBtnTT, 65, -30, 0, 5 );

    GuiTooltip homeBtnTT( tr( "Back to HBC or Wii Menu" ) );
    if ( Settings.wsprompt == yes )
        homeBtnTT.SetWidescreen( CFG.widescreen );
    settingsBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage homeBtnImg( &btnhome );
    homeBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage homeBtnImgOver( &btnhomeOver );
    homeBtnImgOver.SetWidescreen( CFG.widescreen );
    GuiButton homeBtn( &homeBtnImg, &homeBtnImgOver, 0, 3, THEME.home_x, THEME.home_y, &trigA, &btnSoundOver, btnClick2, 1, &homeBtnTT, 15, -30, 1, 5 );
    homeBtn.RemoveSoundClick();
    homeBtn.SetTrigger( &trigHome );

    GuiTooltip poweroffBtnTT( tr( "Power off the Wii" ) );
    if ( Settings.wsprompt == yes )
        poweroffBtnTT.SetWidescreen( CFG.widescreen );
    poweroffBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage poweroffBtnImg( &btnpwroff );
    GuiImage poweroffBtnImgOver( &btnpwroffOver );
    poweroffBtnImg.SetWidescreen( CFG.widescreen );
    poweroffBtnImgOver.SetWidescreen( CFG.widescreen );
    GuiButton poweroffBtn( &poweroffBtnImg, &poweroffBtnImgOver, 0, 3, THEME.power_x, THEME.power_y, &trigA, &btnSoundOver, btnClick2, 1, &poweroffBtnTT, -10, -30, 1, 5 );

    GuiTooltip sdcardBtnTT( tr( "Reload SD" ) );
    if ( Settings.wsprompt == yes )
        sdcardBtnTT.SetWidescreen( CFG.widescreen );
    sdcardBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage sdcardImg( &btnsdcard );
    GuiImage sdcardImgOver( &btnsdcardOver );
    sdcardImg.SetWidescreen( CFG.widescreen );
    sdcardImgOver.SetWidescreen( CFG.widescreen );
    GuiButton sdcardBtn( &sdcardImg, &sdcardImgOver, 0, 3, THEME.sdcard_x, THEME.sdcard_y, &trigA, &btnSoundOver, btnClick2, 1, &sdcardBtnTT, 15, -30, 0, 5 );

    GuiButton gameInfo( 0, 0 );
    gameInfo.SetTrigger( &trig2 );
    gameInfo.SetSoundClick( btnClick2 );


    GuiImage wiiBtnImg( &dataID );
    wiiBtnImg.SetWidescreen( CFG.widescreen );
    GuiButton wiiBtn( &wiiBtnImg, &wiiBtnImg, 0, 4, 0, -10, &trigA, &btnSoundOver, btnClick2, 0 );

    GuiTooltip favoriteBtnTT( tr( "Display favorites" ) );
    if ( Settings.wsprompt == yes )
        favoriteBtnTT.SetWidescreen( CFG.widescreen );
    favoriteBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage favoriteBtnImg( &imgfavIcon );
    favoriteBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage favoriteBtnImg_g( &imgfavIcon_gray );
    if ( favoriteBtnImg_g.GetImage() == NULL ) { favoriteBtnImg_g = favoriteBtnImg; favoriteBtnImg_g.SetGrayscale();}
    favoriteBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton favoriteBtn( &favoriteBtnImg_g, &favoriteBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_favorite_x, THEME.gamelist_favorite_y, &trigA, &btnSoundOver, btnClick2, 1, &favoriteBtnTT, -15, 52, 0, 3 );
    favoriteBtn.SetAlpha( 180 );

    GuiTooltip searchBtnTT( tr( "Set Search-Filter" ) );
    if ( Settings.wsprompt == yes )
        searchBtnTT.SetWidescreen( CFG.widescreen );
    searchBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage searchBtnImg( &imgsearchIcon );
    searchBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage searchBtnImg_g( &imgsearchIcon_gray );
    if ( searchBtnImg_g.GetImage() == NULL ) { searchBtnImg_g = searchBtnImg; searchBtnImg_g.SetGrayscale();}
    searchBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton searchBtn( &searchBtnImg_g, &searchBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_search_x, THEME.gamelist_search_y, &trigA, &btnSoundOver, btnClick2, 1, &searchBtnTT, -15, 52, 0, 3 );
    searchBtn.SetAlpha( 180 );

    GuiTooltip abcBtnTT( Settings.fave ? tr( "Sort by rank" ) : tr( "Sort alphabetically" ) );
    if ( Settings.wsprompt == yes )
        abcBtnTT.SetWidescreen( CFG.widescreen );
    abcBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage abcBtnImg( Settings.fave ? &imgrankIcon : &imgabcIcon );
    abcBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage abcBtnImg_g( Settings.fave ? &imgrankIcon_gray : &imgabcIcon_gray );
    if ( abcBtnImg_g.GetImage() == NULL ) { abcBtnImg_g = abcBtnImg; abcBtnImg_g.SetGrayscale();}
    abcBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton abcBtn( &abcBtnImg_g, &abcBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_abc_x, THEME.gamelist_abc_y, &trigA, &btnSoundOver, btnClick2, 1, &abcBtnTT, -15, 52, 0, 3 );
    abcBtn.SetAlpha( 180 );

    GuiTooltip countBtnTT( tr( "Sort order by most played" ) );
    if ( Settings.wsprompt == yes )
        countBtnTT.SetWidescreen( CFG.widescreen );
    countBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage countBtnImg( &imgplayCountIcon );
    countBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage countBtnImg_g( &imgplayCountIcon_gray );
    if ( countBtnImg_g.GetImage() == NULL ) { countBtnImg_g = countBtnImg; countBtnImg_g.SetGrayscale();}
    countBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton countBtn( &countBtnImg_g, &countBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_count_x, THEME.gamelist_count_y, &trigA, &btnSoundOver, btnClick2, 1, &countBtnTT, -15, 52, 0, 3 );
    countBtn.SetAlpha( 180 );

    GuiTooltip listBtnTT( tr( "Display as a list" ) );
    if ( Settings.wsprompt == yes )
        listBtnTT.SetWidescreen( CFG.widescreen );
    listBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage listBtnImg( &imgarrangeList );
    listBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage listBtnImg_g( &imgarrangeList_gray );
    if ( listBtnImg_g.GetImage() == NULL ) { listBtnImg_g = listBtnImg; listBtnImg_g.SetGrayscale();}
    listBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton listBtn( &listBtnImg_g, &listBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_list_x, THEME.gamelist_list_y, &trigA, &btnSoundOver, btnClick2, 1, &listBtnTT, 15, 52, 1, 3 );
    listBtn.SetAlpha( 180 );

    GuiTooltip gridBtnTT( tr( "Display as a grid" ) );
    if ( Settings.wsprompt == yes )
        gridBtnTT.SetWidescreen( CFG.widescreen );
    gridBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage gridBtnImg( &imgarrangeGrid );
    gridBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage gridBtnImg_g( &imgarrangeGrid_gray );
    if ( gridBtnImg_g.GetImage() == NULL ) { gridBtnImg_g = gridBtnImg; gridBtnImg_g.SetGrayscale();}
    gridBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton gridBtn( &gridBtnImg_g, &gridBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_grid_x, THEME.gamelist_grid_y, &trigA, &btnSoundOver, btnClick2, 1, &gridBtnTT, 15, 52, 1, 3 );
    gridBtn.SetAlpha( 180 );

    GuiTooltip carouselBtnTT( tr( "Display as a carousel" ) );
    if ( Settings.wsprompt == yes )
        carouselBtnTT.SetWidescreen( CFG.widescreen );
    carouselBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage carouselBtnImg( &imgarrangeCarousel );
    carouselBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage carouselBtnImg_g( &imgarrangeCarousel_gray );
    if ( carouselBtnImg_g.GetImage() == NULL ) { carouselBtnImg_g = carouselBtnImg; carouselBtnImg_g.SetGrayscale();}
    carouselBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton carouselBtn( &carouselBtnImg_g, &carouselBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_carousel_x, THEME.gamelist_carousel_y, &trigA, &btnSoundOver, btnClick2, 1, &carouselBtnTT, 15, 52, 1, 3 );
    carouselBtn.SetAlpha( 180 );

    bool canUnlock = ( Settings.parentalcontrol == 0 && Settings.parental.enabled == 1 );

    GuiTooltip lockBtnTT( canUnlock ? tr( "Unlock Parental Control" ) : tr( "Parental Control disabled" ) );
    if ( Settings.wsprompt == yes )
        lockBtnTT.SetWidescreen( CFG.widescreen );
    lockBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage lockBtnImg( &imgLock );
    lockBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage lockBtnImg_g( &imgLock_gray );
    if ( lockBtnImg_g.GetImage() == NULL ) { lockBtnImg_g = lockBtnImg; lockBtnImg_g.SetGrayscale(); }
    lockBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton lockBtn( &lockBtnImg_g, &lockBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_lock_x, THEME.gamelist_lock_y, &trigA, &btnSoundOver, btnClick2, 1, &lockBtnTT, 15, 52, 1, 3 );
    lockBtn.SetAlpha( 180 );

    GuiTooltip unlockBtnTT( tr( "Enable Parental Control" ) );
    if ( Settings.wsprompt == yes )
        unlockBtnTT.SetWidescreen( CFG.widescreen );
    unlockBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage unlockBtnImg( &imgUnlock );
    unlockBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage unlockBtnImg_g( &imgUnlock_gray );
    if ( unlockBtnImg_g.GetImage() == NULL ) { unlockBtnImg_g = unlockBtnImg; unlockBtnImg_g.SetGrayscale(); }
    unlockBtnImg_g.SetWidescreen( CFG.widescreen );

    if ( canUnlock && Settings.godmode )
    {
        lockBtn.SetImage( &unlockBtnImg_g );
        lockBtn.SetImageOver( &unlockBtnImg_g );
        lockBtn.SetToolTip( &unlockBtnTT, 15, 52, 1, 3 );
    }

    /*
        GuiButton unlockBtn(&unlockBtnImg_g, &unlockBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_lock_x, THEME.gamelist_lock_y, &trigA, &btnSoundOver, btnClick2,1, &lockBtnTT, 15, 52, 1, 3);
        unlockBtn.SetAlpha(180);
    */

    GuiTooltip dvdBtnTT( tr( "Mount DVD drive" ) );
    if ( Settings.wsprompt == yes )
        dvdBtnTT.SetWidescreen( CFG.widescreen );
    dvdBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage dvdBtnImg( &imgdvd );
    dvdBtnImg.SetWidescreen( CFG.widescreen );
    GuiImage dvdBtnImg_g( dvdBtnImg );
    dvdBtnImg_g.SetWidescreen( CFG.widescreen );
    GuiButton dvdBtn( &dvdBtnImg_g, &dvdBtnImg_g, ALIGN_LEFT, ALIGN_TOP, THEME.gamelist_dvd_x, THEME.gamelist_dvd_y, &trigA, &btnSoundOver, btnClick2, 1, &dvdBtnTT, 15, 52, 1, 3 );
    dvdBtn.SetAlpha( 180 );

    GuiTooltip homebrewBtnTT( tr( "Homebrew Launcher" ) );
    if ( Settings.wsprompt == yes )
        homebrewBtnTT.SetWidescreen( CFG.widescreen );
    homebrewBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiImage homebrewImg( &homebrewImgData );
    GuiImage homebrewImgOver( &homebrewImgDataOver );
    homebrewImg.SetWidescreen( CFG.widescreen );
    homebrewImgOver.SetWidescreen( CFG.widescreen );
    GuiButton homebrewBtn( &homebrewImg, &homebrewImgOver, ALIGN_LEFT, ALIGN_TOP, THEME.homebrew_x, THEME.homebrew_y, &trigA, &btnSoundOver, btnClick2, 1, &homebrewBtnTT, 15, -30, 1, 5 );

    if ( Settings.fave )
    {
        favoriteBtn.SetImage( &favoriteBtnImg );
        favoriteBtn.SetImageOver( &favoriteBtnImg );
        favoriteBtn.SetAlpha( 255 );
    }
    static bool show_searchwindow = false;
    if ( *gameList.GetCurrentFilter() )
    {
        if ( show_searchwindow && gameList.size() == 1 )
            show_searchwindow = false;
        if ( !show_searchwindow )
            searchBtn.SetEffect( EFFECT_PULSE, 10, 105 );
        searchBtn.SetImage( &searchBtnImg );
        searchBtn.SetImageOver( &searchBtnImg );
        searchBtn.SetAlpha( 255 );
    }
    if ( Settings.sort == ALL )
    {
        abcBtn.SetImage( &abcBtnImg );
        abcBtn.SetImageOver( &abcBtnImg );
        abcBtn.SetAlpha( 255 );
    }
    else if ( Settings.sort == PLAYCOUNT )
    {
        countBtn.SetImage( &countBtnImg );
        countBtn.SetImageOver( &countBtnImg );
        countBtn.SetAlpha( 255 );
    }
    if ( Settings.gameDisplay == list )
    {
        listBtn.SetImage( &listBtnImg );
        listBtn.SetImageOver( &listBtnImg );
        listBtn.SetAlpha( 255 );
    }
    else if ( Settings.gameDisplay == grid )
    {
        gridBtn.SetImage( &gridBtnImg );
        gridBtn.SetImageOver( &gridBtnImg );
        gridBtn.SetAlpha( 255 );
    }
    else if ( Settings.gameDisplay == carousel )
    {
        carouselBtn.SetImage( &carouselBtnImg );
        carouselBtn.SetImageOver( &carouselBtnImg );
        carouselBtn.SetAlpha( 255 );
    }

    if ( Settings.gameDisplay == list )
    {
        favoriteBtn.SetPosition( THEME.gamelist_favorite_x, THEME.gamelist_favorite_y );
        searchBtn.SetPosition( THEME.gamelist_search_x, THEME.gamelist_search_y );
        abcBtn.SetPosition( THEME.gamelist_abc_x, THEME.gamelist_abc_y );
        countBtn.SetPosition( THEME.gamelist_count_x, THEME.gamelist_count_y );
        listBtn.SetPosition( THEME.gamelist_list_x, THEME.gamelist_list_y );
        gridBtn.SetPosition( THEME.gamelist_grid_x, THEME.gamelist_grid_y );
        carouselBtn.SetPosition( THEME.gamelist_carousel_x, THEME.gamelist_carousel_y );
        lockBtn.SetPosition( THEME.gamelist_lock_x, THEME.gamelist_lock_y );
        dvdBtn.SetPosition( THEME.gamelist_dvd_x, THEME.gamelist_dvd_y );
    }
    else if ( Settings.gameDisplay == grid )
    {
        favoriteBtn.SetPosition( THEME.gamegrid_favorite_x, THEME.gamegrid_favorite_y );
        searchBtn.SetPosition( THEME.gamegrid_search_x, THEME.gamegrid_search_y );
        abcBtn.SetPosition( THEME.gamegrid_abc_x, THEME.gamegrid_abc_y );
        countBtn.SetPosition( THEME.gamegrid_count_x, THEME.gamegrid_count_y );
        listBtn.SetPosition( THEME.gamegrid_list_x, THEME.gamegrid_list_y );
        gridBtn.SetPosition( THEME.gamegrid_grid_x, THEME.gamegrid_grid_y );
        carouselBtn.SetPosition( THEME.gamegrid_carousel_x, THEME.gamegrid_carousel_y );
        lockBtn.SetPosition( THEME.gamegrid_lock_x, THEME.gamegrid_lock_y );
        dvdBtn.SetPosition( THEME.gamegrid_dvd_x, THEME.gamegrid_dvd_y );
    }
    else if ( Settings.gameDisplay == carousel )
    {
        favoriteBtn.SetPosition( THEME.gamecarousel_favorite_x, THEME.gamecarousel_favorite_y );
        searchBtn.SetPosition( THEME.gamecarousel_search_x, THEME.gamecarousel_favorite_y );
        abcBtn.SetPosition( THEME.gamecarousel_abc_x, THEME.gamecarousel_abc_y );
        countBtn.SetPosition( THEME.gamecarousel_count_x, THEME.gamecarousel_count_y );
        listBtn.SetPosition( THEME.gamecarousel_list_x, THEME.gamecarousel_list_y );
        gridBtn.SetPosition( THEME.gamecarousel_grid_x, THEME.gamecarousel_grid_y );
        carouselBtn.SetPosition( THEME.gamecarousel_carousel_x, THEME.gamecarousel_carousel_y );
        lockBtn.SetPosition( THEME.gamecarousel_lock_x, THEME.gamecarousel_lock_y );
        dvdBtn.SetPosition( THEME.gamecarousel_dvd_x, THEME.gamecarousel_dvd_y );
    }


    //Downloading Covers
    GuiTooltip DownloadBtnTT( tr( "Click to Download Covers" ) );
    if ( Settings.wsprompt == yes )
        DownloadBtnTT.SetWidescreen( CFG.widescreen );
    DownloadBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiButton DownloadBtn( 0, 0 );
    DownloadBtn.SetAlignment( ALIGN_LEFT, ALIGN_TOP );
    DownloadBtn.SetPosition( THEME.covers_x, THEME.covers_y );

    GuiTooltip IDBtnTT( tr( "Click to change game ID" ) );
    if ( Settings.wsprompt == yes )
        IDBtnTT.SetWidescreen( CFG.widescreen );
    IDBtnTT.SetAlpha( THEME.tooltipAlpha );
    GuiButton idBtn( 0, 0 );
    idBtn.SetAlignment( ALIGN_LEFT, ALIGN_TOP );
    idBtn.SetPosition( THEME.id_x, THEME.id_y );

    if ( Settings.godmode == 1 && mountMethod != 3 )  //only make the button have trigger & tooltip if in godmode
    {
        DownloadBtn.SetSoundOver( &btnSoundOver );
        DownloadBtn.SetTrigger( &trigA );
        DownloadBtn.SetTrigger( &trig1 );
        DownloadBtn.SetToolTip( &DownloadBtnTT, 205, -30 );

        idBtn.SetSoundOver( &btnSoundOver );
        idBtn.SetTrigger( &trigA );
        idBtn.SetToolTip( &IDBtnTT, 205, -30 );

    }
    else
    {
        DownloadBtn.SetRumble( false );
        idBtn.SetRumble( false );
    }

    GuiGameBrowser * gameBrowser = NULL;
    GuiGameGrid * gameGrid = NULL;
    GuiGameCarousel * gameCarousel = NULL;
    if ( Settings.gameDisplay == list )
    {
        gameBrowser = new GuiGameBrowser( THEME.gamelist_w, THEME.gamelist_h, CFG.theme_path, bg_options_png, startat, offset );
        gameBrowser->SetPosition( THEME.gamelist_x, THEME.gamelist_y );
        gameBrowser->SetAlignment( ALIGN_LEFT, ALIGN_CENTRE );
    }
    else if ( Settings.gameDisplay == grid )
    {
        gameGrid = new GuiGameGrid( THEME.gamegrid_w, THEME.gamegrid_h, CFG.theme_path, bg_options_png, 0, 0 );
        gameGrid->SetPosition( THEME.gamegrid_x, THEME.gamegrid_y );
        gameGrid->SetAlignment( ALIGN_LEFT, ALIGN_CENTRE );
    }
    else if ( Settings.gameDisplay == carousel )
    {
        //GuiGameCarousel gameCarousel(THEME.gamecarousel_w, THEME.gamecarousel_h, gameList, gameList.size(), CFG.theme_path, bg_options_png, startat, offset);
        gameCarousel = new GuiGameCarousel( 640, 400, CFG.theme_path, bg_options_png, startat, offset );
        gameCarousel->SetPosition( THEME.gamecarousel_x, THEME.gamecarousel_y );
        gameCarousel->SetAlignment( ALIGN_LEFT, ALIGN_CENTRE );
    }

    GuiText clockTimeBack( "88:88", 40, ( GXColor ) {THEME.clock.r, THEME.clock.g, THEME.clock.b, THEME.clock.a / 6} );
    clockTimeBack.SetAlignment( THEME.clock_align, ALIGN_TOP );
    clockTimeBack.SetPosition( THEME.clock_x, THEME.clock_y );
    clockTimeBack.SetFont( clock_ttf, clock_ttf_size );
    GuiText clockTime( theTime, 40, THEME.clock );
    clockTime.SetAlignment( THEME.clock_align, ALIGN_TOP );
    clockTime.SetPosition( THEME.clock_x, THEME.clock_y );
    clockTime.SetFont( clock_ttf, clock_ttf_size );

    HaltGui();
    GuiWindow w( screenwidth, screenheight );

    if ( THEME.show_hddinfo == -1 || THEME.show_hddinfo == 1 ) //force show hdd info
    {
        w.Append( &usedSpaceTxt );
    }
    if ( THEME.show_gamecount == -1 || THEME.show_gamecount == 1 ) //force show game cnt info
    {
        w.Append( &gamecntBtn );
    }
    w.Append( &sdcardBtn );
    w.Append( &poweroffBtn );
    w.Append( &gameInfo );
    if ( Settings.godmode )
        w.Append( &installBtn );
    w.Append( &homeBtn );
    w.Append( &settingsBtn );
    w.Append( &DownloadBtn );
    w.Append( &idBtn );
    w.Append( &screenShotBtn );


    // Begin Toolbar
    w.Append( &favoriteBtn );
    Toolbar[0] = &favoriteBtn;
    w.Append( &searchBtn );
    Toolbar[1] = &searchBtn;
    w.Append( &abcBtn );
    Toolbar[2] = &abcBtn;
    w.Append( &countBtn );
    Toolbar[3] = &countBtn;
    w.Append( &listBtn );
    Toolbar[4] = &listBtn;
    w.Append( &gridBtn );
    Toolbar[5] = &gridBtn;
    w.Append( &carouselBtn );
    Toolbar[6] = &carouselBtn;
    w.Append( &lockBtn );
    Toolbar[7] = &lockBtn;
    w.Append( &dvdBtn );
    Toolbar[8] = &dvdBtn;
    w.SetUpdateCallback( DiscListWinUpdateCallback );
    // End Toolbar



    if ( Settings.godmode == 1 )
        w.Append( &homebrewBtn );

    if ( ( Settings.hddinfo == hr12 ) || ( Settings.hddinfo == hr24 ) )
    {
        w.Append( &clockTimeBack );
        w.Append( &clockTime );
    }

    if ( Settings.gameDisplay == list )
    {
        mainWindow->Append( gameBrowser );
    }
    if ( Settings.gameDisplay == grid )
    {
        mainWindow->Append( gameGrid );
    }
    if ( Settings.gameDisplay == carousel )
    {
        mainWindow->Append( gameCarousel );
    }
    mainWindow->Append( &w );

    GuiSearchBar *searchBar = NULL;
    if ( show_searchwindow )
    {
        searchBar = new GuiSearchBar( gameList.GetAvailableSearchChars() );
        if ( searchBar )
            mainWindow->Append( searchBar );
    }

    ResumeGui();

//  ShowMemInfo();

    while ( menu == MENU_NONE )
    {

        if ( idiotFlag == 1 )
        {
	    gprintf( "\tIdiot flag\n" );
            char idiotBuffer[200];
            snprintf( idiotBuffer, sizeof( idiotBuffer ), "%s (%s). %s", tr( "You have attempted to load a bad image" ),
                      idiotChar, tr( "Most likely it has dimensions that are not evenly divisible by 4." ) );

            int deleteImg = WindowPrompt( 0, idiotBuffer, tr( "OK" ), tr( "Delete" ) );
            if ( deleteImg == 0 )
            {
                snprintf( idiotBuffer, sizeof( idiotBuffer ), "%s %s.", tr( "You are about to delete " ), idiotChar );
                deleteImg = WindowPrompt( tr( "Confirm" ), idiotBuffer, tr( "Delete" ), tr( "Cancel" ) );
                if ( deleteImg == 1 )
                {
                    remove( idiotChar );
                }
            }
            idiotFlag = -1;
        }

        WDVD_GetCoverStatus( &covert );//for detecting if i disc has been inserted

        // if the idiot is showing favorites and don't have any
        if ( Settings.fave && !gameList.size() )
        {
            WindowPrompt( tr( "No Favorites" ), tr( "You are choosing to display favorites and you do not have any selected." ), tr( "Back" ) );
            Settings.fave = !Settings.fave;
            if ( isInserted( bootDevice ) )
            {
                cfg_save_global();
            }
            gameList.FilterList();
            menu = MENU_DISCLIST;
            break;
        }

        //CLOCK
	time_t rawtime = time( 0 );
        if ( ( ( Settings.hddinfo == hr12 ) || ( Settings.hddinfo == hr24 ) ) && rawtime != lastrawtime )
        {
            lastrawtime = rawtime;
            timeinfo = localtime ( &rawtime );
            if ( dataed < 1 )
            {
                if ( Settings.hddinfo == hr12 )
                {
                    if ( rawtime & 1 )
                        strftime( theTime, sizeof( theTime ), "%I:%M", timeinfo );
                    else
                        strftime( theTime, sizeof( theTime ), "%I %M", timeinfo );
                }
                if ( Settings.hddinfo == hr24 )
                {
                    if ( rawtime & 1 )
                        strftime( theTime, sizeof( theTime ), "%H:%M", timeinfo );
                    else
                        strftime( theTime, sizeof( theTime ), "%H %M", timeinfo );
                }
                clockTime.SetText( theTime );

            }
            else if ( dataed > 0 )
            {

                clockTime.SetTextf( "%i", ( dataed - 1 ) );
            }

        }

        if ( ( datagB < 1 ) && ( Settings.cios == 1 ) && ( Settings.video == ntsc ) && ( Settings.hddinfo == hr12 ) && ( Settings.qboot == 1 ) && ( Settings.wsprompt == 0 ) && ( Settings.language == ger ) && ( Settings.tooltips == 0 ) ) {dataed = 1; dataef = 1;} if ( dataef == 1 ) {if ( cosa > 7 ) {cosa = 1;} datag++; if ( sina == 3 ) {wiiBtn.SetAlignment( ALIGN_LEFT, ALIGN_BOTTOM ); wiiBtnImg.SetAngle( 0 ); if ( datag > 163 ) {datag = 1;} else if ( datag < 62 ) {wiiBtn.SetPosition( ( ( cosa )*70 ), ( -2*( datag ) + 120 ) );} else if ( 62 <= datag ) {wiiBtn.SetPosition( ( ( cosa )*70 ), ( ( datag*2 ) - 130 ) );} if ( datag > 162 ) {wiiBtn.SetPosition( 700, 700 ); w.Remove( &wiiBtn ); datagB = 2; cosa++; sina = lastrawtime % 4;} w.Append( &wiiBtn );} if ( sina == 2 ) {wiiBtn.SetAlignment( ALIGN_RIGHT, ALIGN_TOP ); wiiBtnImg.SetAngle( 270 ); if ( datag > 163 ) {datag = 1;} else if ( datag < 62 ) {wiiBtn.SetPosition( ( ( -2*( datag ) + 130 ) ), ( ( cosa )*50 ) );} else if ( 62 <= datag ) {wiiBtn.SetPosition( ( 2*( datag ) - 120 ), ( ( cosa )*50 ) );} if ( datag > 162 ) {wiiBtn.SetPosition( 700, 700 ); w.Remove( &wiiBtn ); datagB = 2; cosa++; sina = lastrawtime % 4;} w.Append( &wiiBtn );} if ( sina == 1 ) {wiiBtn.SetAlignment( ALIGN_TOP, ALIGN_LEFT ); wiiBtnImg.SetAngle( 180 ); if ( datag > 163 ) {datag = 1;} else if ( datag < 62 ) {wiiBtn.SetPosition( ( ( cosa )*70 ), ( 2*( datag ) - 120 ) );} else if ( 62 <= datag ) {wiiBtn.SetPosition( ( ( cosa )*70 ), ( -2*( datag ) + 130 ) );} if ( datag > 162 ) {wiiBtn.SetPosition( 700, 700 ); w.Remove( &wiiBtn ); datagB = 2; cosa++; sina = lastrawtime % 4;} w.Append( &wiiBtn );} if ( sina == 0 ) {wiiBtn.SetAlignment( ALIGN_TOP, ALIGN_LEFT ); wiiBtnImg.SetAngle( 90 ); if ( datag > 163 ) {datag = 1;} else if ( datag < 62 ) {wiiBtn.SetPosition( ( ( 2*( datag ) - 130 ) ), ( ( cosa )*50 ) );} else if ( 62 <= datag ) {wiiBtn.SetPosition( ( -2*( datag ) + 120 ), ( ( cosa )*50 ) );} if ( datag > 162 ) {wiiBtn.SetPosition( 700, 700 ); w.Remove( &wiiBtn ); datagB = 2; cosa++; sina = lastrawtime % 4;} w.Append( &wiiBtn );}}
        // respond to button presses
        if ( shutdown == 1 )
        {
            gprintf( "\n\tshutdown" );
            Sys_Shutdown();
        }
        if ( reset == 1 )
            Sys_Reboot();

        if ( updateavailable == true )
        {
	    gprintf( "\tUpdate Available\n" );
            HaltGui();
            GuiWindow ww( 640, 480 );
            w.SetState( STATE_DISABLED );
            mainWindow->Append( &ww );
            ResumeGui();
            ProgressUpdateWindow();
            updateavailable = false;
            mainWindow->Remove( &ww );
            w.SetState( STATE_DEFAULT );
            menu = MENU_DISCLIST;
        }

        if ( poweroffBtn.GetState() == STATE_CLICKED )
        {


	    gprintf( "\tpoweroffBtn clicked\n" );
            choice = WindowPrompt( tr( "How to Shutdown?" ), 0, tr( "Full Shutdown" ), tr( "Shutdown to Idle" ), tr( "Cancel" ) );
            if ( choice == 2 )
            {
                Sys_ShutdownToIdel();
            }
            else if ( choice == 1 )
            {
                Sys_ShutdownToStandby();
            }
            else
            {
                poweroffBtn.ResetState();
                if ( Settings.gameDisplay == list )
                {
                    gameBrowser->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == grid )
                {
                    gameGrid->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == carousel )
                {
                    gameCarousel->SetFocus( 1 );
                }
            }

        }
        else if ( gamecntBtn.GetState() == STATE_CLICKED && mountMethod != 3 )
        {
	    gprintf( "\tgameCntBtn clicked\n" );
            gamecntBtn.ResetState();
            char linebuf[150];
            snprintf( linebuf, sizeof( linebuf ), "%s %sGameList ?", tr( "Save Game List to" ), Settings.update_path );
            choice = WindowPrompt( 0, linebuf, "TXT", "CSV", tr( "Back" ) );
            if ( choice )
            {
                if ( save_gamelist( choice - 1 ) )
                    WindowPrompt( 0, tr( "Saved" ), tr( "OK" ) );
                else
                    WindowPrompt( tr( "Error" ), tr( "Could not save." ), tr( "OK" ) );
            }
            menu = MENU_DISCLIST;
            break;

        }
        else if ( screenShotBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tscreenShotBtn clicked\n" );
            screenShotBtn.ResetState();
	    ScreenShot();
        }
        else if ( homeBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\thomeBtn clicked\n" );
            bgMusic->Pause();
            choice = WindowExitPrompt();
            bgMusic->Resume();

            if ( choice == 3 )
            {
                Sys_LoadMenu(); // Back to System Menu
            }
            else if ( choice == 2 )
            {
                Sys_BackToLoader();
            }
            else
            {
                homeBtn.ResetState();
                if ( Settings.gameDisplay == list )
                {
                    gameBrowser->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == grid )
                {
                    gameGrid->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == carousel )
                {
                    gameCarousel->SetFocus( 1 );
                }
            }

        }
        else if ( wiiBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\twiiBtn clicked\n" );

            dataed++;
            wiiBtn.ResetState();
            if ( Settings.gameDisplay == list )
            {
                gameBrowser->SetFocus( 1 );
            }
            else if ( Settings.gameDisplay == grid )
            {
                gameGrid->SetFocus( 1 );
            }
            else if ( Settings.gameDisplay == carousel )
            {
                gameCarousel->SetFocus( 1 );
            }
        }
        else if ( installBtn.GetState() == STATE_CLICKED )
        {
            choice = WindowPrompt( tr( "Install a game" ), 0, tr( "Yes" ), tr( "No" ) );
            if ( choice == 1 )
            {
                menu = MENU_INSTALL;
                break;
            }
            else
            {
                installBtn.ResetState();
                if ( Settings.gameDisplay == list )
                {
                    gameBrowser->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == grid )
                {
                    gameGrid->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == carousel )
                {
                    gameCarousel->SetFocus( 1 );
                }
            }
        }
        else if ( ( covert & 0x2 ) && ( covert != covertOld ) )
        {
	    gprintf( "\tNew Disc Detected\n" );
            choice = WindowPrompt( tr( "New Disc Detected" ), 0, tr( "Install" ), tr( "Mount DVD drive" ), tr( "Cancel" ) );
            if ( choice == 1 )
            {
                menu = MENU_INSTALL;
                break;
            }
            else if ( choice == 2 )
            {
                dvdBtn.SetState( STATE_CLICKED );
            }
            else
            {
                if ( Settings.gameDisplay == list )
                {
                    gameBrowser->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == grid )
                {
                    gameGrid->SetFocus( 1 );
                }
                else if ( Settings.gameDisplay == carousel )
                {
                    gameCarousel->SetFocus( 1 );
                }
            }
        }

        else if ( sdcardBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tsdCardBtn Clicked\n" );
            SDCard_deInit();
            SDCard_Init();
            if ( Settings.gameDisplay == list )
            {
                startat = gameBrowser->GetSelectedOption();
                offset = gameBrowser->GetOffset();
            }
            else if ( Settings.gameDisplay == grid )
            {
                startat = gameGrid->GetSelectedOption();
                offset = gameGrid->GetOffset();
            }
            else if ( Settings.gameDisplay == carousel )
            {
                startat = gameCarousel->GetSelectedOption();
                offset = gameCarousel->GetOffset();
            }
            if ( isInserted( bootDevice ) )
            {
                HaltGui(); // to fix endless rumble when clicking on the SD icon when rumble is disabled because rumble is set to on in Global_Default()
                CFG_Load();
                ResumeGui();
            }
            sdcardBtn.ResetState();
            menu = MENU_DISCLIST;
            break;
        }

        else if ( DownloadBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tDownloadBtn Clicked\n" );
            if ( isInserted( bootDevice ) )
            {
                choice = WindowPrompt( tr( "Cover Download" ), 0, tr( "Normal Covers" ), tr( "3D Covers" ), tr( "Disc Images" ), tr( "Back" ) ); // ask for download choice
                if ( choice != 0 )
                {
                    int choice2 = choice;
                    bool missing;
                    missing = SearchMissingImages( choice2 );
                    if ( IsNetworkInit() == false && missing == true )
                    {
                        WindowPrompt( tr( "Network init error" ), 0, tr( "OK" ) );
                    }
                    else
                    {
                        if ( GetMissingFiles() != NULL && cntMissFiles > 0 )
                        {
                            char tempCnt[40];
                            sprintf( tempCnt, "%i %s", cntMissFiles, tr( "Missing files" ) );
                            if ( choice != 3 )choice = WindowPrompt( tr( "Download Boxart image?" ), tempCnt, tr( "Yes" ), tr( "No" ) );
                            else if ( choice == 3 )choice = WindowPrompt( tr( "Download Discart image?" ), tempCnt, tr( "Yes" ), tr( "No" ) );
                            if ( choice == 1 )
                            {
                                ret = ProgressDownloadWindow( choice2 );
                                if ( ret == 0 )
                                {
                                    WindowPrompt( tr( "Download finished" ), 0, tr( "OK" ) );
                                }
                                else
                                {
                                    sprintf( tempCnt, "%i %s", ret, tr( "files not found on the server!" ) );
                                    WindowPrompt( tr( "Download finished" ), tempCnt, tr( "OK" ) );
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                WindowPrompt( tr( "No SD-Card inserted!" ), tr( "Insert an SD-Card to download images." ), tr( "OK" ) );
            }
            menu = MENU_DISCLIST;
            DownloadBtn.ResetState();
            if ( Settings.gameDisplay == list )
            {
                gameBrowser->SetFocus( 1 );
            }
            else if ( Settings.gameDisplay == grid )
            {
                gameGrid->SetFocus( 1 );
            }
            else if ( Settings.gameDisplay == carousel )
            {
                gameCarousel->SetFocus( 1 );
            }
        }//end download

        else if ( settingsBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tsettingsBtn Clicked\n" );
            if ( Settings.gameDisplay == list )
            {
                startat = gameBrowser->GetSelectedOption();
                offset = gameBrowser->GetOffset();
            }
            else if ( Settings.gameDisplay == grid )
            {
                startat = gameGrid->GetSelectedOption();
                offset = gameGrid->GetOffset();
            }
            else if ( Settings.gameDisplay == carousel )
            {
                startat = gameCarousel->GetSelectedOption();
                offset = gameCarousel->GetOffset();
            }
            menu = MENU_SETTINGS;
            break;

        }

        else if ( favoriteBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tfavoriteBtn Clicked\n" );
            Settings.fave = !Settings.fave;
            if ( isInserted( bootDevice ) )
            {
                cfg_save_global();
            }
            gameList.FilterList();
            menu = MENU_DISCLIST;
            break;

        }

        else if ( searchBtn.GetState() == STATE_CLICKED && mountMethod != 3 )
        {

	    gprintf( "\tsearchBtn Clicked\n" );
            show_searchwindow = !show_searchwindow;
            HaltGui();
            if ( searchBar )
            {
                mainWindow->Remove( searchBar );
                delete searchBar;
                searchBar = NULL;
            }
            if ( show_searchwindow )
            {
		if ( *gameList.GetCurrentFilter() )
                {
                    searchBtn.StopEffect();
                    searchBtn.SetEffectGrow();
                }
		searchBar = new GuiSearchBar( gameList.GetAvailableSearchChars() );
                if ( searchBar )
                    mainWindow->Append( searchBar );
            }
            else
            {
                if ( *gameList.GetCurrentFilter() )
                    searchBtn.SetEffect( EFFECT_PULSE, 10, 105 );
            }
            searchBtn.ResetState();
            ResumeGui();
        }

        else if ( searchBar && ( searchChar = searchBar->GetClicked() ) )
        {
            if ( searchChar > 27 )
            {
                int len = gameList.GetCurrentFilter() ? wcslen( gameList.GetCurrentFilter() ) : 0;
                wchar_t newFilter[len+2];
                if ( gameList.GetCurrentFilter() )
                    wcscpy( newFilter, gameList.GetCurrentFilter() );
                newFilter[len] = searchChar;
                newFilter[len+1] = 0;


                gameList.FilterList( newFilter );
                menu = MENU_DISCLIST;
                break;
            }
            else if ( searchChar == 7 ) // Close
            {
                show_searchwindow = false;
                HaltGui();
                if ( searchBar )
                {
                    mainWindow->Remove( searchBar );
                    delete searchBar;
                    searchBar = NULL;
                }
                if ( *gameList.GetCurrentFilter() )
                {
                    searchBtn.SetEffect( EFFECT_PULSE, 10, 105 );
                    searchBtn.SetImage( &searchBtnImg );
                    searchBtn.SetImageOver( &searchBtnImg );
                    searchBtn.SetAlpha( 255 );
                }
                else
                {
                    searchBtn.StopEffect();
                    searchBtn.SetEffectGrow();
                    searchBtn.SetImage( &searchBtnImg_g );
                    searchBtn.SetImageOver( &searchBtnImg_g );
                    searchBtn.SetAlpha( 180 );
                }

                ResumeGui();
            }
            else if ( searchChar == 8 ) // Backspace
            {
                int len = wcslen( gameList.GetCurrentFilter() );
                wchar_t newFilter[len+1];
                if ( gameList.GetCurrentFilter() )
                    wcscpy( newFilter, gameList.GetCurrentFilter() );
                newFilter[len > 0 ? len-1 : 0] = 0;
                gameList.FilterList( newFilter );
                menu = MENU_DISCLIST;
                break;
            }

        }

        else if ( abcBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tabcBtn clicked\n" );
            if ( Settings.sort != ALL )
            {
                Settings.sort = ALL;
                if ( isInserted( bootDevice ) )
                {
                    cfg_save_global();
                }
                gameList.FilterList();

                menu = MENU_DISCLIST;
                break;
            }
            abcBtn.ResetState();
        }

        else if ( countBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tcountBtn Clicked\n" );
            if ( Settings.sort != PLAYCOUNT )
            {
		Settings.sort = PLAYCOUNT;
                if ( isInserted( bootDevice ) )
                {
                    cfg_save_global();
                }
                gameList.FilterList();

                menu = MENU_DISCLIST;
                break;
            }
            countBtn.ResetState();

        }

        else if ( listBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tlistBtn Clicked\n" );
            if ( Settings.gameDisplay != list )
            {
                Settings.gameDisplay = list;
                menu = MENU_DISCLIST;
                if ( isInserted( bootDevice ) )
                {
                    cfg_save_global();
                }
                listBtn.ResetState();
                break;
            }
            else
            {
                listBtn.ResetState();
            }
        }

        else if ( gridBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tgridBtn Clicked\n" );
            if ( Settings.gameDisplay != grid )
            {

                Settings.gameDisplay = grid;
                menu = MENU_DISCLIST;
                if ( isInserted( bootDevice ) )
                {
                    cfg_save_global();
                }
                gridBtn.ResetState();
                break;
            }
            else
            {
                gridBtn.ResetState();
            }
        }

        else if ( carouselBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tcarouselBtn Clicked\n" );
            if ( Settings.gameDisplay != carousel )
            {
                Settings.gameDisplay = carousel;
                menu = MENU_DISCLIST;
                if ( isInserted( bootDevice ) )
                {
                    cfg_save_global();
                }
                carouselBtn.ResetState();
                break;
            }
            else
            {
                carouselBtn.ResetState();
            }
        }

	else if ( homebrewBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\thomebrewBtn Clicked\n" );
            menu = MENU_HOMEBREWBROWSE;
            break;
        }

	else if ( gameInfo.GetState() == STATE_CLICKED && mountMethod != 3 )
        {
	    gprintf( "\tgameinfo Clicked\n" );
            gameInfo.ResetState();
            if ( selectImg1 >= 0 && selectImg1 < ( s32 )gameList.size() )
            {
                gameSelected = selectImg1;
                rockout();
                struct discHdr *header = gameList[selectImg1];
                snprintf ( IDfull, sizeof( IDfull ), "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4], header->id[5] );
                choice = showGameInfo( IDfull );
                rockout( 2 );
                if ( choice == 2 )
                    homeBtn.SetState( STATE_CLICKED );
                if ( choice == 3 )
                {
                    menu = MENU_DISCLIST;
                    break;
                }
            }
        }
        else if ( lockBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tlockBtn clicked\n" );
            lockBtn.ResetState();
            if ( !canUnlock )
            {
                WindowPrompt( tr( "Parental Control" ), tr( "You don't have Parental Control enabled. If you wish to use Parental Control, enable it in the Wii Settings." ), tr( "OK" ) );
            }
            else
            {
                if ( Settings.godmode )
                {
                    if ( WindowPrompt( tr( "Parental Control" ), tr( "Are you sure you want to enable Parent Control?" ), tr( "Yes" ), tr( "No" ) ) == 1 )
                    {
                        Settings.godmode = 0;
                        lockBtn.SetImage( &lockBtnImg_g );
                        lockBtn.SetImageOver( &lockBtnImg_g );
                        lockBtn.SetToolTip( &lockBtnTT, 15, 52, 1, 3 );

                        // Retrieve the gamelist again
                        menu = MENU_DISCLIST;
                        break;
                    }
                }
                else
                {
                    // Require the user to enter the PIN code
                    char pin[5];
                    memset( &pin, 0, 5 );
                    int ret = OnScreenNumpad( ( char * ) & pin, 5 );

                    if ( ret == 1 )
                    {
                        if ( memcmp( pin, Settings.parental.pin, 4 ) == 0 )
                        {
                            Settings.godmode = 1;
                            lockBtn.SetImage( &unlockBtnImg_g );
                            lockBtn.SetImageOver( &unlockBtnImg_g );
                            lockBtn.SetToolTip( &unlockBtnTT, 15, 52, 1, 3 );

                            // Retrieve the gamelist again
                            menu = MENU_DISCLIST;
                            break;
                        }
                        else
                        {
                            WindowPrompt( tr( "Parental Control" ), tr( "Invalid PIN code" ), tr( "OK" ) );
                        }
                    }
                }
            }
        }
        else if ( dvdBtn.GetState() == STATE_CLICKED )
        {
	    gprintf( "\tdvdBtn Clicked\n" );
            mountMethodOLD = ( mountMethod == 3 ? mountMethod : 0 );

            mountMethod = DiscMount( dvdheader );
            dvdBtn.ResetState();

            rockout();
            //break;
        }
        if ( Settings.gameDisplay == grid )
        {
            int selectimg;
            DownloadBtn.SetSize( 0, 0 );
            selectimg = gameGrid->GetSelectedOption();
            gameSelected = gameGrid->GetClickedOption();
            selectImg1 = selectimg;
        }

        if ( Settings.gameDisplay == carousel )
        {
            int selectimg;
            DownloadBtn.SetSize( 0, 0 );
            selectimg = gameCarousel->GetSelectedOption();
            gameSelected = gameCarousel->GetClickedOption();
            selectImg1 = selectimg;
        }
        if ( Settings.gameDisplay == list )
        {
            //Get selected game under cursor
            int selectimg;
            DownloadBtn.SetSize( 160, 224 );
            idBtn.SetSize( 100, 40 );

            selectimg = gameBrowser->GetSelectedOption();
            gameSelected = gameBrowser->GetClickedOption();
            selectImg1 = selectimg;

            if ( gameSelected > 0 ) //if click occured
                selectimg = gameSelected;

            char gameregion[7];
            if ( ( selectimg >= 0 ) && ( selectimg < ( s32 ) gameList.size() ) )
            {
                if ( selectimg != selectedold )
                {
                    selectedold = selectimg;//update displayed cover, game ID, and region if the selected game changes
                    struct discHdr *header = gameList[selectimg];
                    snprintf ( ID, sizeof( ID ), "%c%c%c", header->id[0], header->id[1], header->id[2] );
                    snprintf ( IDfull, sizeof( IDfull ), "%s%c%c%c", ID, header->id[3], header->id[4], header->id[5] );
                    w.Remove( &DownloadBtn );

                    if ( GameIDTxt )
                    {
                        w.Remove( &idBtn );
                        delete GameIDTxt;
                        GameIDTxt = NULL;
                    }
                    if ( GameRegionTxt )
                    {
                        w.Remove( GameRegionTxt );
                        delete GameRegionTxt;
                        GameRegionTxt = NULL;
                    }

                    switch ( header->id[3] )
                    {
                        case 'E':
                            sprintf( gameregion, "NTSC U" );
                            break;
                        case 'J':
                            sprintf( gameregion, "NTSC J" );
                            break;
                        case 'W':
                            sprintf( gameregion, "NTSC T" );
                            break;
                        default:
                        case 'K':
                            sprintf( gameregion, "NTSC K" );
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
                            sprintf( gameregion, "  PAL " );
                            break;
                    }

                    //load game cover
                    if ( cover )
                    {
                        delete cover;
                        cover = NULL;
                    }

                    cover = LoadCoverImage( header );

                    if ( coverImg )
                    {
                        delete coverImg;
                        coverImg = NULL;
                    }
                    coverImg = new GuiImage( cover );
                    coverImg->SetWidescreen( CFG.widescreen );

                    DownloadBtn.SetImage( coverImg );// put the new image on the download button
                    w.Append( &DownloadBtn );

                    if ( ( Settings.sinfo == GameID ) || ( Settings.sinfo == Both ) )
                    {
                        GameIDTxt = new GuiText( IDfull, 22, THEME.info );
                        GameIDTxt->SetAlignment( ALIGN_LEFT, ALIGN_TOP );
                        //GameIDTxt->SetPosition(THEME.id_x,THEME.id_y);
                        idBtn.SetEffect( EFFECT_FADE, 20 );
                        idBtn.SetLabel( GameIDTxt );
                        w.Append( &idBtn );
                    }
                    //don't try to show region for channels because all the custom channels wont follow the rules
                    if ( ( ( Settings.sinfo == GameRegion ) || ( Settings.sinfo == Both ) ) && mountMethod != 3 )
                    {
                        GameRegionTxt = new GuiText( gameregion, 22, THEME.info );
                        GameRegionTxt->SetAlignment( ALIGN_LEFT, ALIGN_TOP );
                        GameRegionTxt->SetPosition( THEME.region_x, THEME.region_y );
                        GameRegionTxt->SetEffect( EFFECT_FADE, 20 );
                        w.Append( GameRegionTxt );
                    }
                }
            }

            if ( idBtn.GetState() == STATE_CLICKED && mountMethod != 3 )
            {
                gprintf( "\n\tidBtn Clicked" );
                struct discHdr * header = gameList[gameBrowser->GetSelectedOption()];
                //enter new game ID
                char entered[10];
                snprintf( entered, sizeof( entered ), "%s", IDfull );
                //entered[9] = '\0';
                int result = OnScreenKeyboard( entered, 7, 0 );
                if ( result == 1 )
                {
                    WBFS_ReIDGame( header->id, entered );
                    //__Menu_GetEntries();
                    menu = MENU_DISCLIST;
                }

                idBtn.ResetState();
            }
            startat = gameBrowser->GetOffset(), offset = startat;
        }

        if ( ( ( gameSelected >= 0 ) && ( gameSelected < ( s32 )gameList.size() ) )
                || mountMethod == 1
                || mountMethod == 2 )
        {
            if ( searchBar )
            {
                HaltGui();
                mainWindow->Remove( searchBar );
                ResumeGui();
            }
            rockout();
            struct discHdr *header = ( mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected] );
            //  struct discHdr *header = dvdheader:gameList[gameSelected]);
            if ( !mountMethod )//only get this stuff it we are booting a game from USB
            {
                WBFS_GameSize( header->id, &size );
                if ( strlen( get_title( header ) ) < ( MAX_CHARACTERS + 3 ) )
                {
                    sprintf( text, "%s", get_title( header ) );
                }
                else
                {
                    strncpy( text, get_title( header ),  MAX_CHARACTERS );
                    text[MAX_CHARACTERS] = '\0';
                    strncat( text, "...", 3 );
                }
            }

            //check if alt Dol and gct file is present
            FILE *exeFile = NULL;
            char nipple[100];
            header = ( mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected] ); //reset header
            snprintf ( IDfull, sizeof( IDfull ), "%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4], header->id[5] );
            struct Game_CFG* game_cfg = CFG_get_game_opt( header->id );

            if ( game_cfg )
            {
                alternatedol = game_cfg->loadalternatedol;
                ocarinaChoice = game_cfg->ocarina;
            }
            else
            {
                alternatedol = off;
                ocarinaChoice = Settings.ocarina;
            }


            if ( Settings.qboot == yes ) //quickboot game
            {
                if ( alternatedol == on )
                {
                    /* Open dol File and check exist */
                    sprintf( nipple, "%s%s.dol", Settings.dolpath, IDfull );
                    exeFile = fopen ( nipple , "rb" );
                    if ( exeFile == NULL )
                    {
                        sprintf( nipple, "%s %s", nipple, tr( "does not exist!" ) );
                        WindowPrompt( tr( "Error" ), nipple, tr( "OK" ) );
                        menu = MENU_CHECK;
                        wiilight( 0 );
                        break;
                    }
                    else
                    {
                        fclose( exeFile );
                    }
                }
                if ( ocarinaChoice != off )
                {
                    /* Open gct File and check exist */
                    sprintf( nipple, "%s%s.gct", Settings.Cheatcodespath, IDfull );
                    exeFile = fopen ( nipple , "rb" );
                    if ( exeFile == NULL )
                    {
			gprintf( "\ttried to load missing gct.\n" );
                        sprintf( nipple, "%s %s", nipple, tr( "does not exist!  Loading game without cheats." ) );
                        WindowPrompt( tr( "Error" ), nipple, NULL, NULL, NULL, NULL, 170 );
                    }
                    else
                    {
                        fclose( exeFile );
                    }

                }

                wiilight( 0 );
                if ( isInserted( bootDevice ) )
                {
                    //////////save game play count////////////////
                    struct Game_NUM* game_num = CFG_get_game_num( header->id );

                    if ( game_num )
                    {
                        favoritevar = game_num->favorite;
                        playcount = game_num->count;
                    }
                    else
                    {
                        favoritevar = 0;
                        playcount = 0;
                    }
                    playcount += 1;

                    CFG_save_game_num( header->id );
                    gprintf( "\n\tplaycount for %c%c%c%c%c%c raised to %i", header->id[0], header->id[1], header->id[2], header->id[3], header->id[4], header->id[5], playcount );

                }
                menu = MENU_EXIT;
                break;

            }
            bool returnHere = true;// prompt to start game
            while ( returnHere )
            {
                returnHere = false;
                if ( Settings.wiilight != wiilight_forInstall ) wiilight( 1 );
                choice = GameWindowPrompt();
                // header = gameList[gameSelected]; //reset header

                if ( choice == 1 )
                {
                    if ( alternatedol == on )
                    {
                        /* Open dol File and check exist */
                        sprintf( nipple, "%s%s.dol", Settings.dolpath, IDfull );
                        exeFile = fopen ( nipple , "rb" );
                        if ( exeFile == NULL )
                        {
                            gprintf( "\n\tTried to load alt dol that isn't there" );
                            sprintf( nipple, "%s %s", nipple, tr( "does not exist!  You Messed something up, Idiot." ) );
                            WindowPrompt( tr( "Error" ), nipple, tr( "OK" ) );
                            menu = MENU_CHECK;
                            wiilight( 0 );
                            break;
                        }
                        else
                        {
                            fclose( exeFile );
                        }
                    }
                    if ( ocarinaChoice != off )
                    {
                        /* Open gct File and check exist */
                        sprintf( nipple, "%s%s.gct", Settings.Cheatcodespath, IDfull );
                        exeFile = fopen ( nipple , "rb" );
                        if ( exeFile == NULL )
                        {
			    gprintf( "\ttried to load gct file that isn't there\n" );
                            sprintf( nipple, "%s %s", nipple, tr( "does not exist!  Loading game without cheats." ) );
                            WindowPrompt( tr( "Error" ), nipple, NULL, NULL, NULL, NULL, 170 );
                        }
                        else
                        {
                            fclose( exeFile );
                        }

                    }
                    wiilight( 0 );
                    returnHere = false;
                    menu = MENU_EXIT;

                }
                else if ( choice == 2 )
                {
                    wiilight( 0 );
                    HaltGui();
                    if ( Settings.gameDisplay == list ) mainWindow->Remove( gameBrowser );
                    else if ( Settings.gameDisplay == grid ) mainWindow->Remove( gameGrid );
                    else if ( Settings.gameDisplay == carousel ) mainWindow->Remove( gameCarousel );
                    mainWindow->Remove( &w );
                    ResumeGui();

                    //re-evaluate header now in case they changed games while on the game prompt
                    header = ( mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected] );
                    int settret = GameSettings( header );
                    /* unneeded for now, kept in case database gets a separate language setting
                    //menu = MENU_DISCLIST; // refresh titles (needed if the language setting has changed)
                    */
                    HaltGui();
                    if ( Settings.gameDisplay == list )  mainWindow->Append( gameBrowser );
                    else if ( Settings.gameDisplay == grid ) mainWindow->Append( gameGrid );
                    else if ( Settings.gameDisplay == carousel ) mainWindow->Append( gameCarousel );
                    mainWindow->Append( &w );
                    ResumeGui();
                    if ( settret == 1 ) //if deleted
                    {
                        menu = MENU_DISCLIST;
                        break;
                    }
                    returnHere = true;
                    rockout( 2 );
                }

                else if ( choice == 3 && !mountMethod ) //WBFS renaming
                {
                    wiilight( 0 );
                    //re-evaluate header now in case they changed games while on the game prompt
                    header = gameList[gameSelected];

                    //enter new game title
                    char entered[60];
                    snprintf( entered, sizeof( entered ), "%s", get_title( header ) );
                    entered[59] = '\0';
                    int result = OnScreenKeyboard( entered, 60, 0 );
                    if ( result == 1 )
                    {
                        WBFS_RenameGame( header->id, entered );
                        gameList.ReadGameList();
                        gameList.FilterList();
                        menu = MENU_DISCLIST;
                    }
                }
                else if ( choice == 0 )
                {
                    rockout( 2 );
                    if ( mountMethod == 1 || mountMethod == 2 )mountMethod = mountMethodOLD;
                    if ( Settings.gameDisplay == list )
                    {
                        gameBrowser->SetFocus( 1 );
                    }
                    else if ( Settings.gameDisplay == grid )
                    {
                        gameGrid->SetFocus( 1 );
                    }
                    else if ( Settings.gameDisplay == carousel )
                    {
                        gameCarousel->SetFocus( 1 );
                    }
                }


            }
            if ( searchBar )
            {
                HaltGui();
                mainWindow->Append( searchBar );
                ResumeGui();
            }
        }
        // to skip the first call of windowScreensaver at startup when wiimote is not connected
        if ( IsWpadConnected() )
        {
            check = 1;
        }

        // screensaver is called when wiimote shuts down, depending on the wiimotet idletime
        if ( !IsWpadConnected() && check != 0 && Settings.screensaver != 0 )
        {
            check++;
            int screensaverIsOn = 0;
            if ( check == 11500 )   //to allow time for the wii to turn off and not show the screensaver
            {
                screensaverIsOn = WindowScreensaver();
            }
            if ( screensaverIsOn == 1 )check = 0;
        }
        covertOld = covert;
    }

    // set alt dol default
    if ( menu == MENU_EXIT && altdoldefault )
    {
        struct discHdr *header = ( mountMethod == 1 || mountMethod == 2 ? dvdheader : gameList[gameSelected] );
        struct Game_CFG* game_cfg = CFG_get_game_opt( header->id );
        // use default only if no alt dol was selected manually
        if ( game_cfg )
        {
            if ( game_cfg->alternatedolstart != 0 )
                altdoldefault = false;
        }
        if ( altdoldefault )
        {
            int autodol = autoSelectDol( ( char* )header->id, true );
            if ( autodol > 0 )
            {
                alternatedol = 2;
                alternatedoloffset = autodol;
                char temp[20];
                sprintf( temp, "%d", autodol );
            }
            else
            {
                // alt dol menu for games that require more than a single alt dol
                int autodol = autoSelectDolMenu( ( char* )header->id, true );
                if ( autodol > 0 )
                {
                    alternatedol = 2;
                    alternatedoloffset = autodol;
                }
            }
        }
    }
//no need to close sd here.  we still need to get settings and codes  and shit
    /*if (menu == MENU_EXIT) {
        SDCard_deInit();
    }*/
    //if (Settings.gameDisplay==list) {startat=gameBrowser->GetOffset(), offset=startat;}//save the variables in case we are refreshing the list
    //gprintf("\n\tstartat:%d offset:%d",startat,offset);
    HaltGui();
    mainWindow->RemoveAll();
    mainWindow->Append( bgImg );
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

void DiscListWinUpdateCallback( void * e )
{
    GuiWindow *w = ( GuiWindow * )e;
    for ( int i = 0; i < 8; ++i )
    {
        if ( Toolbar[i]->GetState() == STATE_SELECTED )
        {
            w->Remove( Toolbar[i] );
            w->Append( Toolbar[i] ); // draw the selected Icon allways on top
            break;
        }
    }
}

void rockout( int f )
{
    HaltGui();

    char imgPath[100];
    if ( gameSelected >= 0 && gameSelected < gameList.size() &&
            ( strcasestr( get_title( gameList[gameSelected] ), "guitar" ) ||
              strcasestr( get_title( gameList[gameSelected] ), "band" ) ||
              strcasestr( get_title( gameList[gameSelected] ), "rock" ) ) )
    {
        for ( int i = 0; i < 4; i++ )
            delete pointer[i];
        snprintf( imgPath, sizeof( imgPath ), "%srplayer1_point.png", CFG.theme_path );
        pointer[0] = new GuiImageData( imgPath, rplayer1_point_png );
        snprintf( imgPath, sizeof( imgPath ), "%srplayer2_point.png", CFG.theme_path );
        pointer[1] = new GuiImageData( imgPath, rplayer2_point_png );
        snprintf( imgPath, sizeof( imgPath ), "%srplayer3_point.png", CFG.theme_path );
        pointer[2] = new GuiImageData( imgPath, rplayer3_point_png );
        snprintf( imgPath, sizeof( imgPath ), "%srplayer4_point.png", CFG.theme_path );
        pointer[3] = new GuiImageData( imgPath, rplayer4_point_png );
    }
    else
    {

        for ( int i = 0; i < 4; i++ )
            delete pointer[i];
        snprintf( imgPath, sizeof( imgPath ), "%splayer1_point.png", CFG.theme_path );
        pointer[0] = new GuiImageData( imgPath, player1_point_png );
        snprintf( imgPath, sizeof( imgPath ), "%splayer2_point.png", CFG.theme_path );
        pointer[1] = new GuiImageData( imgPath, player2_point_png );
        snprintf( imgPath, sizeof( imgPath ), "%splayer3_point.png", CFG.theme_path );
        pointer[2] = new GuiImageData( imgPath, player3_point_png );
        snprintf( imgPath, sizeof( imgPath ), "%splayer4_point.png", CFG.theme_path );
        pointer[3] = new GuiImageData( imgPath, player4_point_png );
    }
    ResumeGui();
}
