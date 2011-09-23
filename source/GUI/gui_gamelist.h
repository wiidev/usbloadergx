#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "gui_gamebrowser.h"
#include "gui_scrollbar.hpp"
#include "usbloader/disc.h"

class GuiGameList : public GuiGameBrowser, public sigslot::has_slots<>
{
	public:
		GuiGameList(int w, int h, int listOffset = 0);
		virtual ~GuiGameList();
		int GetClickedOption();
		int GetSelectedOption() { return listOffset+selectedItem; }
		void SetSelectedOption(int ind);
		void setListOffset(int off);
		int getListOffset() const { return listOffset; }
		void ResetState();
		void SetFocus(int f);
		void Draw();
		void Update(GuiTrigger * t);
	protected:
		void onListChange(int SelItem, int SelInd);
		void UpdateListEntries();
		int selectedItem;
		int listOffset;
		int pagesize;
		int maxTextWidth;

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
