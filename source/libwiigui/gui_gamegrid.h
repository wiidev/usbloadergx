#ifndef _GUIGAMEGRID_H_
#define _GUIGAMEGRID_H_

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

class GuiGameGrid : public GuiElement
{
	public:
		GuiGameGrid(int w, int h, struct discHdr * l, int gameCnt, const char *themePath, const u8 *imagebg, int selected = 0, int offset = 0);
		~GuiGameGrid();
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
		int selectedItem;
		int listOffset;
		int scrollbaron;
		int pagesize;

		struct discHdr * gameList;
		int gameCnt;

		int * gameIndex;
		GuiButton ** game;
		GuiText ** gameTxt;
		//GuiImage ** gameBg;
		GuiImage ** coverImg;
		GuiImageData ** cover;

		//GuiButton * arrowUpBtn;
		//GuiButton * arrowDownBtn;
		//GuiButton * scrollbarBoxBtn;
		GuiButton * btnRight;
		GuiButton * btnLeft;

		//GuiImage * bgGameImg;
		//GuiImage * scrollbarImg;
		//GuiImage * arrowDownImg;
		//GuiImage * arrowDownOverImg;
		//GuiImage * ttarrowUpImg;
		//GuiImage * ttarrowDownImg;
		//GuiImage * arrowUpImg;
		//GuiImage * arrowUpOverImg;
		//GuiImage * scrollbarBoxImg;
		//GuiImage * scrollbarBoxOverImg;
		
		GuiImage * btnLeftImg;
		GuiImage * btnRightImg;
		
		GuiText * ttarrowDownTxt;
		GuiText * ttarrowUpTxt;

		//GuiImageData * bgGames;
		//GuiImageData * bgGamesEntry;
		//GuiImageData * scrollbar;
		//GuiImageData * arrowDown;
		//GuiImageData * arrowDownOver;
		//GuiImageData * ttarrow;
		//GuiImageData * arrowUp;
		//GuiImageData * arrowUpOver;
		//GuiImageData * scrollbarBox;
		//GuiImageData * scrollbarBoxOver;
		GuiImageData * imgLeft;
		GuiImageData * imgRight;

		GuiSound * btnSoundOver;
		GuiSound * btnSoundClick;
		GuiTrigger * trigA;
		GuiTrigger * trigL;
		GuiTrigger * trigR;
		GuiTrigger * trigPlus;
		GuiTrigger * trigMinus;
		GuiTrigger * trigHeldA;
};
#endif
