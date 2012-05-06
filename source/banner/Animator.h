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

#ifndef WII_BNR_ANIMATOR_H_
#define WII_BNR_ANIMATOR_H_

#include <map>
#include <cstring>
#include <string>
#include "BannerTools.h"

typedef float FrameNumber;

enum AnimationType
{
	ANIMATION_TYPE_PANE = MAKE_FOURCC('R', 'L', 'P', 'A'),
	ANIMATION_TYPE_TEXTURE_SRT = MAKE_FOURCC('R', 'L', 'T', 'S'),
	ANIMATION_TYPE_VISIBILITY = MAKE_FOURCC('R', 'L', 'V', 'I'),
	ANIMATION_TYPE_VERTEX_COLOR = MAKE_FOURCC('R', 'L', 'V', 'C'),
	ANIMATION_TYPE_MATERIAL_COLOR = MAKE_FOURCC('R', 'L', 'M', 'C'),
	ANIMATION_TYPE_TEXTURE_PALETTE = MAKE_FOURCC('R', 'L', 'T', 'P'),
	ANIMATION_TYPE_IND_MATERIAL = MAKE_FOURCC('R', 'L', 'I', 'M'),
};

struct RLAN_Header
{
	u32 magic;
	u16 endian;
	u16 version;
	u32 file_size;
	u16 offset;
	u16 section_count;
} __attribute__((packed));

struct PAI1_Header
{
	u32 magic;
	u32 section_size;
	u16 frame_count;
	u8 loop;
	u8 pad;
	u16 file_count;
	u16 animator_count;
	u32 entry_offset;
} __attribute__((packed));

struct AnimatorHeader
{
	char name[20];
	u8 tag_count;
	u8 is_material;
	u16 apad;
} __attribute__((packed));

struct Anim_Header
{
	u32 animation_type;
	u8 frame_count;
	u8 pad[3];
} __attribute__((packed));

struct KeyFrame_Header
{
	u8 index;
	u8 target;
	u8 data_type;
	u8 pad;
	u16 key_count;
	u16 pad1;
	u32 offset;
} __attribute__((packed));

struct KeyType
{
	KeyType(AnimationType _type, u8 _index, u8 _target)
		: type(_type)
		, index(_index)
		, target(_target)
	{}

	bool operator<(const KeyType& rhs) const
	{
		return memcmp(this, &rhs, sizeof(*this)) < 0;
	}

	const AnimationType type;
	const u8 index, target;
};

class StepKeyHandler
{
public:
	void Load(const u8* file, u16 count);

	struct KeyData
	{
		u8 data1, data2;
	};

	KeyData GetFrame(FrameNumber frame_number) const;

private:
	std::map<FrameNumber, KeyData> keys;
};

class HermiteKeyHandler
{
public:
	void Load(const u8* file, u16 count);

	struct KeyData
	{
		float value, slope;
	};

	float GetFrame(FrameNumber frame_number) const;

private:
	std::multimap<FrameNumber, KeyData> keys;
};

class Layout;
class Animator
{
public:
	static const u32 MAGIC_ANIMATION = MAKE_FOURCC('R', 'L', 'A', 'N');
	static const u32 MAGIC_PANE_ANIMATION_INFO = MAKE_FOURCC('p', 'a', 'i', '1');

	static u32 LoadAnimators(const RLAN_Header *header, Layout& layout, u8 key_set);

	void LoadKeyFrames(const u8 *file, u8 tag_count, u32 offset, u8 key_set);
	virtual void SetFrame(FrameNumber frame, u8 key_set);

protected:
	Animator() {}

	virtual void ProcessHermiteKey(const KeyType& type, float value);
	virtual void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);
private:

	struct
	{
		std::map<KeyType, StepKeyHandler> step_keys;
		std::map<KeyType, HermiteKeyHandler> hermite_keys;
	} keys[2];
};

#endif
