#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

#ifdef __cplusplus
extern "C" {
#endif

    int BootHomebrew(char * path);
    int BootHomebrewFromMem();
    void CopyHomebrewMemory(u32 read, u8 *temp, u32 len);
    int AllocHomebrewMemory(u32 filesize);
    void FreeHomebrewBuffer();

#ifdef __cplusplus
}
#endif

#endif
