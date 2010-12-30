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
#define WBFS_MIN_DEVICE         1
#define WBFS_MAX_DEVICE         2

	extern int wbfs_part_fs;
	extern s32 wbfsDev;
	extern u32 wbfs_part_idx;

    /* Prototypes */
    s32 WBFS_Init(u32);
    s32 WBFS_Format(u32, u32);
    s32 WBFS_GetCount(u32 *);
    s32 WBFS_GetHeaders(struct discHdr *, u32, u32);
    //    s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf);
    wbfs_t *GetHddInfo(void);
    s32 WBFS_CheckGame(u8 *);
    s32 WBFS_AddGame(void);
    s32 WBFS_RemoveGame(u8 *);
    s32 WBFS_GameSize(u8 *, f32 *);
    bool WBFS_ShowFreeSpace(void);
    s32 WBFS_DiskSpace(f32 *, f32 *);
    s32 WBFS_RenameGame(u8 *, const void *);
    s32 WBFS_ReIDGame(u8 *discid, const void *newID);
    f32 WBFS_EstimeGameSize(void);

    int WBFS_GetFragList(u8 *id);
    /*
     s32 __WBFS_ReadUSB(void *fp, u32 lba, u32 count, void *iobuf);
     s32 __WBFS_WriteUSB(void *fp, u32 lba, u32 count, void *iobuf);
     */

    s32 WBFS_OpenPart(int part_num);
    wbfs_disc_t* WBFS_OpenDisc(u8 *discid);
    void WBFS_CloseDisc(wbfs_disc_t *disc);
    bool WBFS_Close();
    bool WBFS_Mounted();
    bool WBFS_Selected();
    int MountWBFS(bool ShowGUI);

#ifdef __cplusplus
}
#endif

#endif
