/****************************************************************************
 * Copyright (C) 2012 Dimok and giantpune
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
#ifndef U8ARCHIVE_H
#define U8ARCHIVE_H

#include <string>
#include <gccore.h>
#include "tools.h"

class U8Archive
{
public:
	U8Archive( const u8 *stuff, u32 len );

	// set the data used for this archive
	virtual void SetData( const u8 *stuff, u32 len );

	// returns a pointer to a file within the archive
	virtual u8* GetFile( const char *path, u32 *size = NULL ) const;
	virtual u8* GetFile( const std::string &path, u32 *size = NULL ) const;
	virtual u8* GetFile( u32 fstIdx, u32 *size = NULL ) const;

	// gets a file and copies it into a newly memalign()'d buffer
	//! if the data looks ASH or LZ77 compressed, it is decompressed
	virtual u8* GetFileAllocated( const char *path, u32 *size = NULL ) const;
	virtual u8* GetFileAllocated( const std::string &path, u32 *size = NULL ) const;
	virtual u8* GetFileAllocated( u32 fstIdx, u32 *size = NULL ) const;

	virtual u32 FileDescriptor( const char *path ) const;
	virtual u32 FileDescriptor( const std::string &path ) const{ return FileDescriptor( path.c_str() ); }
	virtual u8* GetFileFromFd( u32 fd, u32 *size = NULL )const;


protected:
	struct U8Header
	{
		u32 magic;
		u32 rootNodeOffset;
		u32 headerSize;
		u32 dataOffset;
		u8 zeroes[16];
	} __attribute__((packed));

	struct FstEntry
	{
		u8 filetype;
		char name_offset[3];
		u32 fileoffset;
		u32 filelen;
	} __attribute__((packed));

	FstEntry *fst;
	char *name_table;
	u8* data;

	u32 NextEntryInFolder( u32 current, u32 directory ) const ;
	s32 EntryFromPath( const char *path, int d = 0 ) const ;
	char *FstName( const FstEntry *entry ) const;
	u8 *DecompressCopy( const u8 * stuff, u32 len, u32 *size ) const;

	static int strlen_slash( const char *s );

	static int strcasecmp_slash( const char *s1, const char *s2 );

	// lightweight toupper - because U8 archives are case-insinsitive
	static char toupper( char c )
	{
		if( c <= 'z' && c >= 'a' )
			return c - 0x20;
		return c;
	}

	// looks in some common offsets for a U8 tag
	static const u8* FindU8Tag( const u8* stuff, u32 len );
};

// class to access files from an archive that is saved on the nand
// GetFile() will return NULL, use GetfileAllocated()
class U8NandArchive : public U8Archive
{
public:
	U8NandArchive( const char* nandPath );
	~U8NandArchive();

	bool SetFile( const char* nandPath );


	// not implimented in this subclass...
	void SetData( const u8 *stuff, u32 len ){}
	u8* GetFile( const char *path, u32 *size = NULL ) const { return NULL; }
	u8* GetFile( const std::string &path, u32 *size = NULL ) const { return NULL; }
	u8* GetFileFromFd( u32 fd, u32 *size = NULL )const { return NULL; }


	// gets a file and copies it into a newly memalign()'d buffer
	//! if the data looks ASH or LZ77 compressed, it is decompressed
	u8* GetFileAllocated( const char *path, u32 *size = NULL ) const;

private:
	s32 fd;

	// where the U8 header starts within the file
	u32 dataOffset;
	void CloseFile()
	{
		if( fd >= 0 )
		{
			ISFS_Close( fd );
			fd = -1;
		}
	}
};

#endif // U8ARCHIVE_H
