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
#include "gui_cross.hpp"

void GuiCross::Draw()
{
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	f32 x1 = GetLeft();
	f32 x2 = x1 + width;
	f32 y1 = GetTop();
	f32 y2 = y1 + height;

	int alpha = GetAlpha();

	GX_Begin(GX_LINES, GX_VTXFMT0, 4);
	GX_Position3f32(x1, y1, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_Position3f32(x2, y2, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_Position3f32(x2, y1, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_Position3f32(x1, y2, 0.0f);
	GX_Color4u8(color.r, color.g, color.b, alpha);
	GX_End();
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}
