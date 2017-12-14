/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.cpp
 * Video routines
 ***************************************************************************/

#include <gccore.h>
#include <ogc/machine/processor.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "GUI/gui.h"
#include "ImageOperations/TextureConverter.h"
#include "ImageOperations/ImageWrite.h"
#include "settings/CSettings.h"
#include "input.h"
#include "sys.h"
#include "gecko.h"

#define GP_FIFO_SIZE (256 * 1024 * 3)
static u32 *xfb[2] = { NULL, NULL }; // Double buffered
static int whichfb = 0; // Switch
static unsigned char *gp_fifo = NULL;
Mtx44 FSProjection2D;
Mtx FSModelView2D;
GXRModeObj *vmode; // Menu video mode
int screenheight = 480;
int screenwidth = 640;
u32 frameCount = 0;

/****************************************************************************
 * ResetVideo_Menu
 *
 * Reset the video/rendering mode for the menu
 ****************************************************************************/
static void ResetVideo_Menu()
{
	f32 yscale;
	u32 xfbHeight;

	VIDEO_Configure(vmode);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while (VIDEO_GetNextField())
		VIDEO_WaitVSync();

	// clears the bg to color and clears the z buffer
	GXColor background = { 0, 0, 0, 255 };
	GX_SetCopyClear(background, 0x00ffffff);

	yscale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth, xfbHeight);
	GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
	GX_SetFieldMode(vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	if (vmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	for(u32 i = 0; i < 8; i++)
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0+i, GX_TEX_ST, GX_F32, 0);

	GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(FSModelView2D);
	guMtxTransApply(FSModelView2D, FSModelView2D, 0.0F, 0.0F, -9900.0F);

	guOrtho(FSProjection2D, 0, screenheight-1, 0, screenwidth-1, 0, 10000);

	GX_SetViewport(0.0f, 0.0f, vmode->fbWidth, vmode->efbHeight, 0.0f, 1.0f);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetColorUpdate(GX_TRUE);
	GX_SetAlphaUpdate(GX_TRUE);
}

/****************************************************************************
 * InitVideo
 *
 * This function MUST be called at startup.
 * - also sets up menu video mode
 ***************************************************************************/

void InitVideo()
{
	VIDEO_Init();
	
	// If WiiU - Force 16:9 aspect ratio based on WiiU settings
	if(isWiiU() && Settings.widescreen)
	{
		write32(0xd8006a0, 0x30000004), mask32(0xd8006a8, 0, 2);		
	}
	
	vmode = VIDEO_GetPreferredMode(NULL); // get default video mode

	vmode->viWidth = Settings.widescreen ? 708 : 694;

	if (Settings.PAL50)
	{
		vmode->viXOrigin = (VI_MAX_WIDTH_PAL - vmode->viWidth) / 2;
	}
	else
	{
		vmode->viXOrigin = (VI_MAX_WIDTH_NTSC - vmode->viWidth) / 2;
	}

	VIDEO_Configure(vmode);

	screenheight = 480;
	screenwidth = vmode->fbWidth;

	// Allocate the video buffers
	xfb[0] = (u32 *) MEM_K0_TO_K1 ( SYS_AllocateFramebuffer ( vmode ) );
	xfb[1] = (u32 *) MEM_K0_TO_K1 ( SYS_AllocateFramebuffer ( vmode ) );

	// Clear framebuffers etc.
	VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);
	VIDEO_SetNextFramebuffer(xfb[0]);

	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	// Initialize GX
	GXColor background = { 0, 0, 0, 0xff };
	gp_fifo = (u8 *) memalign(32, GP_FIFO_SIZE);
	memset (gp_fifo, 0, GP_FIFO_SIZE);
	GX_Init (gp_fifo, GP_FIFO_SIZE);
	GX_SetCopyClear (background, 0x00ffffff);
	GX_SetDispCopyGamma (GX_GM_1_0);
	GX_SetCullMode (GX_CULL_NONE);

	ResetVideo_Menu();

	VIDEO_SetBlack(FALSE);
	// Finally, the video is up and ready for use :)
}
/****************************************************************************
 * AdjustOverscan
 ***************************************************************************/
void AdjustOverscan(int x, int y)
{
	guOrtho(FSProjection2D, y, screenheight-1 - y, x, screenwidth-1 - x, 0, 10000);
}

/****************************************************************************
 * StopGX
 *
 * Stops GX (when exiting)
 ***************************************************************************/
