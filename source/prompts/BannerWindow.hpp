/****************************************************************************
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef BANNERWINDOW_HPP_
#define BANNERWINDOW_HPP_

#include "GUI/gui.h"
#include "GUI/gui_diskcover.h"
#include "menu/GameBrowseMenu.hpp"
#include "usbloader/disc.h"
#include "SystemMenu/SystemMenuResources.h"
#include "banner/Banner.h"

#define FAVORITE_STARS  5

class BannerWindow : public GuiWindow
{
	public:
		BannerWindow(GameBrowseMenu *m, struct discHdr *header);
		virtual ~BannerWindow();
		int Run();
		int GetSelectedGame() { return gameSelected; }
		void Draw(void);
		void Test();
	protected:
		int MainLoop();
		void Animate(void);
		void ChangeGame(bool playsound);

		static constexpr float fBannerWidth = 608.f;
		static constexpr float fBannerHeight = 448.f;
		static constexpr float fIconWidth = 128.f;
		static constexpr float fIconHeight = 96.f;

		static BannerFrame bannerFrame;

		bool reducedVol;
		int returnVal;
		int gameSelected;
		GameBrowseMenu *browserMenu;
		struct discHdr *dvdheader;

		const int MaxAnimSteps;

		int AnimStep;
		float AnimPosX, AnimPosY;
		float fAnimScale;
		bool AnimZoomIn;
		bool AnimationRunning;
		bool oldAnimationRunning;

		u8 BGAlpha;
		u8 BannerAlpha;

		Mtx modelview;
		Mtx44 projection;
		Mtx44 originalProjection;
		Vec2f ScreenProps;

		Banner *gameBanner;

		GuiTrigger * trigA;
		GuiTrigger * trigB;
		GuiTrigger * trigL;
		GuiTrigger * trigR;
		GuiTrigger * trigPlus;
		GuiTrigger * trigMinus;

		GuiImageData * imgFavorite;
		GuiImageData * imgNotFavorite;
		GuiImageData * imgLeft;
		GuiImageData * imgRight;

		GuiImage * btnLeftImg;
		GuiImage * btnRightImg;
		GuiImage * FavoriteBtnImg[FAVORITE_STARS];

		GuiText * playcntTxt;

		GuiButton * startBtn;
		GuiButton * backBtn;
		GuiButton * settingsBtn;
		GuiButton * btnLeft;
		GuiButton * btnRight;
		GuiButton * FavoriteBtn[FAVORITE_STARS];

		GuiSound * gameSound;
};

#endif
