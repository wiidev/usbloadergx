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
#ifndef DML_CONFIG_H_
#define DML_CONFIG_H_

#include <gctypes.h>

#define DML_CONFIG_ADDRESS			0x80001700
#define DML_CONFIG_ADDRESS_V1_2		0x81200000
#define DML_MAGIC					0xD1050CF6
#define DML_VERSION					0x00000002

enum DMLConfig
{
	DML_CFG_CHEATS			= (1<<0),
	DML_CFG_DEBUGGER		= (1<<1),
	DML_CFG_DEBUGWAIT		= (1<<2),
	DML_CFG_NMM				= (1<<3),
	DML_CFG_NMM_DEBUG		= (1<<4),
	DML_CFG_GAME_PATH		= (1<<5),
	DML_CFG_CHEAT_PATH		= (1<<6),
	DML_CFG_ACTIVITY_LED	= (1<<7),
	DML_CFG_PADHOOK			= (1<<8),
	DML_CFG_NODISC			= (1<<9),	// unused since DML v1.0, removed in v2.1
	DML_CFG_FORCE_WIDE		= (1<<9),	// DM v2.1+, Config v02
	DML_CFG_BOOT_DISC		= (1<<10),
	// DML_CFG_BOOT_DOL		= (1<<11),	// unused since DML v1.0, removed in v2.1
	DML_CFG_BOOT_DISC2		= (1<<11),	// added in DM v2.1+, Config v02, used in v2.6+
	DML_CFG_NODISC2			= (1<<12),	// added back in DM v2.2 update2 (r20) and removed in v2.3
	DML_CFG_SCREENSHOT		= (1<<13),	// added in v2.5
};

enum DMLVideoModes
{
	DML_VID_DML_AUTO		= (0<<16),
	DML_VID_FORCE			= (1<<16),
	DML_VID_NONE			= (2<<16),

	DML_VID_FORCE_PAL50		= (1<<0),
	DML_VID_FORCE_PAL60		= (1<<1),
	DML_VID_FORCE_NTSC		= (1<<2),
	DML_VID_FORCE_PROG		= (1<<3),
	DML_VID_PROG_PATCH		= (1<<4)

};

typedef struct _DML_CFG
{
	u32		Magicbytes;			// 0xD1050CF6
	u32		Version;			// 0x00000002
	u32		VideoMode;
	u32		Config;
	char	GamePath[255];
	char	CheatPath[255];
} DML_CFG;

#endif
