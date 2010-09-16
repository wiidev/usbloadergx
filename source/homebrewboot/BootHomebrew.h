#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_


int BootHomebrew(char * path);
int BootHomebrewFromMem();
void CopyHomebrewMemory(u32 read, u8 *temp, u32 len);
int AllocHomebrewMemory(u32 filesize);
void FreeHomebrewBuffer();
void AddBootArgument(const char * argv);
extern u32 homebrewsize;

#endif
