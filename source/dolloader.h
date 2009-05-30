#ifndef _DOLLOADER_H_
#define _DOLLOADER_H_

#include <wiiuse/wpad.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Apps goes here
#define EXECUTABLE_MEM_ADDR 0x92000000
/* dol header */


extern void __exception_closeall();
typedef void (*entrypoint) (void);


typedef struct _dolheader {
	u32 text_pos[7];
	u32 data_pos[11];
	u32 text_start[7];
	u32 data_start[11];
	u32 text_size[7];
	u32 data_size[11];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
} dolheader;

u32 load_dol_image (void *dolstart, struct __argv *argv);


#ifdef __cplusplus
}
#endif

#endif
