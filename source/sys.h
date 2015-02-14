#ifndef _SYS_H_
#define _SYS_H_

void wiilight(int enable);

/* Prototypes */
void AppCleanUp(void);  //! Deletes all allocated space for everything
void ExitApp(void);	 //! Like AppCleanUp() and additional device unmount
void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_ShutdownToIdle(void);
void Sys_ShutdownToStandby(void);
void Sys_LoadMenu(void);
void Sys_BackToLoader(void);
void Sys_LoadHBC(void);
bool RebootApp(void);
void ScreenShot(void);
bool isWiiU(void);

#endif
