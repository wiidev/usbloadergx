#ifndef _FATMOUNTER_H_
#define _FATMOUNTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

int USBDevice_Init();
void USBDevice_deInit();
int isSdInserted();
int isInserted(const char *path);
int SDCard_Init();
void SDCard_deInit();

#ifdef __cplusplus
}
#endif

#endif
