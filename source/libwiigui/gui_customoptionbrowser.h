#include "gui.h"

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
        int size;
        int coL2;
        int scrollbaron;

        OptionList * options;
        int * optionIndex;
        GuiButton ** optionBtn;
        GuiText ** optionTxt;
        GuiText ** optionVal;
        GuiText ** optionValOver;
        GuiImage ** optionBg;

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

        GuiSound * btnSoundClick;
        GuiTrigger * trigA;
        GuiTrigger * trigHeldA;
};
