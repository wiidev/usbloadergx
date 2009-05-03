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
/**
 * Constructor for the GuiImage class.
 */
GuiImage::GuiImage()
{
	image = NULL;
	width = 0;
	height = 0;
	imageangle = 0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	imgType = IMAGE_DATA;
}

GuiImage::GuiImage(GuiImageData * img)
{
	image = img->GetImage();
	width = img->GetWidth();
	height = img->GetHeight();
	imageangle = 0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	imgType = IMAGE_DATA;
}

GuiImage::GuiImage(u8 * img, int w, int h)
{
	image = img;
	width = w;
	height = h;
	imageangle = 0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	imgType = IMAGE_TEXTURE;
}

GuiImage::GuiImage(int w, int h, GXColor c)
{
	image = (u8 *)memalign (32, w * h * 4);
	width = w;
	height = h;
	imageangle = 0;
	tile = -1;
	stripe = 0;
	widescreen = 0;
	imgType = IMAGE_COLOR;

	if(!image)
		return;

	int x, y;

	for(y=0; y < h; y++)
	{
		for(x=0; x < w; x++)
		{
			this->SetPixel(x, y, c);
		}
	}
	int len = w*h*4;
	if(len%32) len += (32-len%32);
	DCFlushRange(image, len);
}

/**
 * Destructor for the GuiImage class.
 */
GuiImage::~GuiImage()
{
	if(imgType == IMAGE_COLOR && image)
		free(image);
}

u8 * GuiImage::GetImage()
{
	return image;
}

void GuiImage::SetImage(GuiImageData * img)
{
	image = img->GetImage();
	width = img->GetWidth();
	height = img->GetHeight();
	imgType = IMAGE_DATA;
}

void GuiImage::SetImage(u8 * img, int w, int h)
{
	image = img;
	width = w;
	height = h;
	imgType = IMAGE_TEXTURE;
}

void GuiImage::SetAngle(float a)
{
	imageangle = a;
}

void GuiImage::SetTile(int t)
{
	tile = t;
}

void GuiImage::SetWidescreen(short w)
{
	widescreen = w;
}

GXColor GuiImage::GetPixel(int x, int y)
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

void GuiImage::SetPixel(int x, int y, GXColor color)
{
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return;

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	*(image+offset) = color.a;
	*(image+offset+1) = color.r;
	*(image+offset+32) = color.g;
	*(image+offset+33) = color.b;
}

void GuiImage::SetStripe(int s)
{
	stripe = s;
}

void GuiImage::ColorStripe(int shift)
{
	int x, y;
	GXColor color;
	int alt = 0;

	for(y=0; y < this->GetHeight(); y++)
	{
		if(y % 3 == 0)
			alt ^= 1;

		for(x=0; x < this->GetWidth(); x++)
		{
			color = GetPixel(x, y);

			if(alt)
			{
				if(color.r < 255-shift)
					color.r += shift;
				else
					color.r = 255;
				if(color.g < 255-shift)
					color.g += shift;
				else
					color.g = 255;
				if(color.b < 255-shift)
					color.b += shift;
				else
					color.b = 255;

				color.a = 255;
			}
			else
			{
				if(color.r > shift)
					color.r -= shift;
				else
					color.r = 0;
				if(color.g > shift)
					color.g -= shift;
				else
					color.g = 0;
				if(color.b > shift)
					color.b -= shift;
				else
					color.b = 0;

				color.a = 255;
			}
			SetPixel(x, y, color);
		}
	}
}

/**
 * Draw the button on screen
 */
void GuiImage::Draw()
{
	if(!image || !this->IsVisible() || tile == 0)
		return;

	float currScale = this->GetScale();
	int currLeft = this->GetLeft();

	if(tile > 0)
	{
		for(int i=0; i<tile; i++)
			Menu_DrawImg(currLeft+width*i, this->GetTop(), width, height, image, imageangle, widescreen ? currScale*0.8 : currScale, currScale, this->GetAlpha());
	}
	else
	{
		// temporary (maybe), used to correct offset for scaled images
		if(scale != 1)
			currLeft = currLeft - width/2 + (width*scale)/2;

		Menu_DrawImg(currLeft, this->GetTop(), width, height, image, imageangle, widescreen ? currScale*0.8 : currScale, currScale, this->GetAlpha());
	}

	if(stripe > 0)
		for(int y=0; y < this->GetHeight(); y+=6)
			Menu_DrawRectangle(currLeft,this->GetTop()+y,this->GetWidth(),3,(GXColor){0, 0, 0, stripe},1);

	this->UpdateEffects();
}
