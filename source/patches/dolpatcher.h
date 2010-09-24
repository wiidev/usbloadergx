#ifndef DOLPATCHER_C_
#define DOLPATCHER_C_

#include <gctypes.h>

bool PatchDOL(u8 * Address, int Size, const u8 * SearchPattern, int SearchSize, const u8 * PatchData, int PatchSize);

#endif
