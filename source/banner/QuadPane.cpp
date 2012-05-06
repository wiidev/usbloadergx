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
#include "QuadPane.h"
#include "Layout.h"

void QuadPane::Load(QuadPane::Header* file)
{
	if(!file)
		return;

	header = file;
	tex_coords = (const TexCoords *) (header+1);
}

inline void QuadPane::SetVertex(int ind, float x, float y, u8 render_alpha) const
{
	// position
	GX_Position3f32(x, y, 0.f);

	const GXColor &vertex_color = header->vertex_colors[ind];
	// color
	GX_Color4u8(vertex_color.r, vertex_color.g, vertex_color.b,
				MultiplyAlpha(vertex_color.a, render_alpha));

	// texture coord
	for(u32 i = 0; i < header->tex_coord_count; i++)
		GX_TexCoord2f32(tex_coords[i].coords[ind].s, tex_coords[i].coords[ind].t);
}

static inline bool IsModulateColor(GXColor *colors, u8 render_alpha)
{
	if(render_alpha != 0xFF)
		return true;

	u32 *colorPtr = (u32 *) colors;

	for(int i = 0; i < 4; ++i)
	{
		if(colorPtr[i] != 0xFFFFFFFF)
			return true;
	}

	return false;
}

void QuadPane::Draw(const BannerResources& resources, u8 render_alpha, const float ws_scale, Mtx &modelview, u16 material_index, u8 texture_flip) const
{
	if(!header)
		return;

	if (material_index < resources.materials.size())
	{
		bool modulate_color = IsModulateColor(header->vertex_colors, render_alpha);
		resources.materials[material_index]->Apply(resources, render_alpha, modulate_color);
	}

	Mtx m, mv;
	guMtxIdentity (m);

	guMtxTransApply(m,m, -0.5f * GetOriginX(), -0.5f * GetOriginY(), 0.f);
	guMtxScaleApply(m,m, GetWidth(), GetHeight(), 1.f);

	guMtxConcat (modelview, m, mv);

	GX_LoadPosMtxImm (mv, GX_PNMTX0);

	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	for(u32 i = 0; i < header->tex_coord_count; i++)
		GX_SetVtxDesc(GX_VA_TEX0+i, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	if(texture_flip)
	{
		SetVertex(0, 0.f, 0.f, render_alpha);
		SetVertex(1, 1.f, 0.f, render_alpha);
		SetVertex(3, 1.f, 1.f, render_alpha);
		SetVertex(2, 0.f, 1.f, render_alpha);
	}
	else
	{
		SetVertex(2, 0.f, 0.f, render_alpha);
		SetVertex(3, 1.f, 0.f, render_alpha);
		SetVertex(1, 1.f, 1.f, render_alpha);
		SetVertex(0, 0.f, 1.f, render_alpha);
	}
	GX_End();
}

void QuadPane::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.type == ANIMATION_TYPE_VERTEX_COLOR)	// vertex color
	{
		if (type.target < 0x10)
		{
			// vertex colors
			(&header->vertex_colors->r)[type.target] = FLOAT_2_U8(value);
			return;
		}
	}

	Base::ProcessHermiteKey(type, value);
}
