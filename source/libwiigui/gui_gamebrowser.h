#ifndef _GUIGAMEBROWSER_H_
#define _GUIGAMEBROWSER_H_

#include "gui.h"
#include "../disc.h"
/*
class GameBrowserList {
	public:
		GameBrowserList(int size) {
			name = new char * [size];

			for (int i = 0; i < size; i++)
			{
				name[i] = new char[50];
			}
			length = size;
		};
		~GameBrowserList(){
			for (int i = 0; i < length; i++)
			{
				delete [] name[i];
			}
			delete [] name;
		};

	public:
		int length;
		char ** name;
};
*/

class GuiGameBrowser : public GuiElement
{
	public:
		GuiGameBrowser(int w, int h, struct discHdr * l, int gameCnt, const char *themePath, const u8 *imagebg, int selected = 0, int offset = 0);
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

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;

		GuiImage * bgGameImg;
		GuiImage * scrollbarImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * ttarrowUpImg;
		GuiImage * ttarrowDownImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;

		GuiText * ttarrowDownTxt;
		GuiText * ttarrowUpTxt;

		GuiImageData * bgGames;
		GuiImageData * bgGamesEntry;
		GuiImageData * scrollbar;
		GuiImageData * arrowDown;
		GuiImageData * arrowDownOver;
		GuiImageData * ttarrow;
		GuiImageData * arrowUp;
		GuiImageData * arrowUpOver;
		GuiImageData * scrollbarBox;
		GuiImageData * scrollbarBoxOver;

		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;
		GuiTrigger * trigA;
		GuiTrigger * trigHeldA;
};
#endif
