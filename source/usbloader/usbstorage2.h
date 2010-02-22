#ifndef _USBSTORAGE2_H_
#define _USBSTORAGE2_H_ 

#include "ogc/disc_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes */
s32  USBStorage2_GetCapacity(u32 *);
s32  USBStorage2_Init(void);
void USBStorage2_Deinit(void);
s32 USBStorage2_Umount(void);

s32  USBStorage2_ReadSectors(u32, u32, void *);
s32  USBStorage2_WriteSectors(u32, u32, const void *);

s32 USBStorage2_Watchdog(u32 on_off);

s32 USBStorage2_TestMode(u32 on_off);

s32 USBStorage2_EHC_Off(void);

#define DEVICE_TYPE_WII_UMS (('W'<<24)|('U'<<16)|('M'<<8)|'S')

extern const DISC_INTERFACE __io_usbstorage2;

#ifdef __cplusplus
}
#endif

#endif
