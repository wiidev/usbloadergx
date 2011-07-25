#include <malloc.h>
#include "pngu.h"
#include "video.h"
#include "filelist.h"

extern int screenwidth;
extern int screenheight;
static int imagewidth = 0;
static int imageheight = 0;

u8 * GetImageData()
{
    PNGUPROP imgProp;
    IMGCTX ctx;
	u8 * data = NULL;
	int ret;

    if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
        ctx = PNGU_SelectImageFromBuffer(background169_png);
    else
        ctx = PNGU_SelectImageFromBuffer(background_png);

	if (!ctx)
		return NULL;

	ret = PNGU_GetImageProperties(ctx, &imgProp);
	if (ret != PNGU_OK)
		return NULL;

    imagewidth = imgProp.imgWidth;
    imageheight = imgProp.imgHeight;

    int len = ((((imgProp.imgWidth+3)>>2)*((imgProp.imgHeight+3)>>2)*32*2) + 31) & ~31;

    data = (u8 *)memalign (32, len);

    ret = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

	DCFlushRange(data, len);

	PNGU_ReleaseImageContext(ctx);

    return data;
}

void Background_Show(float x, float y, float z, u8 * data, float angle, float scaleX, float scaleY, u8 alpha)
{
	/* Draw image */
	Menu_DrawImg(x, y, z, imagewidth, imageheight, data, angle, scaleX, scaleY, alpha);
    Menu_Render();
}

void fadein(u8 * imgdata)
{
	int i;

	/* fadein of image */
	for(i = 0; i < 255; i = i+10)
	{
		if(i>255) i = 255;
		Background_Show(screenwidth/2-imagewidth/2, screenheight/2-imageheight/2, 0, imgdata, 0, (float)screenwidth/(float)imagewidth, 1, i);
	}
}

void fadeout(u8 * imgdata)
{
	int i;

	/* fadeoout of image */
	for(i = 255; i > 1; i = i-7)
	{
		if(i < 0) i = 0;
		Background_Show(screenwidth/2-imagewidth/2, screenheight/2-imageheight/2, 0, imgdata, 0, (float)screenwidth/(float)imagewidth, 1, i);
	}
}
