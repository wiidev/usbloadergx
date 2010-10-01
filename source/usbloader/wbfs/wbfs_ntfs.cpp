#include "wbfs_ntfs.h"
#include "fatmounter.h"
#include "libs/libntfs/ntfs.h"

s32 Wbfs_Ntfs::Open()
{
    strcpy(wbfs_fs_drive, "NTFS:");
    return MountNTFS(lba);
}

int Wbfs_Ntfs::GetFragList(char *filename, _frag_append_t append_fragment, FragList *fs)
{
    int ret = _NTFS_get_fragments(filename, append_fragment, fs);
    if (ret)
    {
        return ret;
    }

    // offset to start of partition
    for (unsigned int j = 0; j < fs->num; j++)
    {
        fs->frag[j].sector += fs_ntfs_sec;
    }
    return ret;
}

bool Wbfs_Ntfs::ShowFreeSpace(void)
{
    return true;
}
