#ifndef _WBFS_EXT_H
#define _WBFS_EXT_H

#include "wbfs_fat.h"

class Wbfs_Ext: public Wbfs_Fat
{
	public:
		Wbfs_Ext(u32 lba, u32 size, u32 part, u32 port) :
			Wbfs_Fat(lba, size, part, port)
		{
		}
		virtual u8 GetFSType(void) { return PART_FS_EXT; }
};

#endif //_WBFS_NTFS_H
