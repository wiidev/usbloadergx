/****************************************************************************
 * libwiigui
 *
 * gui_gameGrid.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"

#include <unistd.h>
#include "gui_gamegrid.h"
#include "gui_image_async.h"
#include "usbloader/GameList.h"
#include "../settings/CSettings.h"
#include "../prompts/PromptWindows.h"
#include "../language/gettext.h"
#include "../menu.h"
#include "fatmounter.h"

#include <string.h>
#include <math.h>
#include <sstream>

//#define SCALE     0.8f
//#define DEG_OFFSET    7
#define RADIUS      780
//#define IN_SPEED  175
//#define SHIFT_SPEED   100
//#define SPEED_STEP    4
//#define SAFETY        320
#define goSteps     10
#include "../main.h"

extern const int vol;

static int Skew1[7][8] = { { -14, -66, 14, -34, 14, 34, -14, 66 }, { -10, -44, 10, -26, 10, 26, -10, 44 }, { -6, -22,
        6, -14, 6, 14, -6, 22 }, { 0, -11, 0, -11, 0, 11, 0, 11 }, { -6, -14, 6, -22, 6, 22, -6, 14 }, { -10, -26, 10,
        -44, 10, 44, -10, 26 }, { -14, -34, 14, -66, 14, 66, -14, 34 } };
static int Pos1[7][2][2] = {
// {{16:9 x,y},{ 4:3 x,y}}
        { { -230, 74 }, { -320, 74 } }, { { -70, 74 }, { -130, 74 } }, { { 88, 74 }, { 60, 74 } }, { { 239, 74 }, {
                239, 74 } }, { { 390, 74 }, { 420, 74 } }, { { 550, 74 }, { 612, 74 } }, { { 710, 74 }, { 772, 74 } } };
static int Skew2[18][8] = { { -5, -49, 5, -27, 5, 0, -5, 0 }, { -5, 0, 5, 0, 5, 27, -5, 49 },

{ -5, -49, 5, -27, 5, 0, -5, 0 }, { -5, 0, 5, 0, 5, 27, -5, 49 },

{ -4, -22, 4, -14, 4, 0, -4, 0 }, { -4, 0, 4, 0, 4, 14, -4, 22 },

{ 0, -9, 0, -5, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 5, 0, 9 },

{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },

{ 0, -5, 0, -9, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 9, 0, 5 },

{ -4, -14, 4, -22, 4, 0, -4, 0 }, { -4, 0, 4, 0, 4, 22, -4, 14 },

{ -5, -27, 5, -49, 5, 0, -5, 0 }, { -5, 0, 5, 0, 5, 49, -5, 27 },

{ -5, -27, 5, -49, 5, 0, -5, 0 }, { -5, 0, 5, 0, 5, 49, -5, 27 } };
static int Pos2[18][2][2] = {
// {{16:9 x,y},{ 4:3 x,y}}
        { { -91, 50 }, { -166, 50 } }, { { -91, 193 }, { -166, 193 } },

        { { 3, 50 }, { -54, 50 } }, { { 3, 193 }, { -54, 193 } },

        { { 97, 50 }, { 58, 50 } }, { { 97, 193 }, { 58, 193 } },

        { { 187, 50 }, { 166, 50 } }, { { 187, 193 }, { 166, 193 } },

        { { 272, 50 }, { 272, 50 } }, { { 272, 193 }, { 272, 193 } },

        { { 358, 50 }, { 378, 50 } }, { { 358, 193 }, { 378, 193 } },

        { { 449, 50 }, { 487, 50 } }, { { 449, 193 }, { 487, 193 } },

        { { 545, 50 }, { 599, 50 } }, { { 545, 193 }, { 599, 193 } },

        { { 641, 50 }, { 700, 50 } }, { { 641, 193 }, { 700, 193 } } };
static int Skew3[45][8] = { { -38, -110, 15, -42, 15, 65, -38, 32 }, { -38, -75, 15, -48, 15, 45, -38, 72 }, { -38,
        -52, 15, -70, 15, 27, -38, 100 },

{ -38, -110, 15, -42, 15, 65, -38, 32 }, { -38, -75, 15, -48, 15, 45, -38, 72 },
        { -38, -52, 15, -70, 15, 27, -38, 100 },

        { -38, -70, 15, -24, 15, 40, -38, 27 }, { -38, -50, 15, -35, 15, 40, -38, 50 }, { -38, -34, 15, -47, 15, 24,
                -38, 58 },

        { -27, -55, 19, -22, 19, 30, -27, 22 }, { -27, -40, 19, -30, 19, 30, -27, 40 }, { -27, -20, 19, -30, 19, 20,
                -27, 50 },

        { -19, -28, 0, -17, 0, 15, -19, 10 }, { -19, -30, 0, -20, 0, 12, -19, 30 },
        { -19, -15, 0, -20, 0, 10, -19, 24 },

        { -10, -20, 3, -13, 3, 14, -10, 10 }, { -10, -20, 3, -18, 3, 18, -10, 20 },
        { -10, -10, 3, -10, 3, 0, -10, 10 },

        { -10, -15, 3, -12, 3, 13, -10, 13 },
        { -10, -17, 3, -10, 3, 10, -10, 17 },
        { -10, -10, 3, -15, 3, 10, -10, 10 },

        { -10, -10, 3, -10, 3, 14, -10, 14 },
        { -10, -10, 3, -10, 3, 10, -10, 10 },//middle
        { -10, -10, 3, -10, 3, 10, -10, 10 },

        { -14, -10, 4, -20, 3, 10, -14, 10 }, { -14, -10, 4, -17, 3, 17, -14, 10 },
        { -14, -10, 4, -10, 3, 10, -14, 10 },

        { -10, -13, 3, -20, 3, 14, -10, 10 }, { -10, -18, 3, -20, 3, 20, -10, 18 },
        { -10, -10, 3, -10, 3, 20, -10, 5 },

        { -19, -17, 0, -28, 0, 10, -19, 15 }, { -19, -20, 0, -30, 0, 30, -19, 12 },
        { -19, -20, 0, -15, 0, 30, -19, 10 },

        { -27, -22, 19, -55, 19, 22, -27, 30 }, { -27, -30, 19, -40, 19, 40, -27, 30 }, { -27, -30, 19, -20, 19, 55,
                -27, 20 },

        { -38, -24, 15, -70, 15, 27, -38, 40 }, { -38, -35, 15, -50, 15, 50, -38, 40 }, { -38, -47, 15, -34, 15, 58,
                -38, 24 },

        { -38, -42, 15, -110, 15, 32, -38, 60 }, { -38, -48, 15, -75, 15, 70, -38, 45 }, { -38, -70, 15, -52, 15, 100,
                -38, 27 },

        { -38, -42, 15, -110, 15, 32, -38, 60 }, { -38, -48, 15, -75, 15, 70, -38, 45 }, { -38, -70, 15, -52, 15, 100,
                -38, 27 } };
static int Pos3[45][2][2] = {
// {{16:9 x,y},{ 4:3 x,y}}
        { { -42, 49 }, { -91, 49 } }, { { -42, 153 }, { -91, 153 } }, { { -42, 261 }, { -91, 261 } },

        { { 13, 58 }, { -29, 58 } }, { { 13, 153 }, { -29, 153 } }, { { 13, 250 }, { -29, 250 } },

        { { 68, 67 }, { 33, 67 } }, { { 68, 153 }, { 33, 153 } }, { { 68, 239 }, { 33, 239 } },

        { { 120, 74 }, { 92, 74 } }, { { 120, 153 }, { 92, 153 } }, { { 120, 232 }, { 92, 232 } },

        { { 170, 78 }, { 149, 78 } }, { { 170, 153 }, { 149, 153 } }, { { 170, 228 }, { 149, 228 } },

        { { 214, 80 }, { 200, 80 } }, { { 214, 153 }, { 200, 153 } }, { { 214, 226 }, { 200, 226 } },

        { { 258, 81 }, { 251, 81 } }, { { 258, 153 }, { 251, 153 } }, { { 258, 224 }, { 251, 224 } },

        { { 302, 81 }, { 302, 81 } }, { { 302, 153 }, { 302, 153 } }, { { 302, 223 }, { 302, 223 } },

        { { 346, 81 }, { 353, 81 } }, { { 346, 153 }, { 353, 153 } }, { { 346, 223 }, { 353, 223 } },

        { { 390, 80 }, { 404, 80 } }, { { 390, 153 }, { 404, 153 } }, { { 390, 225 }, { 404, 225 } },

        { { 434, 77 }, { 457, 77 } }, { { 434, 153 }, { 457, 153 } }, { { 434, 227 }, { 457, 227 } },

        { { 484, 73 }, { 512, 73 } }, { { 484, 153 }, { 512, 153 } }, { { 484, 231 }, { 512, 231 } },

        { { 537, 67 }, { 572, 67 } }, { { 537, 153 }, { 572, 153 } }, { { 537, 239 }, { 572, 239 } },

        { { 591, 58 }, { 633, 58 } }, { { 591, 153 }, { 633, 153 } }, { { 591, 250 }, { 633, 250 } },

        { { 660, 58 }, { 660, 58 } }, { { 660, 153 }, { 660, 153 } }, { { 660, 250 }, { 660, 250 } }

};
#define VALUE4ROWS(rows, val1, val2, val3)  (rows==3 ? val3 : (rows==2 ? val2 : val1))
#define ROWS2PAGESIZE(rows) (rows==3 ? 45 : (rows==2 ? 18 : 7))
static inline int OFFSETLIMIT(int Offset, int rows, int gameCnt)
{
    gameCnt += (rows - (gameCnt % rows)) % rows; // add count of skiped Entries at end if List
    while (Offset > gameCnt)
        Offset -= gameCnt;
    while (Offset < 0)
        Offset += gameCnt;
    return Offset;
}

// Help-Function to Calc GameIndex
static int GetGameIndex(int pageEntry, int rows, int listOffset, int gameCnt)
{
    int skip = (rows - (gameCnt % rows)) % rows; // count of skiped Entries at end if List
    int pagesize = ROWS2PAGESIZE( rows );

    if (gameCnt < (pagesize - 2 * rows))
    {
        int listStart = (pagesize - gameCnt) >> 1; // align list on the center
        listStart = listStart - (listStart % rows); // align listStart to the top row
        if (pageEntry < listStart || pageEntry >= listStart + gameCnt) return -1;
        return pageEntry - listStart;
    }
    else
    {
        listOffset = listOffset - (listOffset % rows); // align listOffset to the top row
        listOffset = listOffset - 2 * rows; // align listOffset to the left full visible column
        if (listOffset < 0) listOffset += gameCnt + skip; // set the correct Offset
        pageEntry = (listOffset + pageEntry) % (gameCnt + skip); // get offset of pageEntry
        if (pageEntry >= gameCnt) return -1;
        return pageEntry;
    }
}
static GuiImageData *GameGridLoadCoverImage(void * Arg)
{
    return LoadCoverImage((struct discHdr *) Arg, false, false);
}
/**
 * Constructor for the GuiGamegrid class.
 */
