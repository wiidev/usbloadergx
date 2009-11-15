#ifndef _FATMOUNTER_H_
#define _FATMOUNTER_H_

#ifdef __cplusplus
extern "C" {
#endif

	extern int   fat_sd_mount;
	extern sec_t fat_sd_sec;
	extern int   fat_usb_mount;
	extern sec_t fat_usb_sec;
	extern int   fat_wbfs_mount;
	extern sec_t fat_wbfs_sec;

    int USBDevice_Init();
    void USBDevice_deInit();
    int WBFSDevice_Init(u32 sector);
    void WBFSDevice_deInit();
    int isInserted(const char *path);
    int SDCard_Init();
    void SDCard_deInit();

#ifdef __cplusplus
}
#endif

#endif
