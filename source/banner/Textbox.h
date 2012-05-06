/*
Copyright (c) 2010 - Wii Banner Player Project
Copyright (c) 2012 - giantpune
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

#ifndef WII_BNR_TEXTBOX_H_
#define WII_BNR_TEXTBOX_H_

#include "Pane.h"

class WiiFont;

class Textbox : public Pane
{
public:
	typedef Pane Base;

	Textbox() : header(NULL), text(NULL), frameWidth(0.f), frameHeight(0.f)
	{ }

	static const u32 MAGIC = MAKE_FOURCC('t', 'x', 't', '1');

	void Load(Pane::Header *file);

	u8 GetAlignHor() const { return textAlignHor; }
	u8 GetAlignVer() const { return textAlignVer; }

	void SetText(const u16 *t) { text = t; lineWidths.clear(); }

protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);

private:
	void Draw(const BannerResources& resources, u8 render_alpha, const float ws_scale, Mtx &view) const;
	void SetupGX(const BannerResources& resources) const;
	void SetTextWidth(WiiFont *font);

	struct Header
	{
		u16			text_buf_bytes;
		u16			text_str_bytes;
		u16			material_index;
		u16			font_index;
		u8			text_alignment;
		u8			pad1; // ?
		u8			pad2[2];
		u32			text_str_offset;
		GXColor		color[2];
		float		font_size;
		float		height; // seems to work better for offset calculation
		float		space_char;
		float		space_line;
	} __attribute__((packed));

	Textbox::Header *header;
	const u16 *text;
	float frameWidth;
	float frameHeight;
	u8 textAlignVer;
	u8 textAlignHor;
	std::vector<float> lineWidths;
};

#endif
