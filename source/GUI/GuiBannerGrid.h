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
#ifndef _GUIBANNERGRID_H_
#define _GUIBANNERGRID_H_

#include <vector>
#include "gui.h"
#include "gui_gamebrowser.h"
#include "usbloader/disc.h"
#include "banner/BannerAsync.h"
#include "banner/Texture.h"
#include "SystemMenu/StaticFrame.h"

#define MAX_BUTTONS	12

class GuiBannerGrid : public GuiGameBrowser
{
public:
	GuiBannerGrid(int listOffset);
	virtual ~GuiBannerGrid();
	int GetClickedOption(void);
	int GetSelectedOption(void);
	int getListOffset(void) const { return pageNo * 12; }
	void GetIconCoordinates(int icon, f32 *x, f32 *y);
	void Update(GuiTrigger *t);
	void SetPage(int page) { pageNo = LIMIT(page, 0, pageCnt-1); UpdateTooltips(); }
	void Draw();
private:
	void RenderHighliter(Mtx &modelview);
	void UpdateTooltips(void);

	static const float gridwidth = 2048.f;
	static const float gridheight = 288.f;

	static const float chanWidth = 128.f;
	static const float chanHeight = 96.f;

	const int XOffset;
	const int YOffset;

	float fAnimation;
	float fAnimStep;

	bool AnimationRunning;

	int pageNo;
	int pageCnt;
	Vec2f ScreenProps;

	Texture gridFrameTex;
	Texture gridFrameEdgeTex;
	Texture gridHighliteTex;
	StaticFrame staticFrame;
	GXColor gridFrameColor;
	GXColor highliteColor;
	GXColorS10 gridFrameTevColor[3];
	Mtx gridview;
	std::vector<BannerAsync *> bannerList;

	GuiTrigger trigA;
	GuiTrigger trigL;
	GuiTrigger trigR;
	GuiTrigger trigPlus;
	GuiTrigger trigMinus;

	GuiImageData * imgLeft;
	GuiImageData * imgRight;
	GuiImageData * imgNewData;

	GuiImage * btnLeftImg;
	GuiImage * btnRightImg;
	GuiImage * gridNewImg[MAX_BUTTONS];

	GuiButton * btnLeft;
	GuiButton * btnRight;
	GuiButton * gridBtn[MAX_BUTTONS];

	GuiTooltip * gridTT[MAX_BUTTONS];
};

#endif
