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

WiiPointer::WiiPointer(const char *pntrImg)
	: posX(screenwidth/2), posY(screenheight/2),
	  angle(0.0f), lastActivity(301)
{
	pointerImg = Resources::GetImageData(pntrImg);
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

			if(t->pad.btns_h || t->wpad.btns_h)
				lastActivity = 0;

			posX = LIMIT(posX, -50.0f, screenwidth+50.0f);
			posY = LIMIT(posY, -50.0f, screenheight+50.0f);

			if(lastActivity < 300) { // (5s on 60Hz and 6s on 50Hz)
				t->wpad.ir.valid = 1;
				t->wpad.ir.x = posX;
				t->wpad.ir.y = posY;
			}
		}

		if(t->wpad.ir.valid)
			Menu_DrawImg(posX - pointerImg->GetWidth()/2,
						 posY - pointerImg->GetHeight()/2,
						 9900.0f, pointerImg->GetWidth(), pointerImg->GetHeight(),
						 pointerImg->GetImage(), angle,
						 Settings.widescreen ? Settings.WSFactor : 1.f, 1.f, 255, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	++lastActivity;
}
