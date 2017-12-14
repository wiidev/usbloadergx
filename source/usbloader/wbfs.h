#ifndef _WBFS_H_
#define _WBFS_H_

#include "libs/libwbfs/libwbfs.h"
#include "usbloader/disc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PART_FS_WBFS 0
#define PART_FS_FAT  1
#define PART_FS_NTFS 2
#define PART_FS_EXT  3

	/* Macros */
#define WBFS_MIN_DEVICE		 1
#define WBFS_MAX_DEVICE		 2

	/* Prototypes */
	s32 WBFS_Init(u32 device);
	s32 WBFS_ReInit(u32 device);
	s32 WBFS_Format(u32 lba, u32 size, u32 port);
	s32 WBFS_GetCount(int part, u32 *count);
	s32 WBFS_GetHeaders(int part, struct discHdr *, u32, u32);
	s32 WBFS_CheckGame(u8 *gameid);
	s32 WBFS_AddGame(void);
	s32 WBFS_RemoveGame(u8 *gameid);
	s32 WBFS_GameSize(u8 *gameid, f32 *size);
	s32 WBFS_DiskSpace(f32 *used, f32 *free);
	s32 WBFS_RenameGame(u8 *gameid, const void *newname);
	s32 WBFS_ReIDGame(u8 *discid, const void *newID);
	u64 WBFS_EstimeGameSize(void);

	s32 WBFS_GetFragList(u8 *id);

	s32 WBFS_OpenAll();
	s32 WBFS_OpenPart(int part_num);
	wbfs_disc_t* WBFS_OpenDisc(u8 *discid);
	void WBFS_CloseDisc(wbfs_disc_t *disc);
	bool WBFS_Close(int part);
	void WBFS_CloseAll();
	bool WBFS_Selected();

#ifdef __cplusplus
}
#endif

#endif
