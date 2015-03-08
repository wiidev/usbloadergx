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
#include "Textbox.h"
#include "Layout.h"

void Textbox::Load(Pane::Header *file)
{
	if(!file)
		return;

	Pane::Load(file);
	header = (Textbox::Header *) (file + 1);
	text = (const u16 *) (header + 1);

	textAlignVer = (header->text_alignment / 3);
	textAlignHor = (header->text_alignment % 3);
}

void Textbox::SetTextWidth(WiiFont *font)
{
	lineWidths.clear();
	frameWidth = 0.f;
	frameHeight = header->font_size;
	float currentLine = 0.f;
	float scale = header->font_size /(float)font->CharacterHeight();

	for(const u16 *txtString = text; *txtString != 0; txtString++)
	{
		if(*txtString == '\n')
		{
			currentLine *= scale;
			lineWidths.push_back(currentLine);
			frameWidth = MAX(frameWidth, currentLine);
			frameHeight += header->font_size + header->space_line;
			currentLine = 0.f;
			continue;
		}

		const WiiFont::CharInfo *charInfo = font->GetCharInfo(*txtString);
		if(!charInfo)
			continue;

		if(charInfo->unk)
			currentLine += (float) charInfo->advanceKerning;

		currentLine += (float) charInfo->advanceGlyphX;
	}

	currentLine *= scale;
	lineWidths.push_back(currentLine);
	frameWidth = MAX(frameWidth, currentLine);
}

void Textbox::SetupGX(const BannerResources& resources) const
{
	GX_ClearVtxDesc();
	GX_InvVtxCache();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	// channel control
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0,GX_DISABLE,GX_SRC_REG,GX_SRC_VTX,GX_LIGHTNULL,GX_DF_NONE,GX_AF_NONE);

	// texture gen.
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	// texture environment
	GX_SetNumTevStages(1);
	GX_SetNumIndStages(0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1_4);
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
	GX_SetTevDirect(GX_TEVSTAGE0);
	// swap table
	GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
	// alpha compare and blend mode
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

	if(header->material_index < resources.materials.size())
	{
		const Material::Header *matHead = resources.materials[header->material_index]->GetHeader();
		if(!matHead)
			return;

		//GX_SetFog(0, 0.0f, 0.0f, 0.0f, 0.0f, (GXColor){0xff, 0xff, 0xff, 0xff});
		GX_SetTevSwapModeTable(0, 0, 1, 2, 3);
		//GX_SetZTexture(0, 0x11, 0);
		GX_SetNumChans(1 );
		GX_SetChanCtrl(4, 0, 0, 1, 0, 0, 2);
		GX_SetChanCtrl(5, 0, 0, 0, 0, 0, 2);
		GX_SetNumTexGens(1);
		GX_SetTexCoordGen2(0, 1, 4, 0x3c, 0, 0x7D);
		GX_SetNumIndStages(0);
		GX_SetBlendMode(1, 4, 5, 0xf);
		GX_SetNumTevStages(2);
		GX_SetTevDirect(0);
		GX_SetTevDirect(1);
		GX_SetTevSwapMode(0, 0, 0);
		GX_SetTevSwapMode(1, 0, 0);
		GX_SetTevOrder(0, 0, 0, 0xff);

		for( int i = 0; i < 2; i++ )
		{
			// Devkitppc_r27 internal compiler error
			//GX_SetTevColor(i + 1, (GXColor){ LIMIT(matHead->color_regs[i].r, 0, 0xFF),
			//								 LIMIT(matHead->color_regs[i].g, 0, 0xFF),
			//								 LIMIT(matHead->color_regs[i].b, 0, 0xFF),
			//								 LIMIT(matHead->color_regs[i].a, 0, 0xFF) });
											 
			u8 r = (u8) LIMIT(matHead->color_regs[i].r, 0, 0xFF);
			u8 g = (u8) LIMIT(matHead->color_regs[i].g, 0, 0xFF);
			u8 b = (u8) LIMIT(matHead->color_regs[i].b, 0, 0xFF);
			u8 a = (u8) LIMIT(matHead->color_regs[i].a, 0, 0xFF);
			GX_SetTevColor((u8) (i + 1), (GXColor){r,g,b,a});
		}

		GX_SetTevColorIn(0, 2, 4, 8, 0xf);
		GX_SetTevAlphaIn(0, 1, 2, 4, 7);
		GX_SetTevColorOp(0, 0, 0, 0, 1, 0);
		GX_SetTevAlphaOp(0, 0, 0, 0, 1, 0);
		GX_SetTevOrder(1, 0xff, 0xff, 4);
		GX_SetTevColorIn(1, 0xf, 0, 0xa, 0xf);
		GX_SetTevAlphaIn(1, 7, 0,  5, 7);
		GX_SetTevColorOp(1, 0, 0, 0, 1, 0);
		GX_SetTevAlphaOp(1, 0, 0, 0, 1, 0);
	}
}