GuiGameGrid::GuiGameGrid(int w, int h, const char *themePath, const u8 *imagebg, int selected, int offset) :
    noCover(nocoverFlat_png)
{
    width = w;
    height = h;
    //  gameCnt = count;            will be set later in Reload
    //  gameList = l;               will be set later in Reload
    //  listOffset = 0;             will be set later in Reload
    //  goLeft = 0;                 will be set later in Reload
    //  goRight = 0;                will be set later in Reload

    selectable = true;
    focus = 1; // allow focus
    //  selectedItem = -1;          will be set later in Reload
    //  clickedItem = -1;           will be set later in Reload
    /*          will be set later in Reload
     rows = Settings.gridRows;
     if ((count<45)&&(rows==3))rows=2;
     if ((count<18)&&(rows==2))rows=1;

     pagesize = ROWS2PAGESIZE(rows);
     */
    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigL = new GuiTrigger;
    trigL->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
    trigR = new GuiTrigger;
    trigR->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
    trig1 = new GuiTrigger;
    trig1->SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_X, PAD_BUTTON_X);
    trig2 = new GuiTrigger;
    trig2->SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_Y, PAD_BUTTON_Y);
    trigPlus = new GuiTrigger;
    trigPlus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, 0);
    trigMinus = new GuiTrigger;
    trigMinus->SetButtonOnlyTrigger(-1, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS, 0);

    btnSoundClick = new GuiSound(button_click2_pcm, button_click2_pcm_size, Settings.sfxvolume);
    btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);

    int btnHeight = (int) lround(sqrt(RADIUS * RADIUS - 90000) - RADIUS - 50);

    // Button Left
    btnLeft = new GuiButton(0, 0);
    btnLeft->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
    btnLeft->SetPosition(20, btnHeight);
    btnLeft->SetParent(this);
    btnLeft->SetSoundOver(btnSoundOver);
    btnLeft->SetTrigger(trigL);
    btnLeft->SetTrigger(trigMinus);

    // Button Right
    btnRight = new GuiButton(0, 0);
    btnRight->SetParent(this);
    btnRight->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
    btnRight->SetPosition(-20, btnHeight);
    btnRight->SetSoundOver(btnSoundOver);
    btnRight->SetTrigger(trigR);
    btnRight->SetTrigger(trigPlus);

    // Button RowUp
    btnRowUp = new GuiButton(0, 0);
    btnRowUp->SetParent(this);
    btnRowUp->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    btnRowUp->SetPosition(0, 0);
    btnRowUp->SetTrigger(trig2);

    // Button RowDown
    btnRowDown = new GuiButton(0, 0);
    btnRowDown->SetParent(this);
    btnRowDown->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    btnRowDown->SetPosition(0, 0);
    btnRowDown->SetTrigger(trig1);

    // Page-Stuff
    gameIndex = NULL;
    titleTT = NULL;
    //  cover       = NULL;
    coverImg = NULL;
    game = NULL;

    Reload(Settings.gridRows, 0);
}

