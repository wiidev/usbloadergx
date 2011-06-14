#ifndef GUI_OPTIONBROWSER_H_
#define GUI_OPTIONBROWSER_H_

#include "gui.h"
#include "gui_scrollbar.hpp"
#include <vector>

//!Display a list of menu options
class GuiOptionBrowser: public GuiElement, public sigslot::has_slots<>
{
    public:
        GuiOptionBrowser(int w, int h, OptionList * l, const char * background);
        ~GuiOptionBrowser();
        int FindMenuItem(int c, int d);
        int GetClickedOption();
        int GetSelectedOption();
        void SetClickable(bool enable);
        void SetOffset(int optionnumber);
        void ResetState();
        void SetFocus(int f);
        void Draw();
        void Update(GuiTrigger * t);
    protected:
        void onListChange(int SelItem, int SelInd);
        void UpdateListEntries();
        int selectedItem;
        int listOffset;
        int coL2;
        bool scrollbaron;
        bool listChanged;

        OptionList * options;
        int optionIndex[PAGESIZE];
        GuiButton * optionBtn[PAGESIZE];
        GuiText * optionTxt[PAGESIZE];
        GuiText * optionVal[PAGESIZE];
        GuiText * optionValOver[PAGESIZE];
        GuiImage * optionBg[PAGESIZE];

        GuiImage * bgOptionsImg;

        GuiImageData * bgOptions;
        GuiImageData * bgOptionsEntry;

        GuiTrigger * trigA;
        GuiScrollbar scrollBar;
};

#endif
