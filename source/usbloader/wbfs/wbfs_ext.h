#ifndef _WBFS_EXT_H
#define _WBFS_EXT_H

#include "wbfs_fat.h"

class Wbfs_Ext: public Wbfs_Fat
{
    public:
        Wbfs_Ext(u32 lba, u32 size, u32 part) :
            Wbfs_Fat(lba, size, part)
        {
        }
        virtual const u8 GetFSType(void) const { return PART_FS_EXT; }
};

#endif //_WBFS_NTFS_H
