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
#include "SystemMenu/SystemMenuResources.h"
#include "Layout.h"
#include "WiiFont.h"

Layout::Layout()
	: header(0)
{
}

Layout::~Layout()
{
	for(u32 i = 0; i < panes.size(); ++i)
		delete panes[i];

	for(u32 i = 0; i < resources.materials.size(); ++i)
		delete resources.materials[i];

	for(u32 i = 0; i < resources.textures.size(); ++i)
		delete resources.textures[i];

	for(u32 i = 0; i < resources.fonts.size(); ++i) {

		if(resources.fonts[i] == SystemMenuResources::Instance()->GetWbf1() ||
		   resources.fonts[i] == SystemMenuResources::Instance()->GetWbf2())
			continue;

		delete resources.fonts[i];
	}
}

bool Layout::Load(const u8 *brlyt)
{
	if(!brlyt)
		return false;

	const BRLYT_Header *brlytFile = (const BRLYT_Header *) brlyt;

	if(brlytFile->magic != BRLYT_MAGIC || brlytFile->version != BRLYT_VERSION)
		return false;

	Group* last_group = NULL;
	std::stack<std::map<std::string, Group>*> group_stack;
	group_stack.push(&groups);

	Pane* last_pane = NULL;
	std::stack<std::vector<Pane*>*> pane_stack;
	pane_stack.push(&panes);

	const u8 *position = brlyt + brlytFile->header_len;

	for(u32 i = 0; i < brlytFile->section_count; ++i)
	{
		section_t *section = (section_t *) position;
		position += section->size;

		if(section->magic == Layout::MAGIC)
		{
			header = (Layout::Header *) (section + 1);
		}
		else if (section->magic == TextureList::MAGIC)
		{
			const LytItemList *txl1 = (const LytItemList *) (section+1);
			const char *nameoffset = ((const char *)(txl1+1));
			const LytStringTable *stringTable = (const LytStringTable *) (((const u8 *)(txl1+1))+txl1->offset_to_first);

			for(u32 i = 0; i < txl1->num_items; ++i)
			{
				const char *name = nameoffset+stringTable[i].offset_filename;

				Texture *texture = new Texture;
				texture->setName(name);
				resources.textures.push_back(texture);
			}
		}
		else if (section->magic == MaterialList::MAGIC)
		{
			const LytItemList *mat1 = (const LytItemList *) (section+1);
			const u32 *mat_offsets = (const u32 *) (((const u8 *)(mat1+1))+mat1->offset_to_first);

			for(u32 i = 0; i < mat1->num_items; ++i)
			{
				Material *material = new Material;
				material->Load((Material::Header *) (((const u8 *) section)+mat_offsets[i]));
				resources.materials.push_back(material);
			}
		}
		else if (section->magic == FontList::MAGIC)
		{
			// load font list
			const LytItemList *fnl1 = (const LytItemList *) (section+1);
			const char *nameoffset = ((const char *)(fnl1+1));
			const LytStringTable *stringTable = (const LytStringTable *) (((const u8 *)(fnl1+1))+fnl1->offset_to_first);

			for(u32 i = 0; i < fnl1->num_items; i++)
			{
				const char *name = nameoffset+stringTable[i].offset_filename;

				WiiFont *font;

				if(strcmp(name, "wbf1.brfna") == 0 || strcmp(name, "RevoIpl_RodinNTLGPro_DB_32_I4.brfnt") == 0) {
					//! 1st system font or alias for it
					font = SystemMenuResources::Instance()->GetWbf1();
					if(!font) continue;
				}
				else if(strcmp(name, "wbf2.brfna") == 0) {
					//! 2nd system font
					font = SystemMenuResources::Instance()->GetWbf2();
					if(!font) continue;
				}
				else {
					//! font from banner
					font = new WiiFont;
					font->SetName(name);
				}
				resources.fonts.push_back(font);
			}
		}
		else if (section->magic == Pane::MAGIC)
		{
			Pane* pane = new Pane;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Bounding::MAGIC)
		{
			Bounding* pane = new Bounding;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Picture::MAGIC)
		{
			Picture* pane = new Picture;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Window::MAGIC)
		{
			Window* pane = new Window;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Textbox::MAGIC)
		{
			Textbox* pane = new Textbox;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Layout::MAGIC_PANE_PUSH)
		{
			if (last_pane)
				pane_stack.push(&last_pane->panes);
		}
		else if (section->magic == Layout::MAGIC_PANE_POP)
		{
			if (pane_stack.size() > 1)
				pane_stack.pop();
		}
		else if (section->magic == Group::MAGIC)
		{
			const char *grp = (const char *) (section + 1);
			std::string group_name(grp, 0, Layout::Group::NAME_LENGTH);
			Group& group_ref = (*group_stack.top())[group_name];
			grp += Layout::Group::NAME_LENGTH;

			u16 sub_count = *(u16 *) grp;
			grp += 4; // 2 bytes reserved

			while (sub_count--)
			{
				std::string pane_name(grp, 0, Layout::Group::NAME_LENGTH);
				group_ref.panes.push_back(pane_name);
				grp += Layout::Group::NAME_LENGTH;
			}

			last_group = &group_ref;
		}
		else if (section->magic == Layout::MAGIC_GROUP_PUSH)
		{
			if (last_group)
				group_stack.push(&last_group->groups);
		}
		else if (section->magic == Layout::MAGIC_GROUP_POP)
		{
			if (group_stack.size() > 1)
				group_stack.pop();
		}
		else {
			gprintf("Uknown layout section: %08X\n", section->magic);
		}
	}
	return true;
}

