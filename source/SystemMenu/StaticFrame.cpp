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
#include "StaticFrame.h"

bool StaticFrame::Load( const U8Archive &chanSelArc )
{
	// read layout data
	const u8 *brlytFile = chanSelArc.GetFile("/arc/blyt/my_IplTop_b.brlyt");
	if( !brlytFile )
		return false;

	if(!Layout::Load(brlytFile))
		return false;

	u32 length_start = 0, length_loop = 0;

	const u8 *brlan_loop = chanSelArc.GetFile("/arc/anim/my_IplTop_b.brlan");
	if (brlan_loop)
		length_loop = Animator::LoadAnimators((const RLAN_Header *)brlan_loop, *this, 1);

	LoadTextures(chanSelArc);
	SetLanguage("ENG");
	SetLoopStart(length_start);
	SetLoopEnd(length_start + length_loop);
	SetFrame(0);

	return true;
}
