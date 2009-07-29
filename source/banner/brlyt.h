/*
 *  brlyt.h
 *  Parses brlyt file
 *
 *  by nIxx
 *  http://wiibrew.org/wiki/Wii_Animations#Textures_and_Material_lists_.28.2A.brlyt.29 
 *
 */

#ifndef _BRLYT_H_
#define _BRLYT_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

int BRLYT_Initialize(const char *rootpath);

typedef struct BRLYT_objs
{
	char	type[4];	// The type of object (FourCC from BRLYT file)
	u32	offset;		// Offset into the BRLYT file to find this object.
} BRLYT_object;

int BRLYT_ReadObjects(BRLYT_object** objs);
void BRLYT_Finish();

typedef struct
{
  char sig [4]; // "RLYT" in ASCII
  u32 unk; // Always 0xFE 0xFF 0x 00 0x08.
  u32 file_size; // Size of whole file, including the header.
  u32 num;  // number of sections
} brlyt_header;

typedef struct
{
  char sig [4]; // "lyt1" in ASCII.
  u32 size_header;
  u32 unk2;
  u32 width;
  u32 height;
} lyt1_header;

typedef struct
{
  char sig [4]; // "txl1" in ASCII.
  u32 size_section; // Size of the whole section.
  u16 num_textures; // Number of textures in list.
  u16 unk2; // Always zero?
} txl1_header;

typedef struct
{
 u32 offset_filename; // Offset to a null-terminated ASCII string containing the filename.
                      // The offset-base should be after the txl1-header.
 u32 unk; // Always zero?
} txl1_offset;

typedef struct
{
  char sig [4]; // "mat1" in ASCII.
  u32 size_section; // // Size of the whole section.
  u16 num_materials; // 
  u16 size_header; // Offset to list start. Always zero
} mat1_header;

typedef struct
{
  u32 offset; // Offset from beginning of mat1-section.
} mat1_offset;

typedef struct
{
   char name[20];
   s16 tev_color[4];
   s16 unk_color[4];
   s16 unk_color_2[4];
   u32 tev_kcolor[4];
   u32 flags;
} mat1_material;

typedef struct
{
  char sig [4]; // "pan1" in ASCII.
  u32 size_section; 
  u32 unk; // Always 01 04 FF 00?
  char pane_name [0x18]; // Pane name in ASCII.
  float x;
  float y;
  float z;
  float xFlip;
  float yFlip;
  float zFlip;  //rotate
  float xMag;
  float yMag;
  float width;
  float height;
} pan1_header;

typedef struct
{
  char sig [4]; // "pas1" in ASCII.
  u32 size_section;
} pas1_header;



typedef struct
{
  char sig [4]; // "pic1" in ASCII.
  u32 size_section; 
  u16 flags;
  u16 alpha;
  char name[0x18];
  float coords[10]; // x, y, unk, unk, unk, angle, xmag, ymag, width, height.
} pic1_header;

typedef struct
{
  char sig [4]; // "pae1" in ASCII.
  u32 size_section;
} pae1_header;

typedef struct {
  char sig [4]; // "grp1" in ASCII.
  u32 size_section;
  char name[16];
  u16 numsubs;
  u16 unk1;
} grp1_header;

typedef struct
{
  char tplfilename[40]; 
} tpl_files;

#ifdef __cplusplus
}
#endif

#endif //_BRLYT_H_