void Textbox::Draw(const BannerResources& resources, u8 parent_alpha, const float ws_scale, Mtx &modelview) const
{
	if(!text)
		return;

	if(header->font_index >= resources.fonts.size())
		return;

	WiiFont *font = resources.fonts[header->font_index];
	if(!font->IsLoaded())
		return;

	// Ugly...but doing it by going through all panes is more ugly
	// TODO: move it to somewhere else
	if(lineWidths.empty())
		((Textbox *) this)->SetTextWidth(font);

	if(lineWidths.empty())
		return;

	SetupGX(resources);

	GX_LoadPosMtxImm(modelview, GX_PNMTX0);

	// Setup text color
	GXColor color0 = { header->color[0].r,
					   header->color[0].g,
					   header->color[0].b,
					   MultiplyAlpha(header->color[0].a, parent_alpha) };

	GXColor color1 = { header->color[1].r,
					   header->color[1].g,
					   header->color[1].b,
					   MultiplyAlpha(header->color[1].a, parent_alpha) };

	u32 lastSheetIdx = 0xffff;
	float scale = header->font_size /(float)font->CharacterHeight();

	// use complete text width if not aligned to middle
	float textWidth = (GetAlignHor() == 1) ? lineWidths[0] : frameWidth;

	// position offset calculation for first line...why the hell is it that complex?
	float xPos = -0.5f * ( GetOriginX() * GetWidth() * ws_scale +
							GetAlignHor() * (-GetWidth()  * ws_scale + textWidth) );
	float yPos = -0.5f * ( GetAlignVer() * -frameHeight +
							GetHeight() * (GetAlignVer() - (2 - GetOriginY())) )
						 - header->font_size;

	// store the character width here for later use, it's constant over the text
	float charWidth = scale * (float)font->CharacterWidth();
	int lineNumber = 0;

	for(const u16 *txtString = text; *txtString != 0; txtString++)
	{
		if(*txtString == '\n')
		{
			lineNumber++;
			// use complete text width if not aligned to middle
			textWidth = (GetAlignHor() == 1) ? lineWidths[lineNumber] : frameWidth;
			// calculate text position depending on line width
			xPos = -0.5f * (GetOriginX() * GetWidth() * ws_scale +
							GetAlignHor() * (-GetWidth() * ws_scale + textWidth));
			// go one line down
			yPos -= (header->font_size + header->space_line);
			continue;
		}

		const WiiFont::CharInfo *charInfo = font->GetCharInfo(*txtString);
		if(!charInfo)
			continue;

		if(charInfo->sheetIdx != lastSheetIdx)
		{
			lastSheetIdx = charInfo->sheetIdx;

			if(!font->Apply(charInfo->sheetIdx))
				continue;
		}

		if(charInfo->unk)
			xPos += scale * (float)charInfo->advanceKerning;

		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

		GX_Position3f32(xPos, yPos, 0.f);
		GX_Color4u8(color1.r, color1.g, color1.b, color1.a);
		GX_TexCoord2f32(charInfo->s1, charInfo->t2);

		GX_Position3f32(xPos + charWidth, yPos, 0.f);
		GX_Color4u8(color1.r, color1.g, color1.b, color1.a);
		GX_TexCoord2f32(charInfo->s2, charInfo->t2);

		GX_Position3f32(xPos + charWidth, yPos + header->font_size, 0.f);
		GX_Color4u8(color0.r, color0.g, color0.b, color0.a);
		GX_TexCoord2f32(charInfo->s2, charInfo->t1);

		GX_Position3f32(xPos, yPos + header->font_size, 0.f);
		GX_Color4u8(color0.r, color0.g, color0.b, color0.a);
		GX_TexCoord2f32(charInfo->s1, charInfo->t1);

		GX_End();

		xPos += scale * (float)charInfo->advanceGlyphX;
	}
}

void Textbox::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.type == ANIMATION_TYPE_VERTEX_COLOR)	// vertex color
	{
		if(type.target < 4)
		{
			(&header->color[0].r)[type.target] = FLOAT_2_U8(value);
			return;
		}
		else if(type.target >= 8 && type.target < 12)
		{
			(&header->color[1].r)[type.target - 8] = FLOAT_2_U8(value);
			return;
		}
	}
	Base::ProcessHermiteKey(type, value);
}

void Textbox::ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data)
{
	Base::ProcessStepKey(type, data);
}
