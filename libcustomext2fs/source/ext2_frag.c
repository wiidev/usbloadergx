#include "ext2_internal.h"
#include "ext2_frag.h"

typedef struct _PrivData
{
    _ext2_frag_append_t append_fragment;
    void * callback_data;
} PrivDataST;

static int block_iter_callback(ext2_filsys fs, blk64_t *blocknr, e2_blkcnt_t blockcnt, blk64_t ref_block, int ref_offset,  void *privateData)
{
    PrivDataST *priv = (PrivDataST *) privateData;
    blk64_t block;
    block = *blocknr;

    return priv->append_fragment(priv->callback_data, blockcnt*fs->io->block_size/512, block*fs->io->block_size/512, fs->io->block_size/512);
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
    priv.callback_data = callback_data;
    priv.append_fragment = append_fragment;

    int ret = ext2fs_block_iterate3(vd->fs, ni->ino, BLOCK_FLAG_DATA_ONLY, NULL, block_iter_callback, &priv);

    if(ret == 0)
        ret = priv.append_fragment(callback_data, EXT2_I_SIZE(&ni->ni) >> 9, 0, 0);

    ext2UpdateTimes(vd, ni, EXT2_UPDATE_ATIME);

    ext2CloseEntry(vd, ni);

    return ret;
}
