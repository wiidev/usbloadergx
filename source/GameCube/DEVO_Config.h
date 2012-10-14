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
#ifndef DEVO_CONFIG_H_
#define DEVO_CONFIG_H_

#define LAUNCH_DEVO() ((void(*)(void))loader_bin)()

#define DEVO_SIG				 0x3EF9DB23
#define DEVO_CONFIG_VERSION		 0x0110

enum DEVOConfig
{
	DEVO_CFG_WIFILOG		= (1<<0), // added in Devo config version 0x0110
	DEVO_CFG_WIDE			= (1<<1), // added in Devo r142
	DEVO_CFG_NOLED			= (1<<2)
};

typedef struct _DEVO_CFG
{
		u32 signature;					//0x3EF9DB23
		u16 version;					//0x00000110
		u16 device_signature;
		u32 memcard_cluster;
		u32 disc1_cluster;
		u32 disc2_cluster;
		u32 options;					// added in Devo config version 0x0110
} DEVO_CGF;

#endif
