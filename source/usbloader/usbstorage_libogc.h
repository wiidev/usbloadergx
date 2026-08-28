#ifndef __USBSTORAGE_LIBOGC_H__
#define __USBSTORAGE_LIBOGC_H__

#if defined(HW_RVL)

#include <gctypes.h>
#include <ogc/mutex.h>
#include <ogc/disc_io.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

s32 USBStorage_OGC_Initialize();

s32 USBStorage_OGC_Open(usbstorage_handle *dev, s32 device_id, u16 vid, u16 pid);
s32 USBStorage_OGC_Close(usbstorage_handle *dev);
s32 USBStorage_OGC_Reset(usbstorage_handle *dev);

s32 USBStorage_OGC_GetMaxLUN(usbstorage_handle *dev);
s32 USBStorage_OGC_MountLUN(usbstorage_handle *dev, u8 lun);
s32 USBStorage_OGC_Suspend(usbstorage_handle *dev);

s32 USBStorage_OGC_ReadCapacity(usbstorage_handle *dev, u8 lun, u32 *sector_size, u32 *n_sectors);
s32 USBStorage_OGC_Read(usbstorage_handle *dev, u8 lun, u32 sector, u16 n_sectors, u8 *buffer);
s32 USBStorage_OGC_Write(usbstorage_handle *dev, u8 lun, u32 sector, u16 n_sectors, const u8 *buffer);
s32 USBStorage_OGC_StartStop(usbstorage_handle *dev, u8 lun, u8 lo_ej, u8 start, u8 imm);

#define USBSTORAGE_POWER_APM_DISABLED       (1 << 0)
#define USBSTORAGE_POWER_STANDBY_DISABLED   (1 << 1)
#define USBSTORAGE_POWER_ALL_DISABLED       (USBSTORAGE_POWER_APM_DISABLED | USBSTORAGE_POWER_STANDBY_DISABLED)

/**
 * Disable ATA Advanced Power Management and the ATA standby timer on the
 * currently mounted USB/SAT disk. A non-mutating ATA command is used to check
 * pass-through support first. Returns a USBSTORAGE_POWER_* bitmask, zero when
 * unsupported, or a negative transport error. The settings are volatile and
 * do not write sectors or bridge firmware.
 */
s32 USBStorage_OGC_DisablePowerSaving();

extern DISC_INTERFACE __io_usbstorage_ogc;

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif /* HW_RVL */

#endif /* __USBSTORAGE_H__ */
