#ifndef _GUIGAMEBROWSER_H_
#define _GUIGAMEBROWSER_H_

#include "gui.h"
#include "../usbloader/disc.h"

class GuiGameBrowser : public GuiElement
{
	public:
		GuiGameBrowser(int w, int h, struct discHdr *l, int gameCnt, const char *themePath, const u8 *imagebg, int selected = 0, int offset = 0);
		~GuiGameBrowser();
		int FindMenuItem(int c, int d);
		int GetClickedOption();
		int GetSelectedOption();
		void ResetState();
		void SetFocus(int f);
		void Draw();
		void Update(GuiTrigger * t);
		int GetOffset();
		void Reload(struct discHdr * l, int count);
		//GuiText * optionVal[PAGESIZE];
	protected:
		void UpdateListEntries();
		int selectedItem;
		int listOffset;
		int scrollbaron;
		int pagesize;
		int maxTextWidth;

		struct discHdr * gameList;
		int gameCnt;

		int * gameIndex;
		GuiButton ** game;
		GuiText ** gameTxt;
		GuiText ** gameTxtOver;
		GuiImage ** gameBg;
		GuiImage ** newImg;

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;

		GuiImage * bgGameImg;
		GuiImage * scrollbarImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;

		GuiImageData * bgGames;
		GuiImageData * bgGamesEntry;
		GuiImageData * newGames;
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
#endif
