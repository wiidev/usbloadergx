#ifndef _GUIGAMECAROUSEL_H_
#define _GUIGAMECAROUSEL_H_

#include <vector>
#include "gui_gamebrowser.h"
#include "gui_image_async.h"
#include "usbloader/disc.h"

class GuiGameCarousel : public GuiGameBrowser
{
	public:
		GuiGameCarousel(int w, int h, const char *themePath, int listOffset = 0);
		virtual ~GuiGameCarousel();
		int FindMenuItem(int c, int d);
		int GetClickedOption();
		int GetSelectedOption();
		void SetSelectedOption(int ind);
		void setListOffset(int off);
		int getListOffset() const;
		void Refresh();
		void ResetState();
		void SetFocus(int f);
		void Draw();
		void Update(GuiTrigger * t);
	protected:
		GuiImageData noCover;
		int selectedItem;
		int listOffset;
		int scrollbaron;
		int pagesize;
		int speed;
		int clickedItem;

		int * gameIndex;
		std::vector<GuiButton *> game;
		std::vector<GuiImageAsync *> coverImg;

		GuiText * gamename;

		GuiButton * btnRight;
		GuiButton * btnLeft;

		GuiImage * btnLeftImg;
		GuiImage * btnRightImg;

		GuiImageData * imgLeft;
		GuiImageData * imgRight;

		GuiTrigger * trigA;
		GuiTrigger * trigL;
		GuiTrigger * trigR;
		GuiTrigger * trigPlus;
		GuiTrigger * trigMinus;
};
#endif
