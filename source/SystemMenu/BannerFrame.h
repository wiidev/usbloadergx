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
#ifndef BANNERFRAME_H
#define BANNERFRAME_H

#include "banner/Layout.h"

// this is a class to deal with the frame the goes over the large banners
class BannerFrame : public Layout
{
public:
	BannerFrame()
		: Loaded(false),
		  GrowEffectBtnA(false), GrowEffectBtnB(false),
		  ScaleBtnA(1.f), ScaleBtnB(1.f),
		  N_BtnA(0), N_BtnB(0),
		  UTF16ButtonA(0), UTF16ButtonB(0)
	{ }
	virtual ~BannerFrame();
	bool Load( const U8Archive &archive );
	bool IsLoaded() const { return Loaded; }
	void AdvanceFrame();
	void SetButtonAGrow(bool b) { GrowEffectBtnA = b; }
	void SetButtonBGrow(bool b) { GrowEffectBtnB = b; }
	void SetButtonAText(const char *text);
	void SetButtonBText(const char *text);
private:
	bool Loaded;
	bool GrowEffectBtnA;
	bool GrowEffectBtnB;
	float ScaleBtnA;
	float ScaleBtnB;
	Pane *N_BtnA;
	Pane *N_BtnB;
	u16 *UTF16ButtonA;
	u16 *UTF16ButtonB;
};

#endif // BANNERFRAME_H
