/***************************************************************************
 * Copyright (C) 2009
 * by Hibernatus
 *
 * Game_Sound Class by Dimok
 * Many other modifications and adjustments by Dimok for USB Loader GX
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
 #ifndef _GAMESOUND_H_
#define _GAMESOUND_H_

#include "libwiigui/gui.h"

class GameSound : public GuiSound
{
    public:
        GameSound(const u8 * discid);
        ~GameSound();
        void LoadSound(const u8 *discid);
        bool Play();
        bool fromWAV(const u8 *buffer, u32 size);
        bool fromAIFF(const u8 *buffer, u32 size);
        bool fromBNS(const u8 *buffer, u32 size);
    private:
        u8 * sound;
        u32 length;
        u32 freq;
        u8 format;
};

#endif
