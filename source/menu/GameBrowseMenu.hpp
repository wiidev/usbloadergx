#ifndef GAMEBROWSEMENU_HPP_
#define GAMEBROWSEMENU_HPP_

#include "GUI/gui_gamebrowser.h"
#include "GUI/gui_searchbar.h"
#include "utils/ThreadedTask.hpp"

class GameBrowseMenu : public GuiWindow
{
	public:
		GameBrowseMenu();
		virtual ~GameBrowseMenu();
		static int Execute();
		void ReloadBrowser();
		GuiGameBrowser *GetGameBrowser() { return gameBrowser; }
	private:
		int MainLoop();
		int OpenClickedGame(struct discHdr *header);
		int GetSelectedGame() { return (gameBrowser ? gameBrowser->GetSelectedOption() : -1); }
		void UpdateGameInfoText(const u8 * gameId);
		void LoadCover(struct discHdr *header);
		void CheckDiscSlotUpdate();
		void UpdateFreeSpace(void *arg);
		void UpdateClock();
		static void UpdateCallback(void * e);

		TCallback<GameBrowseMenu> HDDSizeCallback;
		u32 DiscDriveCoverOld;
		int returnMenu;
		int gameSelectedOld;
		int gameClicked;
		int GridRowsPreSearch;
		time_t lastrawtime;
		bool show_searchwindow;
		wchar_t searchChar;
		std::vector<GuiButton *> ToolBar;

		GuiGameBrowser * gameBrowser;
		GuiSearchBar * searchBar;

		GuiImageData * listBackground;
		GuiImageData * carouselBackground;
		GuiImageData * gridBackground;
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
		GuiImageData * imgplayersSortIcon;
		GuiImageData * imgarrangeGrid;
		GuiImageData * imgarrangeGrid_gray;
		GuiImageData * imgarrangeCarousel;
		GuiImageData * imgarrangeCarousel_gray;
		GuiImageData * imgarrangeList;
		GuiImageData * imgarrangeList_gray;
		GuiImageData * imgdvd;
		GuiImageData * imgdvd_gray;
		GuiImageData * imgBannerGrid;
		GuiImageData * imgBannerGrid_gray;
		GuiImageData * imgLock;
		GuiImageData * imgLock_gray;
		GuiImageData * imgUnlock;
		GuiImageData * imgUnlock_gray;
		GuiImageData * imgCategory;
		GuiImageData * imgCategory_gray;
		GuiImageData * imgLoaderMode;
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
		GuiImage * bannerGridBtnImg;
		GuiImage * bannerGridBtnImg_g;
		GuiImage * lockBtnImg;
		GuiImage * lockBtnImg_g;
		GuiImage * unlockBtnImg;
		GuiImage * unlockBtnImg_g;
		GuiImage * dvdBtnImg;
		GuiImage * dvdBtnImg_g;
		GuiImage * categBtnImg;
		GuiImage * categBtnImg_g;
		GuiImage * loaderModeBtnImg;
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
		GuiButton * bannerGridBtn;
		GuiButton * lockBtn;
		GuiButton * dvdBtn;
		GuiButton * categBtn;
		GuiButton * loaderModeBtn;
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
		GuiTooltip * bannerGridBtnTT;
		GuiTooltip * lockBtnTT;
		GuiTooltip * dvdBtnTT;
		GuiTooltip * categBtnTT;
		GuiTooltip * loaderModeBtnTT;
		GuiTooltip * homebrewBtnTT;
		GuiTooltip * DownloadBtnTT;
		GuiTooltip * IDBtnTT;
};

#endif
