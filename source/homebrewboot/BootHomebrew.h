#ifndef _BOOTHOMEBREW_H_
#define _BOOTHOMEBREW_H_

//int BootHomebrew();
int BootHomebrewFromMem();
int BootHomebrew( char * filepath );
int CopyHomebrewMemory( u8 *temp, u32 pos, u32 len );
void AddBootArgument( const char * arg );
void FreeHomebrewBuffer();
int LoadHomebrew( const char * filepath );
int AllocHomebrewMemory( u32 filesize );
extern void *innetbuffer;
extern u32 homebrewsize;

#endif
