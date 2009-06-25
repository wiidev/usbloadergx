#ifndef _APPLOADER_H_
#define _APPLOADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Entry point */
typedef void (*entry_point)(void);

/* Prototypes */
s32 Apploader_Run(entry_point *, u8, u8, u8, u8, u8, u8, u8);

#ifdef __cplusplus
}
#endif

#endif
