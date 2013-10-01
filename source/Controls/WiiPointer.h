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
#ifndef WIIPOINTER_H_
#define WIIPOINTER_H_

#include "GUI/gui.h"
#include "utils/timer.h"

class WiiPointer
{
public:
	WiiPointer(const char *pntrImg);
	virtual ~WiiPointer();
	void SetImage(const char *pntrImg);
	void SetPosition(float x, float y, float a) {posX = x; posY = y; angle = a;}
	void Draw(GuiTrigger *t);
	u32 getLastActivCounter(void) { return lastActivity; }
private:
	float posX, posY, angle;
	u32 lastActivity;
	GuiImageData * pointerImg;
	static Mtx44 projection;
};

#endif /* WIIPOINTER_H_ */
