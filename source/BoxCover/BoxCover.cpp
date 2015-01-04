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
#include "BoxCover.hpp"
#include "BoxMesh.hpp"
#include "settings/CSettings.h"
#include "themes/CTheme.h"
#include "menu.h"

BoxCover::BoxCover(GuiImageData * img, bool flat)
	:   GuiImage(img),
		boxBorder(Resources::GetFile("boxBorder.png"), Resources::GetFileSize("boxBorder.png")),
		defaultBox(NULL)
{
	flatCover = flat;
	Zoomable = false;
	moveChan = -1;
	moveStartPosX = 0;
	moveStartPosY = 0;
	movePosX = 0.0f;
	movePosY = 0.0f;
	RotX = 0.0f;
	RotY = 0.0f;
	RotZ = 0.0f;
	PosX = 0.0f;
	PosY = 0.0f;
	PosZ = -27.f;
	AnimRotate = 0.0f;
	last_manual_move_frame = 0;
	guVector camera = (guVector) {0.0F, 0.0F, 0.0F};
	guVector up = (guVector) {0.0F, 1.0F, 0.0F};
	guVector look = (guVector) {0.0F, 0.0F, -1.0F};
	boxColor = (GXColor) {233, 233, 233, 255};

	guLookAt(view, &camera,	&up, &look);
	guPerspective(projection, 8, 640.f/480.f, 1.0f, 300.0F);

	if(flatCover || !image)
	{
		defaultBox = Resources::GetImageData("nocoverFull.png");
		GX_InitTexObj(&defaultBoxTex, defaultBox->GetImage(), defaultBox->GetWidth(), defaultBox->GetHeight(), defaultBox->GetTextureFormat(),GX_CLAMP, GX_CLAMP,GX_FALSE);
	}

	if(!image)
	{
		GX_InitTexObj(&coverTex, defaultBox->GetImage(), defaultBox->GetWidth(), defaultBox->GetHeight(), defaultBox->GetTextureFormat(),GX_CLAMP, GX_CLAMP,GX_FALSE);
		flatCover = false;
	}
	else
		GX_InitTexObj(&coverTex, image, width,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);

	GX_InitTexObj(&boxBorderTex, boxBorder.GetImage(), boxBorder.GetWidth(), boxBorder.GetHeight(), boxBorder.GetTextureFormat(),GX_CLAMP, GX_CLAMP,GX_FALSE);
}

BoxCover::~BoxCover()
{
	delete defaultBox;

	for(int i = 0; i < 4; ++i)
	{
		char name[50];
		snprintf(name, sizeof(name), "player%i_point.png", i+1);
		pointer[i]->SetImage(name);
	}
}

