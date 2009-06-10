/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.cpp
 * Video routines
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "input.h"
#include "libwiigui/gui.h"

#include "cfg.h"

#define DEFAULT_FIFO_SIZE 256 * 1024
static unsigned int *xfb[2] = { NULL, NULL }; // Double buffered
static int whichfb = 0; // Switch
static GXRModeObj *vmode; // Menu video mode
static unsigned char gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static Mtx GXmodelView2D;
int screenheight;
int screenwidth;
u32 frameCount = 0;

/****************************************************************************
 * UpdatePadsCB
 *
 * called by postRetraceCallback in InitGCVideo - scans gcpad and wpad
 ***************************************************************************/
static void
UpdatePadsCB ()
{
	frameCount++;
	#ifdef HW_RVL
	WPAD_ScanPads();
	#endif
	PAD_ScanPads();

	for(int i=3; i >= 0; i--)
	{
		#ifdef HW_RVL
		memcpy(&userInput[i].wpad, WPAD_Data(i), sizeof(WPADData));
		#endif

		userInput[i].chan = i;
		userInput[i].pad.btns_d = PAD_ButtonsDown(i);
		userInput[i].pad.btns_u = PAD_ButtonsUp(i);
		userInput[i].pad.btns_h = PAD_ButtonsHeld(i);
		userInput[i].pad.stickX = PAD_StickX(i);
		userInput[i].pad.stickY = PAD_StickY(i);
		userInput[i].pad.substickX = PAD_SubStickX(i);
		userInput[i].pad.substickY = PAD_SubStickY(i);
		userInput[i].pad.triggerL = PAD_TriggerL(i);
		userInput[i].pad.triggerR = PAD_TriggerR(i);
	}
}

/****************************************************************************
 * StartGX
 *
 * Initialises GX and sets it up for use
 ***************************************************************************/
static void
StartGX ()
{
	GXColor background = { 0, 0, 0, 0xff };

	/*** Clear out FIFO area ***/
	memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

	/*** Initialise GX ***/
	GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear (background, 0x00ffffff);

	GX_SetDispCopyGamma (GX_GM_1_0);
	GX_SetCullMode (GX_CULL_NONE);
}

