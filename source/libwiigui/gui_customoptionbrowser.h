#include "gui.h"

class customOptionList {
	public:
		customOptionList(int size) {
			name = new char * [size];
			value = new char * [size];
			for (int i = 0; i < size; i++)
			{
				name[i] = new char[40];
				value[i] = new char[40];
			}
			length = size;
		};
		~customOptionList(){
			for (int i = 0; i < length; i++)
			{
				delete [] name[i];
				delete [] value[i];
			}
			delete [] name;
			delete [] value;
		};

	public:
		int length;
		char ** name;
		char ** value;
};

//!Display a list of menu options
class GuiCustomOptionBrowser : public GuiElement
{
	public:
		GuiCustomOptionBrowser(int w, int h, customOptionList * l, const char * themePath, const char *custombg, const u8 *imagebg, int scrollbar, int col2);
		~GuiCustomOptionBrowser();
		void SetCol2Position(int x);
		int FindMenuItem(int c, int d);
		int GetClickedOption();
		int GetSelectedOption();
		void ResetState();
		void SetFocus(int f);
		void Draw();
		void Update(GuiTrigger * t);
		GuiText ** optionVal;
	protected:
		int selectedItem;
		int listOffset;
		int size;

		customOptionList * options;
		int * optionIndex;
		GuiButton ** optionBtn;
		GuiText ** optionTxt;
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

		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;
		GuiTrigger * trigA;
		GuiTrigger * trigB;
		GuiTrigger * trigHeldA;
};
