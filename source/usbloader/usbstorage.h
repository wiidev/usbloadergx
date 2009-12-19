#ifndef _USBSTORAGE_H_
#define _USBSTORAGE_H_

#ifdef __cplusplus
extern "C" {
#endif
    /* Prototypes */
    s32  USBStorage_GetCapacity(u32 *);
    s32  USBStorage_Init(void);
    void USBStorage_Deinit(void);
    s32 USBStorage_Watchdog(u32 on_off);
    s32  USBStorage_ReadSectors(u32, u32, void *);
    s32  USBStorage_WriteSectors(u32, u32, const void *);
    
	s32 USBStorage_WBFS_Open(char *buf_id);
	s32 USBStorage_WBFS_Read(u32 woffset, u32 len, void *buffer);
	s32 USBStorage_WBFS_ReadDebug(u32 off, u32 size, void *buffer);
	s32 USBStorage_WBFS_SetDevice(int dev);
	s32 USBStorage_WBFS_SetFragList(void *p, int size);
    
	extern const DISC_INTERFACE __io_wiiums;
	extern const DISC_INTERFACE __io_wiiums_ro;
#ifdef __cplusplus
}
#endif

#endif
