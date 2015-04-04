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

#ifndef WII_BNR_MATERIAL_H_
#define WII_BNR_MATERIAL_H_

#include "Animator.h"
#include "Texture.h"

struct BannerResources;

class Material : public Animator
{
public:
	typedef Animator Base;

	struct Header
	{
		char name[20];
		GXColorS10 color_regs[3];
		GXColor color_constants[4];
		u32 flags;
	} __attribute__((packed));

	Material();
	virtual ~Material() {}

	void Load(Material::Header *mat);
	void Apply(const BannerResources& resources, u8 render_alpha, bool modulate) const;
	const char *getName() const { return header->name; }

	const Material::Header *GetHeader() const { return header; }
protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);

private:
	void ApplyChannelControl(u8 render_alpha, bool &modulate_colors) const;
	void ApplyTevSwapTable(void) const;
	void ApplyTexCoordGens(void) const;
	void ApplyTevStages(bool modulate_colors) const;
	void ApplyIndStages(void) const;
	void ApplyTextures(const BannerResources& resources) const;

	enum
	{
		MAX_TEX_MAP			= 8,
		MAX_TEX_SRT			= 10,
		MAX_TEX_GEN			= 8,
		MAX_IND_STAGES		= 4,
		MAX_TEV_STAGES		= 16,
		DEFAULT_PALETTE		= 0xFF,
	};

	struct MatFlags
	{
		u32 pad2 : 4;
		u32 material_color : 1;
		u32 pad : 1;
		u32 channel_control : 1;
		u32 blend_mode : 1;
		u32 alpha_compare : 1;
		u32 tev_stages : 5;
		u32 ind_stage : 3;
		u32 ind_srt : 2;
		u32 tev_swap_table : 1;
		u32 texture_coord_gen : 4;
		u32 texture_srt : 4;
		u32 texture_map : 4;
	} __attribute__((packed));

	struct TextureMap
	{
		u16 tex_index;
		u8 wrap_s;
		u8 wrap_t;
	} __attribute__((packed));

	struct TextureSrt
	{
		f32 translate_x;
		f32 translate_y;
		f32 rotate;
		f32 scale_x;
		f32 scale_y;
	} __attribute__((packed));

	struct TextureCoordGen
	{
		u8 tgen_typ;
		u8 tgen_src;
		u8 mtxsrc;
		u8 pad;
	} __attribute__((packed));

	struct ChannelControl
	{
		u8 color_matsrc;
		u8 alpha_matsrc;
		u16 pad;
	} __attribute__((packed));

	struct IndSrt
	{
		f32 translate_x;
		f32 translate_y;
		f32 rotate;
		f32 scale_x;
		f32 scale_y;
	} __attribute__((packed));

	struct IndStage
	{
		u8 texcoord;
		u8 tex_map;
		u8 scale_s;
		u8 scale_t;
	} __attribute__((packed));

	struct BlendModes
	{
		u8 type, src_factor, dst_factor, logical_op;

	} __attribute__((packed));

	struct AlphaCompareModes
	{
		u8 compare, op, ref0, ref1;

	} __attribute__((packed));

	struct TevSwap
	{
		u32 a : 2;
		u32 b : 2;
		u32 g : 2;
		u32 r : 2;
	} __attribute__((packed));

	struct TevStage
	{
		u32 texcoord			: 8;
		u32 color				: 8;
		u32 tex_map				: 8;

		u32 unk					: 3;
		u32 tex_sel				: 2;
		u32 ras_sel				: 2;
		u32 lowBit				: 1;

		struct
		{
			u32 b				: 4;
			u32 a				: 4;

			u32 d				: 4;
			u32 c				: 4;

			u32 tevscale		: 2;
			u32 tevbias			: 2;
			u32 tevop			: 4;

			u32 sel				: 5;
			u32 tevregid		: 2;
			u32 clamp			: 1;

		} __attribute__((packed)) color_in, __attribute__((packed)) alpha_in;

		struct
		{
			u32 unk1			: 6;
			u32 indtexid		: 2;

			u32 unk2			: 1;
			u32 mtxid			: 4;
			u32 bias			: 3;

			u32 unk3			: 2;
			u32 wrap_t			: 3;
			u32 wrap_s			: 3;

			u32 unk4			: 2;
			u32 a				: 2;
			u32 utclod			: 1;
			u32 addprev			: 1;
			u32 format			: 2;

		} __attribute__((packed)) ind;
	} __attribute__((packed));

	// Material flags
	MatFlags *flags;

	// Texture
	TextureMap *texture_maps;
	TextureSrt *texture_srts;
	TextureCoordGen *texture_coord_gens;

	// Color channels
	ChannelControl *chan_control;

	// Material colors
	GXColor *mat_color;

	// Tev color swap
	TevSwap *tev_swap_table;

	// Indirect textures
	IndSrt *ind_srt;
	IndStage *ind_stage;

	// Texture environment
	TevStage *tev_stages;

	// Blending and alpha compare
	AlphaCompareModes *alpha_compare;
	BlendModes *blend_mode;

	// palette animation
	u8 palette_texture[MAX_TEX_MAP];

	//! Material header
	Material::Header *header;
};

class MaterialList : public std::vector<Material*>
{
public:
	static const u32 MAGIC = MAKE_FOURCC('m', 'a', 't', '1');
};


#endif