//! Remove me later
void BoxCover::WiiPADControl(GuiTrigger *t)
{
	if((t->wpad.btns_d & WPAD_BUTTON_A) || (t->wpad.btns_h & WPAD_CLASSIC_BUTTON_A) || (t->wupcdata.btns_h & WPAD_CLASSIC_BUTTON_A) || (t->pad.btns_h & PAD_BUTTON_A))
	{
		if(t->wpad.ir.valid)
		{
			moveChan = t->chan;
			moveStartPosX = t->wpad.ir.x;
			moveStartPosY = t->wpad.ir.y;
			PosX += movePosX;
			PosY += movePosY;
			movePosX = 0.0f;
			movePosY = 0.0f;
			
			// GameCube and Classic Controller
			s8 movX = fabs(t->pad.stickX) > 10.0f ? t->pad.stickX : t->WPAD_Stick(0, 0);
			s8 movY = fabs(t->pad.stickY) > 10.0f ? t->pad.stickY : t->WPAD_Stick(0, 1);
			
			// WiiU Pro Classic Controller
			// Todo : Proper stick calibration required to allow moving cover
			
			//! Drop stick moves of less than 10 because of sensitivity
			if(fabs(movX) < 10.0f) movX = 0;
			if(fabs(movY) < 10.0f) movY = 0;
			if(movX < -PADCAL)
				PosX += (movX + PADCAL)  * Settings.PointerSpeed * fabs(PosZ)/3400.f;
			if(movX > PADCAL)
				PosX += (movX - PADCAL)  * Settings.PointerSpeed * fabs(PosZ)/3400.f;
			if(movY < -PADCAL)
				PosY += (movY + PADCAL)  * Settings.PointerSpeed * fabs(PosZ)/3400.f;
			if(movY > PADCAL)
				PosY += (movY - PADCAL)  * Settings.PointerSpeed * fabs(PosZ)/3400.f;
			
			if(moveChan >= 0 && moveChan < 4)
			{
				char name[50];
				snprintf(name, sizeof(name), "player%i_grab.png", moveChan+1);
				pointer[moveChan]->SetImage(name);
			}
		}
		else
			moveChan = -1;
	}
	else if(((t->wpad.btns_h & WPAD_BUTTON_A) || (t->wpad.btns_h & WPAD_CLASSIC_BUTTON_A) || (t->wupcdata.btns_h & WPAD_CLASSIC_BUTTON_A) || (t->pad.btns_h & PAD_BUTTON_A)) && moveChan == t->chan && t->wpad.ir.valid && !effects)
	{
		movePosX = (t->wpad.ir.x-moveStartPosX) * fabs(PosZ)/3400.f;
		movePosY = (moveStartPosY-t->wpad.ir.y) * fabs(PosZ)/3400.f;
		last_manual_move_frame = frameCount;
	}
	else if(!(t->wpad.btns_h & WPAD_BUTTON_A) && !(t->wpad.btns_h & WPAD_CLASSIC_BUTTON_A) && !(t->wupcdata.btns_h & WPAD_CLASSIC_BUTTON_A) && !(t->pad.btns_h & PAD_BUTTON_A) && moveChan == t->chan)
	{
		if(moveChan >= 0 && moveChan < 4)
		{
			char name[50];
			snprintf(name, sizeof(name), "player%i_point.png", moveChan+1);
			pointer[moveChan]->SetImage(name);
		}
	}

	if((t->wpad.btns_h & WPAD_BUTTON_UP) || (t->pad.substickY > PADCAL) || (t->wupcdata.substickY > WUPCCAL))
	{
		RotX -= 2.0f;
		last_manual_move_frame = frameCount;
	}
	if((t->wpad.btns_h & WPAD_BUTTON_DOWN) || (t->pad.substickY < -PADCAL) || (t->wupcdata.substickY < -WUPCCAL))
	{
		RotX += 2.0f;
		last_manual_move_frame = frameCount;
	}
	if((t->wpad.btns_h & WPAD_BUTTON_LEFT) || (t->pad.substickX < -PADCAL) || (t->wupcdata.substickX < -WUPCCAL))
	{
		RotY -= 2.0f;
		last_manual_move_frame = frameCount;
	}
	if((t->wpad.btns_h & WPAD_BUTTON_RIGHT) || (t->pad.substickX > PADCAL) || (t->wupcdata.substickX > WUPCCAL))
	{
		RotY += 2.0f;
		last_manual_move_frame = frameCount;
	}
	if((t->wpad.btns_d & WPAD_BUTTON_2) || (t->pad.btns_d & PAD_BUTTON_X) || (t->wpad.btns_d & WPAD_CLASSIC_BUTTON_X) || (t->wupcdata.btns_d & WPAD_CLASSIC_BUTTON_X))
	{
		if(RotY < 180.0f)
			SetEffect(EFFECT_BOX_ROTATE_X, 10, 180);
		else
			SetEffect(EFFECT_BOX_ROTATE_X, -10, -180);
		last_manual_move_frame = frameCount;
	}
	if((t->wpad.btns_h & WPAD_BUTTON_PLUS) || (t->pad.btns_h & PAD_TRIGGER_R) || (t->wpad.btns_h & WPAD_CLASSIC_BUTTON_FULL_R) || (t->wupcdata.btns_h & WPAD_CLASSIC_BUTTON_FULL_R))
	{
		if(PosZ < -2.8f)
			PosZ += 0.4f*fabs(PosZ)/19.f;
	}
	if((t->wpad.btns_h & WPAD_BUTTON_MINUS) || (t->pad.btns_h & PAD_TRIGGER_L) || (t->wpad.btns_h & WPAD_CLASSIC_BUTTON_FULL_L) || (t->wupcdata.btns_h & WPAD_CLASSIC_BUTTON_FULL_L))
	{
		if(PosZ > -43.0f)
			PosZ -= 0.4f*fabs(PosZ)/19.f;
	}
}

