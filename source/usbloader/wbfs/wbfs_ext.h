#ifndef _WBFS_EXT_H
#define _WBFS_EXT_H

#include <ext2.h>
#include "wbfs_fat.h"

class Wbfs_Ext: public Wbfs_Fat
{
    public:
        Wbfs_Ext(u32 device, u32 lba, u32 size) :
            Wbfs_Fat(device, lba, size)
        {
        }

        virtual s32 Open();
        bool ShowFreeSpace(void) { return true; };
};

#endif //_WBFS_NTFS_H
