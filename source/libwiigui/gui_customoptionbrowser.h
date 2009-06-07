#include "gui.h"

class customOptionList {
	public:
		customOptionList(int size);
		~customOptionList();
		void SetName(int i, const char *format, ...) __attribute__((format (printf, 3, 4)));
		const char *GetName(int i)
		{
			if(i >= 0 && i < length && name[i])
				return name[i];
			else
				return "";
		}
		void SetValue(int i, const char *format, ...) __attribute__((format (printf, 3, 4)));
		const char *GetValue(int i)
		{
			if(i >= 0 && i < length && value[i])
				return value[i];
			else
				return "";
		}
		int GetLength()	{ return length; }
		bool IsChanged() { bool ret = changed; changed = false; return ret;}
	private:
		int length;
		char ** name;
		char ** value;
		bool changed;
};

//!Display a list of menu options
class GuiCustomOptionBrowser : public GuiElement
{
	public:
		GuiCustomOptionBrowser(int w, int h, customOptionList * l, const char * themePath, const char *custombg, const u8 *imagebg, int scrollbar, int col2);
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

		customOptionList * options;
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