void BoxCover::Update(GuiTrigger * t)
{
	s8 movY = t->WPAD_Stick((t->wpad.exp.type == WPAD_EXP_CLASSIC), 0);
	s8 movX = t->WPAD_Stick((t->wpad.exp.type == WPAD_EXP_CLASSIC), 1);
	//! Drop stick moves of less than 10 because of sensitivity
	if(fabs(movY) < 10.0f) movY = 0;
	if(fabs(movX) < 10.0f) movX = 0;

	if(movY != 0 || movX != 0)
		last_manual_move_frame = frameCount;

	RotY += (f32) movY / 50.0f;
	RotX -= (f32) movX / 50.0f;

	if(Zoomable)
		WiiPADControl(t);

	//! Stop movement for about 5 sec after manual move
	if(frameCount-last_manual_move_frame < 250)
		return;

	Animation = sinf(DegToRad(AnimRotate))*2.0f;
	Animation2 = cosf(DegToRad(AnimRotate))*5.0f;
	AnimRotate += 0.1f;
	if(AnimRotate > 360.0f)
		AnimRotate = 0.0f;
}

void BoxCover::Draw()
{
	u8 BoxAlpha = (int) (alpha+alphaDyn) & 0xFF;

	GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);

	//! don't draw inside of the box
	GX_SetCullMode(GX_CULL_FRONT);

	Mtx	modelView;
	Mtx	modelView2;
	Mtx	modelView3;

	guVector cubeAxis = {0,0,1};
	guVector cubeAxis2 = {0,1,0};
	guVector cubeAxis3 = {1,0,0};
	guMtxIdentity(modelView);
	guMtxRotAxisDeg(modelView3, &cubeAxis3, RotX-Animation2);
	guMtxRotAxisDeg(modelView2, &cubeAxis2, RotY+Animation2+xoffsetDyn/2.0f);
	guMtxRotAxisDeg(modelView, &cubeAxis, RotZ-Animation);
	guMtxConcat(modelView3, modelView2, modelView2);
	guMtxConcat(modelView2, modelView, modelView);
	if(Settings.widescreen)
		guMtxScaleApply(modelView, modelView, Settings.WSFactor, 1.0f, 1.0f);
	guMtxTransApply(modelView, modelView, PosX+xoffsetDyn/680.0f+movePosX, PosY+yoffsetDyn/680.0f+movePosY, PosZ);
	guMtxConcat(view,modelView,modelView);

	GX_LoadPosMtxImm(modelView,	GX_PNMTX0);

	//! Border quads
	GX_LoadTexObj(&boxBorderTex, GX_TEXMAP0);
	GX_InvalidateTexAll();

	GX_SetArray(GX_VA_POS, (void *) &g_boxMeshQ[0].pos, sizeof(g_boxMeshQ[0]));
	GX_SetArray(GX_VA_TEX0, (void *) &g_boxMeshQ[0].texCoord, sizeof(g_boxMeshQ[0]));

	GX_Begin(GX_QUADS, GX_VTXFMT0, g_boxMeshQSize);
	for (u32 j = 0; j < g_boxMeshQSize; ++j)
	{
		GX_Position1x8(j);
		GX_Color4u8(boxColor.r, boxColor.g, boxColor.b, BoxAlpha);
		GX_TexCoord1x8(j);
	}
	GX_End();

	//! Border triangles
	GX_SetArray(GX_VA_POS, (void *) &g_boxMeshT[0].pos, sizeof(g_boxMeshT[0]));
	GX_SetArray(GX_VA_TEX0, (void *) &g_boxMeshT[0].texCoord, sizeof(g_boxMeshT[0]));

	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, g_boxMeshTSize);
	for (u32 j = 0; j < g_boxMeshTSize; ++j)
	{
		GX_Position1x8(j);
		GX_Color4u8(boxColor.r, boxColor.g, boxColor.b, BoxAlpha);
		GX_TexCoord1x8(j);
	}
	GX_End();

	//! Back Cover (Might be flat)
	GX_LoadTexObj(flatCover ? &defaultBoxTex : &coverTex, GX_TEXMAP0);
	GX_InvalidateTexAll();

	GX_SetArray(GX_VA_POS, (void *) &g_boxBackCoverMesh[0].pos, sizeof(g_boxBackCoverMesh[0]));
	GX_SetArray(GX_VA_TEX0, (void *) &g_boxBackCoverMesh[0].texCoord, sizeof(g_boxBackCoverMesh[0]));

	GX_Begin(GX_QUADS, GX_VTXFMT0, g_boxBackCoverMeshSize);
	for (u32 j = 0; j < g_boxBackCoverMeshSize; ++j)
	{
		GX_Position1x8(j);
		if(flatCover)
			GX_Color4u8(boxColor.r, boxColor.g, boxColor.b, BoxAlpha);
		else
			GX_Color4u8(0xff, 0xff, 0xff, BoxAlpha);
		GX_TexCoord1x8(j);
	}
	GX_End();

	if(flatCover)
	{
		//! Front Flat Cover
		GX_LoadTexObj(&coverTex, GX_TEXMAP0);
		GX_InvalidateTexAll();

		GX_SetArray(GX_VA_POS, (void *) &g_flatCoverMesh[0].pos, sizeof(g_flatCoverMesh[0]));
		GX_SetArray(GX_VA_TEX0, (void *) &g_flatCoverMesh[0].texCoord, sizeof(g_flatCoverMesh[0]));

		GX_Begin(GX_QUADS, GX_VTXFMT0, g_flatCoverMeshSize);
		for (u32 j = 0; j < g_flatCoverMeshSize; ++j)
		{
			GX_Position1x8(j);
			GX_Color4u8(0xff, 0xff, 0xff, 0xff);
			GX_TexCoord1x8(j);
		}
		GX_End();
	}
	else
	{
		//! Front Cover
		GX_SetArray(GX_VA_POS, (void *) &g_boxCoverMesh[0].pos, sizeof(g_boxCoverMesh[0]));
		GX_SetArray(GX_VA_TEX0, (void *) &g_boxCoverMesh[0].texCoord, sizeof(g_boxCoverMesh[0]));

		GX_Begin(GX_QUADS, GX_VTXFMT0, g_boxCoverMeshSize);
		for (u32 j = 0; j < g_boxCoverMeshSize; ++j)
		{
			GX_Position1x8(j);
			GX_Color4u8(0xff, 0xff, 0xff, BoxAlpha);
			GX_TexCoord1x8(j);
		}
		GX_End();
	}

	//! stop cull
	GX_SetCullMode(GX_CULL_NONE);

	UpdateEffects();
}

