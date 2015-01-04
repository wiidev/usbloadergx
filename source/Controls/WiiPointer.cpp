/****************************************************************************
 * Copyright (C) 2011 Dimok
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
#include "WiiPointer.h"
#include "settings/CSettings.h"
#include "themes/Resources.h"
#include "utils/tools.h"
#include "video.h"
#include "input.h"


Mtx44 WiiPointer::projection;

WiiPointer::WiiPointer(const char *pntrImg)
	: posX(screenwidth/2), posY(screenheight/2),
	  angle(0.0f), lastActivity(301)
{
	pointerImg = Resources::GetImageData(pntrImg);

	//! create projection matrix
	guOrtho(projection, Settings.AdjustOverscanY, screenheight - 1 - Settings.AdjustOverscanY,
						Settings.AdjustOverscanX, screenwidth  - 1 - Settings.AdjustOverscanX, 0, 10000);
}

WiiPointer::~WiiPointer()
{
	delete pointerImg;
}

void WiiPointer::SetImage(const char *pntrImg)
{
	GuiImageData * newPointer = Resources::GetImageData(pntrImg);
	if(!newPointer)
		return;

	// let's save us the work with mutex here
	GuiImageData * temp2 = pointerImg;
	pointerImg = newPointer;
	delete temp2;
}

void WiiPointer::Draw(GuiTrigger *t)
{
	if(t && pointerImg)
	{
		if(t->wpad.ir.valid)
		{
			lastActivity = 0;
			posX = t->wpad.ir.x;
			posY = t->wpad.ir.y;
			angle = t->wpad.ir.angle;
		}
		else
		{
			angle = 0.0f;
			// GC PAD
			// x-axis
			if(t->pad.stickX < -PADCAL)
			{
				posX += (t->pad.stickX + PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}
			else if(t->pad.stickX > PADCAL)
			{
				posX += (t->pad.stickX - PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}
			// y-axis
			if(t->pad.stickY < -PADCAL)
			{
				posY -= (t->pad.stickY + PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}
			else if(t->pad.stickY > PADCAL)
			{
				posY -= (t->pad.stickY - PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}

			//Wii u pro x-axis
			if(t->wupcdata.stickX < -WUPCCAL)
			{
				posX += (t->wupcdata.stickX + WUPCCAL) * Settings.PointerSpeed/8;
				lastActivity = 0;
			}
			else if(t->wupcdata.stickX > WUPCCAL)
			{
				posX += (t->wupcdata.stickX - WUPCCAL) * Settings.PointerSpeed/8;
				lastActivity = 0;
			}
			//Wii u pro y-axis
			if(t->wupcdata.stickY < -WUPCCAL)
			{
				posY -= (t->wupcdata.stickY + WUPCCAL) * Settings.PointerSpeed/8;
				lastActivity = 0;
			}
			else if(t->wupcdata.stickY > WUPCCAL)
			{
				posY -= (t->wupcdata.stickY - WUPCCAL) * Settings.PointerSpeed/8;
				lastActivity = 0;
			}

			int wpadX = t->WPAD_Stick(0, 0);
			int wpadY = t->WPAD_Stick(0, 1);

			// Wii Extensions
			// x-axis
			if(wpadX < -PADCAL)
			{
				posX += (wpadX + PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}
			else if(wpadX > PADCAL)
			{
				posX += (wpadX - PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}
			// y-axis
			if(wpadY < -PADCAL)
			{
				posY -= (wpadY + PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}
			else if(wpadY > PADCAL)
			{
				posY -= (wpadY - PADCAL) * Settings.PointerSpeed;
				lastActivity = 0;
			}

			if(t->pad.btns_h || t->wpad.btns_h || t->wupcdata.btns_h)
				lastActivity = 0;

			posX = LIMIT(posX, -50.0f, screenwidth+50.0f);
			posY = LIMIT(posY, -50.0f, screenheight+50.0f);

			if(lastActivity < 180) { // (3s on 60Hz and 3.6s on 50Hz)
				t->wpad.ir.valid = 1;
				t->wpad.ir.x = posX;
				t->wpad.ir.y = posY;
			}
		}

		if(t->wpad.ir.valid)
		{
			GXTexObj texObj;
			GX_InitTexObj(&texObj, pointerImg->GetImage(), pointerImg->GetWidth(), pointerImg->GetHeight(), GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
			GX_LoadTexObj(&texObj, GX_TEXMAP0);

			GX_ClearVtxDesc();
			GX_InvVtxCache();
			GX_InvalidateTexAll();

			GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
			GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
			GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

			Mtx mv;
			guMtxIdentity(mv);
			guMtxRotDeg (mv, 'z', angle);
			guMtxTransApply(mv, mv, posX, posY, 9900.f);
			guMtxConcat(FSModelView2D, mv, mv);

			GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);
			GX_LoadPosMtxImm(mv, GX_PNMTX0);

			// pointer is pointing to center of the texture
			f32 width = 0.5f * pointerImg->GetWidth();
			f32 height = 0.5f * pointerImg->GetHeight();

			GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position3f32(-width, -height, 0);
			GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
			GX_TexCoord2f32(0, 0);

			GX_Position3f32(width, -height, 0);
			GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
			GX_TexCoord2f32(1, 0);

			GX_Position3f32(width, height, 0);
			GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
			GX_TexCoord2f32(1, 1);

			GX_Position3f32(-width, height, 0);
			GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
			GX_TexCoord2f32(0, 1);

			GX_End();
		}
	}

	++lastActivity;
}