bool Layout::LoadTextures(const U8Archive &banner_file)
{
	bool success = false;

	for(u32 i = 0; i < resources.textures.size(); ++i)
	{
		const u8 *file = banner_file.GetFile("/arc/timg/" + resources.textures[i]->getName());
		if (file)
			resources.textures[i]->Load(file);
		else
			success = false;
	}

	return success;
}

bool Layout::LoadFonts(const U8Archive &banner_file)
{
	bool success = false;

	for(u32 i = 0; i < resources.fonts.size(); ++i)
	{
		if(resources.fonts[i]->IsLoaded())
			continue;

		const u8 *file = banner_file.GetFile("/arc/font/" + resources.fonts[i]->getName());
		if (file)
			resources.fonts[i]->Load(file);
		else
			success = false;
	}

	return success;
}

void Layout::Render(Mtx &modelview, const Vec2f &ScreenProps, bool widescreen, u8 render_alpha) const
{
	if(!header)
		return;

	Mtx mv;
	// we draw inverse
	guMtxScaleApply(modelview, mv, 1.0f, -1.0f, 1.0f);

	// centered draw
	if(header->centered)
		guMtxTransApply(mv, mv, ScreenProps.x * 0.5f, ScreenProps.y * 0.5f, 0.f);

	// render all panes
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->Render(resources, render_alpha, mv, widescreen);
}

void Layout::SetFrame(FrameNumber frame_number)
{
	frame_current = frame_number;

	const u8 key_set = (frame_current >= frame_loop_start);
	if (key_set)
		frame_number -= frame_loop_start;

	resources.cur_set = key_set;

	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->SetFrame(frame_number, key_set);

	for(u32 i = 0; i < resources.materials.size(); ++i)
		resources.materials[i]->SetFrame(frame_number, key_set);
}

void Layout::AdvanceFrame()
{
	++frame_current;

	if (frame_current >= frame_loop_end)
		frame_current = frame_loop_start;

	SetFrame(frame_current);
}

void Layout::SetLanguage(const std::string& language)
{
	if(!header)
		return;

	// check if that language is found
	bool lang_found = false;

	// hide panes of non-matching languages
	std::map<std::string, Group>::iterator itr;
	for(itr = groups["RootGroup"].groups.begin(); itr != groups["RootGroup"].groups.end(); itr++)
	{
		// check if that language exists
		if(!lang_found && itr->first == language)
			lang_found = true;

		// some hax, there are some odd "Rso0" "Rso1" groups that shouldn't be hidden
		// only the 3 character language groups should be
		if (itr->first != language && itr->first.length() == 3)
		{
			std::list<std::string>::iterator itr2;
			for(itr2 = itr->second.panes.begin(); itr2 != itr->second.panes.end(); itr2++)
			{
				Pane* const found = FindPane(*itr2);
				if (found)
					found->SetHide(true);
			}
		}
	}

	// show english if langauge is not found
	if(!lang_found && language != "ENG")
	{
		SetLanguage("ENG");
		return;
	}

	// unhide panes of matching language, some banners list language specific panes in multiple language groups
	std::list<std::string>::iterator itr2;
	for(itr2 = groups["RootGroup"].groups[language].panes.begin(); itr2 != groups["RootGroup"].groups[language].panes.end(); itr2++)
	{
		Pane* const found = FindPane(*itr2);
		if (found)
		{
			found->SetHide(false);
			found->SetVisible(true); // all games with languages start as visible
		}
	}
}

void Layout::AddPalette(const std::string &name, u8 key_set)
{
	resources.palettes[key_set].push_back(name);

	if(FindTexture(name) != 0)
		return;

	Texture *texture = new Texture;
	texture->setName(name);
	resources.textures.push_back(texture);
}

Pane* Layout::FindPane(const std::string& find_name)
{
	for(u32 i = 0; i < panes.size(); ++i)
	{
		Pane* found = panes[i]->FindPane(find_name);
		if(found)
			return found;
	}

	return NULL;
}

Material* Layout::FindMaterial(const std::string& find_name)
{
	for(u32 i = 0; i < resources.materials.size(); ++i)
	{
		if (find_name.compare(0, 20, resources.materials[i]->getName()) == 0)
			return resources.materials[i];
	}

	return NULL;
}

Texture* Layout::FindTexture(const std::string& find_name)
{
	for(u32 i = 0; i < resources.textures.size(); ++i)
	{
		if (find_name == resources.textures[i]->getName())
			return resources.textures[i];
	}

	return NULL;
}
