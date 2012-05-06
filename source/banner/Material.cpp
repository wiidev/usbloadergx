/*
Copyright (c) 2010 - Wii Banner Player Project
Copyright (c) 2012 - Dimok and giantpune

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

#include <algorithm>
#include <math.h>
#include "Layout.h"
#include "Material.h"

Material::Material()
	: flags(0)
	, texture_maps(0)
	, texture_srts(0)
	, texture_coord_gens(0)
	, chan_control(0)
	, mat_color(0)
	, tev_swap_table(0)
	, ind_srt(0)
	, ind_stage(0)
	, tev_stages(0)
	, alpha_compare(0)
	, blend_mode(0)
	, header(0)
{
	for( int i = 0; i < 8; i++ )
		palette_texture[i] = DEFAULT_PALETTE;
}

void Material::Load(Material::Header *file)
{
	header = file;

	// Flags
	flags = (MatFlags *) &header->flags;

	flags->texture_map = std::min((int)MAX_TEX_MAP, (int)flags->texture_map);
	flags->texture_srt = std::min((int)MAX_TEX_SRT, (int)flags->texture_srt);
	flags->texture_coord_gen = std::min((int)MAX_TEX_GEN, (int)flags->texture_coord_gen);
	flags->ind_srt = flags->ind_srt;
	flags->ind_stage = std::min((int)MAX_IND_STAGES, (int)flags->ind_stage);
	flags->tev_stages = std::min((int)MAX_TEV_STAGES, (int)flags->tev_stages);

	u8 *buf_offset = (u8 *) (header+1);

	// texture map
	if(flags->texture_map)
	{
		texture_maps = (TextureMap *) buf_offset;
		buf_offset += sizeof(TextureMap) * flags->texture_map;
	}

	// texture srt
	if(flags->texture_srt)
	{
		texture_srts = (TextureSrt *) buf_offset;
		buf_offset += sizeof(TextureSrt) * flags->texture_srt;
	}

	// texture coord gen
	if(flags->texture_coord_gen)
	{
		texture_coord_gens = (TextureCoordGen *) buf_offset;
		buf_offset += sizeof(TextureCoordGen) * flags->texture_coord_gen;
	}

	// channel control
	if (flags->channel_control)
	{
		chan_control = (ChannelControl *) buf_offset;
		buf_offset += sizeof(ChannelControl);
	}

	// material color
	if (flags->material_color)
	{
		mat_color = (GXColor *) buf_offset;
		buf_offset += sizeof(GXColor);
	}
	//else Default to 0xFFFFFFFF

	// tev swap table
	if (flags->tev_swap_table)
	{
		tev_swap_table = (TevSwap *) buf_offset;
		buf_offset += sizeof(TevSwap) * 4;
	}

	// ind srt
	if(flags->ind_srt)
	{
		ind_srt = (IndSrt *) buf_offset;
		buf_offset += sizeof(IndSrt) * flags->ind_srt;
	}

	// ind stage
	if(flags->ind_stage)
	{
		ind_stage = (IndStage *) buf_offset;
		buf_offset += sizeof(IndStage) * flags->ind_stage;
	}

	// tev stage
	if(flags->tev_stages)
	{
		tev_stages = (TevStage *) buf_offset;
		buf_offset += sizeof(TevStage) * flags->tev_stages;
	}

	// alpha compare
	if (flags->alpha_compare)
	{
		alpha_compare = (AlphaCompareModes *) buf_offset;
		buf_offset += sizeof(AlphaCompareModes);
	}

	// blend mode
	if (flags->blend_mode)
	{
		blend_mode = (BlendModes *) buf_offset;
		buf_offset += sizeof(BlendModes);
	}
}

inline void Material::ApplyChannelControl(u8 render_alpha, bool &modulate_colors) const
{
	if(flags->channel_control)
	{
		GX_SetChanCtrl(0, 0, 0, chan_control->color_matsrc, 0, 0, 2 );
		GX_SetChanCtrl(2, 0, 0, chan_control->alpha_matsrc, 0, 0, 2 );

		if(chan_control->alpha_matsrc != 1 && chan_control->color_matsrc != 1)
			modulate_colors = false;

		if(!chan_control->alpha_matsrc || !chan_control->color_matsrc)
		{
			GXColor matColor = (GXColor){0xff, 0xff, 0xff, MultiplyAlpha(0xff, render_alpha) };

			if(flags->material_color)
				matColor = (GXColor){ mat_color->r, mat_color->g, mat_color->b,
									  MultiplyAlpha(mat_color->a, render_alpha) };

			GX_SetChanMatColor(4, matColor);

			if((*(u32 *)&matColor) == 0xFFFFFFFF)
				modulate_colors = true;
		}
	}
	else
	{
		GX_SetChanCtrl(4, 0, 0, 1, 0, 0, 2);
	}

	GX_SetNumChans(1);
}

inline void Material::ApplyTexCoordGens(void) const
{
	// texture coord gen
	for(u32 i = 0; i != flags->texture_coord_gen; ++i)
	{
		const TextureCoordGen &tcg = texture_coord_gens[i];
		GX_SetTexCoordGen(GX_TEXCOORD0 + i, tcg.tgen_typ, tcg.tgen_src, tcg.mtxsrc);

		const u8 mtrx = (tcg.mtxsrc - GX_TEXMTX0) / 3;

		if (tcg.tgen_typ == 1 && tcg.mtxsrc != GX_IDENTITY && mtrx < flags->texture_srt)
		{
			const TextureSrt& srt = texture_srts[mtrx];

			const float rotate_rad = DegToRad(srt.rotate);
			const float cosF = cosf(rotate_rad);
			const float sinF = sinf(rotate_rad);

			// setup texture matrix
			Mtx m;
			m[0][0] = srt.scale_x * cosF;
			m[0][1] = srt.scale_y * -sinF;
			m[0][2] = 0.0f;
			m[0][3] = -0.5f * (m[0][0] + m[0][1]) + srt.translate_x + 0.5f;
			m[1][0] = srt.scale_x * sinF;
			m[1][1] = srt.scale_y * cosF;
			m[1][2] = 0.0f;
			m[1][3] = -0.5f * (m[1][0] + m[1][1]) + srt.translate_y + 0.5f;
			m[2][0] = 0.0f;
			m[2][1] = 0.0f;
			m[2][2] = 1.0f;
			m[2][3] = 0.0f;

			GX_LoadTexMtxImm(m, tcg.mtxsrc, GX_MTX3x4);
		}
	}

	GX_SetNumTexGens(flags->texture_coord_gen);
}

inline void Material::ApplyTevSwapTable(void) const
{
	if (flags->tev_swap_table)
	{
		for(int i = 0; i < 4; i++)
			GX_SetTevSwapModeTable(GX_TEV_SWAP0 + i,
								   tev_swap_table[i].r, tev_swap_table[i].g,
								   tev_swap_table[i].b, tev_swap_table[i].a);
	}
	else
	{
		GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
		GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
		GX_SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
		GX_SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
	}
}

inline void Material::ApplyTevStages(bool modulate_colors) const
{
	u32 tev_stages_cnt = 0;

	if(flags->tev_stages)
	{
		// tev stages
		for(u32 i = 0; i < flags->tev_stages; ++i)
		{
			const TevStage &ts = tev_stages[i];

			GX_SetTevOrder(i, ts.texcoord, ts.tex_map | (ts.lowBit << 8), ts.color );
			GX_SetTevSwapMode(i, ts.ras_sel, ts.tex_sel);

			GX_SetTevColorIn(i, ts.color_in.a, ts.color_in.b, ts.color_in.c, ts.color_in.d);
			GX_SetTevColorOp(i, ts.color_in.tevop, ts.color_in.tevbias, ts.color_in.tevscale, ts.color_in.clamp, ts.color_in.tevregid );
			GX_SetTevKColorSel(i, ts.color_in.sel );

			GX_SetTevAlphaIn(i, ts.alpha_in.a, ts.alpha_in.b, ts.alpha_in.c, ts.alpha_in.d);
			GX_SetTevAlphaOp(i, ts.alpha_in.tevop, ts.alpha_in.tevbias, ts.alpha_in.tevscale, ts.alpha_in.clamp, ts.alpha_in.tevregid );
			GX_SetTevKAlphaSel(i, ts.alpha_in.sel );

			GX_SetTevIndirect(i, ts.ind.indtexid, ts.ind.format, ts.ind.bias, ts.ind.mtxid,
				ts.ind.wrap_s, ts.ind.wrap_t, ts.ind.addprev, ts.ind.utclod, ts.ind.a);

			tev_stages_cnt++;
		}
	}
	else
	{
		if(flags->texture_map == 0)
		{
			// 1st stage
			GX_SetTevOrder(GX_TEVSTAGE0, 0xFF, 0xFF, 4);
			GX_SetTevColorIn(GX_TEVSTAGE0, 0xF, 4, 0xA, 0xF);
			GX_SetTevAlphaIn(GX_TEVSTAGE0, 0x7, 2, 0x5, 0x7);
			tev_stages_cnt++;
		}
		else if(flags->texture_map == 1)
		{
			// 1st stage
			GX_SetTevOrder(GX_TEVSTAGE0, 0, 0, 0xFF);
			GX_SetTevColorIn(GX_TEVSTAGE0, 2, 4, 8, 0xF);
			GX_SetTevAlphaIn(GX_TEVSTAGE0, 1, 2, 4, 7);
			tev_stages_cnt++;

			// 2nd stage
			if(modulate_colors)
			{
				GX_SetTevOrder(GX_TEVSTAGE0 + tev_stages_cnt, 0xFF, 0xFF, 4);
				GX_SetTevColorIn(GX_TEVSTAGE0 + tev_stages_cnt, 0xF, 0, 0xA, 0xF);
				GX_SetTevAlphaIn(GX_TEVSTAGE0 + tev_stages_cnt, 7, 0, 5, 7);
				tev_stages_cnt++;
			}
		}
		else if(flags->texture_map == 2)
		{
			// 1st stage
			GX_SetTevOrder(GX_TEVSTAGE0, 0, 0, 0xFF);
			GX_SetTevColorIn(GX_TEVSTAGE0, 0xF, 0xF, 0xF, 8);
			GX_SetTevAlphaIn(GX_TEVSTAGE0, 7, 7, 7, 4);
			tev_stages_cnt++;

			// 2nd stage
			GX_SetTevOrder(GX_TEVSTAGE0 + tev_stages_cnt, 1, 1, 0xFF);
			GX_SetTevColorIn(GX_TEVSTAGE0 + tev_stages_cnt, 8, 0, 0xE, 0xF);
			GX_SetTevAlphaIn(GX_TEVSTAGE0 + tev_stages_cnt, 4, 0, 6, 7);
			GX_SetTevKColorSel(GX_TEVSTAGE0 + tev_stages_cnt, 0x1f);
			GX_SetTevKAlphaSel(GX_TEVSTAGE0 + tev_stages_cnt, 0x1f);
			tev_stages_cnt++;

			// 3rd stage
			if(modulate_colors)
			{
				GX_SetTevOrder(GX_TEVSTAGE0 + tev_stages_cnt, 0xFF, 0xFF, 4);
				GX_SetTevColorIn(GX_TEVSTAGE0 + tev_stages_cnt, 0xF, 0, 0xA, 0xF);
				GX_SetTevAlphaIn(GX_TEVSTAGE0 + tev_stages_cnt, 7, 0, 5, 7);
				tev_stages_cnt++;
			}
		}
		else
		{
			u32 TevKDefault[] = { 0x1F, 0x1B, 0x17, 0x13, 0x1E, 0x1A, 0x16, 0x12 };

			for(int i = 0; i < flags->texture_map; i++)
			{
				GX_SetTevOrder(i, i, i, 0xff );

				GX_SetTevColorIn(i, 0xf, 8, 0xe, i ? 0xf : 0 );
				GX_SetTevAlphaIn(i, 7, 4, 6, i ? 7 : 0 );
				GX_SetTevKColorSel(i, TevKDefault[i] );
				GX_SetTevKAlphaSel(i, TevKDefault[i] );
				tev_stages_cnt++;
			}

			GX_SetTevOrder(GX_TEVSTAGE0 + tev_stages_cnt, 0xff, 0xff, 0xff );
			GX_SetTevColorIn(GX_TEVSTAGE0 + tev_stages_cnt, 2, 4, 0, 0xf );
			GX_SetTevAlphaIn(GX_TEVSTAGE0 + tev_stages_cnt, 1, 2, 0, 7 );
			tev_stages_cnt++;

			if(modulate_colors)
			{
				GX_SetTevOrder(GX_TEVSTAGE0 + tev_stages_cnt, 0xFF, 0xFF, 4);
				GX_SetTevColorIn(GX_TEVSTAGE0 + tev_stages_cnt, 0xF, 0, 0xA, 0xF);
				GX_SetTevAlphaIn(GX_TEVSTAGE0 + tev_stages_cnt, 7, 0, 5, 7);
				tev_stages_cnt++;
			}
		}

		for(u32 i = 0; i < tev_stages_cnt; i++)
		{
			GX_SetTevColorOp(GX_TEVSTAGE0 + i, 0, 0, 0, 1, 0);
			GX_SetTevAlphaOp(GX_TEVSTAGE0 + i, 0, 0, 0, 1, 0);
			GX_SetTevDirect(GX_TEVSTAGE0 + i);
			GX_SetTevSwapMode(GX_TEVSTAGE0 + i, 0, 0);
		}
	}

	// enable correct number of tev stages
	GX_SetNumTevStages(tev_stages_cnt);
}

inline void Material::ApplyIndStages(void) const
{
	for( int i = 0; i < flags->ind_srt; i++ )
	{
		const IndSrt &ind = ind_srt[i];

		const float rotate_rad = DegToRad(ind.rotate);
		// maybe add a look up table
		float cosF = cosf(rotate_rad);
		float sinF = sinf(rotate_rad);

		int scale_exp = 0;
		f32 mtx23[2][3];
		f32 mtxabs23[2][3];

		mtx23[0][0] = ind.scale_x * cosF;
		mtx23[0][1] = ind.scale_y * -sinF;
		mtx23[0][2] = ind.translate_x;

		mtx23[1][0] = ind.scale_x * sinF;
		mtx23[1][1] = ind.scale_y * cosF;
		mtx23[1][2] = ind.translate_y;

		// create matrix with abs values
		// compiler will optimize the loops
		for(int n = 0; n < 2; n++)
			for(int m = 0; m < 3; m++)
				mtxabs23[n][m] = fabs(mtx23[n][m]);

		// hardcore clamping going on here
		if(		(mtxabs23[0][0] >= 1.0f)
			||	(mtxabs23[0][1] >= 1.0f)
			||	(mtxabs23[0][2] >= 1.0f)
			||	(mtxabs23[1][0] >= 1.0f)
			||	(mtxabs23[1][1] >= 1.0f)
			||	(mtxabs23[1][2] >= 1.0f))
		{
		   	while(	scale_exp < 0x2E
				&& ((mtxabs23[0][0] >= 1.0f)
				||	(mtxabs23[0][1] >= 1.0f)
				||	(mtxabs23[0][2] >= 1.0f)
				||	(mtxabs23[1][0] >= 1.0f)
				||	(mtxabs23[1][1] >= 1.0f)
				||	(mtxabs23[1][2] >= 1.0f)))
		   	{
		   		for(int n = 0; n < 2; n++)
		   		{
					for(int m = 0; m < 3; m++)
					{
						mtx23[n][m] *= 0.5f;
						mtxabs23[n][m] *= 0.5f;
					}
		   		}

		   		scale_exp++;
		   	}
		}
		else if(	(mtxabs23[0][0] < 0.5f)
				&&	(mtxabs23[0][1] < 0.5f)
				&&	(mtxabs23[0][2] < 0.5f)
				&&	(mtxabs23[1][0] < 0.5f)
				&&	(mtxabs23[1][1] < 0.5f)
				&&	(mtxabs23[1][2] < 0.5f))
		{
		   	while(	scale_exp > -0x11
				&&	(mtxabs23[0][0] < 0.5f)
				&&	(mtxabs23[0][1] < 0.5f)
				&&	(mtxabs23[0][2] < 0.5f)
				&&	(mtxabs23[1][0] < 0.5f)
				&&	(mtxabs23[1][1] < 0.5f)
				&&	(mtxabs23[1][2] < 0.5f))
		   	{
		   		for(int n = 0; n < 2; n++)
		   		{
					for(int m = 0; m < 3; m++)
					{
						mtx23[n][m] *= 2.0f;
						mtxabs23[n][m] *= 2.0f;
					}
		   		}

		   		scale_exp--;
		   	}
		}

		GX_SetIndTexMatrix(GX_ITM_0 + i, mtx23, scale_exp);
	}

	for( int i = 0; i < flags->ind_stage; i++ )
	{
		const IndStage &stage = ind_stage[i];
		GX_SetIndTexOrder(i, stage.texcoord, stage.tex_map);
		GX_SetIndTexCoordScale(i, stage.scale_s, stage.scale_t);
	}

	GX_SetNumIndStages(flags->ind_stage);
}

inline void Material::ApplyTextures(const BannerResources& resources) const
{
	u8 tlut_name = 0;

	for(u32 i = 0; i < flags->texture_map; ++i)
	{
		const TextureMap &tr = texture_maps[i];

		if(palette_texture[i] == DEFAULT_PALETTE)
		{
			if (tr.tex_index < resources.textures.size())
				resources.textures[tr.tex_index]->Apply(tlut_name, i, tr.wrap_s, tr.wrap_t);
		}
		else
		{
			// find texture from palette
			if(palette_texture[i] >= resources.palettes[resources.cur_set].size())
			{
				gprintf( "palette index is out of range %i\n", palette_texture[i]);
				return;
			}
			for(u32 n = 0; n < resources.textures.size(); n++)
			{
				if(resources.textures[n]->getName() == resources.palettes[resources.cur_set][palette_texture[i]])
				{
					resources.textures[n]->Apply(tlut_name, i, tr.wrap_s, tr.wrap_t);
					break;
				}
			}
		}
	}

	// invalidate texture cache
	GX_InvalidateTexAll();
}

void Material::Apply(const BannerResources& resources, u8 render_alpha, bool modulate_colors) const
{
	// channel control and material color
	ApplyChannelControl(render_alpha, modulate_colors);

	// texture coordinates gen
	ApplyTexCoordGens();

	// bind textures
	ApplyTextures(resources);

	for (u32 i = 0; i < 4; ++i)
	{
		// tev reg colors
		if(i < 3)
			GX_SetTevColorS10(GX_TEVREG0 + i, header->color_regs[i]);

		// tev k colors
		GX_SetTevKColor(GX_KCOLOR0 + i, header->color_constants[i]);
	}

	// tev swap colors
	ApplyTevSwapTable();

	// tev stages
	ApplyTevStages(modulate_colors);

	// ind stages
	ApplyIndStages();

	// alpha compare
	if(flags->alpha_compare)
		GX_SetAlphaCompare(alpha_compare->compare & 0xf, alpha_compare->ref0,
			alpha_compare->op, alpha_compare->compare >> 4, alpha_compare->ref1);
	else
		GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

	// blend mode
	if (flags->blend_mode)
		GX_SetBlendMode(blend_mode->type, blend_mode->src_factor, blend_mode->dst_factor, blend_mode->logical_op);
	else
		GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}

void Material::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.type == ANIMATION_TYPE_TEXTURE_SRT)	// texture scale/rotate/translate
	{
		if (type.target < 5 && type.index < flags->texture_srt)
		{
			(&texture_srts[type.index].translate_x)[type.target] = value;
			return;
		}
		// TODO: Something is still here: target 0-4 and index 1-9 while texture_srt is 1, value is always 0 or 1
		return; // TODO remove this
	}
	else if (type.type == ANIMATION_TYPE_IND_MATERIAL)	// ind texture crap
	{
		if (type.target < 5 && type.index < flags->ind_srt)
		{
			(&ind_srt[type.index].translate_x)[type.target] = value;
			return;
		}
		return; // TODO remove this
	}
	else if (type.type == ANIMATION_TYPE_MATERIAL_COLOR)	// material color
	{
		if (type.target < 4)
		{
			// mat_color
			if(flags->material_color)
				(&mat_color->r)[type.target] = FLOAT_2_U8(value);
			return;
		}
		else if (type.target < 0x10)
		{
			(&header->color_regs->r)[type.target - 4] = FLOAT_2_S16(value);
			return;
		}
		else if (type.target < 0x20)
		{
			(&header->color_constants->r)[type.target - 0x10] = FLOAT_2_U8(value);
			return;
		}
	}

	Base::ProcessHermiteKey(type, value);
}

void Material::ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data)
{
	if (type.type == ANIMATION_TYPE_TEXTURE_PALETTE)	// tpl palette
	{
		if(type.index < MAX_TEX_MAP)
		{
			palette_texture[type.index] = data.data2;
			return;
		}
	}

	Base::ProcessStepKey(type, data);
}
