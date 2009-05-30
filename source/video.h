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

void InitVideo ();
void StopGX();
void ResetVideo_Menu();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, f32 zpos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF );

#endif
