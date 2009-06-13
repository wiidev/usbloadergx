#ifndef _SDHC_H_
#define _SDHC_H_

/* Constants */
#define SDHC_SECTOR_SIZE	0x200

#ifdef __cplusplus
extern "C"
{
#endif

/* Prototypes */
bool SDHC_Init(void);
bool SDHC_Close(void);
bool SDHC_ReadSectors(u32, u32, void *);
bool SDHC_WriteSectors(u32, u32, void *);

#ifdef __cplusplus
}
#endif

#endif
