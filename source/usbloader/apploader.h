#ifndef _APPLOADER_H_
#define _APPLOADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

	/* Entry point */
	typedef void (*entry_point)(void);

	/* Prototypes */
	s32 Apploader_Run(entry_point *entry, char * dolpath, u8 alternatedol, u32 alternatedoloffset);

#ifdef __cplusplus
}
#endif

#endif
