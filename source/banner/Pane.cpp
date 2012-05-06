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

#include "Pane.h"
#include "Layout.h"

void Pane::Load(Pane::Header *pan)
{
	if(!pan)
		return;

	header = pan;
	hide = false;
	RootPane = !strcmp( header->name, "RootPane" );
	AlignHor = header->origin % 3;
	AlignVer = 2 - header->origin / 3;
}

Pane::~Pane()
{
	// delete children
	for(u32 i = 0; i < panes.size(); ++i)
		delete panes[i];
}

void Pane::SetFrame(FrameNumber frame, u8 key_set)
{
	// setframe on self
	Animator::SetFrame(frame, key_set);

	// setframe on children
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->SetFrame(frame, key_set);
}

void Pane::Render(const BannerResources& resources, u8 parent_alpha, Mtx &modelview,
					bool widescreen, bool modify_alpha) const
{
	if (!header || !GetVisible() || GetHide())
		return;

	u8 render_alpha = header->alpha;

	if(RootPane && parent_alpha != 0xFF)
	{
		modify_alpha = true;
		render_alpha = MultiplyAlpha(header->alpha, parent_alpha);
	}
	if(!RootPane && modify_alpha)
	{
		render_alpha = MultiplyAlpha(header->alpha, parent_alpha);
	}
	else if(GetInfluencedAlpha() && header->alpha != 0xff)
	{
		modify_alpha = true;
		parent_alpha = MultiplyAlpha(header->alpha, parent_alpha);
	}

	float ws_scale = 1.0f;

	if(widescreen && GetWidescren())
	{
		ws_scale *= 0.82f; // should actually be 0.75?
		widescreen = false;
	}

	Mtx m1,m2,m3,m4, mv;
	guMtxIdentity (m1);

	// Scale
	guMtxScaleApply(m1,m1, header->scale.x * ws_scale, header->scale.y, 1.f);

	// Rotate
	guMtxRotDeg ( m2, 'x', header->rotate.x );
	guMtxRotDeg ( m3, 'y', header->rotate.y );
	guMtxRotDeg ( m4, 'z', header->rotate.z );
	guMtxConcat(m2, m3, m2);
	guMtxConcat(m2, m4, m2);
	guMtxConcat(m1, m2, m1);

	// Translate
	guMtxTransApply(m1,m1, header->translate.x, header->translate.y, header->translate.z);

	guMtxConcat (modelview, m1, mv);

	// render self
	Draw(resources, render_alpha, ws_scale, mv);

	// render children
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->Render(resources, render_alpha, mv, widescreen, modify_alpha);
}

Pane* Pane::FindPane(const std::string& find_name)
{
	if(!header)
		return NULL;

	if (find_name.compare(0, 0x10, getName()) == 0)
		return this;

	for(u32 i = 0; i < panes.size(); ++i)
	{
		Pane *found = panes[i]->FindPane(find_name);
		if (found)
			return found;
	}

	return NULL;
}

void Pane::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.type == ANIMATION_TYPE_VERTEX_COLOR)	// vertex color
	{
		// only alpha is supported for Panes afaict
		if (0x10 == type.target)
		{
			header->alpha = FLOAT_2_U8(value);
			return;
		}
	}
	else if (type.type == ANIMATION_TYPE_PANE)	// pane animation
	{
		if (type.target < 10)
		{
			(&header->translate.x)[type.target] = value;
			return;
		}
	}

	Base::ProcessHermiteKey(type, value);
}

void Pane::ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data)
{
	if (type.type == ANIMATION_TYPE_VISIBILITY)	// visibility
	{
		SetVisible(!!data.data2);
		return;
	}

	Base::ProcessStepKey(type, data);
}
