#ifndef _SYS_H_
#define _SYS_H_

#ifdef __cplusplus
extern "C" {
#endif

void wiilight(int enable);

/* Prototypes */
void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_ShutdownToIdel(void);
void Sys_ShutdownToStandby(void);
void Sys_LoadMenu(void);
void Sys_BackToLoader(void);
int Sys_ChangeIos(int ios);
int Sys_IosReload(int IOS);
bool Sys_IsHermes();
s32 IOS_ReloadIOSsafe(int ios);
void ScreenShot();

void ShowMemInfo();
extern s32 ios222rev;
extern s32 ios223rev;
extern s32 ios249rev;
extern s32 ios250rev;

#define IOS_TYPE_UNK    0
#define IOS_TYPE_WANIN  1
#define IOS_TYPE_HERMES 2
#define IOS_TYPE_KWIIRK 3

int get_ios_type();
int is_ios_type(int type);
int ios_supports_frag();

#ifdef __cplusplus
}
#endif

#endif

