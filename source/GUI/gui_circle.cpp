/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "gui_circle.hpp"

GuiCircle::GuiCircle()
	: radius(100.0f), filled(true), accuracy(36)
{

	color = (GXColor) {0, 0, 0, 255};
	SetLinewidth(1.0f);
}

GuiCircle::GuiCircle(float r)
	: radius(r), filled(true), accuracy(36)
{
	color = (GXColor) {0, 0, 0, 255};
	SetLinewidth(1.0f);
}

void GuiCircle::SetLinewidth(float s)
{
	LOCK(this);
	Linewidth = s;
	GX_SetLineWidth((u8) (s*6.0f), 0);
}

void GuiCircle::Draw()
{
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	int loopAmount = filled ? accuracy : accuracy+1;

	GX_Begin(filled ? GX_TRIANGLEFAN : GX_LINESTRIP, GX_VTXFMT0, loopAmount);
	for(int i = 0; i < loopAmount; ++i)
	{
		f32 rad = (f32) i / (f32) accuracy * 360.0f;
		f32 x = cos(DegToRad(rad)) * radius + GetLeft();
		f32 y = sin(DegToRad(rad)) * radius + GetTop();

		GX_Position3f32(x, y, 0.0f);
		GX_Color4u8(color.r, color.g, color.b, color.a);
	}
	GX_End();
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}
