/*
 *  brlan.h
 *  BannerPlayer
 *
 *  Created by Alex Marshall on 09/03/16.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _BRLAN_H_
#define _BRLAN_H_

#include <gctypes.h>


typedef char fourcc[4];

#define le16	be16
#define le32	be32
#define le64	be64

u16 be16(u16 x);
u32 be32(u32 x);
u64 be64(u64 x);

typedef enum
{
	RLPA_ENTRY	= 0,
	RLTS_ENTRY	= 1,
	RLVI_ENTRY	= 2,
	RLVC_ENTRY	= 3,
	RLMC_ENTRY	= 4,
	RLTP_ENTRY	= 5
} brlan_entry_type;

typedef struct
{
	fourcc		magic;			// "pai1" in ASCII.
	u32		size;			// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u16		framesize;		// Framesize
	u8		flags;			// Flags
	u8		unk1;			// Unknown
	u16		num_timgs;		// Number of timgs?
	u16		num_entries;		// Number of tags in the brlan.
	u32		unk2;			// Only if bit 25 of flags is set.
	u32		entry_offset;		// Offset to entries. (Relative to start of pai1 header.)
} brlan_pai1_header_type2;

typedef struct
{
	fourcc		magic;			// "RLAN" in ASCII.
	u32		unk1;			// Always 0xFEFF 0x0008. Possibly a versioning string.
	u32		file_size;		// Size of whole file, including the header.
	u16		pai1_offset;		// The offset to the pai1 header from the start of file.
	u16		pai1_count;		// How many pai1 sections there are (duh, only 1... wtf?)
} brlan_header;

typedef struct
{
	fourcc		magic;			// "pai1" in ASCII.
	u32		size;			// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u16		framesize;		// Framesize
	u8		flags;			// Flags
	u8		unk1;			// Unknown
	u16		num_timgs;		// Number of timgs?
	u16		num_entries;		// Number of tags in the brlan.
} brlan_pai1_universal;

typedef struct
{
	fourcc		magic;			// "pai1" in ASCII.
	u32		size;			// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u16		framesize;		// Framesize
	u8		flags;			// Flags
	u8		unk1;			// Unknown
	u16		num_timgs;		// Number of timgs?
	u16		num_entries;		// Number of tags in the brlan.
	u32		entry_offset;		// Offset to entries. (Relative to start of pai1 header.)
} brlan_pai1_header_type1;

typedef struct
{
	char		name[20];		// Name of the BRLAN entry. (Must be defined in the BRLYT)
	u32		flags;			// Flags? (If bit 25 is set, we have another u32 after the entry. It's use is unknown.)
	u32		anim_header_len;	// Length of the animation header which is directly after this entry.
} brlan_entry;

typedef struct
{
	fourcc		magic;
	u8		entry_count;		// How many entries in this chunk.
	u8		pad1;			// All cases I've seen is zero.
	u8		pad2;			// All cases I've seen is zero.
	u8		pad3;			// All cases I've seen is zero.
} tag_header;

typedef struct
{
	u32		offset;			// Offset to the data pointed to by this entry.
	// Relative to the start of the RLPA header.
} tag_entry;

typedef struct
{
	u16		type;			// Type (look at animtypes)
	u16		unk1;			// ??? Every case has been 0x0200
	u16		coord_count;		// How many coordinates.
	u16		pad1;			// All cases I've seen is zero.
	u32		unk2;			// ??? In every case I've seen, it is 0x0000000C.
} tag_entryinfo;

typedef struct
{						// Bits not listed here are currently unknown.
	u32		part1;			// If Bit 9 is set in flags, this is an f32, with a coordinate. (Bit 17 seems to act the same)
	u32		part2;			// If Bit 16 is set in flags, this is an f32, with another coordinate. (Bit 17 seems to act the same)
	u32		part3;			// With Bit 16 set in flags, this seems to be yet another coordinate. (Bit 17 seems to act the same)
} tag_data;

typedef struct BRLAN_trip
{
	f32		frame;		// Frame number.
	f32		value;		// Value at the frame.
	f32		blend;		// Interpolation value.
} BRLAN_triplets;

typedef struct BRLAN_entr
{
	u16		animtype;	// What subtype of animation.
	u16		tripletcount;	// Number of triplets.
	BRLAN_triplets	triplets[20];	// Shouldn't ever be more than 20.
} BRLAN_entries;

typedef struct BRLAN_anims
{
	char		type[4];	// The type of animation (FourCC from BRLAN file)
	char		name[20];	// Name.
	u32		offset;		// Offset into the BRLAN file to find this animation.
	u16		entrycount;	// How many entries.
	BRLAN_entries	entries[20];	// The entries. Shouldn't ever be more than 20.
} BRLAN_animation;

void BRLAN_Initialize(char rootpath[]);
u16 BRLAN_ReadAnimations(BRLAN_animation **anims);
void BRLAN_Finish();

#endif //_BRLAN_H_
