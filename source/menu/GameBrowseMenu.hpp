#ifndef GAMEBROWSEMENU_HPP_
#define GAMEBROWSEMENU_HPP_

#include "libwiigui/gui.h"
#include "libwiigui/gui_gamebrowser.h"
#include "libwiigui/gui_gamegrid.h"
#include "libwiigui/gui_gamecarousel.h"
#include "libwiigui/gui_searchbar.h"
#include "utils/ThreadedTask.hpp"

class GameBrowseMenu : public GuiWindow
{
    public:
        GameBrowseMenu();
        ~GameBrowseMenu();
        int Show();
    protected:
        int MainLoop();
        void ReloadBrowser();
        int OpenClickedGame();
        int GetSelectedGame();
        int GetClickedGame();
        void UpdateGameInfoText(const u8 * gameId);
        void LoadCover(struct discHdr *header);
        void CheckAlternativeDOL(const char * IDfull);
        void CheckOcarina(const char * IDfull);
        void CheckDiscSlotUpdate();
        void UpdateFreeSpace(void *arg);
        void UpdateClock(time_t &rawtime);
        static void UpdateCallback(void * e);

        TCallback<GameBrowseMenu> HDDSizeCallback;
        u32 DiscDriveCoverOld;
        int returnMenu;
        int gameSelectedOld;
        int gameClicked;
        time_t lastrawtime;
        bool show_searchwindow;
        wchar_t searchChar;
        std::vector<GuiButton *> ToolBar;

        GuiGameBrowser * gameBrowser;
        GuiGameGrid * gameGrid;
        GuiGameCarousel * gameCarousel;
        GuiSearchBar * searchBar;

        GuiImageData * btnInstall;
        GuiImageData * btnInstallOver;
        GuiImageData * btnSettings;
        GuiImageData * btnSettingsOver;
        GuiImageData * btnpwroff;
        GuiImageData * btnpwroffOver;
        GuiImageData * btnhome;
        GuiImageData * btnhomeOver;
        GuiImageData * btnsdcardOver;
        GuiImageData * btnsdcard;
        GuiImageData * imgfavIcon;
        GuiImageData * imgfavIcon_gray;
        GuiImageData * imgsearchIcon;
        GuiImageData * imgsearchIcon_gray;
        GuiImageData * imgabcIcon;
        GuiImageData * imgrankIcon;
        GuiImageData * imgplayCountIcon;
        GuiImageData * imgarrangeGrid;
        GuiImageData * imgarrangeGrid_gray;
        GuiImageData * imgarrangeCarousel;
        GuiImageData * imgarrangeCarousel_gray;
        GuiImageData * imgarrangeList;
        GuiImageData * imgarrangeList_gray;
        GuiImageData * imgdvd;
        GuiImageData * imgdvd_gray;
        GuiImageData * imgLock;
        GuiImageData * imgLock_gray;
        GuiImageData * imgUnlock;
        GuiImageData * imgUnlock_gray;
        GuiImageData * homebrewImgData;
        GuiImageData * homebrewImgDataOver;
        GuiImageData * gameCover;

        GuiTrigger * trigA;
        GuiTrigger * trigHome;
        GuiTrigger * trig1;
        GuiTrigger * trig2;

        GuiImage * installBtnImg;
        GuiImage * installBtnImgOver;
        GuiImage * settingsBtnImg;
        GuiImage * settingsBtnImgOver;
        GuiImage * homeBtnImg;
        GuiImage * homeBtnImgOver;
        GuiImage * poweroffBtnImg;
        GuiImage * poweroffBtnImgOver;
        GuiImage * sdcardImg;
        GuiImage * sdcardImgOver;
        GuiImage * favoriteBtnImg;
        GuiImage * favoriteBtnImg_g;
        GuiImage * searchBtnImg;
        GuiImage * searchBtnImg_g;
        GuiImage * sortBtnImg;
        GuiImage * listBtnImg;
        GuiImage * listBtnImg_g;
        GuiImage * gridBtnImg;
        GuiImage * gridBtnImg_g;
        GuiImage * carouselBtnImg;
        GuiImage * carouselBtnImg_g;
        GuiImage * lockBtnImg;
        GuiImage * lockBtnImg_g;
        GuiImage * unlockBtnImg;
        GuiImage * unlockBtnImg_g;
        GuiImage * dvdBtnImg;
        GuiImage * dvdBtnImg_g;
        GuiImage * homebrewImg;
        GuiImage * homebrewImgOver;
        GuiImage * gameCoverImg;

        GuiText * usedSpaceTxt;
        GuiText * gamecntTxt;
        GuiText * clockTimeBack;
        GuiText * clockTime;
        GuiText * GameRegionTxt;
        GuiText * GameIDTxt;

        GuiButton * gamecntBtn;
        GuiButton * installBtn;
        GuiButton * settingsBtn;
        GuiButton * homeBtn;
        GuiButton * poweroffBtn;
        GuiButton * sdcardBtn;
        GuiButton * gameInfo;
        GuiButton * favoriteBtn;
        GuiButton * searchBtn;
        GuiButton * sortBtn;
        GuiButton * listBtn;
        GuiButton * gridBtn;
        GuiButton * carouselBtn;
        GuiButton * lockBtn;
        GuiButton * dvdBtn;
        GuiButton * homebrewBtn;
        GuiButton * DownloadBtn;
        GuiButton * idBtn;

        GuiTooltip * installBtnTT;
        GuiTooltip * settingsBtnTT;
        GuiTooltip * homeBtnTT;
        GuiTooltip * poweroffBtnTT;
        GuiTooltip * sdcardBtnTT;
        GuiTooltip * favoriteBtnTT;
        GuiTooltip * searchBtnTT;
        GuiTooltip * sortBtnTT;
        GuiTooltip * listBtnTT;
        GuiTooltip * gridBtnTT;
        GuiTooltip * carouselBtnTT;
        GuiTooltip * lockBtnTT;
        GuiTooltip * dvdBtnTT;
        GuiTooltip * homebrewBtnTT;
        GuiTooltip * DownloadBtnTT;
        GuiTooltip * IDBtnTT;
};

#endif