/**
 * Destructor for the GuiGameGrid class.
 */
GuiGameGrid::~GuiGameGrid()
{

    delete btnRight;
    delete btnLeft;
    delete btnRowUp;
    delete btnRowDown;

    delete trigA;
    delete trigL;
    delete trigR;
    delete trigPlus;
    delete trigMinus;
    delete trig1;
    delete trig2;
    delete btnSoundClick;
    delete btnSoundOver;

    for (int i = pagesize - 1; i >= 0; i--)
    {
        delete game[i];
        delete coverImg[i];
        delete titleTT[i];
    }

    delete[] gameIndex;
    delete[] game;
    delete[] coverImg;
    delete[] titleTT;
}

void GuiGameGrid::SetFocus(int f)
{
    LOCK( this );
    if (!gameList.size()) return;

    focus = f;

    for (int i = 0; i < pagesize; i++)
        game[i]->ResetState();

    if (f == 1 && selectedItem >= 0) game[selectedItem]->SetState(STATE_SELECTED);
}

void GuiGameGrid::ResetState()
{
    LOCK( this );
    if (state != STATE_DISABLED)
    {
        state = STATE_DEFAULT;
        stateChan = -1;
    }

    for (int i = 0; i < pagesize; i++)
    {
        game[i]->ResetState();
    }
}

