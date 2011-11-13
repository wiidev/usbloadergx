/***************************************************************************
 * Copyright (C) 2010 by dude
 * Copyright (C) 2011 by Miigotu
 * Copyright (C) 2011 by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#ifndef _CHANNELS_H_
#define _CHANNELS_H_

#include <string>
#include <vector>
#include <gccore.h>
#include "usbloader/disc.h"

using namespace std;

class Channels
{
public:
    static Channels *Instance(void) { if(!instance) instance = new Channels(); return instance; }
    static void DestroyInstance(void) { if(instance) delete instance; instance = NULL; }

    static u32 LoadChannel(u64 chantitle);
    static u8 GetRequestedIOS(u64 title);
    static u8 *GetDol(u64 title);
    static u8 *GetOpeningBnr(u64 title);

    void GetChannelList();
    void GetEmuChannelList();
    vector<struct discHdr> & GetDiscHeaderList(void);
private:
    static Channels *instance;

    static bool Identify(u64 titleid, u32 *ios);

    bool ParseTitleDir(char *path, int language);
    bool GetEmuChanTitle(char *tmdpath, int language, std::string &Title);

    vector<struct discHdr> NandChannels;
    vector<struct discHdr> EmuChannels;
};

#endif
