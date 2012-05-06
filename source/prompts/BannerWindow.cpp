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
#include <unistd.h>
#include "BannerWindow.hpp"
#include "GUI/GuiBannerGrid.h"
#include "banner/BannerAsync.h"
#include "banner/CustomBanner.h"
#include "banner/OpeningBNR.hpp"
#include "settings/CSettings.h"
#include "settings/CGameStatistics.h"
#include "settings/menus/GameSettingsMenu.hpp"
#include "SystemMenu/SystemMenuResources.h"
#include "prompts/GameWindow.hpp"
#include "themes/CTheme.h"
#include "language/gettext.h"
#include "menu/menus.h"
#include "utils/tools.h"

// Load only once
BannerFrame BannerWindow::bannerFrame;

BannerWindow::BannerWindow(GameBrowseMenu *m, struct discHdr *header)
	: GuiWindow(screenwidth, screenheight)
	, browserMenu(m)
	, MaxAnimSteps(Settings.BannerZoomDuration)
{
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;

	f32 xOffset = Settings.BannerProjectionOffsetX;
	f32 yOffset = Settings.BannerProjectionOffsetY;

	guMtxIdentity(modelview);
	guMtxTransApply (modelview, modelview, xOffset, yOffset, 0.0F);

	memcpy(&originalProjection, &FSProjection2D, sizeof(Mtx44));

	returnVal = -1;
	gameSelected = 0;
	gameSound = NULL;
	dvdheader = NULL;
	reducedVol = false;

	if(!bannerFrame.IsLoaded())
		bannerFrame.Load(U8Archive(SystemMenuResources::Instance()->GetChanTtlAsh(),
								   SystemMenuResources::Instance()->GetChanTtlAshSize()));

	AnimStep = 0;
	AnimPosX = 0.5f * (ScreenProps.x - fIconWidth);
	AnimPosY = 0.5f * (ScreenProps.y - fIconHeight);
	AnimZoomIn = true;
	AnimationRunning = false;

	int gameIdx;

	//! get the game index to this header
	for(gameIdx = 0; gameIdx < gameList.size(); ++gameIdx)
	{
		if(gameList[gameIdx] == header)
		{
			gameSelected = gameIdx;
			break;
		}
	}

	//! Set dvd header if the header does not match any of the list games
	if(gameIdx == gameList.size())
		dvdheader = header;

	GuiBannerGrid *bannerBrowser = dynamic_cast<GuiBannerGrid *>(browserMenu->GetGameBrowser());
	if(bannerBrowser)
		bannerBrowser->GetIconCoordinates(gameSelected, &AnimPosX, &AnimPosY);

	gameBanner = new Banner;

	imgLeft = Resources::GetImageData("startgame_arrow_left.png");
	imgRight = Resources::GetImageData("startgame_arrow_right.png");

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	trigL = new GuiTrigger;
	trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
	trigR = new GuiTrigger;
	trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_TRIGGER_R);
	trigMinus = new GuiTrigger;
	trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, PAD_TRIGGER_L);

	playcntTxt = new GuiText((char*) NULL, 18, thColor("r=0 g=0 b=0 a=255 - banner window playcount text color"));
	playcntTxt->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	playcntTxt->SetPosition(thInt("0 - banner window play count pos x"),
							thInt("215 - banner window play count pos y") - Settings.AdjustOverscanY / 2);

	settingsBtn = new GuiButton(215, 75);
	settingsBtn->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	settingsBtn->SetSoundOver(btnSoundOver);
	settingsBtn->SetSoundClick(btnSoundClick2);
	settingsBtn->SetPosition(-120, 175);
	settingsBtn->SetTrigger(trigA);

	startBtn = new GuiButton(215, 75);
	startBtn->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	startBtn->SetSoundOver(btnSoundOver);
	startBtn->SetSoundClick(btnSoundClick2);
	startBtn->SetPosition(110, 175);
	startBtn->SetTrigger(trigA);

	backBtn = new GuiButton(215, 75);
	backBtn->SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);
	backBtn->SetSoundOver(btnSoundOver);
	backBtn->SetSoundClick(btnSoundClick2);
	backBtn->SetPosition(-screenwidth, -screenheight); // set out of screen
	backBtn->SetTrigger(0, trigA);
	backBtn->SetTrigger(1, trigB);

	btnLeftImg = new GuiImage(imgLeft);
	if (Settings.wsprompt) btnLeftImg->SetWidescreen(Settings.widescreen);
	btnLeft = new GuiButton(btnLeftImg, btnLeftImg, ALIGN_LEFT, ALIGN_MIDDLE, 20, -50, trigA, btnSoundOver, btnSoundClick2, 1);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);

	btnRightImg = new GuiImage(imgRight);
	if (Settings.wsprompt) btnRightImg->SetWidescreen(Settings.widescreen);
	btnRight = new GuiButton(btnRightImg, btnRightImg, ALIGN_RIGHT, ALIGN_MIDDLE, -20, -50, trigA, btnSoundOver, btnSoundClick2, 1);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);

	if (Settings.ShowPlayCount)
		Append(playcntTxt);
	Append(backBtn);

	if (!dvdheader) //stuff we don't show if it is a DVD mounted
	{
		Append(btnLeft);
		Append(btnRight);
	}

	bannerFrame.SetButtonBText(tr("Start"));

	//check if unlocked
	if (Settings.godmode || !(Settings.ParentalBlocks & BLOCK_GAME_SETTINGS))
	{
		bannerFrame.SetButtonAText(tr("Settings"));
		Append(settingsBtn);
	}
	else
	{
		bannerFrame.SetButtonAText(tr("Back"));
		backBtn->SetPosition(-120, 175);
	}

	Append(startBtn); //! Appending the disc on top of all

	ChangeGame(false);
}