int GuiGameGrid::GetOffset()
{
    LOCK( this );
    return listOffset;
}

int GuiGameGrid::GetClickedOption()
{
    LOCK( this );
    int found = -1;
    if (clickedItem >= 0)
    {
        game[clickedItem]->SetState(STATE_SELECTED);
        found = gameIndex[clickedItem];
        clickedItem = -1;
    }
    return found;
}

int GuiGameGrid::GetSelectedOption()
{
    LOCK( this );
    int found = -1;
    for (int i = 0; i < pagesize; i++)
    {
        if (game[i]->GetState() == STATE_SELECTED)
        {
            game[i]->SetState(STATE_SELECTED);
            found = gameIndex[i];
            break;
        }
    }
    return found;
}

/**
 * Draw the button on screen
 */
void GuiGameGrid::Draw()
{
    LOCK( this );
    if (!this->IsVisible() || !gameList.size()) return;

    if (goLeft > 0)
    {
        goLeft--;
        int wsi = Settings.widescreen ? 0 : 1;
        float f2 = ((float) goLeft) / goSteps;
        float f1 = 1.0 - f2;
        int (*Pos)[2][2] = VALUE4ROWS( rows, Pos1, Pos2, Pos3 );
        int (*Skew)[8] = VALUE4ROWS( rows, Skew1, Skew2, Skew3 );

        for (int i = 0; i < pagesize - rows; i++)
        {
            game[i]->SetPosition(Pos[i][wsi][0] * f1 + Pos[i + rows][wsi][0] * f2 + THEME.gamegrid_x, Pos[i][wsi][1]
                    * f1 + Pos[i + rows][wsi][1] * f2 + THEME.gamegrid_y);

            game[i]->SetSkew(Skew[i][0] * f1 + Skew[i + rows][0] * f2, Skew[i][1] * f1 + Skew[i + rows][1] * f2,
                    Skew[i][2] * f1 + Skew[i + rows][2] * f2, Skew[i][3] * f1 + Skew[i + rows][3] * f2, Skew[i][4] * f1
                            + Skew[i + rows][4] * f2, Skew[i][5] * f1 + Skew[i + rows][5] * f2, Skew[i][6] * f1
                            + Skew[i + rows][6] * f2, Skew[i][7] * f1 + Skew[i + rows][7] * f2);
        }
    }
    else if (goRight > 0)
    {
        goRight--;
        int wsi = Settings.widescreen ? 0 : 1;
        float f2 = ((float) goRight) / goSteps;
        float f1 = 1.0 - f2;
        int (*Pos)[2][2] = VALUE4ROWS( rows, Pos1, Pos2, Pos3 );
        int (*Skew)[8] = VALUE4ROWS( rows, Skew1, Skew2, Skew3 );
        for (int i = rows; i < pagesize; i++)
        {
            game[i]->SetPosition(Pos[i][wsi][0] * f1 + Pos[i - rows][wsi][0] * f2 + THEME.gamegrid_x, Pos[i][wsi][1]
                    * f1 + Pos[i - rows][wsi][1] * f2 + THEME.gamegrid_y);

            game[i]->SetSkew(Skew[i][0] * f1 + Skew[i - rows][0] * f2, Skew[i][1] * f1 + Skew[i - rows][1] * f2,
                    Skew[i][2] * f1 + Skew[i - rows][2] * f2, Skew[i][3] * f1 + Skew[i - rows][3] * f2, Skew[i][4] * f1
                            + Skew[i - rows][4] * f2, Skew[i][5] * f1 + Skew[i - rows][5] * f2, Skew[i][6] * f1
                            + Skew[i - rows][6] * f2, Skew[i][7] * f1 + Skew[i - rows][7] * f2);
        }
    }

    for (int i = 0; i < pagesize; i++)
        game[i]->Draw();
    if (gameList.size() > pagesize - 2 * rows)
    {
        btnRight->Draw();
        btnLeft->Draw();
    }

    btnRowUp->Draw();
    btnRowDown->Draw();

    if (focus && Settings.tooltips == TooltipsOn) for (int i = 0; i < pagesize; i++)
        game[i]->DrawTooltip();

    this->UpdateEffects();
}

