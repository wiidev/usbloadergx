#ifndef _DOLLOADER_H_
#define _DOLLOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EXECUTABLE_MEM_ADDR 0x92000000

extern void __exception_closeall();
typedef void (*entrypoint) (void);

u32 load_dol_image(const void *dolstart, struct __argv *argv);


#ifdef __cplusplus
}
#endif

#endif
