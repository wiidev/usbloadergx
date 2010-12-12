#include "ext2_internal.h"
#include "ext2_frag.h"
#include "mem_allocate.h"

typedef struct _DataBlocks
{
    u32 block;
    u32 blockcnt;
} DataBlocks;

typedef struct _PrivData
{
    DataBlocks * blocks;
    u32 blocksCnt;
} PrivDataST;

static int block_iter_callback(ext2_filsys fs, blk_t *blocknr, int blockcnt,  void *privateData)
{
    PrivDataST *priv = (PrivDataST *) privateData;

    if(!priv->blocks)
        priv->blocks = (DataBlocks *) mem_alloc(sizeof(DataBlocks));

    priv->blocksCnt++;

    DataBlocks * tmp = (DataBlocks *) mem_realloc(priv->blocks, priv->blocksCnt*sizeof(DataBlocks));
    if(!tmp)
    {
        free(priv->blocks);
        priv->blocks = NULL;
        return -1;
    }

    priv->blocks = tmp;

    priv->blocks[priv->blocksCnt-1].block = *blocknr;
    priv->blocks[priv->blocksCnt-1].blockcnt = blockcnt;
    return 0;
}

int _EXT2_get_fragments(const char *in_path, _ext2_frag_append_t append_fragment, void *callback_data)
{
    ext2_inode_t *ni = NULL;
    ext2_vd *vd;

    vd = ext2GetVolume(in_path);

    if(!vd)
    {
        errno = EXDEV;
        return -1;
    }

    // Get the actual path of the entry
    const char * path = ext2RealPath(in_path);
    if (!path) {
        errno = EINVAL;
        return -1;
    }

    // Find the entry
    ni = ext2OpenEntry(vd, path);
    if (!ni) {
        errno = ENOENT;
        return -1;
    }

    PrivDataST priv;
    priv.blocks = NULL;
    priv.blocksCnt = 0;

    int ret = ext2fs_block_iterate(vd->fs, ni->ino, BLOCK_FLAG_DATA_ONLY, NULL, block_iter_callback, &priv);
    if(ret == 0 && priv.blocksCnt > 0)
    {
        int i = 0;
        u32 size = 1;
        u32 block_size = vd->fs->io->block_size/512;
        int printfs = 30;

        for(i = 0; i < priv.blocksCnt-1; ++i)
        {
            //size = priv.blocks[i+1].blockcnt-priv.blocks[i].blockcnt;
            ret = append_fragment(callback_data, priv.blocks[i].blockcnt*block_size, priv.blocks[i].block*block_size, size*block_size);
            if(ret)
                break;
            if(printfs > 0)
            {
                printfs--;
            }
        }

        if(ret == 0)
            ret = append_fragment(callback_data, priv.blocks[i].blockcnt*block_size, priv.blocks[i].block*block_size, block_size);
    }

	if(ret == 0)
		ret = append_fragment(callback_data, EXT2_I_SIZE(&ni->ni) >> 9, 0, 0);

    if(priv.blocks)
        mem_free(priv.blocks);

    ext2UpdateTimes(vd, ni, EXT2_UPDATE_ATIME);

    ext2CloseEntry(vd, ni);

    return ret;
}
