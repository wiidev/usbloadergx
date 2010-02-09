#ifndef _GUIGAMEGRID_H_
#define _GUIGAMEGRID_H_

#include "gui.h"
#include "../usbloader/disc.h"
class GuiImageAsync;
class GuiGameGrid : public GuiElement {
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
    void Reload(struct discHdr * l, int count, int Rows, int ListOffset);
    void ChangeRows(int n);
protected:
    GuiImageData	noCover;
    int selectedItem;
    int listOffset;
    int pagesize;
    int clickedItem;
    int rows;
    int goLeft;
    int goRight;

    struct discHdr * gameList;
    int gameCnt;

    int * gameIndex;
    GuiButton ** game;
    GuiTooltip ** titleTT;
    GuiImageAsync ** coverImg;


    GuiButton * btnRight;
    GuiButton * btnLeft;
    GuiButton * btnRowUp;
    GuiButton * btnRowDown;

    GuiImage * btnLeftImg;
    GuiImage * btnRightImg;

    GuiImageData * imgLeft;
    GuiImageData * imgRight;

    GuiSound * btnSoundOver;
    GuiSound * btnSoundClick;
    GuiTrigger * trigA;
    GuiTrigger * trigL;
    GuiTrigger * trigR;
    GuiTrigger * trigPlus;
    GuiTrigger * trigMinus;
    GuiTrigger * trig1;
    GuiTrigger * trig2;
};
#endif
