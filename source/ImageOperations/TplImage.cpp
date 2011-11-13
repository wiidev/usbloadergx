/***************************************************************************
 * Copyright (C) 2010
 * Dimok
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
 * TplImage.cpp
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include "FileOperations/fileops.h"
#include "TextureConverter.h"
#include "TplImage.h"

TplImage::TplImage(const char * filepath)
{
	TPLBuffer = NULL;
	TPLSize = 0;

	u8 * buffer = NULL;
	u32 filesize = 0;
	LoadFileToMem(filepath, &buffer, &filesize);

	if(buffer)
	{
		LoadImage(buffer, filesize);
		free(buffer);
	}
}

TplImage::TplImage(const u8 * imgBuffer, u32 imgSize)
{
	TPLBuffer = NULL;
	TPLSize = 0;

	if(imgBuffer)
	{
		LoadImage(imgBuffer, imgSize);
	}
}

TplImage::~TplImage()
{
	if(TPLBuffer)
		free(TPLBuffer);

	Texture.clear();
	TextureHeader.clear();
	TplTextureBuffer.clear();
}

bool TplImage::LoadImage(const u8 * imgBuffer, u32 imgSize)
{
	if(TPLBuffer)
		free(TPLBuffer);

	TPLBuffer = NULL;
	TPLSize = 0;

	if(!imgBuffer)
		return false;

	TPLBuffer = (u8 *) malloc(imgSize);
	if(!TPLBuffer)
		return false;

	TPLSize = imgSize;

	memcpy(TPLBuffer, imgBuffer, imgSize);

	return ParseTplFile();
}

bool TplImage::ParseTplFile()
{
	if(!TPLBuffer)
		return false;

	TPLHeader = (const TPL_Header *) TPLBuffer;

	if(TPLHeader->magic != 0x0020AF30)
		return false;

	if(TPLHeader->head_size != 12)
		return false;

	const TPL_Texture * curTexture = (const TPL_Texture *) (TPLHeader+1);

	for(u32 i = 0; i < TPLHeader->num_textures; i++)
	{
		Texture.resize(i+1);
		TextureHeader.resize(i+1);
		TplTextureBuffer.resize(i+1);

		Texture[i] = curTexture;

		TextureHeader[i] = (const TPL_Texture_Header *) ((const u8 *) TPLBuffer+Texture[i]->text_header_offset);

		TplTextureBuffer[i] = TPLBuffer + TextureHeader[i]->offset;

		curTexture++;
	}

	return true;

}

int TplImage::GetWidth(int pos)
{
	if(pos < 0 || pos >= (int) Texture.size())
	{
		return 0;
	}

	return TextureHeader[pos]->width;
}

int TplImage::GetHeight(int pos)
{
	if(pos < 0 || pos >= (int) TextureHeader.size())
	{
		return 0;
	}

	return TextureHeader[pos]->height;
}

u32 TplImage::GetFormat(int pos)
{
	if(pos < 0 || pos >= (int) TextureHeader.size())
	{
		return 0;
	}

	return TextureHeader[pos]->format;
}

const u8 * TplImage::GetTextureBuffer(int pos)
{
	if(pos < 0 || pos >= (int) TplTextureBuffer.size())
	{
		return 0;
	}

	return TplTextureBuffer[pos];
}

int TplImage::GetTextureSize(int pos)
{
	int width = GetWidth(pos);
	int height = GetHeight(pos);
	int len = 0;

	switch(GetFormat(pos))
	{
			case GX_TF_I4:
			case GX_TF_CI4:
			case GX_TF_CMPR:
				len = ((width+7)>>3)*((height+7)>>3)*32;
				break;
			case GX_TF_I8:
			case GX_TF_IA4:
			case GX_TF_CI8:
				len = ((width+7)>>3)*((height+7)>>2)*32;
				break;
			case GX_TF_IA8:
			case GX_TF_CI14:
			case GX_TF_RGB565:
			case GX_TF_RGB5A3:
				len = ((width+3)>>2)*((height+3)>>2)*32;
				break;
			case GX_TF_RGBA8:
				len = ((width+3)>>2)*((height+3)>>2)*32*2;
				break;
			default:
				len = ((width+3)>>2)*((height+3)>>2)*32*2;
				break;
	}

	return len;
}

gdImagePtr TplImage::ConvertToGD(int pos)
{
	if(pos < 0 || pos >= (int) Texture.size())
	{
		return 0;
	}

	gdImagePtr gdImg = 0;

	switch(TextureHeader[pos]->format)
	{
		case GX_TF_RGB565:
			RGB565ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_RGB5A3:
			RGB565A3ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_RGBA8:
			RGBA8ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_I4:
			I4ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_I8:
			I8ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_IA4:
			IA4ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_IA8:
			IA8ToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		case GX_TF_CMPR:
			CMPToGD(TplTextureBuffer[pos], TextureHeader[pos]->width, TextureHeader[pos]->height, &gdImg);
			break;
		default:
			gdImg = 0;
			break;
	}

	return gdImg;
}
