#include "ext2_internal.h"
#include "ext2_frag.h"

typedef struct _PrivData
{
    _ext2_frag_append_t append_fragment;
    void * callback_data;
} PrivDataST;

static int block_iter_callback(ext2_filsys fs, blk_t *blocknr, int blockcnt,  void *privateData)
{
    PrivDataST *priv = (PrivDataST *) privateData;
    blk_t block;
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

    ext2fs_block_iterate(vd->fs, ni->ino, 0, NULL, block_iter_callback, &priv);

    int ret = priv.append_fragment(callback_data, EXT2_I_SIZE(&ni->ni) >> 9, 0, 0);

    ext2UpdateTimes(vd, ni, EXT2_UPDATE_ATIME);

    ext2CloseEntry(vd, ni);

    return ret;
}
