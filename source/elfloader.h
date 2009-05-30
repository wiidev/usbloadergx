#ifndef _ELFLOADER_H_
#define _ELFLOADER_H_

#include <wiiuse/wpad.h>

#ifdef __cplusplus
extern "C"
{
#endif

s32 valid_elf_image (void *addr);
u32 load_elf_image (void *addr);

#ifdef __cplusplus
}
#endif

#endif
