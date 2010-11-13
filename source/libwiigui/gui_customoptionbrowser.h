#ifndef GUI_CUSTOMBROWSER_H_
#define GUI_CUSTOMBROWSER_H_

#include "gui.h"
#include <vector>

//!Display a list of menu options
class GuiCustomOptionBrowser: public GuiElement
{
    public:
        GuiCustomOptionBrowser(int w, int h, OptionList * l, const char * background, int scrollbar, int col2);
        ~GuiCustomOptionBrowser();
        int FindMenuItem(int c, int d);
        int GetClickedOption();
        int GetSelectedOption();
        void SetClickable(bool enable);
        void SetScrollbar(int enable);
        void SetOffset(int optionnumber);
        void ResetState();
        void SetFocus(int f);
        void Draw();
        void Update(GuiTrigger * t);
    protected:
        void UpdateListEntries();
        int selectedItem;
        int listOffset;
        int coL2;
        int scrollbaron;

        OptionList * options;
        int optionIndex[PAGESIZE];
        GuiButton * optionBtn[PAGESIZE];
        GuiText * optionTxt[PAGESIZE];
        GuiText * optionVal[PAGESIZE];
        GuiText * optionValOver[PAGESIZE];
        GuiImage * optionBg[PAGESIZE];

        GuiButton * arrowUpBtn;
        GuiButton * arrowDownBtn;
        GuiButton * scrollbarBoxBtn;

        GuiImage * bgOptionsImg;
        GuiImage * scrollbarImg;
        GuiImage * arrowDownImg;
        GuiImage * arrowDownOverImg;
        GuiImage * arrowUpImg;
        GuiImage * arrowUpOverImg;
        GuiImage * scrollbarBoxImg;
        GuiImage * scrollbarBoxOverImg;

        GuiImageData * bgOptions;
        GuiImageData * bgOptionsEntry;
        GuiImageData * scrollbar;
        GuiImageData * arrowDown;
        GuiImageData * arrowDownOver;
        GuiImageData * arrowUp;
        GuiImageData * arrowUpOver;
        GuiImageData * scrollbarBox;
        GuiImageData * scrollbarBoxOver;

        GuiTrigger * trigA;
        GuiTrigger * trigHeldA;
};

#endif
