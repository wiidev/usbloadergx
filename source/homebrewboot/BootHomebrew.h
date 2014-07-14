#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

int BootHomebrew(const char * filepath);
int BootHomebrewFromMem();
int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len);
void AddBootArgument(const char * argv);
void AddBootArgument(const char * argv, unsigned int size);
void FreeHomebrewBuffer();

#endif
