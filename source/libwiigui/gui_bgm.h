/****************************************************************************
 * SettingsPrompts
 * USB Loader GX 2009
 *
 * Backgroundmusic
 ***************************************************************************/

#ifndef _BGM_H_
#define _BGM_H_

#include "libwiigui/gui.h"

enum
{
    ONCE = 0, LOOP, RANDOM_BGM, DIR_LOOP
};

class GuiBGM: public GuiSound
{
    public:
        GuiBGM(const u8 *s, int l, int v);
        ~GuiBGM();
        bool Load(const char *path);
        bool LoadStandard();
        bool ParsePath(const char * folderpath);
        bool PlayNext();
        bool PlayPrevious();
        bool PlayRandom();
        void SetLoop(bool l);
        void SetLoop(int l);
        void UpdateState();
    protected:
        void AddEntrie(const char * filename);
        void ClearList();

        int currentPlaying;
        int loopMode;
        char * currentPath;
        std::vector<char *> PlayList;
};

#endif