BannerWindow::~BannerWindow()
{
	if(parentElement)
		((GuiWindow * ) parentElement)->Remove(this);

	RemoveAll();

	delete trigA;
	delete trigB;
	delete trigL;
	delete trigR;
	delete trigPlus;
	delete trigMinus;

	delete imgLeft;
	delete imgRight;

	delete btnLeftImg;
	delete btnRightImg;

	delete playcntTxt;

	delete startBtn;
	delete backBtn;
	delete settingsBtn;
	delete btnLeft;
	delete btnRight;

	if(gameSound) gameSound->Stop();
	delete gameSound;
	bgMusic->SetVolume(Settings.volume);

	delete gameBanner;

	memcpy(&FSProjection2D, &originalProjection, sizeof(Mtx44));
	ResumeGui();
}

void BannerWindow::ChangeGame(bool playsound)
{
	struct discHdr * header = (dvdheader ? dvdheader : gameList[gameSelected]);

	//! Stop thread because all the extract functions are not thread safe
	//! Let it finish the current loading though
	BannerAsync::HaltThread();

	Banner *newBanner = NULL;
	// continue playing sound during loading process
	if((header->type == TYPE_GAME_GC_IMG) || (header->type == TYPE_GAME_GC_DISC) || (header->type == TYPE_GAME_GC_EXTRACTED))
	{
		//! try cache file first and if that fails create the default one
		if(BNRInstance::Instance()->Load(header) && BNRInstance::Instance()->Get() != NULL)
			newBanner = new Banner;
		else
			newBanner = BNRInstance::Instance()->CreateGCBanner(header);
	}
	else {
		BNRInstance::Instance()->Load(header);
		newBanner = new Banner;
	}

	//! remove game sound
	if (gameSound)
	{
		gameSound->Stop();
		delete gameSound;
		gameSound = NULL;
	}

	playcntTxt->SetTextf("%s: %i", tr( "Play Count" ), GameStatistics.GetPlayCount(header));

	HaltGui();

	// set the new banner
	delete gameBanner;
	gameBanner = newBanner;

	// Do not load stuff on game cube games
	if(BNRInstance::Instance()->Get() != NULL)
		gameBanner->LoadBanner(BNRInstance::Instance()->Get(), BNRInstance::Instance()->GetSize());

	if (Settings.gamesoundvolume != 0)
	{
		if(   (BNRInstance::Instance()->Get() != NULL)
		   &&  gameBanner->LoadSound(BNRInstance::Instance()->Get(), BNRInstance::Instance()->GetSize())
		   &&  gameBanner->getSound())
		{
			gameSound = new GuiSound(gameBanner->getSound(), gameBanner->getSoundSize(), Settings.gamesoundvolume);
		}
		else if((header->type == TYPE_GAME_GC_IMG) || (header->type == TYPE_GAME_GC_DISC) || (header->type == TYPE_GAME_GC_EXTRACTED))
		{
			//! on game cube load the default sound
			gameSound = new GuiSound(Resources::GetFile("gc_banner.ogg"), Resources::GetFileSize("gc_banner.ogg"), Settings.gamesoundvolume);
		}
		if(gameSound)
		{
			bgMusic->SetVolume(0);
			if (Settings.gamesound == 2)
				gameSound->SetLoop(1);
			// If the game is changed within window play sound here directly
			if(playsound)
				gameSound->Play();
		}
	}

	//! Resume all threads
	BannerAsync::ResumeThread();
	ResumeGui();
}

