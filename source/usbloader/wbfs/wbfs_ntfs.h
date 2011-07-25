#ifndef _WBFS_NTFS_H
#define _WBFS_NTFS_H

#include "wbfs_fat.h"

class Wbfs_Ntfs: public Wbfs_Fat
{
	public:
		Wbfs_Ntfs(u32 lba, u32 size, u32 part, u32 port) :
			Wbfs_Fat(lba, size, part, port)
		{
		}
		virtual u8 GetFSType(void) { return PART_FS_NTFS; }
};

#endif //_WBFS_NTFS_H
