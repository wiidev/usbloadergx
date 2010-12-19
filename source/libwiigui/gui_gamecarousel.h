#ifndef _GUIGAMECAROUSEL_H_
#define _GUIGAMECAROUSEL_H_

#include <vector>
#include "gui.h"
#include "usbloader/disc.h"
class GuiImageAsync;
class GuiGameCarousel: public GuiElement
{
    public:
        GuiGameCarousel(int w, int h, const char *themePath, const u8 *imagebg, int imagebgsize, int selectedGame = 0);
        ~GuiGameCarousel();
        int FindMenuItem(int c, int d);
        int GetClickedOption();
        int GetSelectedOption();
        void ResetState();
        void SetFocus(int f);
        void Draw();
        void Update(GuiTrigger * t);
        int GetOffset();
        void Reload();
        //GuiText * optionVal[PAGESIZE];
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