int BannerWindow::Run()
{
	int choice = -1;

	while(choice == -1)
	{
		usleep(50000);

		if (shutdown) //for power button
			Sys_Shutdown();
		else if (reset) //for reset button
			Sys_Reboot();

		choice = MainLoop();
	}

	return choice;
}

int BannerWindow::MainLoop()
{
	if (startBtn->GetState() == STATE_CLICKED)
	{
		// If this function was left then the game start was canceled
		GameWindow::BootGame(dvdheader ? dvdheader : gameList[gameSelected]);
		// If it returns from that function reload the list
		gameList.FilterList();
		startBtn->ResetState();
	}

	else if (backBtn->GetState() == STATE_CLICKED) //back
	{
		GuiBannerGrid *bannerBrowser = dynamic_cast<GuiBannerGrid *>(browserMenu->GetGameBrowser());
		if(bannerBrowser)
		{
			bannerBrowser->GetIconCoordinates(gameSelected, &AnimPosX, &AnimPosY);
			bannerBrowser->SetPage(gameSelected / 12);
		}
		// activate rendering again
		browserMenu->GetGameBrowser()->SetVisible(true);

		// finish on going animations first
		while(AnimStep < MaxAnimSteps)
			usleep(1000);

		// set new animation for zoom out
		AnimZoomIn = false;
		AnimStep = 0;

		// finish animation
		while(AnimStep < MaxAnimSteps)
			usleep(1000);

		mainWindow->SetState(STATE_DEFAULT);
		returnVal = 0;
	}

	else if(settingsBtn->GetState() == STATE_CLICKED) //settings
	{
		this->SetState(STATE_DISABLED);

		wiilight(0);
		int settret = GameSettingsMenu::Execute(browserMenu, dvdheader ? dvdheader : gameList[gameSelected]);

		this->SetState(STATE_DEFAULT);
		settingsBtn->ResetState();

		// Show the window again or return to browser on uninstall
		if (settret == MENU_DISCLIST)
			returnVal = 1;
	}

	else if (btnRight->GetState() == STATE_CLICKED) //next game
	{
		if(Settings.xflip == XFLIP_YES)
		{
			gameSelected = (gameSelected - 1 + gameList.size()) % gameList.size();
			ChangeGame(true);
		}
		else if(Settings.xflip == XFLIP_SYSMENU)
		{
			gameSelected = (gameSelected + 1) % gameList.size();
			ChangeGame(true);
		}
		else if(Settings.xflip == XFLIP_WTF)
		{
			gameSelected = (gameSelected - 1 + gameList.size()) % gameList.size();
			ChangeGame(true);
		}
		else
		{
			gameSelected = (gameSelected + 1) % gameList.size();
			ChangeGame(true);
		}

		btnRight->ResetState();
	}

	else if (btnLeft->GetState() == STATE_CLICKED) //previous game
	{
		if(Settings.xflip == XFLIP_YES)
		{
			gameSelected = (gameSelected + 1) % gameList.size();
			ChangeGame(true);
		}
		else if(Settings.xflip == XFLIP_SYSMENU)
		{
			gameSelected = (gameSelected - 1 + gameList.size()) % gameList.size();
			ChangeGame(true);
		}
		else if(Settings.xflip == XFLIP_WTF)
		{
			gameSelected = (gameSelected + 1) % gameList.size();
			ChangeGame(true);
		}
		else
		{
			gameSelected = (gameSelected - 1 + gameList.size()) % gameList.size();
			ChangeGame(true);
		}

		btnLeft->ResetState();
	}

	if (reducedVol)
	{
		if (gameSound)
		{
			if (Settings.gamesound == 1 && !gameSound->IsPlaying())
			{
				bgMusic->SetVolume(Settings.volume);
				reducedVol = false;
			}
		}
		else
		{
			bgMusic->SetVolume(Settings.volume);
			reducedVol = false;
		}
	}

	return returnVal;
}

