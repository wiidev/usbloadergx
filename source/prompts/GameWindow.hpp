#ifndef GAMEWINDOW_HPP_
#define GAMEWINDOW_HPP_

#include "GUI/gui.h"
#include "GUI/gui_diskcover.h"
#include "banner/BannerAsync.h"
#include "menu/GameBrowseMenu.hpp"
#include "usbloader/disc.h"

#define FAVORITE_STARS  5

class GameWindow : public GuiWindow
{
	public:
		GameWindow(GameBrowseMenu *m, struct discHdr *header);
		virtual ~GameWindow();
		int Run();
		int GetSelectedGame() { return gameSelected; }
		static void BootGame(struct discHdr *header);
	protected:
		int MainLoop();
		void LoadGameSound(const struct discHdr * header);
		void LoadDiscImage(const u8 * id);
		void SetWindowEffect(int direction, int in_out);
		void ChangeGame(int EffectDirection);
		void Hide();
		void Show();

		bool reducedVol;
		bool hidden;
		int returnVal;
		int gameSelected;
		GameBrowseMenu *browserMenu;
		struct discHdr *dvdheader;
		Banner gameBanner;

		GuiTrigger * trigA;
		GuiTrigger * trigB;
		GuiTrigger * trigL;
		GuiTrigger * trigR;
		GuiTrigger * trigPlus;
		GuiTrigger * trigMinus;

		GuiImageData * diskImgData;
		GuiImageData * diskImgData2;
		GuiImageData * dialogBox;
		GuiImageData * btnOutline;
		GuiImageData * imgFavorite;
		GuiImageData * imgNotFavorite;
		GuiImageData * imgLeft;
		GuiImageData * imgRight;

		GuiDiskCover * diskImg;
		GuiDiskCover * diskImg2;

		GuiImage * dialogBoxImg;
		GuiImage * backBtnImg;
		GuiImage * settingsBtnImg;
		GuiImage * btnLeftImg;
		GuiImage * btnRightImg;
		GuiImage * FavoriteBtnImg[FAVORITE_STARS];

		GuiTooltip * nameBtnTT;

		GuiText * sizeTxt;
		GuiText * playcntTxt;
		GuiText * nameTxt;
		GuiText * backBtnTxt;
		GuiText * settingsBtnTxt;
		GuiText * detailsBtnTxt;
		GuiText * detailsBtnOverTxt;

		GuiButton * nameBtn;
		GuiButton * gameBtn;
		GuiButton * backBtn;
		GuiButton * settingsBtn;
		GuiButton * detailsBtn;
		GuiButton * btnLeft;
		GuiButton * btnRight;
		GuiButton * FavoriteBtn[FAVORITE_STARS];

		GuiSound * gameSound;
};

#endif
