/*
 CACHE.h
 The CACHE is not visible to the user. It should be flushed
 when any file is closed or changes are made to the filesystem.

 This CACHE implements a least-used-page replacement policy. This will
 distribute sectors evenly over the pages, so if less than the maximum
 pages are used at once, they should all eventually remain in the CACHE.
 This also has the benefit of throwing out old sectors, so as not to keep
 too many stale pages around.

 Copyright (c) 2006 Michael "Chishm" Chisholm
 Copyright (c) 2009 shareese, rodries

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _DISC_CACHE_H
#define _DISC_CACHE_H

#include <stddef.h>
#include <stdint.h>
#include <gctypes.h>
#include <ogc/disc_io.h>
#include <gccore.h>

typedef struct
{
	sec_t           sector;
	unsigned int    count;
	u64             last_access;
	bool            dirty;
	u8*             cache;
} CACHE_ENTRY;

typedef struct
{
	const DISC_INTERFACE* disc;
	sec_t		          endOfPartition;
	unsigned int          numberOfPages;
	unsigned int          sectorsPerPage;
	sec_t                 sectorSize;
	CACHE_ENTRY*     cacheEntries;
} CACHE;

/*
Read data from a sector in the CACHE
If the sector is not in the CACHE, it will be swapped in
offset is the position to start reading from
size is the amount of data to read
Precondition: offset + size <= BYTES_PER_READ
*/
/*
Write data to a sector in the CACHE
If the sector is not in the CACHE, it will be swapped in.
When the sector is swapped out, the data will be written to the disc
offset is the position to start writing to
size is the amount of data to write
Precondition: offset + size <= BYTES_PER_READ
*/

/*
Write data to a sector in the CACHE, zeroing the sector first
If the sector is not in the CACHE, it will be swapped in.
When the sector is swapped out, the data will be written to the disc
offset is the position to start writing to
size is the amount of data to write
Precondition: offset + size <= BYTES_PER_READ
*/

/*
Read several sectors from the CACHE
*/
bool cache_readSectors (CACHE* DISC_CACHE, sec_t sector, sec_t numSectors, void* buffer);

/*
Read a full sector from the CACHE
*/
/*
Write a full sector to the CACHE
*/
bool cache_writeSectors (CACHE* DISC_CACHE, sec_t sector, sec_t numSectors, const void* buffer);

/*
Write any dirty sectors back to disc and clear out the contents of the CACHE
*/
bool cache_flush (CACHE* DISC_CACHE);

/*
Clear out the contents of the CACHE without writing any dirty sectors first
*/
void cache_invalidate (CACHE* DISC_CACHE);

CACHE* cache_constructor (unsigned int numberOfPages, unsigned int sectorsPerPage, const DISC_INTERFACE* discInterface, sec_t endOfPartition, sec_t sectorSize);

void cache_destructor (CACHE* DISC_CACHE);

#endif // _CACHE_H

