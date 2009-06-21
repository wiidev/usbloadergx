#ifndef _ALTERNATEDOL_H_
#define _ALTERNATEDOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* not the full path is needed here, the path where the dol is */

bool Load_Dol(void **buffer, int* dollen, char * path);
bool Remove_001_Protection(void *Address, int Size);
u32 load_dol_image(void * dolstart);
char hiddenfix[1000][1000]; // temporary fix for alternative dol not working for some people
#ifdef __cplusplus
}
#endif

#endif
