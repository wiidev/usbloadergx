/***************************************************************************
 * Copyright (C) 2010
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
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include "gui.h"
#include "ImageOperations/TextureConverter.h"
#include "ImageOperations/TplImage.h"
#include "FileOperations/fileops.h"
#include "utils/ResourceManager.h"

#define ALIGN32(x) (((x) + 31) & ~31)

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const char * filepath)
{
	data = NULL;
	width = 0;
	height = 0;
	format = GX_TF_RGBA8;

	u8 *buffer = NULL;
	u32 size = 0;

	if(LoadFileToMem(filepath, &buffer, &size) < 0)
		return;

	LoadImage(buffer, size);

	if(buffer)
		free(buffer);
}

GuiImageData::GuiImageData(const u8 * img, int imgSize, bool cache)
{
	data = NULL;
	width = 0;
	height = 0;
	format = GX_TF_RGBA8;

	if(cache)
	{
		ImageData * Image = ResourceManager::GetImageData(img);
		if(Image != NULL && Image->data != NULL)
		{
			data = Image->data;
			width = Image->width;
			height = Image->height;
			format = Image->format;
			return;
		}
	}

	LoadImage(img, imgSize);

	if(data && cache)
	{
		ImageData NewImage;
		NewImage.data = data;
		NewImage.width = width;
		NewImage.height = height;
		NewImage.format = format;
		ResourceManager::AddImageData(img, NewImage);
	}
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData()
{
	if(data)
		ResourceManager::Remove(data);
}

void GuiImageData::LoadImage(const u8 * img, int imgSize)
{
	if(!img)
		return;

	else if (imgSize < 8)
	{
		return;
	}
	else if (img[0] == 0x89 && img[1] == 'P' && img[2] == 'N' && img[3] == 'G')
	{
		// IMAGE_PNG
		LoadPNG(img, imgSize);
	}
	else if (img[0] == 0xFF && img[1] == 0xD8)
	{
		// IMAGE_JPEG
		LoadJpeg(img, imgSize);
	}
	else if (img[0] == 'B' && img[1] == 'M')
	{
		// IMAGE_BMP
		LoadBMP(img, imgSize);
	}
	else if (img[0] == 'G' && img[1] == 'I' && img[2] == 'F')
	{
		// IMAGE_GIF
		LoadGIF(img, imgSize);
	}
	else if (img[0] == 0x00 && img[1] == 0x20 && img[2] == 0xAF && img[3] == 0x30)
	{
		// IMAGE_TPL
		LoadTPL(img, imgSize);
	}
}

void GuiImageData::LoadPNG(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromPngPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadJpeg(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromJpegPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadGIF(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromGifPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadBMP(const u8 *img, int imgSize)
{
	gdImagePtr gdImg = gdImageCreateFromBmpPtr(imgSize, (u8*) img);
	if(gdImg == 0)
		return;

	data = GDImageToRGBA8(&gdImg, &width, &height);
	gdImageDestroy(gdImg);
}

void GuiImageData::LoadTPL(const u8 *img, int imgSize)
{
	TplImage TplFile(img, imgSize);

	width = TplFile.GetWidth(0);
	height = TplFile.GetHeight(0);
	format = (u8) TplFile.GetFormat(0);

	const u8 * ImgPtr = TplFile.GetTextureBuffer(0);

	if(ImgPtr)
	{
		int len =  ALIGN32(TplFile.GetTextureSize(0));

		data = (u8 *) memalign(32, len);
		if(!data)
			return;

		memcpy(data, ImgPtr, len);
		DCFlushRange(data, len);
	}
}
