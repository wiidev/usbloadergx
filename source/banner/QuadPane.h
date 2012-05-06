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
#ifndef _QUAD_PANE_H_
#define _QUAD_PANE_H_

#include "Pane.h"

// used by Picture and Window
class QuadPane : public Pane
{
public:
	typedef Pane Base;

	struct Header
	{
		GXColor vertex_colors[4];
		u16 material_index;
		u8 tex_coord_count;
		u8 pad;
	} __attribute__((packed));

	QuadPane() : header(NULL) {}
	void Load(QuadPane::Header *file);

protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	//! overload
	void Draw(const BannerResources& resources, u8 render_alpha, const float ws_scale, Mtx &view,
				u16 material_index, u8 texture_flip) const;
	//! main virtual draw function
	void Draw(const BannerResources& resources, u8 render_alpha, const float ws_scale, Mtx &view) const {
		Draw(resources, render_alpha, ws_scale, view, header->material_index, 0);
	}

private:
	void SetVertex(int ind, float x, float y, u8 render_alpha) const;

	struct TexCoords
	{
		struct TexCoord
		{
			float s;
			float t;

		} coords[4];
	};

	QuadPane::Header *header;
	const TexCoords *tex_coords;
};

#endif
