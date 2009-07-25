/*
 *  brlan.c
 *  BannerPlayer
 *
 *  Created by Alex Marshall on 09/03/16.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "brlan.h"
//#include "endian.h"

char* BRLAN_rootpath = NULL;
u8*   BRLAN_file = NULL;
u32   BRLAN_filesize = 0;
u8*   BRLAN_startfile = NULL;
u32   BRLAN_startfilesize = 0;
int   BRLAN_looping = 0;

void BRLAN_Initialize(char rootpath[])
{
	char brlanpath[256];
	sprintf(brlanpath, "%s/arc/anim/banner.brlan", rootpath);
	FILE* fp = fopen(brlanpath, "rb");
	if(fp == NULL) {
		fprintf(stderr, "Couldn't find banner.brlan. Let's assume it's a looping banner then.\n");
		BRLAN_looping = 1;
		sprintf(brlanpath, "%s/arc/anim/banner_start.brlan", rootpath);
		fp = fopen(brlanpath, "rb");
		if(fp == NULL) {
			fprintf(stderr, "No BRLAN file found. Quitting.\n");
			exit(1);
		}
		fseek(fp, 0, SEEK_END);
		BRLAN_startfilesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fread(BRLAN_startfile, BRLAN_startfilesize, 1, fp);
		fclose(fp);
		sprintf(brlanpath, "%s/arc/anim/banner_loop.brlan", rootpath);
		fp = fopen(brlanpath, "rb");
		if(fp == NULL) {
			fprintf(stderr, "No BRLAN file found. Quitting.\n");
			exit(1);
		}
	}else
		BRLAN_looping = 0;
	fseek(fp, 0, SEEK_END);
	BRLAN_filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(BRLAN_file, BRLAN_filesize, 1, fp);
	fclose(fp);
}

#define MAXIMUM_TAGS_SIZE		(0x1000)

fourcc tag_FourCCs[] = { "RLPA", "RLTS", "RLVI", "RLVC", "RLMC", "RLTP" };

static size_t BRLAN_fileoffset = 0;

static void BRLAN_ReadDataFromMemoryX(void* destination, void* input, size_t size)
{
	u8* out = (u8*)destination;
	u8* in = ((u8*)input) + BRLAN_fileoffset;
	memcpy(out, in, size);
}

static void BRLAN_ReadDataFromMemory(void* destination, void* input, size_t size)
{
	BRLAN_ReadDataFromMemoryX(destination, input, size);
	BRLAN_fileoffset += size;
}

static void CreateGlobal_pai1(brlan_pai1_header_type2 *pai1_header, brlan_pai1_header_type1 pai1_header1,
			      brlan_pai1_header_type2 pai1_header2, int pai1_header_type)
{
	if(pai1_header_type == 1) {
		pai1_header->magic[0]		= pai1_header1.magic[0];
		pai1_header->magic[1]		= pai1_header1.magic[1];
		pai1_header->magic[2]		= pai1_header1.magic[2];
		pai1_header->magic[3]		= pai1_header1.magic[3];
		pai1_header->size		= pai1_header1.size;
		pai1_header->framesize		= pai1_header1.framesize;
		pai1_header->flags		= pai1_header1.flags;
		pai1_header->unk1		= pai1_header1.unk1;
		pai1_header->num_timgs		= pai1_header1.num_timgs;
		pai1_header->num_entries	= pai1_header1.num_entries;
		pai1_header->unk2		= 0;
		pai1_header->entry_offset	= pai1_header1.entry_offset;
	}else
		memcpy(pai1_header, &pai1_header2, sizeof(brlan_pai1_header_type2));
}

static int FourCCsMatch(fourcc cc1, fourcc cc2)
{
	if((cc1[0] == cc2[0]) && (cc1[1] == cc2[1]) && (cc1[2] == cc2[2]) && (cc1[3] == cc2[3]))
		return 1;
	else
		return 0;
}

static int FourCCInList(fourcc cc)
{
	int i;
	for(i = 0; i < 6; i++)
		if(FourCCsMatch(cc, tag_FourCCs[i])) return 1;
	return 0;
}

static void ReadTagFromBRLAN(BRLAN_animation **anims, int idx)
{
	int taghead_location = BRLAN_fileoffset;
	tag_header head;
	tag_entry* entries;
	tag_entryinfo* entryinfo;
	BRLAN_ReadDataFromMemory(&head, BRLAN_file, sizeof(tag_header));
	memcpy(anims[idx]->type, head.magic, 4);
	anims[idx]->entrycount = head.entry_count;
	int i, z;
	entries = (tag_entry*)calloc(anims[idx]->entrycount, sizeof(tag_entry));
	entryinfo = (tag_entryinfo*)calloc(anims[idx]->entrycount, sizeof(tag_entryinfo));
	for(i = 0; i < anims[idx]->entrycount; i++) {
		BRLAN_ReadDataFromMemory(&entries[i], BRLAN_file, sizeof(tag_entry));
	}
	for(i = 0; i < anims[idx]->entrycount; i++) {
		BRLAN_fileoffset = be32(entries[i].offset) + taghead_location;
		BRLAN_ReadDataFromMemory(&entryinfo[i], BRLAN_file, sizeof(tag_entryinfo));
		anims[idx]->entries[i].animtype = be16(entryinfo[i].type);
		anims[idx]->entries[i].tripletcount = be16(entryinfo[i].coord_count);
		for(z = 0; z < be16(entryinfo[i].coord_count); z++) {
			BRLAN_ReadDataFromMemory(&anims[idx]->entries[i].triplets[z], BRLAN_file, sizeof(f32) * 3);
			anims[idx]->entries[i].triplets[z].frame = le32(anims[idx]->entries[i].triplets[z].frame);
			anims[idx]->entries[i].triplets[z].value = le32(anims[idx]->entries[i].triplets[z].value);
			anims[idx]->entries[i].triplets[z].blend = le32(anims[idx]->entries[i].triplets[z].blend);
		}
	}
	free(entries);
	free(entryinfo);
}

u16 BRLAN_ReadAnimations(BRLAN_animation **anims)
{
	BRLAN_fileoffset = 0;
	brlan_header header;
	BRLAN_ReadDataFromMemoryX(&header, BRLAN_file, sizeof(brlan_header));
	BRLAN_fileoffset = be16(header.pai1_offset);
	brlan_pai1_universal universal;
	BRLAN_ReadDataFromMemoryX(&universal, BRLAN_file, sizeof(brlan_pai1_universal));

	int pai1_header_type;
	brlan_pai1_header_type1 pai1_header1;
	brlan_pai1_header_type2 pai1_header2;
	brlan_pai1_header_type2 pai1_header;

	if((be32(universal.flags) & (1 << 25)) >= 1) {
		pai1_header_type = 2;
		BRLAN_ReadDataFromMemory(&pai1_header2, BRLAN_file, sizeof(brlan_pai1_header_type2));
	} else {
		pai1_header_type = 1;
		BRLAN_ReadDataFromMemory(&pai1_header1, BRLAN_file, sizeof(brlan_pai1_header_type1));
	}

	CreateGlobal_pai1(&pai1_header, pai1_header1, pai1_header2, pai1_header_type);

	int tagcount = be16(pai1_header.num_entries);
	u32 *taglocations = (u32*)calloc(tagcount, sizeof(u32));
	*anims = (BRLAN_animation*)calloc(tagcount, sizeof(BRLAN_animation));
	fourcc CCs[256];
	memset(CCs, 0, 256*4);
	BRLAN_fileoffset = be32(pai1_header.entry_offset) + be16(header.pai1_offset);
	BRLAN_ReadDataFromMemory(taglocations, BRLAN_file, tagcount * sizeof(u32));
	int animcnt = 1;
	int i;
	for(i = 0; i < tagcount; i++) {
		BRLAN_fileoffset = be32(taglocations[i]) + be16(header.pai1_offset);
		brlan_entry tmpentry;
		BRLAN_ReadDataFromMemory(&tmpentry, BRLAN_file, sizeof(brlan_entry));
		if((be32(tmpentry.flags) & (1 << 25)) >= 1)
			BRLAN_fileoffset += sizeof(u32);
		memcpy(anims[animcnt]->name, tmpentry.name, 20);
		fourcc magick;
		BRLAN_ReadDataFromMemoryX(magick, BRLAN_file, 4);
		memcpy(CCs[i], magick, 4);
		if(FourCCInList(CCs[i]) == 1) {
			anims[animcnt]->offset = BRLAN_fileoffset;
			ReadTagFromBRLAN(anims, animcnt);
			animcnt++;
		}
	}
	return tagcount;
}

void BRLAN_Finish()
{
	if(BRLAN_file != NULL)
		free(BRLAN_file);
	if(BRLAN_startfile != NULL)
		free(BRLAN_startfile);
}












