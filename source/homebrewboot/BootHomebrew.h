#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

int BootHomebrew(const char * filepath);
int BootHomebrewFromMem();
int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len);
void AddBootArgument(const char * arg);
void FreeHomebrewBuffer();

#endif
