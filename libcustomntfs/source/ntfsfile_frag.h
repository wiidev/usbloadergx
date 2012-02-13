#ifndef NTFS_FRAG_H_
#define NTFS_FRAG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*_ntfs_frag_append_t)(void *ff, u32 offset, u32 sector, u32 count);
int _NTFS_get_fragments (const char *path, _ntfs_frag_append_t append_fragment, void *callback_data);

#ifdef __cplusplus
}
#endif

#endif
