#ifndef _GUIGAMEBROWSER_H_
#define _GUIGAMEBROWSER_H_

#include "gui.h"
#include "gui_scrollbar.hpp"
#include "usbloader/disc.h"

class GuiGameBrowser: public GuiElement, public sigslot::has_slots<>
{
    public:
        GuiGameBrowser(int w, int h, int selectedGame = 0);
        ~GuiGameBrowser();
        int FindMenuItem(int c, int d);
        int GetClickedOption();
        int GetSelectedOption() { return listOffset+selectedItem; }
        void ResetState();
        void SetFocus(int f);
        void Draw();
        void Update(GuiTrigger * t);
        int GetOffset();
    protected:
        void onListChange(int SelItem, int SelInd);
        void UpdateListEntries();
        int selectedItem;
        int listOffset;
        int scrollbaron;
        int pagesize;
        int maxTextWidth;

        int * gameIndex;
        GuiButton ** game;
        GuiText ** gameTxt;
        GuiText ** gameTxtOver;
        GuiImage ** gameBg;
        GuiImage ** newImg;

        GuiImage * bgGameImg;

        GuiImageData * bgGames;
        GuiImageData * bgGamesEntry;
        GuiImageData * newGames;

        GuiTrigger * trigA;

        GuiScrollbar scrollBar;
};
#endif
