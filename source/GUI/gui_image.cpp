/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_image.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "settings/CSettings.h"
/**
 * Constructor for the GuiImage class.
 */
GuiImage::GuiImage()
{
	image = NULL;
	width = 0;
	height = 0;
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
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

GuiImage::GuiImage(GuiImageData * img)
{
	if (img)
	{
		image = img->GetImage();
		width = img->GetWidth();
		height = img->GetHeight();
	}
	else
	{
		image = NULL;
		width = 0;
		height = 0;
	}
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = 0;
	parentangle = true;
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

GuiImage::GuiImage(u8 * img, int w, int h)
{
	image = img;
	width = w;
	height = h;
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = 0;
	parentangle = true;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_TEXTURE;
}

GuiImage::GuiImage(int w, int h, GXColor c)
{
	image = (u8 *) memalign(32, w * h * 4);
	width = w;
	height = h;
	imageangle = 0;
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = 0;
	parentangle = true;
	xx1 = 0;
	yy1 = 0;
	xx2 = 0;
	yy2 = 0;
	xx3 = 0;
	yy3 = 0;
	xx4 = 0;
	yy4 = 0;
	imgType = IMAGE_COLOR;

	if (!image) return;

	int x, y;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			this->SetPixel(x, y, c);
		}
	}
	int len = w * h * 4;
	if (len % 32) len += (32 - len % 32);
	DCFlushRange(image, len);
}

