#ifndef _SYS_H_
#define _SYS_H_

void wiilight(int enable);

/* Prototypes */
void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_ShutdownToIdel(void);
void Sys_ShutdownToStandby(void);
void Sys_LoadMenu(void);
void Sys_BackToLoader(void);
bool Sys_IsHermes();
void ScreenShot();

#endif
