/****************************************************************************
 * Copyright (C) 2012 giantpune
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
#ifndef ASH_H
#define ASH_H

#include <gctypes.h>

// check if data is ash compressed
bool IsAshCompressed( const u8 *stuff, u32 len );

// decompress ash compressed data
//! len is the size of the compressed data, and is set to the size of the decompressed data
//! this allocates memory with memalign, free it when you are done with it

u8*	DecompressAsh( const u8 *stuff, u32 &len );
#endif // ASH_H
