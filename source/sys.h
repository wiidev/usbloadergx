#ifndef _SYS_H_
#define _SYS_H_
#include <asndlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
/* Prototypes */
void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_LoadMenu(void);
s32  Sys_GetCerts(signed_blob **, u32 *);

#ifdef __cplusplus
}
#endif

#endif
