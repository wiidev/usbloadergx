#ifndef _WBFS_NTFS_H
#define _WBFS_NTFS_H

#include "wbfs_fat.h"

class Wbfs_Ntfs: public Wbfs_Fat
{
    public:
        Wbfs_Ntfs(u32 lba, u32 size, u32 part) :
            Wbfs_Fat(lba, size, part)
        {
        }
        virtual const u8 GetFSType(void) const { return PART_FS_NTFS; }
};

#endif //_WBFS_NTFS_H
