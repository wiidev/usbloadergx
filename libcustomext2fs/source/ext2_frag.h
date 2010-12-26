#ifndef EXT2_FRAG_H_
#define EXT2_FRAG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*_ext2_frag_append_t)(void *ff, u32 offset, u32 sector, u32 count);

int _EXT2_get_fragments(const char *in_path, _ext2_frag_append_t append_fragment, void *callback_data);

#ifdef __cplusplus
}
#endif

#endif
