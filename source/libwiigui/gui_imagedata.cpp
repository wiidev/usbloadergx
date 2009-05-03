/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_imagedata.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * img)
{
	data = NULL;
	width = 0;
	height = 0;

	if(img)
	{
		PNGUPROP imgProp;
		IMGCTX ctx = PNGU_SelectImageFromBuffer(img);

		if(!ctx)
			return;

		int res = PNGU_GetImageProperties(ctx, &imgProp);

		if(res == PNGU_OK)
		{
			int len = imgProp.imgWidth * imgProp.imgHeight * 4;
			if(len%32) len += (32-len%32);
			data = (u8 *)memalign (32, len);

			if(data)
			{
				res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

				if(res == PNGU_OK)
				{
					width = imgProp.imgWidth;
					height = imgProp.imgHeight;
					DCFlushRange(data, len);
				}
				else
				{
					free(data);
					data = NULL;
				}
			}
		}
		PNGU_ReleaseImageContext (ctx);
	}
}

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const char * imgPath, const u8 * buffer)
{
	data = NULL;
	width = 0;
	height = 0;

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
				data = (u8 *)memalign (32, len);

				if(data)
				{
					res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

					if(res == PNGU_OK)
					{
						width = imgProp.imgWidth;
						height = imgProp.imgHeight;
						DCFlushRange(data, len);
					}
					else
					{
						free(data);
						data = NULL;
					}
				}
			}
			PNGU_ReleaseImageContext (ctx);
		}
	}
	
	if (!data) //use buffer data instead
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
				data = (u8 *)memalign (32, len);

				if(data)
				{
					res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

					if(res == PNGU_OK)
					{
						width = imgProp.imgWidth;
						height = imgProp.imgHeight;
						DCFlushRange(data, len);
					}
					else
					{
						free(data);
						data = NULL;
					}
				}
			}
			PNGU_ReleaseImageContext (ctx);
		}
	}
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData()
{
	if(data)
	{
		free(data);
		data = NULL;
	}
}

u8 * GuiImageData::GetImage()
{
	return data;
}

int GuiImageData::GetWidth()
{
	return width;
}

int GuiImageData::GetHeight()
{
	return height;
}
