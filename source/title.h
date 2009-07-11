#ifndef _TITLE_H_
#define _TITLE_H_

#ifdef __cplusplus
extern "C"
{
#endif
/* Constants */
#define BLOCK_SIZE	1024

/* Prototypes */
s32 Title_GetList(u64 **, u32 *);
s32 Title_GetTicketViews(u64, tikview **, u32 *);
s32 Title_GetTMD(u64, signed_blob **, u32 *);
s32 Title_GetVersion(u64, u16 *);
s32 Title_GetSysVersion(u64, u64 *);
s32 Title_GetSize(u64, u32 *);
s32 Title_GetIOSVersions(u8 **, u32 *);

#ifdef __cplusplus
}
#endif

#endif
