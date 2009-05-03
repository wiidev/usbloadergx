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
// Globals
u32 hooktype;
int patched;
u8 configbytes[2];
u32 regionfree;

// Function prototypes
void dogamehooks(void *addr, u32 len);
void langpatcher(void *addr, u32 len);
void vidolpatcher(void *addr, u32 len);
void patchdebug(void *addr, u32 len);



#ifdef __cplusplus
}
#endif

#endif // __PATCHCODE_H__
