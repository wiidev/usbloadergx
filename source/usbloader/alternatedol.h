#ifndef _ALTERNATEDOL_H_
#define _ALTERNATEDOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* not the full path is needed here, the path where the dol is */
bool Load_Dol(void **buffer, int* dollen, const char * filepath);
u32 load_dol_image(void *dolstart);
u32 Load_Dol_from_disc(u32 offset);

#ifdef __cplusplus
}
#endif

#endif
