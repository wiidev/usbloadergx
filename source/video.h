/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.h
 * Video routines
 ***************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <ogcsys.h>

void InitVideo();
void StopGX();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, f32 zpos, f32 width, f32 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY,
		u8 alphaF, int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4);
void Menu_DrawTPLImg(f32 xpos, f32 ypos, f32 zpos, f32 width, f32 height, GXTexObj *texObj, f32 degrees, f32 scaleX,
		f32 scaleY, u8 alpha, int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4);
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled);
s32 TakeScreenshot(const char *path);
void VIDEO_SetWidescreen(bool widescreen);
void AdjustOverscan(int x, int y);
void ReSetup_GX(void);

extern GXRModeObj *vmode;
extern Mtx44 FSProjection2D;
extern Mtx FSModelView2D;
extern int screenheight;
extern int screenwidth;
extern u32 frameCount;

#endif