GuiImage::GuiImage(GuiImage &srcimage) :
	GuiElement()
{
	width = srcimage.GetWidth();
	height = srcimage.GetHeight();
	int len = width * height * 4;
	if (len % 32) len += (32 - len % 32);
	image = (u8 *) memalign(32, len);
	memcpy(image, srcimage.GetImage(), len);
	DCFlushRange(image, len);
	imageangle = srcimage.GetAngle();
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = 0;
	parentangle = true;
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

GuiImage::GuiImage(GuiImage *srcimage) :
	GuiElement()
{
	width = srcimage->GetWidth();
	height = srcimage->GetHeight();
	int len = width * height * 4;
	if (len % 32) len += (32 - len % 32);
	image = (u8 *) memalign(32, len);
	memcpy(image, srcimage->GetImage(), len);
	DCFlushRange(image, len);
	imageangle = srcimage->GetAngle();
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = 0;
	parentangle = true;
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

GuiImage &GuiImage::operator=(GuiImage & srcimage)
{
	if ((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image)
	{
		free(image);
		image = NULL;
	}

	width = srcimage.GetWidth();
	height = srcimage.GetHeight();
	int len = width * height * 4;
	if (len % 32) len += (32 - len % 32);
	image = (u8 *) memalign(32, len);
	memcpy(image, srcimage.GetImage(), len);
	DCFlushRange(image, len);
	imageangle = srcimage.GetAngle();
	tileHorizontal = -1;
	tileVertical = -1;
	stripe = 0;
	widescreen = 0;
	parentangle = true;
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
 * Destructor for the GuiImage class.
 */
GuiImage::~GuiImage()
{
	if ((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image)
	{
		free(image);
		image = NULL;
	}
}

u8 * GuiImage::GetImage()
{
	return image;
}

void GuiImage::SetImage(GuiImageData * img)
{
	LOCK( this );
	if ((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image)
	{
		free(image);
		image = NULL;
	}

	image = NULL;
	width = 0;
	height = 0;
	imgType = IMAGE_DATA;

	if(img)
	{
		image = img->GetImage();
		width = img->GetWidth();
		height = img->GetHeight();
	}
}

void GuiImage::SetImage(u8 * img, int w, int h)
{
	LOCK( this );
	if ((imgType == IMAGE_COLOR || imgType == IMAGE_COPY) && image)
	{
		free(image);
		image = NULL;
	}
	image = img;
	width = w;
	height = h;
	imgType = IMAGE_TEXTURE;
}

void GuiImage::SetAngle(float a)
{
	LOCK( this );
	imageangle = a;
}
float GuiImage::GetAngle()
{
	return imageangle;
}

void GuiImage::SetTileHorizontal(int t)
{
	LOCK( this );
	tileHorizontal = t;
}

void GuiImage::SetTileVertical(int t)
{
	LOCK( this );
	tileVertical = t;
}

void GuiImage::SetWidescreen(bool w)
{
	LOCK( this );
	widescreen = w;
}
void GuiImage::SetParentAngle(bool a)
{
	LOCK( this );
	parentangle = a;
}

GXColor GuiImage::GetPixel(int x, int y)
{
	if (!image || this->GetWidth() <= 0 || x < 0 || y < 0) return ( GXColor )
	{   0, 0, 0, 0};

	u32 offset = (((y >> 2) << 4) * this->GetWidth()) + ((x >> 2) << 6) + (((y % 4 << 2) + x % 4) << 1);
	GXColor color;
	color.a = *(image + offset);
	color.r = *(image + offset + 1);
	color.g = *(image + offset + 32);
	color.b = *(image + offset + 33);
	return color;
}

void GuiImage::SetPixel(int x, int y, GXColor color)
{
	LOCK( this );
	if (!image || this->GetWidth() <= 0 || x < 0 || y < 0) return;

	u32 offset = (((y >> 2) << 4) * this->GetWidth()) + ((x >> 2) << 6) + (((y % 4 << 2) + x % 4) << 1);
	*(image + offset) = color.a;
	*(image + offset + 1) = color.r;
	*(image + offset + 32) = color.g;
	*(image + offset + 33) = color.b;
}

void GuiImage::SetGrayscale(void)
{
	LOCK( this );
	GXColor color;
	u32 offset, gray;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			offset = (((y >> 2) << 4) * width) + ((x >> 2) << 6) + (((y % 4 << 2) + x % 4) << 1);
			color.r = *(image + offset + 1);
			color.g = *(image + offset + 32);
			color.b = *(image + offset + 33);

			gray = (77 * color.r + 150 * color.g + 28 * color.b) / 255;

			*(image + offset + 1) = gray;
			*(image + offset + 32) = gray;
			*(image + offset + 33) = gray;
		}
	}

	int len = width * height * 4;
	if (len % 32) len += (32 - len % 32);
	DCFlushRange(image, len);
}

void GuiImage::SetStripe(int s)
{
	LOCK( this );
	stripe = s;
}

void GuiImage::SetSkew(int XX1, int YY1, int XX2, int YY2, int XX3, int YY3, int XX4, int YY4)
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
void GuiImage::SetSkew(int *skew)
{

	xx1 = *skew++;
	yy1 = *skew++;
	xx2 = *skew++;
	yy2 = *skew++;
	xx3 = *skew++;
	yy3 = *skew++;
	xx4 = *skew++;
	yy4 = *skew;
}

void GuiImage::ColorStripe(int shift)
{
	LOCK( this );
	int x, y;
	GXColor color;
	int alt = 0;

	for (y = 0; y < this->GetHeight(); y++)
	{
		if (y % 3 == 0) alt ^= 1;

		for (x = 0; x < this->GetWidth(); x++)
		{
			color = GetPixel(x, y);

			if (alt)
			{
				if (color.r < 255 - shift)
					color.r += shift;
				else color.r = 255;
				if (color.g < 255 - shift)
					color.g += shift;
				else color.g = 255;
				if (color.b < 255 - shift)
					color.b += shift;
				else color.b = 255;

				color.a = 255;
			}
			else
			{
				if (color.r > shift)
					color.r -= shift;
				else color.r = 0;
				if (color.g > shift)
					color.g -= shift;
				else color.g = 0;
				if (color.b > shift)
					color.b -= shift;
				else color.b = 0;

				color.a = 255;
			}
			SetPixel(x, y, color);
		}
	}

	int len = width * height * 4;
	if (len % 32) len += (32 - len % 32);
	DCFlushRange(image, len);
}

/**
 * Draw the button on screen
 */

void GuiImage::Draw()
{
	LOCK( this );
	if (!image || !this->IsVisible() || tileHorizontal == 0) return;

	float currScale = this->GetScale();
	int currLeft = this->GetLeft();

	float currAngleDyn = this->GetAngleDyn();

	if (currAngleDyn && parentangle) imageangle = currAngleDyn;

	if (tileHorizontal > 0)
	{
		for (int i = 0; i < tileHorizontal; i++)
			Menu_DrawImg(currLeft + width * i, this->GetTop(), zoffset, width, height, image, imageangle,
					widescreen ? currScale * Settings.WSFactor : currScale, currScale, this->GetAlpha(), xx1,
					yy1, xx2, yy2, xx3, yy3, xx4, yy4);
	}
	else if(tileVertical > 0)
	{
		for (int i = 0; i < tileVertical; i++)
			Menu_DrawImg(currLeft, this->GetTop() + height * i, zoffset, width, height, image, imageangle,
					widescreen ? currScale * Settings.WSFactor : currScale, currScale, this->GetAlpha(), xx1,
					yy1, xx2, yy2, xx3, yy3, xx4, yy4);
	}
	else
	{
		// temporary (maybe), used to correct offset for scaled images
		if (scale != 1) currLeft = currLeft - width / 2 + (width * scale) / 2;

		Menu_DrawImg(currLeft, this->GetTop(), zoffset, width, height, image, imageangle, widescreen ? currScale * Settings.WSFactor
				: currScale, currScale, this->GetAlpha(), xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4);
	}

	if (stripe > 0) for (int y = 0; y < this->GetHeight(); y += 6)
		Menu_DrawRectangle(currLeft, this->GetTop() + y, this->GetWidth(), 3, ( GXColor )
		{   0, 0, 0, stripe}, 1);

	this->UpdateEffects();
}
