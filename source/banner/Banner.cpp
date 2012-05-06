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
#include <malloc.h>
#include "SystemMenu/SystemMenuResources.h"
#include "utils/U8Archive.h"
#include "utils/LanguageCode.h"
#include "Banner.h"

Banner::Banner()
	: banner_bin(NULL)
	, icon_bin(NULL)
	, sound_bin(NULL)
	, banner_bin_size(0)
	, icon_bin_size(0)
	, sound_bin_size(0)
	, layout_banner(NULL)
	, layout_icon(NULL)
{
}

Banner::~Banner()
{
	delete layout_banner;
	delete layout_icon;

	if(banner_bin)
		free(banner_bin);
	if(icon_bin)
		free(icon_bin);
	if(sound_bin)
		free(sound_bin);
}

bool Banner::LoadBanner(const u8 *opening_bnr, u32 len)
{
	delete layout_banner;
	if(banner_bin)
		free(banner_bin);

	U8Archive openingArchive(opening_bnr, len);

	banner_bin = openingArchive.GetFileAllocated("/meta/banner.bin", &banner_bin_size);
	if(!banner_bin) {
		return false;
	}

	layout_banner = LoadLayout(U8Archive(banner_bin, banner_bin_size), "banner", CONF_GetLanguageString());

	return (layout_banner != NULL);
}

bool Banner::LoadIcon(const u8 *opening_bnr, u32 len)
{
	delete layout_icon;
	if(icon_bin)
		free(icon_bin);

	U8Archive openingArchive(opening_bnr, len);

	icon_bin = openingArchive.GetFileAllocated("/meta/icon.bin", &icon_bin_size);
	if(!icon_bin) {
		return false;
	}

	layout_icon = LoadLayout(U8Archive(icon_bin, icon_bin_size), "icon", CONF_GetLanguageString());

	return (layout_icon != NULL);
}

bool Banner::LoadSound(const u8 *opening_bnr, u32 len)
{
	if(sound_bin)
		free(sound_bin);

	U8Archive openingArchive(opening_bnr, len);

	sound_bin = openingArchive.GetFileAllocated("/meta/sound.bin", &sound_bin_size);
	if(!sound_bin) {
		return false;
	}
	return true;
}

Layout* Banner::LoadLayout(const U8Archive &archive, const std::string& lyt_name, const std::string &language)
{
	const u8 *brlyt = archive.GetFile("/arc/blyt/" + lyt_name + ".brlyt");
	if(!brlyt) {
		return NULL;
	}

	Layout *layout = new Layout;
	layout->Load(brlyt);

	u32 length_start = 0, length_loop = 0;

	const u8 *brlan_start = archive.GetFile("/arc/anim/" + lyt_name + "_Start.brlan");
	const u8 *brlan_loop = 0;

	// try the alternative file
	if(!brlan_start)
		brlan_start = archive.GetFile("/arc/anim/" + lyt_name + "_In.brlan");

	if (brlan_start)
		length_start = Animator::LoadAnimators((const RLAN_Header *)brlan_start, *layout, 0);

	brlan_loop = archive.GetFile("/arc/anim/" + lyt_name + ".brlan");
	if(!brlan_loop)
		brlan_loop = archive.GetFile("/arc/anim/" + lyt_name + "_Loop.brlan");
	if(!brlan_loop)
		brlan_loop = archive.GetFile("/arc/anim/" + lyt_name + "_Rso0.brlan");// added for "artstyle" wiiware

	if (brlan_loop)
		length_loop = Animator::LoadAnimators((const RLAN_Header *)brlan_loop, *layout, 1);

	// load textures after loading the animations so we get the list of tpl filenames from the brlans
	layout->LoadTextures(archive);
	layout->LoadFonts(archive);
	layout->SetLanguage(language);
	layout->SetLoopStart(length_start);
	layout->SetLoopEnd(length_start + length_loop);
	layout->SetFrame(0);

	return layout;
}

void Banner::swap(Banner &ban)
{
	u8 *tmp_banner_bin = this->banner_bin;
	u8 *tmp_icon_bin = this->icon_bin;
	u8 *tmp_sound_bin = this->sound_bin;
	u32 tmp_banner_bin_size = this->banner_bin_size;
	u32 tmp_icon_bin_size = this->icon_bin_size;
	u32 tmp_sound_bin_size = this->sound_bin_size;
	Layout *tmp_layout_banner = this->layout_banner;
	Layout *tmp_layout_icon = this->layout_icon;

	this->banner_bin = ban.banner_bin;
	this->icon_bin = ban.icon_bin;
	this->sound_bin = ban.sound_bin;
	this->banner_bin_size = ban.banner_bin_size;
	this->icon_bin_size = ban.icon_bin_size;
	this->sound_bin_size = ban.sound_bin_size;
	this->layout_banner = ban.layout_banner;
	this->layout_icon = ban.layout_icon;

	ban.banner_bin = tmp_banner_bin;
	ban.icon_bin = tmp_icon_bin;
	ban.sound_bin = tmp_sound_bin;
	ban.banner_bin_size = tmp_banner_bin_size;
	ban.icon_bin_size = tmp_icon_bin_size;
	ban.sound_bin_size = tmp_sound_bin_size;
	ban.layout_banner = tmp_layout_banner;
	ban.layout_icon = tmp_layout_icon;
}
