/****************************************************************************
 * Copyright (C) 2015 Cyan
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
#ifndef NEEK_CONFIG_H_
#define NEEK_CONFIG_H_

#include <gctypes.h>

#define NEEK_MAGIC					0x666c6f77 // 'FLOW'. 0x47414d45 'GAME' autoboot disc?
#define NEEK_CONFIG_ADDRESS			0x81200000

typedef struct _NEEK_CFG
{
	u32		magic;			// always has to be 0x666c6f77
	u64		titleid;		// the full path of the NAND title to boot (both folder names)
	u32		config;			// see below
	u64		returnto;		// same as titleid above
	u32		gameid;			// game for DI to autoboot use 4-digit game id
	u32		gamemagic;		// set to 0x5d1c9ea3 for Wii game, 0xC2339F3D for gamecube games
	char	dipath[256];	// string specifying path DI should use to find games (/wbfs/ or usb1/wbfs?)
	char	nandpath[256];	// string specifying where the emuNAND is stored if it's not in the normal place. (/nands/ or usb1/nands/ ?)
} NEEK_CFG;

enum neekconfig
{
	NCON_EXT_DI_PATH		= (1<<0), // 1 if you're using/specifying dipath
	NCON_EXT_NAND_PATH		= (1<<1), // 1 if you're using/specifying nandpath
	NCON_HIDE_EXT_PATH		= (1<<2), // 1 to have it not SAVE the last used custom path?
	NCON_EXT_RETURN_TO		= (1<<3), // 1 if you're using returnto
	NCON_EXT_BOOT_GAME		= (1<<4), // 1 if you're using gameid
};

#define NANDCONFIG_MAXNAND 8
#define NANDCONFIG_HEADER_SIZE 0x10
#define NANDCONFIG_NANDINFO_SIZE 0x100

typedef struct
{
	char	Path[128];
	char	Name[64];
	char	DiPath[64];
} NandInfo;

typedef struct _NandConfig
{
	u32			NandCnt;
	u32			NandSel;
	u32			Padding1;
	u32			Padding2;
	NandInfo	Nands[];
} NandConfig;



#ifdef __cplusplus
extern "C"
{
#endif


bool neekLoadKernel(const char* nandpath);
int neekBoot(void);

int neekIsNeek2o(const char* nandpath);
int neekPathFormat(char* nandpath_out, const char* nandpath_in, u32 len);
bool neek2oSetBootSettings(NEEK_CFG* neek_config, u64 TitleID, u32 Magic, u64 returnto, const char* nandpath );
int neek2oSetNAND(const char* nandpath);

#ifdef __cplusplus
}
#endif



#endif
