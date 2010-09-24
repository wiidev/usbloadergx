#ifndef _IOSLOADER_H_
#define _IOSLOADER_H_

#include <gctypes.h>

class IosLoader
{
    public:
        static s32 LoadAppCios();
        static s32 ReloadIosSafe(s32 ios);
};

#endif
