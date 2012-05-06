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

#include <math.h>
#include "Animator.h"
#include "Layout.h"

// load keyframes from a brlan file
u32 Animator::LoadAnimators(const RLAN_Header *header, Layout& layout, u8 key_set)
{
	u32 frame_count = 0;

	if (!header || header->magic != MAGIC_ANIMATION || header->endian != 0xFEFF || header->version != 0x0008)
		return 0;	// bad header

	// first section
	const u8 *position = ((const u8 *) header) + header->offset;

	for(u32 i = 0; i < header->section_count; ++i)
	{
		section_t *section = (section_t *) position;
		position += section->size;

		if (section->magic == MAGIC_PANE_ANIMATION_INFO)
		{
			const PAI1_Header *pai = (const PAI1_Header *) section;
			u16 animator_count = pai->animator_count;
			frame_count += pai->frame_count;

			// read animation file names
			const u32 *nameOffsets = (const u32 *)(pai + 1);
			for(u32 i = 0; i < pai->file_count; i++)
			{
				const char* name = (((const char *) nameOffsets) + nameOffsets[i]);
				layout.AddPalette(name, key_set);
			}

			const u32 *offsets = (const u32 *) (((const u8 *)section) + pai->entry_offset);
			// read each animator
			for(u32 n = 0; n < animator_count; n++)
			{
				const AnimatorHeader *animHdr = (const AnimatorHeader *) (((const u8 *)section) + offsets[n]);
				std::string anim_name(animHdr->name, 0, 20);

				Animator* animator = animHdr->is_material ?
						(Animator*) layout.FindMaterial(anim_name) :
						(Animator*) layout.FindPane(anim_name);

				if (animator)
					animator->LoadKeyFrames((const u8 *) animHdr, animHdr->tag_count, sizeof(AnimatorHeader), key_set);
			}
		}
		else
		{
			gprintf("Unknown: %c%c%c%c\n", position[0], position[1], position[2], position[3]);
		}
	}

	return frame_count;
}

void Animator::LoadKeyFrames(const u8 *file, u8 tag_count, u32 offset, u8 key_set)
{
	const u32 *tag_offsets = (const u32 *) (file + offset);

	for(u32 tag = 0; tag < tag_count; tag++)
	{
		const Anim_Header *animHdr = (const Anim_Header *) (file + tag_offsets[tag]);

		u32 animation_type = animHdr->animation_type;
		u8 frame_count = animHdr->frame_count;

		const u32 *frame_offsets = (const u32 *) (animHdr + 1);

		for(u32 frame = 0; frame < frame_count; frame++)
		{
			const KeyFrame_Header *keyFrame = (const KeyFrame_Header *)(((const u8 *) animHdr) + frame_offsets[frame]);
			const KeyType frame_type(static_cast<AnimationType>(animation_type), keyFrame->index, keyFrame->target);

			switch (keyFrame->data_type)
			{
				// step key frame
			case 0x01:
				keys[key_set].step_keys[frame_type].Load((const u8 *) (keyFrame+1), keyFrame->key_count);
				break;

				// hermite key frame
			case 0x02:
				keys[key_set].hermite_keys[frame_type].Load((const u8 *) (keyFrame+1), keyFrame->key_count);
				break;

			default:
				break;
			}
		}
	}
}

void Animator::SetFrame(FrameNumber frame_number, u8 key_set)
{
	std::map<KeyType, HermiteKeyHandler>::iterator itr;
	for(itr = keys[key_set].hermite_keys.begin(); itr != keys[key_set].hermite_keys.end(); itr++)
	{
		const KeyType& frame_type = itr->first;
		const float frame_value = itr->second.GetFrame(frame_number);

		ProcessHermiteKey(frame_type, frame_value);
	}

	std::map<KeyType, StepKeyHandler>::iterator itr2;
	for(itr2 = keys[key_set].step_keys.begin(); itr2 != keys[key_set].step_keys.end(); itr2++)
	{
		const KeyType& frame_type = itr2->first;
		StepKeyHandler::KeyData const frame_data = itr2->second.GetFrame(frame_number);

		ProcessStepKey(frame_type, frame_data);
	}
}