void BannerWindow::Animate(void)
{
	// animation is on going
	if(AnimStep < MaxAnimSteps)
	{
		AnimationRunning = true;
		AnimStep++;

		// zoom in animation
		if(AnimZoomIn) {
			BGAlpha = std::min(255.f * AnimStep * 2.f / MaxAnimSteps, 255.f);
			if(AnimStep < 0.4f * MaxAnimSteps)
				BannerAlpha = 0;
			else
				BannerAlpha = std::min(255.f * (AnimStep - 0.4f * MaxAnimSteps) / (0.6f * MaxAnimSteps), 255.f);
		}
		// zoom out animation
		else {
			BGAlpha = std::min(255.f * (MaxAnimSteps-AnimStep) * 2.f / MaxAnimSteps, 255.f);
			if((MaxAnimSteps - AnimStep) < 0.4f * MaxAnimSteps)
				BannerAlpha = 0;
			else
				BannerAlpha = std::min(255.f * ((MaxAnimSteps - AnimStep) - 0.4f * MaxAnimSteps) / (0.6f * MaxAnimSteps), 255.f);
		}

		float curAnimStep = AnimZoomIn ? ((float)(MaxAnimSteps - AnimStep)/(float)MaxAnimSteps) : ((float)AnimStep/(float)MaxAnimSteps);

		float stepx1 = Settings.AdjustOverscanX - AnimPosX;
		float stepy1 = Settings.AdjustOverscanY - AnimPosY;
		float stepx2 = (screenwidth - 1 - Settings.AdjustOverscanX) - (AnimPosX + fIconWidth);
		float stepy2 = (screenheight - 1 - Settings.AdjustOverscanY) - (AnimPosY + fIconHeight);

		float top = AnimPosY + stepy1 * curAnimStep;
		float bottom = AnimPosY + fIconHeight + stepy2 * curAnimStep;
		float left = AnimPosX + stepx1 * curAnimStep;
		float right = AnimPosX + fIconWidth + stepx2 * curAnimStep;

		// set main projection of all GUI stuff if we are using the banner browser
		if(dynamic_cast<GuiBannerGrid *>(browserMenu->GetGameBrowser()) != NULL)
			guOrtho(FSProjection2D, top, bottom, left, right, 0, 10000);

		float xDiff = 0.5f * Settings.BannerProjectionWidth;
		float yDiff = 0.5f * Settings.BannerProjectionHeight;

		// this just looks better for banner/icon ratio
		float iconWidth = fIconWidth - 20;
		float iconHeight = fIconHeight - 20;

		f32 ratioX = xDiff * 2.f / iconWidth;
		f32 ratioY = yDiff * 2.f / iconHeight;
		stepx1 = ((ScreenProps.x * 0.5f - xDiff) - (AnimPosX + 0.5f * fIconWidth - 0.5f * iconWidth)) * ratioX;
		stepx2 = ((ScreenProps.x * 0.5f + xDiff) - (AnimPosX + 0.5f * fIconWidth + 0.5f * iconWidth)) * ratioX;
		stepy1 = ((ScreenProps.y * 0.5f - yDiff) - (AnimPosY + 0.5f * fIconHeight - 0.5f * iconHeight)) * ratioY;
		stepy2 = ((ScreenProps.y * 0.5f + yDiff) - (AnimPosY + 0.5f * fIconHeight + 0.5f * iconHeight)) * ratioY;

		//! This works good for banners
		top = (ScreenProps.y * 0.5f - yDiff) + stepy1 * curAnimStep;
		bottom = (ScreenProps.y * 0.5f + yDiff) + stepy2 * curAnimStep;
		left = (ScreenProps.x * 0.5f - xDiff) + stepx1 * curAnimStep;
		right = (ScreenProps.x * 0.5f + xDiff) + stepx2 * curAnimStep;

		// set banner projection
		guOrtho(projection,top, bottom, left, right,-100,10000);
	}
	// last animation step
	else if(AnimationRunning)
	{
		// set back original projection and stop animation/render of the browser (save some CPU ;P)
		memcpy(&FSProjection2D, &originalProjection, sizeof(Mtx44));
		browserMenu->GetGameBrowser()->SetVisible(false);
		AnimationRunning = false;
	}
}

