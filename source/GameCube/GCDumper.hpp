/***************************************************************************
 * Copyright (C) 2012
 * by OverjoY and FIX94 for Wiiflow
 *
 * Adjustments for USB Loader GX by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/

#ifndef GCDUMPER_H_
#define GCDUMPER_H_

#include <gctypes.h>
#include <vector>

using namespace std;

class GCDumper
{
public:
	GCDumper();
	~GCDumper();
	s32 InstallGame(const char *installpath, u32 game, const char *installedGamePath);
	s32 ReadDiscHeader(void);
	int ReadDiscInfo(const u64 &game_offset);
	void SetForceAlign(bool b) { force_align32 = b; }
	void SetCompressed(bool b) { compressed = b; }
	vector<struct discHdr> & GetDiscHeaders() { return discHeaders; }
	vector<u32> & GetDiscSizes() { return gameSizes; }
private:
	s32 CopyDiscData(FILE *f, u64 offset, u32 length, u8 *buffer);

	vector<struct discHdr> discHeaders;
	vector<u32> gameSizes;
	vector<u64> gameOffsets;
	bool force_align32;
	bool compressed;
	u32 discWrote;
	u32 discTotal;
	u8 *ReadBuffer;

	typedef struct
	{
		union
		{
			struct
			{
				u32 Type		:8;
				u32 NameOffset	:24;
			};
			u32 TypeName;
		};
		union
		{
			struct
			{
				u32 FileOffset;
				u32 FileLength;
			};
			struct
			{
				u32 ParentOffset;
				u32 NextOffset;
			};
			u32 entry[2];
		};
	} FST;
};
#endif
