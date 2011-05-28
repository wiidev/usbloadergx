#ifndef _LWISO9660_DEVOPTAB_H
#define _LWISO9660_DEVOPTAB_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern int WIIDVD_Init(bool dvdx);
    extern void WIIDVD_Close(void);
    extern int WIIDVD_Mount(void);
    extern void WIIDVD_Unmount(void);
    extern int WIIDVD_DiscPresent(void);

#ifdef __cplusplus
}

#endif

#endif
