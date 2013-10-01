#ifndef _DISC_H_
#define _DISC_H_

#include <gccore.h> /* for define ATTRIBUTE_PACKED */

#ifdef __cplusplus
extern "C"
{
#endif

	/* Disc header structure */
	struct discHdr
	{
			/* Game ID */
			u8 id[6];

			/* Game Disc number */
			u8 disc_no;

			/* Game version */
			u8 disc_ver;

			/* Audio streaming */
			u8 streaming;
			u8 bufsize;

			/* Padding */
			u8 is_ciso;

			/* Unused, on channel list mode this is the full 64 bit tid */
			u64 tid;

			/* Unused, using in loader internally to detect title type */
			u8 type;

			/* rest of unused */
			u8 unused[4];

			/* Magic word */
			u32 magic;

			/* Padding */
			u32 gc_magic;

			/* Game title */
			char title[64];

			/* Encryption/Hashing */
			u8 encryption;
			u8 h3_verify;

			/* Padding */
			u8 unused3[30];
	} ATTRIBUTE_PACKED;

	/* Prototypes */
	s32 Disc_Init(void);
	s32 Disc_Open(void);
	s32 Disc_Wait(void);
	void Disc_SetLowMem(void);
	s32 Disc_SetUSB(const u8 *);
	s32 Disc_ReadHeader(void *);
	s32 Disc_IsWii(void);
	s32 Disc_FindPartition(u64 *outbuf);
	s32 Disc_Mount(struct discHdr *header);
	void PatchCountryStrings(void *Address, int Size);
	void Disc_SelectVMode(u8 videoselected, bool devolution, u32 *dml_VideoMode, u32 *nin_VideoMode);
	void Disc_SetVMode(void);
	s32 Disc_JumpToEntrypoint(s32 hooktype, u32 dolparameter);
	
	extern GXRModeObj *rmode;

#ifdef __cplusplus
}
#endif

#endif