/**
 * Change the number of rows
 */
void GuiGameGrid::ChangeRows(int n)
{
    LOCK( this );
    if (n != rows) Reload(n, -1);
}

void GuiGameGrid::Update(GuiTrigger * t)
{
    LOCK( this );
    if (state == STATE_DISABLED || !t || !gameList.size()) return;

    if (!(game[0]->GetEffect() || game[0]->GetEffectOnOver()))
    {
        for (int i = 0; i < pagesize; i++)
            game[i]->SetEffectGrow();
    }

    btnRight->Update(t);
    btnLeft->Update(t);
    btnRowUp->Update(t);
    btnRowDown->Update(t);

    selectedItem = -1;
    clickedItem = -1;
    for (int i = 0; i < pagesize; i++)
    {
        game[i]->Update(t);
        if (game[i]->GetState() == STATE_SELECTED)
        {
            selectedItem = i;
        }
        if (game[i]->GetState() == STATE_CLICKED)
        {
            clickedItem = i;
        }

    }
    // navigation
    if (focus && gameList.size() >= (pagesize - 2 * rows) && goLeft == 0 && goRight == 0)
    {
        // Left/Right Navigation

        if (btnLeft->GetState() == STATE_CLICKED)
        {
            WPAD_ScanPads();
            u16 buttons = 0;
            for (int i = 0; i < 4; i++)
                buttons |= WPAD_ButtonsHeld(i);
            if (!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_MINUS) || t->Left()))
            {
                btnLeft->ResetState();
                return;
            }

            if (Settings.xflip == sysmenu || Settings.xflip == yes || Settings.xflip == disk3d)
                goRight = goSteps;
            else goLeft = goSteps;
        }
        else if (btnRight->GetState() == STATE_CLICKED)
        {
            WPAD_ScanPads();
            u16 buttons = 0;
            for (int i = 0; i < 4; i++)
                buttons |= WPAD_ButtonsHeld(i);
            if (!((buttons & WPAD_BUTTON_A) || (buttons & WPAD_BUTTON_PLUS) || t->Right()))
            {
                btnRight->ResetState();
                return;
            }
            if (Settings.xflip == sysmenu || Settings.xflip == yes || Settings.xflip == disk3d)
                goLeft = goSteps;
            else goRight = goSteps;
        }

        if (goLeft == goSteps)
        {
            GuiButton *tmpButton[rows];
            GuiTooltip *tmpTooltip[rows];
            listOffset = OFFSETLIMIT(listOffset + rows, rows, gameList.size()); // set the new listOffset
            // Save left Tooltip & Button and destroy left Image + Image-Data
            for (int i = 0; i < rows; i++)
            {
                delete coverImg[i];
                coverImg[i] = NULL;
                game[i]->SetImage(NULL);
                tmpTooltip[i] = titleTT[i];
                tmpButton[i] = game[i];
            }
            // Move all Page-Entries one step left
            for (int i = 0; i < (pagesize - rows); i++)
            {
                titleTT[i] = titleTT[i + rows];
                coverImg[i] = coverImg[i + rows];
                game[i] = game[i + rows];
                gameIndex[i] = gameIndex[i + rows];
            }
            // set saved Tooltip, Button & gameIndex to right
            int wsi = Settings.widescreen ? 0 : 1;
            int (*Pos)[2][2] = VALUE4ROWS( rows, Pos1, Pos2, Pos3 );
            int (*Skew)[8] = VALUE4ROWS( rows, Skew1, Skew2, Skew3 );

            for (int i = 0; i < rows; i++)
            {
                int ii = i + pagesize - rows;
                gameIndex[ii] = GetGameIndex(ii, rows, listOffset, gameList.size());
                titleTT[ii] = tmpTooltip[i];
                coverImg[ii] = NULL;
                if (gameIndex[ii] != -1)
                {
                    coverImg[ii] = new GuiImageAsync(GameGridLoadCoverImage, gameList[gameIndex[ii]],
                            sizeof(struct discHdr), &noCover);
                    if (coverImg[ii])
                    {
                        coverImg[ii] ->SetWidescreen(Settings.widescreen);
                        coverImg[ii] ->SetScale(VALUE4ROWS( rows, 1.0, 0.6, 0.26 ));
                        coverImg[ii] ->SetPosition(0, VALUE4ROWS( rows, 0, -50, -80 ));
                    }
                    titleTT[ii] ->SetText(get_title(gameList[gameIndex[ii]]));
                }
                else
                {
                    titleTT[ii] ->SetText(NULL);
                }

                game[ii] = tmpButton[i];
                game[ii] ->SetImage(coverImg[ii]);
                game[ii] ->SetPosition(Pos[ii][wsi][0], Pos[ii][wsi][1]);
                game[ii] ->SetSkew(&Skew[ii][0]);
                game[ii] ->RemoveToolTip();
                if (gameIndex[ii] != -1)
                {
                    game[ii] ->SetClickable(true);
                    game[ii] ->SetVisible(true);
                }
                else
                {
                    game[ii] ->SetVisible(false);
                    game[ii] ->SetClickable(false);
                    game[ii] ->RemoveSoundOver();
                }
            }
            // Set Tooltip-Position
            int ttoffset_x = Settings.widescreen ? VALUE4ROWS( rows, 70, 35, 0 ) : VALUE4ROWS( rows, 150, 55, 25 );
            int ttoffset_y = -VALUE4ROWS( rows, 224, 133, 68 ) / 4;
            for (int i = 0; i < pagesize; i++)
            {
                switch ((i * 3) / pagesize)
                {
                    case 0:
                        game[i]->SetToolTip(titleTT[i], ttoffset_x, ttoffset_y, ALIGN_LEFT, ALIGN_MIDDLE);
                        break;
                    case 1:
                        game[i]->SetToolTip(titleTT[i], 0, ttoffset_y, ALIGN_CENTRE, ALIGN_MIDDLE);
                        break;
                    case 2:
                        game[i]->SetToolTip(titleTT[i], -ttoffset_x, ttoffset_y, ALIGN_RIGHT, ALIGN_MIDDLE);
                        break;
                    default:
                        break;
                }
            }
        }
        else if (goRight == goSteps)
        {
            GuiButton *tmpButton[rows];
            GuiTooltip *tmpTooltip[rows];
            listOffset = OFFSETLIMIT(listOffset - rows, rows, gameList.size()); // set the new listOffset
            // Save right Button & Tooltip and destroy right Image-Data
            for (int i = 0; i < rows; i++)
            {
                int ii = i + pagesize - rows;
                delete coverImg[ii];
                coverImg[ii] = NULL;
                game[ii]->SetImage(NULL);
                tmpTooltip[i] = titleTT[ii];
                tmpButton[i] = game[ii];
            }
            // Move all Page-Entries one step right
            for (int i = pagesize - 1; i >= rows; i--)
            {
                titleTT[i] = titleTT[i - rows];
                coverImg[i] = coverImg[i - rows];
                game[i] = game[i - rows];
                gameIndex[i] = gameIndex[i - rows];
            }
            // set saved Image, Button & gameIndex to left
            int wsi = Settings.widescreen ? 0 : 1;
            int (*Pos)[2][2] = VALUE4ROWS( rows, Pos1, Pos2, Pos3 );
            int (*Skew)[8] = VALUE4ROWS( rows, Skew1, Skew2, Skew3 );

            for (int i = 0; i < rows; i++)
            {
                gameIndex[i] = GetGameIndex(i, rows, listOffset, gameList.size());
                titleTT[i] = tmpTooltip[i];
                coverImg[i] = NULL;
                if (gameIndex[i] != -1)
                {
                    coverImg[i] = new GuiImageAsync(GameGridLoadCoverImage, gameList[gameIndex[i]],
                            sizeof(struct discHdr), &noCover);
                    if (coverImg[i])
                    {
                        coverImg[i] ->SetWidescreen(Settings.widescreen);
                        coverImg[i] ->SetScale(VALUE4ROWS( rows, 1.0, 0.6, 0.26 ));
                        coverImg[i] ->SetPosition(0, VALUE4ROWS( rows, 0, -50, -80 ));
                    }
                    titleTT[i] ->SetText(get_title(gameList[gameIndex[i]]));
                }
                else
                {
                    titleTT[i] ->SetText(NULL);
                }
                game[i] = tmpButton[i];
                game[i] ->SetImage(coverImg[i]);
                game[i] ->SetPosition(Pos[i][wsi][0], Pos[i][wsi][1]);
                game[i] ->SetSkew(&Skew[i][0]);
                game[i] ->RemoveToolTip();
                if (gameIndex[i] != -1)
                {
                    game[i] ->SetClickable(true);
                    game[i] ->SetVisible(true);
                }
                else
                {
                    game[i] ->SetVisible(false);
                    game[i] ->SetClickable(false);
                    game[i] ->RemoveSoundOver();
                }
            }
            // Set Tooltip-Position
            int ttoffset_x = Settings.widescreen ? VALUE4ROWS( rows, 70, 35, 0 ) : VALUE4ROWS( rows, 150, 55, 25 );
            int ttoffset_y = -VALUE4ROWS( rows, 224, 133, 68 ) / 4;
            for (int i = 0; i < pagesize; i++)
            {
                switch ((i * 3) / pagesize)
                {
                    case 0:
                        game[i]->SetToolTip(titleTT[i], ttoffset_x, ttoffset_y, ALIGN_LEFT, ALIGN_MIDDLE);
                        break;
                    case 1:
                        game[i]->SetToolTip(titleTT[i], 0, ttoffset_y, ALIGN_CENTRE, ALIGN_MIDDLE);
                        break;
                    case 2:
                        game[i]->SetToolTip(titleTT[i], -ttoffset_x, ttoffset_y, ALIGN_RIGHT, ALIGN_MIDDLE);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if ((btnRowUp->GetState() == STATE_CLICKED))
    {
        if ((rows == 1) && (gameList.size() >= 18))
            this->ChangeRows(2);
        else if ((rows == 2) && (gameList.size() >= 45)) this->ChangeRows(3);
        btnRowUp->ResetState();
        return;
    }

    if ((btnRowDown->GetState() == STATE_CLICKED))
    {
        if (rows == 3)
            this->ChangeRows(2);
        else if (rows == 2) this->ChangeRows(1);
        btnRowDown->ResetState();
        return;
    }

    if (updateCB) updateCB(this);
}

void GuiGameGrid::Reload(int Rows, int ListOffset)
{
    LOCK( this );

    // CleanUp
    if (game) for (int i = pagesize - 1; i >= 0; i--)
        delete game[i];

    if (coverImg) for (int i = pagesize - 1; i >= 0; i--)
        delete coverImg[i];

    //  if(cover)
    //      for(int i=pagesize-1; i>=0; i--)
    //          delete cover[i];

    if (titleTT) for (int i = pagesize - 1; i >= 0; i--)
        delete titleTT[i];

    delete[] gameIndex;
    delete[] game;
    delete[] coverImg;
    //  delete [] cover;
    delete[] titleTT;

    goLeft = 0;
    goRight = 0;

    rows = Rows > 3 ? 3 : (Rows < 1 ? 1 : Rows);
    if ((gameList.size() < 45) && (rows == 3)) rows = 2;
    if ((gameList.size() < 18) && (rows == 2)) rows = 1;

    if (ListOffset >= 0) // if ListOffset < 0 then no change
    listOffset = ListOffset;
    listOffset = OFFSETLIMIT(listOffset, rows, gameList.size());

    selectedItem = -1;
    clickedItem = -1;

    pagesize = ROWS2PAGESIZE( rows );

    // Page-Stuff
    gameIndex = new int[pagesize];
    titleTT = new GuiTooltip *[pagesize];
    //  cover       = new GuiImageData *[pagesize];
    coverImg = new GuiImageAsync *[pagesize];
    game = new GuiButton *[pagesize];

    int wsi = Settings.widescreen ? 0 : 1;
    int (*Pos)[2][2] = VALUE4ROWS( rows, Pos1, Pos2, Pos3 );
    int (*Skew)[8] = VALUE4ROWS( rows, Skew1, Skew2, Skew3 );

    int ttoffset_x = Settings.widescreen ? VALUE4ROWS( rows, 70, 35, 0 ) : VALUE4ROWS( rows, 150, 55, 25 );
    int ttoffset_y = -VALUE4ROWS( rows, 224, 133, 68 ) / 4;

    for (int i = 0; i < pagesize; i++)
    {
        //------------------------
        // Index
        //------------------------
        gameIndex[i] = GetGameIndex(i, rows, listOffset, gameList.size());

        //------------------------
        // Tooltip
        //------------------------
        if (gameIndex[i] != -1)
            titleTT[i] = new GuiTooltip(get_title(gameList[gameIndex[i]]), THEME.tooltipAlpha);
        else titleTT[i] = new GuiTooltip(NULL, THEME.tooltipAlpha);

        //------------------------
        // ImageData
        //------------------------
        //      if( gameIndex[i] != -1 )
        //          cover[i] = LoadCoverImage(&gameList[gameIndex[i]], false /*bool Prefere3D*/);
        //      else
        //          cover[i] = new GuiImageData(NULL);

        //------------------------
        // Image
        //------------------------
        coverImg[i] = NULL;
        if (gameIndex[i] != -1)
        {
            coverImg[i] = new GuiImageAsync(GameGridLoadCoverImage, gameList[gameIndex[i]], sizeof(struct discHdr),
                    &noCover);
            if (coverImg[i])
            {
                coverImg[i]->SetWidescreen(Settings.widescreen);
                //      if ( rows == 2 )        coverImg[i]->SetScale(.6);      //these are the numbers for 2 rows
                //      else if ( rows == 3 )   coverImg[i]->SetScale(.26); //these are the numbers for 3 rows
                coverImg[i]->SetScale(VALUE4ROWS( rows, 1.0, 0.6, 0.26 ));
                coverImg[i]->SetPosition(0, VALUE4ROWS( rows, 0, -50, -80 ));
            }
        }

        //------------------------
        // GameButton
        //------------------------
        game[i] = new GuiButton(VALUE4ROWS( rows, 160, 75, 35 ), VALUE4ROWS( rows, 224, 133, 68 ));
        game[i]->SetParent(this);
        game[i]->SetImage(coverImg[i]);
        game[i]->SetAlignment(ALIGN_TOP, ALIGN_LEFT);
        game[i]->SetPosition(Pos[i][wsi][0] + THEME.gamegrid_x, Pos[i][wsi][1] + THEME.gamegrid_y);
        game[i]->SetSkew(&Skew[i][0]);
        game[i]->SetTrigger(trigA);
        game[i]->SetSoundOver(btnSoundOver);
        game[i]->SetSoundClick(btnSoundClick);
        game[i]->SetRumble(false);
        switch ((i * 3) / pagesize)
        {
            case 0:
                game[i]->SetToolTip(titleTT[i], ttoffset_x, ttoffset_y, ALIGN_LEFT, ALIGN_MIDDLE);
                break;
            case 1:
                game[i]->SetToolTip(titleTT[i], 0, ttoffset_y, ALIGN_CENTRE, ALIGN_MIDDLE);
                break;
            case 2:
                game[i]->SetToolTip(titleTT[i], -ttoffset_x, ttoffset_y, ALIGN_RIGHT, ALIGN_MIDDLE);
                break;
            default:
                break;
        }
        if (gameIndex[i] >= 0)
        {
            game[i]->SetClickable(true);
            game[i]->SetVisible(true);
        }
        else
        {
            game[i]->SetVisible(false);
            game[i]->SetClickable(false);
            //      game[i]->RemoveSoundOver();
        }
    }
    Settings.gridRows = rows;
}

