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
#include "GuiBannerGrid.h"
#include "themes/CTheme.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "settings/newtitles.h"
#include "SystemMenu/SystemMenuResources.h"
#include "usbloader/GameList.h"
#include "gecko.h"

//! some math to get the row and column from channel idx
static inline int Idx2Row(int sIdx)
{
	if(sIdx > 0)
		return (sIdx / 4) % 3;
	else if(sIdx < 0)
		return (2 + ((sIdx + 1) / 4) % 3);
	else
		return 0;
}

static inline int Idx2Column(int sIdx)
{
	if(sIdx == 0)
		return 0;

	if(sIdx > 0) {
		return ( (sIdx / 12) * 4 + sIdx % 4 );
	}
	else
	{
		int column = (sIdx % 4);
		if(column == 0)
			column = -4;
		column += ((sIdx + 1) / 12) * 4;

		return column;
	}
}

GuiBannerGrid::GuiBannerGrid(int listOffset)
	: XOffset(thInt("0 - game bannergrid layout pos x"))
	, YOffset(thInt("-50 - game bannergrid layout pos y"))
	, fAnimation(0.f)
	, fAnimStep(Settings.BannerGridSpeed)
	, AnimationRunning(false)
	, gridFrameColor(thColor("r=237 g=237 b=237 a=255 - banner icon frame color"))
	, highliteColor(thColor("r=52 g=190 b=237 a=255 - banner icon highlite color"))
{
	GXColor gridTevColor[3];
	gridTevColor[0] = thColor("r=130 g=130 b=130 a=0 - banner icon frame edge tev color 1");
	gridTevColor[1] = thColor("r=180 g=180 b=180 a=255 - banner icon frame edge tev color 2");
	gridTevColor[2] = thColor("r=255 g=255 b=255 a=255 - banner icon frame edge tev color 3");

	for(int i = 0; i < 3; i++)
		gridFrameTevColor[i] = (GXColorS10) { gridTevColor[i].r, gridTevColor[i].g, gridTevColor[i].b, gridTevColor[i].a };

	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_TRIGGER_R);
	trigMinus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, PAD_TRIGGER_L);

	imgLeft = Resources::GetImageData("startgame_arrow_left.png");
	imgRight = Resources::GetImageData("startgame_arrow_right.png");
	imgNewData = Resources::GetImageData("new.png");

	btnLeftImg = new GuiImage(imgLeft);
	if (Settings.wsprompt) btnLeftImg->SetWidescreen(Settings.widescreen);
	btnLeft = new GuiButton(btnLeftImg, btnLeftImg, ALIGN_LEFT, ALIGN_MIDDLE, 20, YOffset, &trigA, btnSoundOver, btnSoundClick2, 1);
	btnLeft->SetTrigger(&trigL);
	btnLeft->SetTrigger(&trigMinus);
	btnLeft->SetParent(this);

	btnRightImg = new GuiImage(imgRight);
	if (Settings.wsprompt) btnRightImg->SetWidescreen(Settings.widescreen);
	btnRight = new GuiButton(btnRightImg, btnRightImg, ALIGN_RIGHT, ALIGN_MIDDLE, -20, YOffset, &trigA, btnSoundOver, btnSoundClick2, 1);
	btnRight->SetTrigger(&trigR);
	btnRight->SetTrigger(&trigPlus);
	btnRight->SetParent(this);

	//! Get chanSel archive
	if(SystemMenuResources::Instance()->GetChanSelAsh())
	{
		U8Archive chanSelArc(SystemMenuResources::Instance()->GetChanSelAsh(), SystemMenuResources::Instance()->GetChanSelAshSize());

		//! Create texture for the
		gridFrameTex.Load(chanSelArc.GetFile("/arc/timg/IplTopMask4x3.tpl"));
		gridFrameEdgeTex.Load(chanSelArc.GetFile("/arc/timg/IplTopMaskEgde4x3.tpl"));
		gridHighliteTex.Load(chanSelArc.GetFile("/arc/timg/my_TV_f.tpl"));

		//! create static image frame
		staticFrame.Load(chanSelArc);
	}

	//! create vector with empty banner list
	bannerList.resize(gameList.size(), NULL);

	//! Calculate the page count
	//! 1 page is minumum to show statics even if no games are loaded
	pageCnt = std::max((int)(bannerList.size() + 11) / 12, 1);
	pageNo = LIMIT(listOffset / 12, 0, pageCnt-1);

	//! set screen properties
	width = ScreenProps.x = screenwidth;
	height = ScreenProps.y = screenheight;

	//! setup the gridview
	guMtxIdentity(gridview);
	guMtxTransApply(gridview, gridview, 0.0F, 0.0F, -9900.0F);

	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		int row = Idx2Row(i);
		int column = Idx2Column(i);
		float fx = XOffset + chanWidth * column + ScreenProps.x * 0.5f  - chanWidth * 2.f;
		float fy = YOffset + chanHeight * row + (ScreenProps.y - chanHeight) * 0.5f - chanHeight;

		gridNewImg[i] = new GuiImage(imgNewData);
		gridNewImg[i]->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
		gridNewImg[i]->SetPosition(-5, -5);
		gridNewImg[i]->SetScale(0.95f);
		gridNewImg[i]->SetEffect(EFFECT_PULSE, 5, 100);

		gridTT[i] = new GuiTooltip(NULL);
		gridTT[i]->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
		gridTT[i]->SetAlpha(thInt("255 - tooltip alpha"));

		gridBtn[i] = new GuiButton(chanWidth, chanHeight);
		gridBtn[i]->SetPosition(fx, fy);
		gridBtn[i]->SetTrigger(&trigA);
		gridBtn[i]->SetSoundClick(btnSoundClick2);
		gridBtn[i]->SetParent(this);
		gridBtn[i]->SetIcon(gridNewImg[i]);
		if(i >= (int) bannerList.size())
			gridBtn[i]->SetState(STATE_DISABLED);
	}

	UpdateTooltips();
}