/****************************************************************************
 * ResetVideo_Menu
 *
 * Reset the video/rendering mode for the menu
****************************************************************************/
void
ResetVideo_Menu()
{
	Mtx44 p;
	f32 yscale;
	u32 xfbHeight;

	VIDEO_Configure (vmode);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
		while (VIDEO_GetNextField())
			VIDEO_WaitVSync();

	// clears the bg to color and clears the z buffer
	GXColor background = {0, 0, 0, 255};
	GX_SetCopyClear (background, 0x00ffffff);

	yscale = GX_GetYScaleFactor(vmode->efbHeight,vmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetDispCopySrc(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(vmode->aa,vmode->sample_pattern,GX_TRUE,vmode->vfilter);
	GX_SetFieldMode(vmode->field_rendering,((vmode->viHeight==2*vmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	if (vmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
	GX_InvVtxCache ();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_CLR0, GX_DIRECT);

	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(GXmodelView2D);
	guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -200.0F);
	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

	guOrtho(p,0,479,0,639,0,300);
	GX_LoadProjectionMtx(p, GX_ORTHOGRAPHIC);

	GX_SetViewport(0,0,vmode->fbWidth,vmode->efbHeight,0,1);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
}

/****************************************************************************
 * InitVideo
 *
 * This function MUST be called at startup.
 * - also sets up menu video mode
 ***************************************************************************/

void
InitVideo ()
{
	VIDEO_Init();
	vmode = VIDEO_GetPreferredMode(NULL); // get default video mode

	// widescreen fix
	if(CFG.widescreen)
	{
		vmode->viWidth = VI_MAX_WIDTH_PAL-12;
		vmode->viXOrigin = ((VI_MAX_WIDTH_PAL - vmode->viWidth) / 2) + 2;
	}

	VIDEO_Configure (vmode);

	screenheight = 480;
	screenwidth = vmode->fbWidth;

	// Allocate the video buffers
	xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
	xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));

	// A console is always useful while debugging
	console_init (xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);

	// Clear framebuffers etc.
	VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);
	VIDEO_SetNextFramebuffer (xfb[0]);

	// video callback
	VIDEO_SetPostRetraceCallback ((VIRetraceCallback)UpdatePadsCB);

	VIDEO_SetBlack (FALSE);
	VIDEO_Flush ();
	VIDEO_WaitVSync ();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync ();

	StartGX();
	ResetVideo_Menu();
	// Finally, the video is up and ready for use :)
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
}

/****************************************************************************
 * Menu_Render
 *
 * Renders everything current sent to GX, and flushes video
 ***************************************************************************/
void Menu_Render()
{
	GX_DrawDone ();

	whichfb ^= 1; // flip framebuffer
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(xfb[whichfb],GX_TRUE);
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
}

/****************************************************************************
 * Menu_DrawImg
 *
 * Draws the specified image on screen using GX
 ***************************************************************************/
void Menu_DrawImg(f32 xpos, f32 ypos, f32 zpos, f32 width, f32 height, u8 data[],
	f32 degrees, f32 scaleX, f32 scaleY, u8 alpha, int XX1, int YY1,int XX2, int YY2,int XX3, int YY3,int XX4, int YY4)
{
	if(data == NULL)
		return;

	GXTexObj texObj;

	GX_InitTexObj(&texObj, data, width,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_InvalidateTexAll();

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	Vector axis = (Vector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
//	guMtxConcat(m2,m1,m);
	guMtxConcat(m1,m2,m);

	guMtxTransApply(m,m, xpos+width+0.5,ypos+height+0.5,zpos);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
//

	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(-width+XX1 , -height+YY1,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(0, 0);

	GX_Position3f32(width+XX2, -height+YY2,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(1, 0);

	GX_Position3f32(width+XX3, height+YY3,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(1, 1);

	GX_Position3f32(-width+XX4, height+YY4,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(0, 1);
	
//

	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
}

/****************************************************************************
 * Menu_DrawRectangle
 *
 * Draws a rectangle at the specified coordinates using GX
 ***************************************************************************/
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled)
{
	u8 fmt;
	long n;
	int i;
	f32 x2 = x+width;
	f32 y2 = y+height;
	Vector v[] = {{x,y,0.0f}, {x2,y,0.0f}, {x2,y2,0.0f}, {x,y2,0.0f}, {x,y,0.0f}};

	if(!filled)
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
	for(i=0; i<n; i++)
	{
		GX_Position3f32(v[i].x, v[i].y,  v[i].z);
		GX_Color4u8(color.r, color.g, color.b, color.a);
	}
	GX_End();
}

void Menu_DrawDiskCover(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u16 distance,u8 data[],
	f32 deg_alpha, f32 deg_beta, f32 scaleX, f32 scaleY, u8 alpha, bool shadow)
{
	if(data == NULL)
		return;

	GXTexObj texObj;

	GX_InitTexObj(&texObj, data, width,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_InvalidateTexAll();

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
	
	
	f32 cos_beta = cos(DegToRad(deg_beta));
	f32 s_offset_y = (zpos + (cos_beta * distance)) * tan(DegToRad(5));
	f32 s_offset_x = (cos_beta<0?-cos_beta:cos_beta) * s_offset_y;
	f32 s_offset_z = (s_offset_y<0 ? 0 : s_offset_y)*2;

	Mtx m,m1,m2,m3,m4, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m4);
	guMtxTransApply(m4,m4, 0, 0, distance);
	
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	Vector axis2 = (Vector) {0 , 1, 0 };
	guMtxRotAxisDeg (m2, &axis2, deg_beta);
	Vector axis = (Vector) {0 , 0, 1 };
	guMtxRotAxisDeg (m3, &axis, deg_alpha);
//	guMtxConcat(m2,m1,m);
	guMtxConcat(m3,m4,m3); // move distance then rotate z-axis 
	guMtxConcat(m2,m3,m2); // rotate y-axis 
	guMtxConcat(m1,m2,m); // scale
	
	if(shadow)
		guMtxTransApply(m,m, xpos+width+0.5+s_offset_x,ypos+height+0.5+s_offset_y,zpos-s_offset_z);
	else
		guMtxTransApply(m,m, xpos+width+0.5,ypos+height+0.5,zpos);



	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);

	if(shadow)
	{
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(-width, -height,  0);
	GX_Color4u8(0,0,0,alpha);
	GX_TexCoord2f32(0, 0);

	GX_Position3f32(width, -height,  0);
	GX_Color4u8(0,0,0,alpha);
	GX_TexCoord2f32(1, 0);

	GX_Position3f32(width, height,  0);
	GX_Color4u8(0,0,0,alpha);
	GX_TexCoord2f32(1, 1);

	GX_Position3f32(-width, height,  0);
	GX_Color4u8(0,0,0,alpha);
	GX_TexCoord2f32(0, 1);
	}
	else
	{
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(-width, -height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(0, 0);

	GX_Position3f32(width, -height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(1, 0);

	GX_Position3f32(width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(1, 1);

	GX_Position3f32(-width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
	GX_TexCoord2f32(0, 1);
	}

	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
}
