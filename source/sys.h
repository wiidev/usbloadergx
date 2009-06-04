#ifndef _SYS_H_
#define _SYS_H_


void wiilight(int enable);

#ifdef __cplusplus
extern "C"
//{
#endif
/* Prototypes */
void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_ShutdownToIdel(void);
void Sys_ShutdownToStandby(void);
void Sys_LoadMenu(void);
void Sys_BackToLoader(void);
int Sys_IosReload(int IOS);
s32  Sys_GetCerts(signed_blob **, u32 *);

#ifdef __cplusplus
//}
#endif

#endif