GuiBannerGrid::~GuiBannerGrid()
{
	for(int i = 0; i < MAX_BUTTONS; ++i)
	{
		delete gridBtn[i];
		delete gridTT[i];
		delete gridNewImg[i];
	}

	for(u32 i = 0; i < bannerList.size(); ++i)
	{
		if(bannerList[i] != NULL)
			BannerAsync::RemoveBanner(bannerList[i]);
	}

	delete imgLeft;
	delete imgRight;
	delete imgNewData;

	delete btnLeftImg;
	delete btnRightImg;

	delete btnLeft;
	delete btnRight;
}

int GuiBannerGrid::GetClickedOption(void)
{
	for (int i = 0; i < MAX_BUTTONS; ++i)
	{
		if (gridBtn[i]->GetState() == STATE_CLICKED)
		{
			gridBtn[i]->SetState(STATE_SELECTED);
			return pageNo * 12 + i;
		}
	}
	return -1;
}

int GuiBannerGrid::GetSelectedOption(void)
{
	for (int i = 0; i < MAX_BUTTONS; ++i)
	{
		if (gridBtn[i]->GetState() == STATE_SELECTED)
			return pageNo * 12 + i;
	}
	return -1;
}

void GuiBannerGrid::UpdateTooltips(void)
{
	int chIdx = pageNo * 12;

	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		if(chIdx < 0 || chIdx >= (int) bannerList.size())
		{
			gridBtn[i]->SetState(STATE_DISABLED);
		}
		else
		{
			if(gridBtn[i]->GetState() == STATE_DISABLED)
				gridBtn[i]->SetState(STATE_DEFAULT);
			gridTT[i]->SetText(GameTitles.GetTitle(gameList[chIdx]));
			gridBtn[i]->SetToolTip(gridTT[i], 0, 30, ALIGN_CENTER, ALIGN_BOTTOM);

			if(gridTT[i]->GetLeft() < 20)
				gridBtn[i]->SetToolTip(gridTT[i], 20 - gridTT[i]->GetLeft(),
									   30, ALIGN_CENTER, ALIGN_BOTTOM);

			else if((gridTT[i]->GetLeft() + gridTT[i]->GetWidth()) > (screenwidth - 20))
				gridBtn[i]->SetToolTip(gridTT[i], (screenwidth - 20) -  (gridTT[i]->GetLeft() + gridTT[i]->GetWidth()),
									   30, ALIGN_CENTER, ALIGN_BOTTOM);
		}
		chIdx++;
	}
}

