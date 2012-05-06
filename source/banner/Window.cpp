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
#include "Window.h"

void Window::Load(Pane::Header *file)
{
	if(!file)
		return;

	const u8 *section_start = (const u8 *)file;

	Pane::Load(file);
	header = (Window::Header *) (file+1);

	// read content
	QuadPane::Load((QuadPane::Header *)(section_start + header->content_offset));

	// read frames
	const u32 *frame_offsets = (const u32 *) (section_start + header->frame_table_offset);
	for(u32 i = 0; i < header->frame_count; i++)
		frames.push_back((Frame *) (section_start + frame_offsets[i]));
}

void Window::Draw(const BannerResources& resources, u8 render_alpha, const float ws_scale, Mtx &view) const
{
	// TODO: handle "inflation"
	// TODO: handle "frames" and "texture_flip"

	QuadPane::Draw(resources, render_alpha, ws_scale, view);
}
