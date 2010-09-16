#ifndef _DOLLOADER_H_
#define _DOLLOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void __exception_closeall();
typedef void (*entrypoint) (void);

u32 load_dol(const void *dolstart, struct __argv *argv);


#ifdef __cplusplus
}
#endif

#endif
