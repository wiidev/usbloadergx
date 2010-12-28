#ifndef _WBFS_FAT_H
#define _WBFS_FAT_H

#include <ogcsys.h>

#include "usbloader/splits.h"
#include "wbfs_base.h"

class Wbfs_Fat: public Wbfs
{
    public:
        Wbfs_Fat(u32 device, u32 lba, u32 size);
        ~Wbfs_Fat();

        virtual s32 Open();
        virtual void Close();
        wbfs_disc_t* OpenDisc(u8 *);
        void CloseDisc(wbfs_disc_t *);

        s32 GetCount(u32 *);
        s32 GetHeaders(struct discHdr *, u32, u32);

        s32 AddGame();
        s32 RemoveGame(u8 *);

        s32 DiskSpace(f32 *, f32 *);

        s32 RenameGame(u8 *, const void *);
        s32 ReIDGame(u8 *, const void *);

        f32 EstimateGameSize();

        int GetFragList(u8 *);
        virtual bool ShowFreeSpace(void);

    protected:
        static char wbfs_fs_drive[16];
        bool Mounted;
    private:
        split_info_t split;

        static u32 fat_sector_size;
        static char wbfs_fat_dir[16];
        static char invalid_path[];
        static struct discHdr *fat_hdr_list;
        static u32 fat_hdr_count;

        wbfs_t* OpenPart(char *fname);
        void ClosePart(wbfs_t* part);
        wbfs_t* CreatePart(u8 *id, char *path);
        int FindFilename(u8 *id, char *fname, int len);
        void Filename(u8 *id, char *fname, int len, char *path);
        bool CheckLayoutB(char *fname, int len, u8* id, char *fname_title);
        s32 GetHeadersCount();
        void GetDir(struct discHdr *header, char *path);

        void mk_title_txt(struct discHdr *header, char *path);
        void mk_gameid_title(struct discHdr *header, char *name, int re_space, int layout);
        void title_filename(char *title);
        bool is_gameid(char *id);

        static int nop_rw_sector(void *_fp, u32 lba, u32 count, void* buf)
        {
            return 0;
        }
};

#endif //_WBFS_FAT_H
