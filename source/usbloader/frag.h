
// worst case wbfs fragmentation scenario:
// 9GB (dual layer) / 2mb (wbfs sector size) = 4608
#define MAX_FRAG 20000
// max that ehcmodule_frag will allow at the moment is about:
// 40000/4/3-1 = 21844

#ifdef __cplusplus
extern "C" {
#endif

#include "libwbfs/libwbfs.h"

typedef struct
{
	u32 offset; // file offset, in sectors unit
	u32 sector;
	u32 count;
} Fragment;

typedef struct
{
	u32 size; // num sectors
	u32 num;  // num fragments
	u32 maxnum;
	Fragment frag[MAX_FRAG];
} FragList;

typedef int (*frag_append_t)(void *ff, u32 offset, u32 sector, u32 count);

int _FAT_get_fragments (const char *path, _frag_append_t append_fragment, void *callback_data);

void frag_init(FragList *ff, int maxnum);
int  frag_append(FragList *ff, u32 offset, u32 sector, u32 count);
int  _frag_append(void *ff, u32 offset, u32 sector, u32 count);
int  frag_concat(FragList *ff, FragList *src);

// in case a sparse block is requested,
// the returned poffset might not be equal to requested offset
// the difference should be filled with 0
int frag_get(FragList *ff, u32 offset, u32 count,
		u32 *poffset, u32 *psector, u32 *pcount);

int frag_remap(FragList *ff, FragList *log, FragList *phy);

int get_frag_list(u8 *id);
int set_frag_list(u8 *id);

#ifdef __cplusplus
}
#endif
