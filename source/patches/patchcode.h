/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PATCHCODE_H__
#define __PATCHCODE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_GCT_SIZE 2056

// Function prototypes
void dogamehooks(u32 hooktype, void *addr, u32 len);
void load_handler(u32 hooktype, u32 debugger, u32 pauseAtStart);
void langpatcher(void *addr, u32 len, u8 languageChoice);
void vidolpatcher(void *addr, u32 len);
void patchdebug(void *addr, u32 len);
int LoadGameConfig(const char *CheatFilepath);
int ocarina_load_code(const char *CheatFilepath, u8 *gameid);

#ifdef __cplusplus
}
#endif

#endif // __PATCHCODE_H__
