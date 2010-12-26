#include "wbfs_ntfs.h"
#include "fatmounter.h"
#include "libs/libntfs/ntfsfile_frag.h"

s32 Wbfs_Ntfs::Open()
{
    strcpy(wbfs_fs_drive, "NTFS:");
    return MountNTFS(lba);
}

bool Wbfs_Ntfs::ShowFreeSpace(void)
{
    return true;
}
