/****************************************************************************
 * Copyright (C) 2013 Cyan
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
#ifndef NIN_CONFIG_H_
#define NIN_CONFIG_H_

#include <gctypes.h>

#define NIN_MAGIC					0x01070CF6
#define NIN_CFG_VERSION				0x00000008

typedef struct NIN_CFG 
{
	u32		Magicbytes;		// 0x01070CF6
	u32		Version;		// v4 since v3.354, v5 since v3.358, v6 since v3.368, v7 since 4.424, v8 since 4.431
	u32		Config;
	u32		VideoMode;
	u32		Language;
	char	GamePath[255];
	char	CheatPath[255];
	u32		MaxPads;		// added in r42 - cfg version 2
	u32		GameID;			// added in r83 - cfg version 2
	union
	{
		u32 MemCardBlocks;	// added in v1.135 - cfg version 3 - u32 in v3, Char in v4
		struct
		{
			char		MemCardBlocksV4;// replaced in v3.354 - cfg version 4 - from u32 in v3 to Char in v4
			char		VideoScale;		// added in v3.354 - cfg version 4
			char		VideoOffset;	// added in v3.354 - cfg version 4
			char		Unused; 		// added in v3.354 - cfg version 4
		};
	};
} NIN_CFG;

enum ninconfig
{
	NIN_CFG_CHEATS		= (1<<0),
	NIN_CFG_DEBUGGER	= (1<<1),	// Only for Wii Version
	NIN_CFG_DEBUGWAIT	= (1<<2),	// Only for Wii Version
	NIN_CFG_MEMCARDEMU	= (1<<3),
	NIN_CFG_CHEAT_PATH	= (1<<4),
	NIN_CFG_FORCE_WIDE	= (1<<5),
	NIN_CFG_FORCE_PROG	= (1<<6),
	NIN_CFG_AUTO_BOOT	= (1<<7),
	NIN_CFG_HID			= (1<<8),	// Unused since v3.304
	NIN_CFG_REMLIMIT	= (1<<8),	// v3.358 cfg version 5
	NIN_CFG_OSREPORT	= (1<<9),
	NIN_CFG_USB			= (1<<10),	// r40
	NIN_CFG_LED			= (1<<11),	// v1.45
	NIN_CFG_LOG			= (1<<12),	// v1.109
	NIN_CFG_MC_MULTI	= (1<<13),	// v1.135
	NIN_CFG_NATIVE_SI	= (1<<14),	// v2.189
	NIN_CFG_WIIU_WIDE	= (1<<15),	// v2.258
	NIN_CFG_ARCADE_MODE = (1<<16),	// v4.424
	NIN_CFG_CC_RUMBLE	= (1 << 17),// v4.431 cfg version 8
	NIN_CFG_SKIP_IPL	= (1 << 18),// v4.435
};

enum ninvideomode
{
	NIN_VID_AUTO		= (0<<16),
	NIN_VID_FORCE		= (1<<16),
	NIN_VID_NONE		= (2<<16), // replaced by FORCE_DF in v2.200 - v2.207
	NIN_VID_FORCE_DF	= (4<<16), // v2.208+

	NIN_VID_MASK		= NIN_VID_AUTO|NIN_VID_FORCE|NIN_VID_NONE|NIN_VID_FORCE_DF,

	NIN_VID_FORCE_PAL50	= (1<<0),
	NIN_VID_FORCE_PAL60	= (1<<1),
	NIN_VID_FORCE_NTSC	= (1<<2),
	NIN_VID_FORCE_MPAL	= (1<<3),

	NIN_VID_FORCE_MASK	= NIN_VID_FORCE_PAL50|NIN_VID_FORCE_PAL60|NIN_VID_FORCE_NTSC|NIN_VID_FORCE_MPAL,

	NIN_VID_PROG		= (1<<4),
	NIN_VID_PATCH_PAL50 = (1<<5),		// v3.368 cfg version 6
};

enum ninlanguage
{
	NIN_LAN_ENGLISH		= 0,
	NIN_LAN_GERMAN		= 1,
	NIN_LAN_FRENCH		= 2,
	NIN_LAN_SPANISH		= 3,
	NIN_LAN_ITALIAN		= 4,
	NIN_LAN_DUTCH		= 5,

/* Auto will use English for E/P region codes and 
   only other languages when these region codes are used: D/F/S/I  */

	NIN_LAN_AUTO		= -1, 
};

// blocks  = value , internal code , file size/bytes
//Mem0059 = 0, 0x04, 0x0080000
//Mem0123 = 1, 0x08, 0x0100000
//Mem0251 = 2, 0x10, 0x0200000
//Mem0507 = 3, 0x20, 0x0400000
//Mem1019 = 4, 0x40, 0x0800000
//Mem2043 = 5, 0x80, 0x1000000
#define MEM_CARD_MAX (5)
#define MEM_CARD_CODE(x) (1<<(x+2))
#define MEM_CARD_SIZE(x) (1<<(x+19))
#define MEM_CARD_BLOCKS(x) ((1<<(x+6))-5)


#endif
