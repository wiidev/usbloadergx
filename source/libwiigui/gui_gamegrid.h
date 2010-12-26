#ifndef _GUIGAMEGRID_H_
#define _GUIGAMEGRID_H_

#include <vector>
#include "gui.h"
#include "usbloader/disc.h"

class GuiImageAsync;
class GuiGameGrid: public GuiElement
{
    public:
        GuiGameGrid(int w, int h, const char *themePath, const u8 *imagebg, int selectedGame = 0);
        ~GuiGameGrid();
        int FindMenuItem(int c, int d);
        int GetClickedOption();
        int GetSelectedOption();
        void ResetState();
        void SetFocus(int f);
        void Draw();
        void Update(GuiTrigger * t);
        int GetOffset();
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

        int * gameIndex;
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