void BannerWindow::Draw(void)
{
	bool btnAGrow = (settingsBtn->GetState() == STATE_SELECTED || backBtn->GetState() == STATE_SELECTED);
	bannerFrame.SetButtonAGrow(btnAGrow);
	bannerFrame.SetButtonBGrow(startBtn->GetState() == STATE_SELECTED);

	//! Start playing banner sound after last animation frame if animation after zoom is enabled
	//! or on first frame if during zoom is enable
	if( AnimZoomIn && gameSound && (((Settings.BannerAnimStart == BANNER_START_ON_ZOOM) && AnimStep == 0)
	   || ((Settings.BannerAnimStart == BANNER_START_AFTER_ZOOM) && ((AnimStep + 1) == MaxAnimSteps))))
	{
		reducedVol = true;
		gameSound->Play();
	}

	// Run window animation
	Animate();

	// draw a black background image first
	Menu_DrawRectangle(0.0f, 0.0f, ScreenProps.x, ScreenProps.y, (GXColor) {0, 0, 0, BGAlpha}, true);

	// no banner alpha means its the start of the animation
	if(BannerAlpha == 0)
		return;

	// cut the unneeded crap
	Mtx mv1, mv2, mv3;
	guMtxIdentity (mv2);
	guMtxIdentity (mv3);
	guMtxScaleApply(modelview,mv1, 1.f, -1.f, 1.f);
	guMtxTransApply(mv1,mv1, 0.5f * ScreenProps.x, 0.5f * ScreenProps.y, 0.f);
	guMtxTransApply(mv2,mv2, -0.5f * fBannerWidth, 0.5f * fBannerHeight, 0.f);
	guMtxTransApply(mv3,mv3, 0.5f * fBannerWidth, -0.5f * fBannerHeight, 0.f);
	guMtxConcat (mv1, mv2, mv2);
	guMtxConcat (mv1, mv3, mv3);

	f32 viewportv[6];
	f32 projectionv[7];

	GX_GetViewportv(viewportv, vmode);
	GX_GetProjectionv(projectionv, projection, GX_ORTHOGRAPHIC);

	guVector vecTL;
	guVector vecBR;
	GX_Project(0.0f, 0.0f, 0.0f, mv2, projectionv, viewportv, &vecTL.x, &vecTL.y, &vecTL.z);
	GX_Project(0.0f, 0.0f, 0.0f, mv3, projectionv, viewportv, &vecBR.x, &vecBR.y, &vecBR.z);

	// round up scissor box offset and round down the size
	u32 scissorX = (u32)(0.5f + std::max(vecTL.x, 0.0f));
	u32 scissorY = (u32)(0.5f + std::max(vecTL.y, 0.0f));
	u32 scissorW = (u32)std::max(vecBR.x - vecTL.x, 0.0f);
	u32 scissorH = (u32)std::max(vecBR.y - vecTL.y, 0.0f);

	GX_SetScissor( scissorX, scissorY, scissorW, scissorH );

	// load projection matrix
	GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

	if(gameBanner->getBanner())
	{
		gameBanner->getBanner()->Render(modelview, ScreenProps, Settings.widescreen, BannerAlpha);

		// advance only when animation isnt running on certain options
		if(Settings.BannerAnimStart != BANNER_START_AFTER_ZOOM || !AnimationRunning)
		{
			gameBanner->getBanner()->AdvanceFrame();

			// skip every 6th frame on PAL50 since all banners are 60 Hz
			if(Settings.PAL50 && (frameCount % 6 == 0)) {
				gameBanner->getBanner()->AdvanceFrame();
			}
		}
	}

	// render big frame and animate button over effects
	bannerFrame.Render(modelview, ScreenProps, Settings.widescreen, BannerAlpha);
	bannerFrame.AdvanceFrame();

	// Setup GX
	ReSetup_GX();

	if(AnimationRunning) {
		// remove scissors again as we draw the background layout too
		GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);

		// only render gui stuff when animation is done
		return;
	}

	GuiWindow::Draw();
}
