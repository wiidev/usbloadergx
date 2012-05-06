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

#ifndef WII_BNR_PANE_H_
#define WII_BNR_PANE_H_

#include <vector>
#include <gccore.h>

#include "Animator.h"

struct BannerResources;

class Pane;
typedef std::vector<Pane*> PaneList;

class Pane : public Animator
{
public:
	typedef Animator Base;

	static const u32 MAGIC = MAKE_FOURCC('p', 'a', 'n', '1');

	struct Header
	{
	  u32 magic;
	  u32 size_section;
	  u8 flags;
	  u8 origin;
	  u8 alpha;
	  u8 padding;
	  char name [0x10];
	  char user_data [0x08];
	  Vec3f translate;
	  Vec3f rotate;
	  Vec2f scale;
	  float width;
	  float height;
	} __attribute__((packed));

	Pane() : header(NULL) {}
	virtual ~Pane();

	void Load(Pane::Header *file);

	const char *getName() const { if(!header) return ""; return header->name; }

	void Render(const BannerResources& resources, u8 parent_alpha, Mtx &modelview,
				bool widescreen, bool modify_alpha = false) const;

	void SetFrame(FrameNumber frame, u8 key_set);

	void SetScale(float scale) { if(header) header->scale.x = header->scale.y = scale; }

	bool GetHide() const { return hide; }
	void SetHide(bool _hide) { hide = _hide; }

	bool GetVisible() const { if(!header) return false; return ((header->flags & (1 << FLAG_VISIBLE)) != 0); }
	void SetVisible(bool visible)
	{
		if(!header)
			return;

		if(visible)
			header->flags |= (1 << FLAG_VISIBLE);
		else
			header->flags &= ~(1 << FLAG_VISIBLE);
	}

	u8 GetOriginX() const { return AlignHor; }
	u8 GetOriginY() const { return AlignVer; }

	float GetWidth() const { if(!header) return 0.f; return header->width; }
	float GetHeight() const { if(!header) return 0.f; return header->height; }

	bool GetInfluencedAlpha() const { if(!header) return false; return ((header->flags & (1 << FLAG_INFLUENCED_ALPHA)) != 0); }
	void SetInfluencedAlpha(bool influenced)
	{
		if(!header)
			return;

		if(influenced)
			header->flags |= (1 << FLAG_INFLUENCED_ALPHA);
		else
			header->flags &= ~(1 << FLAG_INFLUENCED_ALPHA);
	}

	bool GetWidescren() const { return ((header->flags & (1 << FLAG_WIDESCREEN)) != 0); }

	Pane* FindPane(const std::string& name);	// recursive

	PaneList panes;

protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);

private:
	virtual void Draw(const BannerResources&, u8, const float, Mtx&) const {}

	enum
	{
		FLAG_VISIBLE = 0x00,
		FLAG_INFLUENCED_ALPHA = 0x01,
		FLAG_WIDESCREEN = 0x02
	};

	Pane::Header *header;
	bool hide;	// used by the groups
	bool RootPane;
	u8 AlignVer;
	u8 AlignHor;
};

// apparently Bounding is just a regular pane
class Bounding : public Pane
{
public:
	static const u32 MAGIC = MAKE_FOURCC('b', 'n', 'd', '1');
};

#endif
