/****************************************************************************
 * Copyright (C) 2011
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
#include "gui_checksign.hpp"

void GuiChecksign::Draw()
{
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	f32 x1Line1 = (float) GetLeft() + width*0.1f;
	f32 y1Line1 = (float) GetTop() + height*0.65f;
	f32 x2Line1 = GetLeft() + width*0.3f;
	f32 y2Line1 = (float) GetTop() + (float) height - height*0.1f;

	f32 x1Line2 = x2Line1;
	f32 y1Line2 = y2Line1;
	f32 x2Line2 = (float) GetLeft() + (float) width - width*0.1f;
	f32 y2Line2 = (float) GetTop() + height*0.1f;

	int alpha = GetAlpha();

	GX_Begin(GX_LINES, GX_VTXFMT0, 4);
	GX_Position3f32(x1Line1, y1Line1, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_Position3f32(x2Line1, y2Line1, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_Position3f32(x1Line2, y1Line2, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_Position3f32(x2Line2, y2Line2, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_End();
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}
