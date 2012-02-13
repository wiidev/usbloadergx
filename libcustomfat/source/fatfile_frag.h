#ifndef FAT_FRAG_H_
#define FAT_FRAG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*_fat_frag_append_t)(void *ff, u32 offset, u32 sector, u32 count);

int _FAT_get_fragments (const char *path, _fat_frag_append_t append_fragment, void *callback_data);

#ifdef __cplusplus
}
#endif

#endif
