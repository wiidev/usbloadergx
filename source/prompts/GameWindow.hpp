#ifndef GAMEWINDOW_HPP_
#define GAMEWINDOW_HPP_

#include "libwiigui/gui.h"
#include "libwiigui/gui_diskcover.h"

#define FAVORITE_STARS  5

class GameWindow : public GuiWindow
{
    public:
        GameWindow(int GameSelected);
        ~GameWindow();
        int Show();
        int GetSelectedGame() { return gameSelected; };
    protected:
        int MainLoop();
        void LoadGameSound(const u8 * id);
        void LoadDiscImage(const u8 * id);
        void SetWindowEffect(int direction, int in_out);
        void ChangeGame(int EffectDirection);

        bool reducedVol;
        int returnVal;
        int gameSelected;

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

        GuiButton * nameBtn;
        GuiButton * gameBtn;
        GuiButton * backBtn;
        GuiButton * settingsBtn;
        GuiButton * btnLeft;
        GuiButton * btnRight;
        GuiButton * FavoriteBtn[FAVORITE_STARS];

        GuiSound * gameSound;
};

#endif
