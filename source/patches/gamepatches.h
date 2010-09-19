#ifndef GAMEPATCHES_H_
#define GAMEPATCHES_H_

#include <gccore.h>

bool Anti_002_fix( u8 * Address, int Size );
bool NSMBPatch( u8 * Address, int Size );
bool PoPPatch();
bool Search_and_patch_Video_Modes( u8 * Address, u32 Size, GXRModeObj* Table[] );
void VideoModePatcher( u8 * dst, int len, u8 videoSelected );
bool PatchReturnTo( void *Address, int Size, u32 id );

#endif
