#ifndef _WBFS_NTFS_H
#define _WBFS_NTFS_H

#include "wbfs_fat.h"

class Wbfs_Ntfs: public Wbfs_Fat
{
    public:
        Wbfs_Ntfs(u32 device, u32 lba, u32 size) :
            Wbfs_Fat(device, lba, size)
        {
        }

        virtual s32 Open();

        bool ShowFreeSpace(void) { return true; };
};

#endif //_WBFS_NTFS_H
