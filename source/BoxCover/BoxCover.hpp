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
#ifndef BOX_COVER_HPP_
#define BOX_COVER_HPP_

#include "GUI/gui.h"

#define EFFECT_BOX_FLY_CENTRE   0x2000000
#define EFFECT_BOX_FLY_BACK	 0x4000000
#define EFFECT_BOX_ROTATE_X	 0x8000000

class BoxCover : public GuiImage
{
	public:
		BoxCover(GuiImageData * img, bool flat = false);
		virtual ~BoxCover();
		//! Colors:
		//! Gray Box (Default): r:233 g:233 b:233
		//! Red Box (NSMB): r:198 g:34 b:4
		void SetBoxColor(GXColor c) { LOCK(this); boxColor = c; };
		void SetPosition(f32 x, f32 y, f32 z) { LOCK(this); PosXOrig = PosX = x; PosYOrig = PosY = y; PosZOrig = PosZ = z; };
		void SetEffect(int eff, int amount, int target = 0);
		void SetImage(GuiImageData *img); //forbid this call
		void SetZoomable(bool z) { LOCK(this); Zoomable = z; };
		void Draw();
		void Update(GuiTrigger * t);
		void UpdateEffects();
	private:
		void WiiPADControl(GuiTrigger *t);

		f32 RotX;
		f32 RotY;
		f32 RotZ;
		f32 PosX;
		f32 PosY;
		f32 PosZ;
		f32 PosXOrig;
		f32 PosYOrig;
		f32 PosZOrig;
		f32 AnimRotate;
		f32 Animation;
		f32 Animation2;
		u32 last_manual_move_frame;
		int moveStartPosX;
		int moveStartPosY;
		f32 movePosX;
		f32 movePosY;
		int moveChan;
		bool flatCover;
		bool Zoomable;
		Mtx44 projection;
		GuiImageData boxBorder;
		GuiImageData *defaultBox;
		Mtx	view;
		GXTexObj coverTex;
		GXTexObj boxBorderTex;
		GXTexObj defaultBoxTex;
		GXColor boxColor;
};

#endif
