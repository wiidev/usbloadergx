#include <ogcsys.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "fatmounter.h"
#include "libntfs/ntfs.h"
#include "libwbfs/libwbfs.h"
#include "wbfs.h"
#include "usbstorage2.h"
#include "frag.h"
#include "utils.h"

FragList *frag_list = NULL;

void frag_init(FragList *ff, int maxnum)
{
    memset(ff, 0, sizeof(Fragment) * (maxnum + 1));
    ff->maxnum = maxnum;
}

int frag_append(FragList *ff, u32 offset, u32 sector, u32 count)
{
    int n;
    if (count)
    {
        n = ff->num - 1;
        if (ff->num > 0 && ff->frag[n].offset + ff->frag[n].count == offset && ff->frag[n].sector + ff->frag[n].count
                == sector)
        {
            // merge
            ff->frag[n].count += count;
        }
        else
        {
            // add
            if (ff->num >= ff->maxnum)
            {
                // too many fragments
                return -500;
            }
            n = ff->num;
            ff->frag[n].offset = offset;
            ff->frag[n].sector = sector;
            ff->frag[n].count = count;
            ff->num++;
        }
    }
    ff->size = offset + count;
    return 0;
}

int _frag_append(void *ff, u32 offset, u32 sector, u32 count)
{
    return frag_append(ff, offset, sector, count);
}

int frag_concat(FragList *ff, FragList *src)
{
    int i, ret;
    u32 size = ff->size;
    //printf("concat: %d %d <- %d %d\n", ff->num, ff->size, src->num, src->size);
    for (i = 0; i < src->num; i++)
    {
        ret = frag_append(ff, size + src->frag[i].offset, src->frag[i].sector, src->frag[i].count);
        if (ret) return ret;
    }
    ff->size = size + src->size;
    //printf("concat: -> %d %d\n", ff->num, ff->size);
    return 0;
}

// in case a sparse block is requested,
// the returned poffset might not be equal to requested offset
// the difference should be filled with 0
int frag_get(FragList *ff, u32 offset, u32 count, u32 *poffset, u32 *psector, u32 *pcount)
{
    int i;
    u32 delta;
    //printf("frag_get(%u %u)\n", offset, count);
    for (i = 0; i < ff->num; i++)
    {
        if (ff->frag[i].offset <= offset && ff->frag[i].offset + ff->frag[i].count > offset)
        {
            delta = offset - ff->frag[i].offset;
            *poffset = offset;
            *psector = ff->frag[i].sector + delta;
            *pcount = ff->frag[i].count - delta;
            if (*pcount > count) *pcount = count;
            goto out;
        }
        if (ff->frag[i].offset > offset && ff->frag[i].offset < offset + count)
        {
            delta = ff->frag[i].offset - offset;
            *poffset = ff->frag[i].offset;
            *psector = ff->frag[i].sector;
            *pcount = ff->frag[i].count;
            count -= delta;
            if (*pcount > count) *pcount = count;
            goto out;
        }
    }
    // not found
    if (offset + count > ff->size)
    {
        // error: out of range!
        return -1;
    }
    // if inside range, then it must be just sparse, zero filled
    // return empty block at the end of requested
    *poffset = offset + count;
    *psector = 0;
    *pcount = 0;
    out:
    //printf("=>(%u %u %u)\n", *poffset, *psector, *pcount);
    return 0;
}

int frag_remap(FragList *ff, FragList *log, FragList *phy)
{
    int i;
    int ret;
    u32 offset;
    u32 sector;
    u32 count;
    u32 delta;
    for (i = 0; i < log->num; i++)
    {
        delta = 0;
        count = 0;
        do
        {
            ret = frag_get(phy, log->frag[i].sector + delta + count, log->frag[i].count - delta - count, &offset,
                    &sector, &count);
            if (ret) return ret; // error
            delta = offset - log->frag[i].sector;
            ret = frag_append(ff, log->frag[i].offset + delta, sector, count);
            if (ret) return ret; // error
        } while (count + delta < log->frag[i].count);
    }
    return 0;
}

int get_frag_list(u8 *id)
{
    return WBFS_GetFragList(id);
}

int set_frag_list(u8 *id)
{
    if (wbfs_part_fs == PART_FS_WBFS) return 0;
    if (frag_list == NULL)
    {
        if (wbfs_part_fs == PART_FS_FAT)
        {
            // fall back to old fat method
            //          printf("FAT: fallback to old method\n");
            return 0;
        }
        // ntfs has no fallback, return error
        return -1;
    }

    // (+1 for header which is same size as fragment)
    int size = sizeof(Fragment) * (frag_list->num + 1);
    int ret = USBStorage_WBFS_SetFragList(frag_list, size);
    if (ret)
    {
        //      printf("set_frag: %d\n", ret);
        return ret;
    }

    // verify id matches
    char discid[8];
    memset(discid, 0, sizeof(discid));
    ret = USBStorage_WBFS_Read(0, 6, discid);
    return 0;
}

