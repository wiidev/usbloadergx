/****************************************************************************
 * Cover Class
 * by dimok
 ***************************************************************************/

#include "cover.h"
/**
 * Constructor for the CoverImage class.
 */
CoverImage::CoverImage(const char * imgPath, const u8 * buffer)
{
    image = NULL;
	width = 0;
	height = 0;
	imageangle = 0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;

	if(imgPath)
	{
		PNGUPROP imgProp;
		IMGCTX ctx = PNGU_SelectImageFromDevice(imgPath);

		if(ctx)
		{
			int res = PNGU_GetImageProperties(ctx, &imgProp);

			if(res == PNGU_OK)
			{
				int len = imgProp.imgWidth * imgProp.imgHeight * 4;
				if(len%32) len += (32-len%32);
				image = (u8 *)memalign (32, len);

				if(image)
				{
					res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, image, 255);

					if(res == PNGU_OK)
					{
						width = imgProp.imgWidth;
						height = imgProp.imgHeight;
						DCFlushRange(image, len);
					}
					else
					{
						free(image);
						image = NULL;
					}
				}
			}
			PNGU_ReleaseImageContext (ctx);
		}
	}

	if (!image) //use buffer data instead
	{
		width = 0;
		height = 0;
		if(buffer)
		{
			PNGUPROP imgProp;
			IMGCTX ctx = PNGU_SelectImageFromBuffer(buffer);

			if(!ctx)
				return;

			int res = PNGU_GetImageProperties(ctx, &imgProp);

			if(res == PNGU_OK)
			{
				int len = imgProp.imgWidth * imgProp.imgHeight * 4;
				if(len%32) len += (32-len%32);
				image = (u8 *)memalign (32, len);

				if(image)
				{
					res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, image, 255);

					if(res == PNGU_OK)
					{
						width = imgProp.imgWidth;
						height = imgProp.imgHeight;
						DCFlushRange(image, len);
					}
					else
					{
						free(image);
						image = NULL;
					}
				}
			}
			PNGU_ReleaseImageContext (ctx);
		}
	}
	imgType = IMAGE_COPY;
}

CoverImage::CoverImage(u8 * img, int w, int h)
{
	if((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image) {
        free(image);
        image = NULL;
	}

	width = w;
	height = h;
    int len = width * height * 4;
    if(len%32) len += (32-len%32);
    image = (u8 *)memalign (32, len);
	memcpy(image, img, len);
	DCFlushRange(image, len);
	imageangle =  0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_COPY;
}

CoverImage::CoverImage(GuiImageData * img)
{
	image = img->GetImage();
	width = img->GetWidth();
	height = img->GetHeight();
	imageangle = 0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_DATA;
}

CoverImage::CoverImage(CoverImage &srcimage)
{
	if((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image) {
        free(image);
        image = NULL;
	}

	width = srcimage.GetWidth();
	height = srcimage.GetHeight();
    int len = width * height * 4;
    if(len%32) len += (32-len%32);
    image = (u8 *)memalign (32, len);
	memcpy(image, srcimage.GetImage(), len);
	DCFlushRange(image, len);
	imageangle =  srcimage.GetAngle();
	tile = -1;
	stripe = 0;
	widescreen = 0;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_COPY;
}

CoverImage::CoverImage(CoverImage *srcimage)
{
	if((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image) {
        free(image);
        image = NULL;
	}

	width = srcimage->GetWidth();
	height = srcimage->GetHeight();
    int len = width * height * 4;
    if(len%32) len += (32-len%32);
    image = (u8 *)memalign (32, len);
	memcpy(image, srcimage->GetImage(), len);
	DCFlushRange(image, len);
	imageangle =  srcimage->GetAngle();
	tile = -1;
	stripe = 0;
	widescreen = 0;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_COPY;
}

CoverImage &CoverImage::operator=(CoverImage &srcimage)
{
	if((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image) {
        free(image);
        image = NULL;
    }

	width = srcimage.GetWidth();
	height = srcimage.GetHeight();
    int len = width * height * 4;
    if(len%32) len += (32-len%32);
    image = (u8 *)memalign (32, len);
	memcpy(image, srcimage.GetImage(), len);
	DCFlushRange(image, len);
	imageangle = srcimage.GetAngle();
	tile = -1;
	stripe = 0;
	widescreen = 0;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_COPY;
	return *this;
}

/**
 * Destructor for the CoverImage class.
 */
CoverImage::~CoverImage()
{
	if((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image) {
		free(image);
        image = NULL;
	}
}

u8 * CoverImage::GetImage()
{
	return image;
}

void CoverImage::SetAngle(float a)
{
	LOCK(this);
	imageangle = a;
}
float CoverImage::GetAngle()
{
	return imageangle;
}

void CoverImage::SetTile(int t)
{
	LOCK(this);
	tile = t;
}

void CoverImage::SetWidescreen(bool w)
{
	LOCK(this);
	widescreen = w;
}

GXColor CoverImage::GetPixel(int x, int y)
{
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return (GXColor){0, 0, 0, 0};

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	GXColor color;
	color.a = *(image+offset);
	color.r = *(image+offset+1);
	color.g = *(image+offset+32);
	color.b = *(image+offset+33);
	return color;
}

void CoverImage::SetPixel(int x, int y, GXColor color)
{
	LOCK(this);
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return;

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	*(image+offset) = color.a;
	*(image+offset+1) = color.r;
	*(image+offset+32) = color.g;
	*(image+offset+33) = color.b;
}


void CoverImage::SetSkew(int XX1, int YY1,int XX2, int YY2,int XX3, int YY3,int XX4, int YY4)
{

		xx1 = XX1;
		yy1 = YY1;
		xx2 = XX2;
		yy2 = YY2;
		xx3 = XX3;
		yy3 = YY3;
		xx4 = XX4;
		yy4 = YY4;
}

/**
 * Draw the button on screen
 */

void CoverImage::Draw()
{
	LOCK(this);
	if(!image || !this->IsVisible() || tile == 0)
		return;

	float currScale = this->GetScale();
	int currLeft = this->GetLeft();

    float currAngleDyn = this->GetAngleDyn();

    if(currAngleDyn)
    imageangle = currAngleDyn;



	if(tile > 0)
	{
		for(int i=0; i<tile; i++)
			Menu_DrawImg(currLeft+width*i, this->GetTop(), 0, width, height, image, imageangle, widescreen ? currScale*0.80 : currScale, currScale, this->GetAlpha(), xx1,yy1,xx2,yy2,xx3,yy3,xx4,yy4);
		}
	else
	{
		// temporary (maybe), used to correct offset for scaled images
		if(scale != 1)
			currLeft = currLeft - width/2 + (width*scale)/2;

		Menu_DrawImg(currLeft, this->GetTop(), 0, width, height, image, imageangle, widescreen ? currScale*0.80 : currScale, currScale, this->GetAlpha(), xx1,yy1,xx2,yy2,xx3,yy3,xx4,yy4);
		}

	if(stripe > 0)
		for(int y=0; y < this->GetHeight(); y+=6)
			Menu_DrawRectangle(currLeft,this->GetTop()+y,this->GetWidth(),3,(GXColor){0, 0, 0, stripe},1);




	this->UpdateEffects();
}
