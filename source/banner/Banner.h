/*
Copyright (c) 2010 - Wii Banner Player Project
Copyright (c) 2012 - Dimok

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef _BANNER_H_
#define _BANNER_H_

#include "Layout.h"

class Banner
{
public:
	Banner();
	virtual ~Banner();

	bool LoadBanner(const u8 *opening_bnr, u32 len);
	bool LoadIcon(const u8 *opening_bnr, u32 len);
	bool LoadSound(const u8 *opening_bnr, u32 len);

	Layout *getBanner() const { return layout_banner; }
	Layout *getIcon() const { return layout_icon; }
	const u8 *getSound() const { return sound_bin; }
	u32 getSoundSize() const { return sound_bin_size; }

	void swap(Banner &ban);

protected:
	Layout* LoadLayout(const U8Archive &banner_file, const std::string& lyt_name, const std::string &language);

	u8 *banner_bin, *icon_bin, *sound_bin;
	u32 banner_bin_size, icon_bin_size, sound_bin_size;
	Layout *layout_banner, *layout_icon;
};

#endif
