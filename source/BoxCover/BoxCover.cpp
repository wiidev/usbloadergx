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
#include "themes/CTheme.h"

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
    PosZ = -4.5f;
    BoxScale = 1.0f;
    AnimRotate = 0.0f;
    last_manual_move_frame = 0;
    camera = (guVector) {0.0F, 0.0F, 0.0F};
    up = (guVector) {0.0F, 1.0F, 0.0F};
    look = (guVector) {0.0F, 0.0F, -1.0F};
    boxColor = (GXColor) {233, 233, 233, 255};

    guLookAt(view, &camera,	&up, &look);

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
}

void BoxCover::WiiPADControl(GuiTrigger *t)
{
    if(t->wpad.btns_d & WPAD_BUTTON_A)
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
        }
        else
            moveChan = -1;
    }
    else if((t->wpad.btns_h & WPAD_BUTTON_A) && moveChan == t->chan && t->wpad.ir.valid)
    {
        movePosX = (t->wpad.ir.x-moveStartPosX) / 220.0f;
        movePosY = (moveStartPosY-t->wpad.ir.y) / 180.0f;
        last_manual_move_frame = frameCount;
    }

    if(t->wpad.btns_h & WPAD_BUTTON_UP)
    {
        RotX -= 1.0f;
        last_manual_move_frame = frameCount;
    }
    if(t->wpad.btns_h & WPAD_BUTTON_DOWN)
    {
        RotX += 1.0f;
        last_manual_move_frame = frameCount;
    }
    if(t->wpad.btns_h & WPAD_BUTTON_LEFT)
    {
        RotY -= 1.0f;
        last_manual_move_frame = frameCount;
    }
    if(t->wpad.btns_h & WPAD_BUTTON_RIGHT)
    {
        RotY += 1.0f;
        last_manual_move_frame = frameCount;
    }
    if(t->wpad.btns_h & WPAD_BUTTON_PLUS)
    {
        if(PosZ < -3.4f)
            PosZ += 0.1f;
        else if(BoxScale < 2.4f)
            BoxScale += 0.05f;
    }
    if(t->wpad.btns_h & WPAD_BUTTON_MINUS)
    {
        if(BoxScale > 1.0f)
            BoxScale -= 0.05f;
        else
        {
            BoxScale = 1.0f;
            PosZ -= 0.1f;
            if(PosZ < -6.0f) PosZ = -6.0f;
        }
    }
}

void BoxCover::Update(GuiTrigger * t)
{
    s8 movY = t->WPAD_Stick(0, 0) ;
    s8 movX = t->WPAD_Stick(0, 1);
    //! Drop nunchuck moves of less than 5 because of sensitivity
    if(fabs(movY) < 5.0f) movY = 0;
    if(fabs(movX) < 5.0f) movX = 0;

    if(movY != 0 || movX != 0)
        last_manual_move_frame = frameCount;

    RotY += (f32) movY / 50.0f;
    RotX -= (f32) movX / 50.0f;

    if(Zoomable)
        WiiPADControl(t);

    //! Stop movement for about 5 sec after manual move
    if(frameCount-last_manual_move_frame < 250)
        return;

    Animation = sin(DegToRad(AnimRotate))*2.0f;
    Animation2 = cos(DegToRad(AnimRotate))*5.0f;
    AnimRotate += 0.1f;
    if(AnimRotate > 360.0f)
        AnimRotate = 0.0f;
}

void BoxCover::Draw()
{
    u8 BoxAlpha = (int) (alpha+angleDyn) & 0xFF;

    Mtx44 projection;
    guPerspective(projection, 45, (f32)screenwidth/(f32)screenheight, fabs(PosZ)-1.3f > 1.0f ? fabs(PosZ)-1.3f : 1.0f, -300.0F);
    GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);

    GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);

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
    guMtxScaleApply(modelView, modelView, BoxScale, BoxScale, BoxScale);
    guMtxTransApply(modelView, modelView, PosX+xoffsetDyn/680.0f+movePosX, PosY+yoffsetDyn/680.0f+movePosY, PosZ);
    guMtxConcat(view,modelView,modelView);

    GX_LoadPosMtxImm(modelView,	GX_PNMTX0);

    GX_LoadTexObj(&boxBorderTex, GX_TEXMAP0);
    GX_InvalidateTexAll();

    //! Border quads
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

    GX_LoadTexObj(flatCover ? &defaultBoxTex : &coverTex, GX_TEXMAP0);
    GX_InvalidateTexAll();

    //! Back Cover
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

    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

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
        if(PosX > 0.01f)
            PosX -= effectAmount/1000.0f;
        if(PosY > 0.01f)
            PosY -= effectAmount/1000.0f;
        if(PosX < -0.01f)
            PosX += effectAmount/1000.0f;
        if(PosY < -0.01f)
            PosY += effectAmount/1000.0f;

        movePosX = 0.0f;
        movePosY = 0.0f;
        PosZ += 0.1f;
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
            PosX -= effectAmount/1000.0f;
        if(PosY > PosYOrig+0.1f)
            PosY -= effectAmount/1000.0f;
        if(PosX < PosXOrig-0.1f)
            PosX += effectAmount/1000.0f;
        if(PosY < PosYOrig-0.1f)
            PosY += effectAmount/1000.0f;

        PosZ -= 0.1f;
        RotY -= effectAmount/4.9f;
        if(BoxScale > 1.0f)
            BoxScale -= 0.08f;

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
            BoxScale = 1.0f;
            PosX = PosXOrig;
            PosY = PosYOrig;
            PosZ = PosZOrig;
            effects = 0;
            effectAmount = 0;
        }
    }
}
