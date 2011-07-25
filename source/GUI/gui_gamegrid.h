#ifndef _GUIGAMEGRID_H_
#define _GUIGAMEGRID_H_

#include <vector>
#include "gui_gamebrowser.h"
#include "gui_image_async.h"
#include "usbloader/disc.h"

class GuiGameGrid : public GuiGameBrowser
{
	public:
		GuiGameGrid(int w, int h, const char *themePath, int selectedGame = 0);
		virtual ~GuiGameGrid();
		int FindMenuItem(int c, int d);
		int GetClickedOption();
		int GetSelectedOption();
		void SetSelectedOption(int ind);
		void setListOffset(int off) { listOffset = off; Reload(rows, listOffset); }
		int getListOffset() const { return listOffset; }
		void ResetState();
		void SetFocus(int f);
		void Draw();
		void Update(GuiTrigger * t);
		void Reload(int Rows, int ListOffset);
		void ChangeRows(int n);
	protected:
		GuiImageData noCover;
		int selectedItem;
		int listOffset;
		int pagesize;
		int clickedItem;
		int rows;
		int goLeft;
		int goRight;
		int theme_posX;
		int theme_posY;

		std::vector<int> gameIndex;
		std::vector<GuiButton *> game;
		std::vector<GuiTooltip *> titleTT;
		std::vector<GuiImageAsync *> coverImg;

		GuiButton * btnRight;
		GuiButton * btnLeft;
		GuiButton * btnRowUp;
		GuiButton * btnRowDown;

		GuiImage * btnLeftImg;
		GuiImage * btnRightImg;

		GuiImageData * imgLeft;
		GuiImageData * imgRight;

		GuiTrigger * trigA;
		GuiTrigger * trigL;
		GuiTrigger * trigR;
		GuiTrigger * trigPlus;
		GuiTrigger * trigMinus;
		GuiTrigger * trig1;
		GuiTrigger * trig2;
};
#endif