void BoxCover::SetEffect(int eff, int amount, int target)
{
	GuiImage::SetEffect(eff, amount, target);
}

void BoxCover::UpdateEffects()
{
	GuiImage::UpdateEffects();

	if(effects & EFFECT_BOX_FLY_CENTRE)
	{
		if(PosX > 0.1f)
			PosX -= effectAmount/1200.f;
		if(PosY > 0.1f)
			PosY -= effectAmount/1200.f;
		if(PosX < -0.1f)
			PosX += effectAmount/1200.f;
		if(PosY < -0.1f)
			PosY += effectAmount/1200.f;

		movePosX = 0.0f;
		movePosY = 0.0f;
		PosZ += 0.4f;
		RotY += effectAmount/4.9f;

		if(fabs(PosX) < 0.1f && fabs(PosY) < 0.1f)
		{
			PosX = 0.0f;
			PosY = 0.0f;
			effects = 0;
			effectAmount = 0;
		}
	}
	else if(effects & EFFECT_BOX_FLY_BACK)
	{
		if(PosX > PosXOrig+0.1f)
			PosX -= effectAmount/1200.f;
		if(PosY > PosYOrig+0.1f)
			PosY -= effectAmount/1200.f;
		if(PosX < PosXOrig-0.1f)
			PosX += effectAmount/1200.f;
		if(PosY < PosYOrig-0.1f)
			PosY += effectAmount/1200.f;

		PosZ -= 0.4f;
		RotY -= effectAmount/4.9f;

		if(movePosX > 0.1f)
			movePosX -= 0.1f;
		else if(movePosX < 0.1f)
			movePosX += 0.1f;
		if(movePosY > 0.1f)
			movePosY -= 0.1f;
		else if(movePosY < 0.1f)
			movePosY += 0.1f;

		if(fabs(PosXOrig-PosX) < 0.1f && fabs(PosYOrig-PosY) < 0.1f)
		{
			movePosX = 0.0f;
			movePosY = 0.0f;
			PosX = PosXOrig;
			PosY = PosYOrig;
			PosZ = PosZOrig;
			effects = 0;
			effectAmount = 0;
		}
	}
	else if(effects & EFFECT_BOX_ROTATE_X)
	{
		RotY += effectAmount;
		effectTarget -= effectAmount;

		if(fabs(effectTarget) < fabs(effectAmount))
		{
			effects = 0;
			effectAmount = 0;
			effectTarget = 0;
		}
	}
}