void StopGX()
{
	GX_AbortFrame();
	GX_Flush();

	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

/****************************************************************************
 * Menu_Render
 *
 * Renders everything current sent to GX, and flushes video
 ***************************************************************************/
void Menu_Render()
{
	whichfb ^= 1; // flip framebuffer
	GX_CopyDisp(xfb[whichfb], GX_TRUE);
	GX_DrawDone();
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	frameCount++;
}

/****************************************************************************
 * Menu_DrawImg
 *
 * Draws the specified image on screen using GX
 ***************************************************************************/
void Menu_DrawImg(f32 xpos, f32 ypos, f32 zpos, f32 width, f32 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY,
		u8 alpha, int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4)
{
	if (data == NULL) return;

	GX_LoadProjectionMtx(FSProjection2D, GX_ORTHOGRAPHIC);

	GXTexObj texObj;

	GX_InitTexObj(&texObj, data, width, height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	Mtx m, m1, m2, mv;
	width *= 0.5f;
	height *= 0.5f;
	guMtxIdentity(m1);
	guMtxScaleApply(m1, m1, scaleX, scaleY, 1.0f);
	guVector axis = (guVector) {0 , 0, 1};
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m1, m2, m);

	guMtxTransApply(m, m, xpos + width + 0.5f, ypos + height + 0.5f, zpos);
	guMtxConcat(FSModelView2D, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(-width + XX1, -height + YY1, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(0, 0);

	GX_Position3f32(width + XX2, -height + YY2, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(1, 0);

	GX_Position3f32(width + XX3, height + YY3, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(1, 1);

	GX_Position3f32(-width + XX4, height + YY4, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(0, 1);

	GX_End();
}

/****************************************************************************
 * Menu_DrawRectangle
 *
 * Draws a rectangle at the specified coordinates using GX
 ***************************************************************************/
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled)
{
	GX_LoadProjectionMtx(FSProjection2D, GX_ORTHOGRAPHIC);
	GX_LoadPosMtxImm(FSModelView2D, GX_PNMTX0);

	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	u8 fmt;
	long n;
	int i;
	f32 x2 = x + width;
	f32 y2 = y + height;
	guVector v[] = { { x, y, 0.0f }, { x2, y, 0.0f }, { x2, y2, 0.0f }, { x, y2, 0.0f }, { x, y, 0.0f } };

	if (!filled)
	{
		fmt = GX_LINESTRIP;
		n = 5;
	}
	else
	{
		fmt = GX_TRIANGLEFAN;
		n = 4;
	}

	GX_Begin(fmt, GX_VTXFMT0, n);
	for (i = 0; i < n; i++)
	{
		GX_Position3f32(v[i].x, v[i].y, v[i].z);
		GX_Color4u8(color.r, color.g, color.b, color.a);
	}
	GX_End();
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}

void Menu_DrawDiskCover(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance, u8 data[], f32 deg_alpha,
		f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow)
{
	if (data == NULL) return;

	GX_LoadProjectionMtx(FSProjection2D, GX_ORTHOGRAPHIC);

	GXTexObj texObj;

	GX_InitTexObj(&texObj, data, width, height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	f32 cos_beta = cos(DegToRad( deg_beta ));
	f32 s_offset_y = (zpos + (cos_beta * distance)) * tan(DegToRad( 5 ));
	f32 s_offset_x = (cos_beta < 0 ? -cos_beta : cos_beta) * s_offset_y;
	f32 s_offset_z = (s_offset_y < 0 ? 0 : s_offset_y) * 2;

	Mtx m, m1, m2, m3, m4, mv;
	width *= .5;
	height *= .5;
	guMtxIdentity(m4);
	guMtxTransApply(m4, m4, 0, 0, distance);

	guMtxIdentity(m1);
	guMtxScaleApply(m1, m1, scaleX, scaleY, 1.0);
	guVector axis2 = (guVector) {0 , 1, 0};
	guMtxRotAxisDeg ( m2, &axis2, deg_beta );
	guVector axis = (guVector) {0 , 0, 1};
	guMtxRotAxisDeg ( m3, &axis, deg_alpha );
	guMtxConcat(m3, m4, m3); // move distance then rotate z-axis
	guMtxConcat(m2, m3, m2); // rotate y-axis
	guMtxConcat(m1, m2, m); // scale

	if (shadow)
		guMtxTransApply(m, m, xpos + width + 0.5 + s_offset_x, ypos + height + 0.5 + s_offset_y, zpos - s_offset_z);
	else
	guMtxTransApply(m, m, xpos + width + 0.5, ypos + height + 0.5, zpos);

	guMtxConcat(FSModelView2D, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	if (shadow)
	{
		GX_Position3f32(-width, -height, 0);
		GX_Color4u8(0, 0, 0, alpha);
		GX_TexCoord2f32(0, 0);

		GX_Position3f32(width, -height, 0);
		GX_Color4u8(0, 0, 0, alpha);
		GX_TexCoord2f32(1, 0);

		GX_Position3f32(width, height, 0);
		GX_Color4u8(0, 0, 0, alpha);
		GX_TexCoord2f32(1, 1);

		GX_Position3f32(-width, height, 0);
		GX_Color4u8(0, 0, 0, alpha);
		GX_TexCoord2f32(0, 1);
	}
	else
	{
		GX_Position3f32(-width, -height, 0);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0, 0);

		GX_Position3f32(width, -height, 0);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1, 0);

		GX_Position3f32(width, height, 0);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1, 1);

		GX_Position3f32(-width, height, 0);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0, 1);
	}

	GX_End();
}

void Menu_DrawTPLImg(f32 xpos, f32 ypos, f32 zpos, f32 width, f32 height, GXTexObj *texObj, f32 degrees, f32 scaleX,
		f32 scaleY, u8 alpha, int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4)
{
	GX_LoadProjectionMtx(FSProjection2D, GX_ORTHOGRAPHIC);

	GX_LoadTexObj(texObj, GX_TEXMAP0);
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	Mtx m, m1, m2, mv;
	width *= .5;
	height *= .5;
	guMtxIdentity(m1);
	guMtxScaleApply(m1, m1, scaleX, scaleY, 1.0);
	guVector axis = (guVector) {0 , 0, 1};
	guMtxRotAxisDeg ( m2, &axis, degrees );
	guMtxConcat(m1, m2, m);

	guMtxTransApply(m, m, xpos + width + 0.5, ypos + height + 0.5, zpos);
	guMtxConcat(FSModelView2D, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(-width + XX1, -height + YY1, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(0, 0);

	GX_Position3f32(width + XX2, -height + YY2, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(1, 0);

	GX_Position3f32(width + XX3, height + YY3, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(1, 1);

	GX_Position3f32(-width + XX4, height + YY4, 0);
	GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
	GX_TexCoord2f32(0, 1);

	GX_End();
}
/****************************************************************************
 * TakeScreenshot
 *
 * Copies the current screen into a file "path"
 ***************************************************************************/
s32 TakeScreenshot(const char *path)
{
	gprintf("\nTakeScreenshot(%s)", path);
	int size = (2 * vmode->fbWidth * vmode->xfbHeight + 31) & ~31;

	u8 * buffer = (u8 *) memalign(32, size);
	if(!buffer)
		return -1;

	memcpy(buffer, xfb[whichfb], size);

	gdImagePtr gdImg = 0;
	YCbYCrToGD(buffer, vmode->fbWidth, vmode->xfbHeight, &gdImg);

	free(buffer);

	if(gdImg == 0)
		return -1;

	if(Settings.widescreen)
	{
		gdImagePtr dst = gdImageCreateTrueColor(768, screenheight);
		if(dst == 0)
		{
			gdImageDestroy(gdImg);
			return -1;
		}
		gdImageCopyResampled(dst, gdImg, 0, 0, 0, 0, 768, screenheight, screenwidth, screenheight);
		gdImageDestroy(gdImg);
		gdImg = dst;
	}

	WriteGDImage(path,gdImg, IMAGE_PNG, 0);
	gdImageDestroy(gdImg);

	return 1;
}

/****************************************************************************
 * Restore GX to 2D mode drawing
 ***************************************************************************/
void ReSetup_GX(void)
{
	// channel control
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0,GX_DISABLE,GX_SRC_REG,GX_SRC_VTX,GX_LIGHTNULL,GX_DF_NONE,GX_AF_NONE);

	// texture gen.
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	// texture environment
	GX_SetNumTevStages(1);
	GX_SetNumIndStages(0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1_4);
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
	GX_SetTevDirect(GX_TEVSTAGE0);
	// swap table
	GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
	// alpha compare and blend mode
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}
