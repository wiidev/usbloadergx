/****************************************************************************
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <malloc.h>
#include "CustomBanner.h"
#include "ImageOperations/TextureConverter.h"
#include "utils/StringTools.h"


CustomBanner::CustomBanner()
{
}

CustomBanner::~CustomBanner()
{
	for(u32 i = 0; i < text_list.size(); i++)
		delete [] text_list[i];
}

void CustomBanner::SetBannerTexture(const char *tex_name, const u8 *data, float width, float height, u8 fmt)
{
	if(!layout_banner)
		return;

	Texture *texture = layout_banner->FindTexture( tex_name );
	if(texture != NULL && data != NULL) {
		texture->LoadTextureData(data, width, height, fmt);
	}
}

void CustomBanner::SetBannerText(const char *text_name, const char *text)
{
	if(!layout_banner || !text)
		return;

	Textbox *tbox = dynamic_cast<Textbox *>(layout_banner->FindPane(text_name));
	if(tbox)
	{
		const wchar_t *wText = wfmt("%s", text);
		int len = wcslen(wText);
		u16 *text_char16 = new u16[len+1];

		for(int i = 0; i < len; i++)
			text_char16[i] = (u16) wText[i];
		text_char16[len] = 0;

		tbox->SetText(text_char16);
		text_list.push_back(text_char16);
	}
}

void CustomBanner::SetIconTexture(const char *tex_name, const u8 *data, float width, float height, u8 fmt)
{
	if(!layout_icon)
		return;

	Texture *texture = layout_icon->FindTexture( tex_name );
	if(texture != NULL && data != NULL) {
		texture->LoadTextureData(data, width, height, fmt);
	}
}

u8 *CustomBanner::LoadTextureFromPng(const u8 * img, u32 imgSize, int *width, int *height)
{
	if(!img)
		return NULL;

	// load png
	gdImagePtr gdImg = gdImageCreateFromPngPtr(imgSize, (u8 *)img);
	if(!gdImg)
		return NULL;

	u8 *texture = GDImageToRGBA8(&gdImg, width, height);

	// free image object
	gdImageDestroy( gdImg );

	return texture;
}

void CustomBanner::SetBannerPngImage(const char *tex_name, const u8 * img, u32 imgSize)
{
	int pngWidth;
	int pngHeight;

	u8 * texData = LoadTextureFromPng(img, imgSize, &pngWidth, &pngHeight);

	if(texData)
	{
		SetBannerTexture(tex_name, texData, pngWidth, pngHeight, GX_TF_RGBA8);
		free(texData);
	}
}

void CustomBanner::SetIconPngImage(const char *tex_name, const u8 * img, u32 imgSize)
{
	int pngWidth;
	int pngHeight;

	u8 * texData = LoadTextureFromPng(img, imgSize, &pngWidth, &pngHeight);

	if(texData)
	{
		SetIconTexture(tex_name, texData, pngWidth, pngHeight, GX_TF_RGBA8);
		free(texData);
	}
}

void CustomBanner::SetBannerTextureScale(float scale)
{
	Pane *pane = layout_banner->FindPane("HBPic");
	if(pane)
		pane->SetScale(scale);

	pane = layout_banner->FindPane("HBPicSha");
	if(pane)
		pane->SetScale(scale);
}

void CustomBanner::SetBannerPaneVisible(const char *pane_name, bool visible)
{
	Pane *pane = layout_banner->FindPane(pane_name);
	if(pane)
		pane->SetVisible(visible);
}