void GuiBannerGrid::GetIconCoordinates(int icon, f32 *x, f32 *y)
{
	int row = Idx2Row(icon % 12);
	int column = Idx2Column(icon % 12);
	*x = XOffset + chanWidth * column + ScreenProps.x * 0.5f  - chanWidth * 2.f;
	*y = YOffset + chanHeight * row + (ScreenProps.y - chanHeight) * 0.5f - chanHeight;
}

void GuiBannerGrid::RenderHighliter(Mtx &modelview)
{
	u8 Tlut = 0;
	gridHighliteTex.Apply(Tlut, GX_TEXMAP0, GX_MIRROR, GX_MIRROR);

	Mtx m1, mv;
	guMtxIdentity(mv);
	guMtxScaleApply(modelview, m1, 1.f, -1.f, 1.f);
	guMtxTransApply(m1, m1,ScreenProps.x * 0.5f, ScreenProps.y * 0.5f, 0.f);
	guMtxScaleApply(mv, mv, chanWidth, chanHeight, 0.f);
	guMtxTransApply(mv, mv, -chanWidth * 0.5f, -chanHeight * 0.5f, 0.f);
	guMtxConcat(m1, mv, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_Color4u8(highliteColor.r, highliteColor.g, highliteColor.b, highliteColor.a);
	GX_TexCoord2f32(0.f, 2.f);

	GX_Position3f32(1.f, 0.f, 0.f);
	GX_Color4u8(highliteColor.r, highliteColor.g, highliteColor.b, highliteColor.a);
	GX_TexCoord2f32(2.f, 2.f);

	GX_Position3f32(1.f, 1.f, 0.f);
	GX_Color4u8(highliteColor.r, highliteColor.g, highliteColor.b, highliteColor.a);
	GX_TexCoord2f32(2.f, 0.f);

	GX_Position3f32(0.f, 1.f, 0.f);
	GX_Color4u8(highliteColor.r, highliteColor.g, highliteColor.b, highliteColor.a);
	GX_TexCoord2f32(0.f, 0.f);
	GX_End();
}

void GuiBannerGrid::Update(GuiTrigger *t)
{
	if(!t || state == STATE_DISABLED)
		return;

	for(int i = 0; i < MAX_BUTTONS && !AnimationRunning; i++)
		gridBtn[i]->Update(t);

	if(pageNo > 0)
	{
		btnLeft->Update(t);

		if((btnLeft->GetState() == STATE_CLICKED) && !AnimationRunning) {
			btnLeft->SetState(STATE_DEFAULT);
			fAnimation -= chanWidth * 4.f;
			pageNo--;
			UpdateTooltips();
		}
	}


	if(pageNo < pageCnt-1)
	{
		btnRight->Update(t);

		if((btnRight->GetState() == STATE_CLICKED) && !AnimationRunning) {
			btnRight->SetState(STATE_DEFAULT);
			fAnimation += chanWidth * 4.f;
			pageNo++;
			UpdateTooltips();
		}
	}
}

void GuiBannerGrid::Draw()
{
	if(!this->IsVisible())
		return;

	u8 Tlut = 0;

	if(fAnimation > -fAnimStep  && fAnimation < fAnimStep)
		fAnimation = 0.0f;
	else if(fAnimation > 0.0f)
		fAnimation -= fAnimStep;
	else if(fAnimation < 0.0f)
		fAnimation += fAnimStep;

	AnimationRunning = (fAnimation != 0.0f);

	bool bSkipFrame = Settings.PAL50 && ((frameCount % 6) == 0);

	Mtx modelview;
	guMtxTransApply(gridview, modelview, fAnimation, 0.f, 0.f);

	int chIdx = pageNo * 12;

	//! removed unneeded banners
	for(int i = 0; i < (int) bannerList.size(); i++)
	{
		if((i < (chIdx - 24) || i > (chIdx + 36)) && bannerList[i] != NULL)
		{
			BannerAsync::RemoveBanner(bannerList[i]);
			bannerList[i] = NULL;
		}
	}

	//! Load the games that are seen first and after that the rest
	for(int i = chIdx+11; i >= chIdx; i--) // counting backwards so the loading is upwards
	{
		if(i >= 0 && i < (int) bannerList.size())
		{
			if(!bannerList[i])
				bannerList[i] = new BannerAsync(gameList[i]);

			if(!bannerList[i]->getIcon())
				BannerAsync::PushFront(bannerList[i]);
		}
	}

	//! we start at the channels on pre-pre-page
	chIdx -= 24;

	int GridCutLeft = 0;
	int GridCutRight = vmode->fbWidth;

	for(int sIdx = -24; sIdx < 36; sIdx++, chIdx++)
	{
		int row = Idx2Row(sIdx);
		int column = Idx2Column(sIdx);

		if(chIdx < 0 || chIdx >= pageCnt*12)
			continue;

		if(chIdx >= 0 && chIdx < (int) bannerList.size() && !bannerList[chIdx])
			bannerList[chIdx] = new BannerAsync(gameList[chIdx]);

		Mtx mv1, mv2, iconview;
		guMtxTransApply(modelview, iconview, XOffset + chanWidth * column - chanWidth * 1.5f,
											-YOffset - chanHeight * row + chanHeight, 0.f);

		guMtxScaleApply(iconview, mv1, 1.f, -1.f, 1.f);
		guMtxTransApply(mv1,mv1, (ScreenProps.x - chanWidth) * 0.5f, (ScreenProps.y - chanHeight) * 0.5f, 0.f);
		guMtxTransApply(mv1,mv2, chanWidth, chanHeight, 0.f);

		f32 viewportv[6];
		f32 projectionv[7];

		GX_GetViewportv(viewportv, vmode);
		GX_GetProjectionv(projectionv, FSProjection2D, GX_ORTHOGRAPHIC);

		guVector vecTL;
		guVector vecBR;
		GX_Project(0.0f, 0.0f, 0.0f, mv1, projectionv, viewportv, &vecTL.x, &vecTL.y, &vecTL.z);
		GX_Project(0.0f, 0.0f, 0.0f, mv2, projectionv, viewportv, &vecBR.x, &vecBR.y, &vecBR.z);

		//! Round scissor box offset up and the box size down
		u32 scissorX = (u32)(0.5f + std::max(vecTL.x, (f32)std::max(-Settings.AdjustOverscanX, 0)));
		u32 scissorY = (u32)(0.5f + std::max(vecTL.y, (f32)std::max(-Settings.AdjustOverscanY, 0)));
		u32 scissorW = (u32)std::max(std::min(vecBR.x, ScreenProps.x-1+Settings.AdjustOverscanX) - scissorX, 0.0f);
		u32 scissorH = (u32)std::max(vecBR.y - scissorY, 0.0f);

		GX_SetScissor(scissorX, scissorY, scissorW, scissorH );

		// save scissor value for grid cut of left/right part
		if(chIdx == 0)
			GridCutLeft = scissorX;
		if(chIdx == pageCnt*12-1)
			GridCutRight = scissorX+scissorW;

		if(chIdx >= (int) bannerList.size() || !bannerList[chIdx]->getIcon())
		{
			//! If out of range or the icon is not loaded yet render the static frame
			staticFrame.Render(iconview, ScreenProps, Settings.widescreen);
		}
		else
		{
			Menu_DrawRectangle(0, 0, screenwidth, screenheight, (GXColor) { 0, 0, 0, 255}, 1);
			bannerList[chIdx]->getIcon()->Render(iconview, ScreenProps, Settings.widescreen);
			bannerList[chIdx]->getIcon()->AdvanceFrame();
			if(bSkipFrame)
				bannerList[chIdx]->getIcon()->AdvanceFrame();
		}
	}

	//! only advance the static animation once for the whole loop
	staticFrame.AdvanceFrame();
	if(bSkipFrame)
		staticFrame.AdvanceFrame();

	//! scissor box for the grid
	//! don't draw grid outside of overscan render range and cut off the stuff before first and after last element
	u32 scissorX = (u32)std::max(-Settings.AdjustOverscanX, GridCutLeft);
	u32 scissorY = (u32)std::max(-Settings.AdjustOverscanY, 0);
	u32 scissorW = (u32)LIMIT(ScreenProps.x-1 + Settings.AdjustOverscanX * 2.f, 0.f, GridCutRight - scissorX);
	u32 scissorH = (u32)std::max(ScreenProps.x-1 + Settings.AdjustOverscanY * 2.f, 0.f);
	GX_SetScissor(scissorX, scissorY, scissorW, scissorH);

	//! Reset GX after icons
	ReSetup_GX();

	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	Mtx mv;
	guMtxIdentity(mv);
	guMtxScaleApply(mv, mv, gridwidth, gridheight, 1.0f);
	guMtxTransApply(mv, mv, (ScreenProps.x - gridwidth) * 0.5f + XOffset,
							(ScreenProps.y - gridheight) * 0.5f + YOffset,
							0.f);
	guMtxConcat(modelview, mv, mv);

	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	// this texture uses the default environment
	gridFrameTex.Apply(Tlut, GX_TEXMAP0, GX_MIRROR, GX_MIRROR);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_Color4u8(gridFrameColor.r, gridFrameColor.g, gridFrameColor.b, gridFrameColor.a);
	GX_TexCoord2f32(0.f, 6.f); // 3 rows

	GX_Position3f32(1.f, 0.f, 0.f);
	GX_Color4u8(gridFrameColor.r, gridFrameColor.g, gridFrameColor.b, gridFrameColor.a);
	GX_TexCoord2f32(32.f, 6.f); // 16 columns, 3 rows

	GX_Position3f32(1.f, 1.f, 0.f);
	GX_Color4u8(gridFrameColor.r, gridFrameColor.g, gridFrameColor.b, gridFrameColor.a);
	GX_TexCoord2f32(32.f, 0.f); // 16 columns

	GX_Position3f32(0.f, 1.f, 0.f);
	GX_Color4u8(gridFrameColor.r, gridFrameColor.g, gridFrameColor.b, gridFrameColor.a);
	GX_TexCoord2f32(0.f, 0.f);
	GX_End();

	// this texture uses it's own tev and the same position as the above
	gridFrameEdgeTex.Apply(Tlut, GX_TEXMAP0, GX_MIRROR, GX_MIRROR);

	// color registers
	GX_SetTevColorS10(GX_TEVREG0, gridFrameTevColor[0]);
	GX_SetTevColorS10(GX_TEVREG1, gridFrameTevColor[1]);
	GX_SetTevColorS10(GX_TEVREG2, gridFrameTevColor[2]);

	// texture environment
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL );
	GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C1);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV );
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K3_A );
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV );
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K3_A );
	GX_SetNumTevStages(1);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0.f, 0.f, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(0.f, 6.f); // 3 rows

	GX_Position3f32(1.f, 0.f, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(32.f, 6.f); // 16 columns, 3 rows

	GX_Position3f32(1.f, 1.f, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(32.f, 0.f); // 16 columns

	GX_Position3f32(0.f, 1.f, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(0.f, 0.f);
	GX_End();

	// reset GX configs
	ReSetup_GX();

	// render highlter
	for(int sIdx = 0; sIdx < MAX_BUTTONS; sIdx++)
	{
		// only render selected and when no animation is on going
		if(!AnimationRunning && gridBtn[sIdx]->GetState() == STATE_SELECTED)
		{
			int row = Idx2Row(sIdx);
			int column = Idx2Column(sIdx);
			Mtx hlview;
			guMtxTransApply(modelview, hlview, XOffset + chanWidth * column - chanWidth * 1.5f,
											  -YOffset - chanHeight * row + chanHeight, 0.f);
			RenderHighliter(hlview);
		}
	}

	// reset GX configs
	ReSetup_GX();

	// remove scissor again
	GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);

	if(pageNo > 0)
		btnLeft->Draw();
	if(pageNo < pageCnt-1)
		btnRight->Draw();

	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		if(AnimationRunning)
			gridBtn[i]->ResetState();

		if (!AnimationRunning && Settings.marknewtitles && (pageNo * 12 + i) < gameList.size()
			&& NewTitles::Instance()->IsNew(gameList[pageNo * 12 + i]->id))
				gridBtn[i]->Draw();

		gridBtn[i]->DrawTooltip();
	}
}
