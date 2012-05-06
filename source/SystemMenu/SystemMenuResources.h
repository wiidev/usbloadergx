/****************************************************************************
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef SYSTEMMENURESOURCES_H
#define SYSTEMMENURESOURCES_H

#include <gctypes.h>
#include "BannerFrame.h"
#include "StaticFrame.h"
#include "utils/U8Archive.h"

class SystemMenuResources
{
public:
	static SystemMenuResources *Instance() { if(!instance) instance = new SystemMenuResources; return instance; }

	static void DestroyInstance() { delete instance; instance = NULL; }

	bool Init();
	bool IsLoaded() const { return isInited; }

	WiiFont *GetWbf1() const { return wbf1; }
	WiiFont *GetWbf2() const { return wbf2; }

	const u8 *GetChanSelAsh() const { return chanSelAsh; }
	u32 GetChanSelAshSize() const { return chanSelAshSize; }

	const u8 *GetChanTtlAsh() const { return chanTtlAsh; }
	u32 GetChanTtlAshSize() const { return chanTtlAshSize; }

	const u8 *GetGCBannAsh() const { return GCBannAsh; }
	u32 GetGCBannAshSize() const { return GCBannAshSize; }

	const u8 *GetSystemFont() const { return systemFont; }
	u32 GetSystemFontSize() const { return systemFontSize; }

protected:
	SystemMenuResources();
	~SystemMenuResources();

	static SystemMenuResources *instance;

	bool InitFontArchive(void);
	bool InitSystemFontArchive(bool korean, u8 *contentMap, u32 mapsize);

	bool isInited;

	//! banner font
	WiiFont *wbf1;
	WiiFont *wbf2;
	u8 *wbf1Buffer;
	u8 *wbf2Buffer;

	//! chanTtl.ash contains the frame and buttons that goes around the large banners
	u8 *chanTtlAsh;
	u32 chanTtlAshSize;

	//! chanSel.ash contains the channel grid layouts
	u8 *chanSelAsh;
	u32 chanSelAshSize;

	//! GCBann.ash contains the gamecube channel banner
	u8 *GCBannAsh;
	u32 GCBannAshSize;

	//! system font
	u8 *systemFont;
	u32 systemFontSize;

	// free data
	void FreeEverything();
};

#endif // SYSTEMMENURESOURCES_H
