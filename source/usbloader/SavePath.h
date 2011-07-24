#ifndef SAVEPATH_H_
#define SAVEPATH_H_

#include "usbloader/disc.h"

void CreateTitleTMD(const char *path, const struct discHdr *hdr);
void CreateSavePath(const struct discHdr *hdr);

#endif
