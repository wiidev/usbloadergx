#ifndef _WPAD_H_
#define _WPAD_H_

#include <wiiuse/wpad.h>

#include <asndlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Prototypes */
s32  Wpad_Init(void);
void Wpad_Disconnect(void);
u32  Wpad_GetButtons(void);
u32  Wpad_WaitButtons(void);

#ifdef __cplusplus
}
#endif

#endif
