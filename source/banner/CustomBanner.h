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
#ifndef CUSTOM_BANNER_H_
#define CUSTOM_BANNER_H_

#include <vector>
#include "Banner.h"

using namespace std;

class CustomBanner : public Banner
{
public:

	CustomBanner();
	virtual ~CustomBanner();
	void SetBannerText(const char *text_name, const char *text);
	void SetBannerPngImage(const char *tex_name, const u8 * img, u32 imgSize);
	void SetIconPngImage(const char *tex_name, const u8 * img, u32 imgSize);
	void SetBannerTexture(const char *tex_name, const u8 *data, float width, float height, u8 fmt);
	void SetIconTexture(const char *tex_name, const u8 *data, float width, float height, u8 fmt);
	void SetBannerTextureScale(float scale);
	void SetBannerPaneVisible(const char *pane, bool visible);
private:
	u8 *LoadTextureFromPng(const u8 * img, u32 imgSize, int *width, int *height);
	vector<u16 *> text_list;
};

#endif /*CUSTOM_BANNER_H_*/
