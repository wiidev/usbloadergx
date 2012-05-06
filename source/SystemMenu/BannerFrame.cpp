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
#include "BannerFrame.h"
#include "SystemMenu/SystemMenuResources.h"
#include "utils/U8Archive.h"
#include "utils/StringTools.h"

BannerFrame::~BannerFrame()
{
	delete [] UTF16ButtonA;
	delete [] UTF16ButtonB;
}

bool BannerFrame::Load( const U8Archive &chanTtlArc )
{
	// read layout data
	u8 *brlytFile = chanTtlArc.GetFile( "/arc/blyt/my_ChTop_a.brlyt" );
	if( !brlytFile )
		return false;

	if(!Layout::Load(brlytFile))
		return false;

	// load textures
	LoadTextures(chanTtlArc);

	N_BtnA = FindPane("N_BtnA");
	N_BtnB = FindPane("N_BtnB");

	Loaded = true;

	return true;
}

void BannerFrame::SetButtonAText(const char *text)
{
	//!< Set button text for left button
	const wchar_t *buttonText = wfmt("%s", text);
	int len = wcslen(buttonText);

	delete [] UTF16ButtonA;
	UTF16ButtonA = new u16[len+1];

	for(int i = 0; i < len; i++)
		UTF16ButtonA[i] = (u16) buttonText[i];
	UTF16ButtonA[len] = 0;

	Textbox *T_BtnA = (Textbox *)FindPane("T_BtnA");
	if(T_BtnA)
		T_BtnA->SetText(UTF16ButtonA);
}

void BannerFrame::SetButtonBText(const char *text)
{
	//!< Set button text for right button
	const wchar_t *buttonText = wfmt("%s", text);
	int len = wcslen(buttonText);

	delete [] UTF16ButtonB;
	UTF16ButtonB = new u16[len+1];

	for(int i = 0; i < len; i++)
		UTF16ButtonB[i] = (u16) buttonText[i];
	UTF16ButtonB[len] = 0;

	Textbox *T_BtnB = (Textbox *)FindPane("T_BtnB");
	if(T_BtnB)
		T_BtnB->SetText(UTF16ButtonB);
}


void BannerFrame::AdvanceFrame()
{
	if(!N_BtnA || !N_BtnB)
		return;

	if(GrowEffectBtnA && ScaleBtnA < 1.05f)
	{
		ScaleBtnA += 0.01f;
		N_BtnA->SetScale(ScaleBtnA);
	}
	else if(!GrowEffectBtnA && ScaleBtnA > 1.0f)
	{
		ScaleBtnA -= 0.01f;
		N_BtnA->SetScale(ScaleBtnA);
	}

	if(GrowEffectBtnB && ScaleBtnB < 1.05f)
	{
		ScaleBtnB += 0.01f;
		N_BtnB->SetScale(ScaleBtnB);
	}
	else if(!GrowEffectBtnB && ScaleBtnB > 1.0f)
	{
		ScaleBtnB -= 0.01f;
		N_BtnB->SetScale(ScaleBtnB);
	}

	// we don't load any brlan for it
	// Layout::AdvanceFrame();
}
