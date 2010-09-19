#ifndef _APPLOADER_H_
#define _APPLOADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /* Entry point */
typedef void ( *entry_point )( void );

    /* Prototypes */
s32 Apploader_Run( entry_point *entry, u8 cheat, u8 videoSelected, u8 vipatch, u8 patchcountrystring, u8 error002fix, u8 alternatedol, u32 alternatedoloffset, u32 returnTo );
void gamepatches( void * dst, int len, u8 videoSelected, u8 patchcountrystring, u8 vipatch, u8 cheat );

#ifdef __cplusplus
}
#endif

#endif
