#ifndef _IOSLOADER_H_
#define _IOSLOADER_H_

#include <gctypes.h>
#include "../wad/nandtitle.h"

class IosLoader
{
    public:
        IosLoader(NandTitle titles);
        ~IosLoader();
        s32 CheckForCios();
        s32 LoadAppCios();
    private:
        NandTitle nandTitles;
        s32 ReloadIosSafe(s32 ios);
};

#endif
