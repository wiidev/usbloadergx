#ifndef GUI_SEARCHBAR_H_
#define GUI_SEARCHBAR_H_

#include <set>
#include <vector>
#include "gui.h"
#include "usbloader/disc.h"
#include "wstring.hpp"

class cSearchButton;

class GuiSearchBar: public GuiWindow
{
	public:
		GuiSearchBar();
		virtual ~GuiSearchBar();
		void Draw();
		void Update(GuiTrigger * t);
		wchar_t GetClicked();

		static void FilterList(std::vector<struct discHdr *> &List, wString &GameFilter);
	private:
		static std::set<wchar_t> SearchChars;

		u16 inSide;

		GuiText text;

		GuiImageData* imgBacspaceBtn;
		GuiImage* BacspaceBtnImg;
		GuiImage* BacspaceBtnImg_Over;
		GuiButton* BacspaceBtn;

		GuiImageData* imgClearBtn;
		GuiImage* ClearBtnImg;
		GuiImage* ClearBtnImg_Over;
		GuiButton* ClearBtn;

		GuiButton* CloseBtn;

		cSearchButton *searchModeBtn;
		cSearchButton **buttons;
		int cnt;
		GuiImageData keyImageData;
		GuiImageData keyOverImageData;
		GuiTrigger trig;
		GuiTrigger trigB;

};

#endif
