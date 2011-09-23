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
		virtual ~GuiOptionBrowser();
		int GetClickedOption();
		int GetSelectedOption();
		void SetOffset(int optionnumber);
		void ResetState();
		void Draw();
		void Update(GuiTrigger * t);
	protected:
		void onListChange(int SelItem, int SelInd);
		void UpdateListEntries();

		int oldSelectedItem;
		int selectedItem;
		int listOffset;
		int coL2;

		OptionList * options;
		std::vector<GuiButton *> optionBtn;
		std::vector<GuiText *> optionTxt;
		std::vector<GuiText *> optionVal;
		std::vector<GuiImage *> optionBg;

		GuiImage * bgOptionsImg;

		GuiImageData * bgOptions;
		GuiImageData * bgOptionsEntry;

		GuiTrigger * trigA;
		GuiScrollbar scrollBar;
};

#endif
