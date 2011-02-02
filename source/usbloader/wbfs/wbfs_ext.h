#ifndef _WBFS_EXT_H
#define _WBFS_EXT_H

#include "wbfs_fat.h"

class Wbfs_Ext: public Wbfs_Fat
{
    public:
        Wbfs_Ext(u32 device, u32 lba, u32 size) :
            Wbfs_Fat(device, lba, size)
        {
        }
        virtual const u8 GetFSType(void) const { return PART_FS_EXT; }
};

#endif //_WBFS_NTFS_H