void StepKeyHandler::Load(const u8 *file, u16 count)
{
	while (count--)
	{
		FrameNumber frame;
		frame = *((FrameNumber *) file);
		file += 4;

		KeyData& data = keys[frame];
		data.data1 = *file;
		file++;
		data.data2 = *file;
		file++;

		file += 2;
	}
}

void HermiteKeyHandler::Load(const u8 *file, u16 count)
{
	while (count--)
	{
		std::pair<FrameNumber, KeyData> pair;

		// read the frame number, value and slope
		pair.first = *((FrameNumber *) file);
		file += 4;
		pair.second.value = *((float *) file);
		file += 4;
		pair.second.slope = *((float *) file);
		file += 4;

		keys.insert(pair);

		//std::cout << "\t\t\t" "frame: " << frame << ' ' << keys[frame] << '\n';
	}
}

StepKeyHandler::KeyData StepKeyHandler::GetFrame(FrameNumber frame_number) const
{
	// assuming not empty, a safe assumption currently

	// find the current frame, or the one after it
	std::map<FrameNumber, KeyData>::const_iterator frame_it = keys.lower_bound(frame_number);

	// current frame is higher than any keyframe, use the last keyframe
	if (keys.end() == frame_it)
		--frame_it;

	// if this is after the current frame and not the first keyframe, use the previous one
	if (frame_number < frame_it->first && keys.begin() != frame_it)
		--frame_it;

	return frame_it->second;
}

float HermiteKeyHandler::GetFrame(FrameNumber frame_number) const
{
	// assuming not empty, a safe assumption currently

	// find the current keyframe, or the one after it
	std::multimap<FrameNumber, KeyData>::const_iterator next = keys.lower_bound(frame_number);

	// current frame is higher than any keyframe, use the last keyframe
	if (keys.end() == next)
		--next;

	std::multimap<FrameNumber, KeyData>::const_iterator prev = next;

	// if this is after the current frame and not the first keyframe, use the previous one
	if (frame_number < prev->first && keys.begin() != prev)
		--prev;

	const float nf = next->first - prev->first;
	if (fabs(nf) < 0.01)
	{
		// same frame numbers, just return the first's value
		return prev->second.value;
	}
	else
	{
		// different frames, blend them together
		// this is a "Cubic Hermite spline" apparently

		frame_number =	 (frame_number < prev->first) ? prev->first :
						((frame_number > next->first) ? next->first : frame_number);

		const float t = (frame_number - prev->first) / nf;

		// old curve-less code
		//return prev->second.value + (next->second.value - prev->second.value) * t;

		// curvy code from marcan, :p
		return
			prev->second.slope * nf * (t + powf(t, 3) - 2 * powf(t, 2)) +
			next->second.slope * nf * (powf(t, 3) - powf(t, 2)) +
			prev->second.value * (1 + (2 * powf(t, 3) - 3 * powf(t, 2))) +
			next->second.value * (-2 * powf(t, 3) + 3 * powf(t, 2));
	}
}

void Animator::ProcessHermiteKey(const KeyType& type, float value)
{
//	std::cout << "unhandled key (" << GetName() << "): type: " << FourCC(type.type)
//	<< " index: " << (int)type.index
//	<< " target: " << (int)type.target
//	<< " value: " << value
//	<< '\n';
	gprintf("Animator::ProcessHermiteKey\n");
}

void Animator::ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data)
{
//	std::cout << "unhandled key (" << GetName() << "): type: " << FourCC(type.type)
//	<< " index: " << (int)type.index
//	<< " target: " << (int)type.target
//	<< " data:" << (int)data.data1 << " " << (int)data.data2
//	<< '\n';
	gprintf("Animator::ProcessStepKey\n");
}
