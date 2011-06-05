/****************************************************************************
 * libwiigui
 *
 * gui_gamebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"

#include <unistd.h>
#include "gui_gamebrowser.h"
#include "../settings/CSettings.h"
#include "../main.h"
#include "settings/newtitles.h"
#include "settings/GameTitles.h"
#include "usbloader/GameList.h"
#include "themes/CTheme.h"
#include "menu.h"

#include <string.h>
#include <sstream>

#define GAMESELECTSIZE      30
// scrolldelay affects how fast the list scrolls
// when the arrows are clicked
static const u32 DEFAULT_SCROLL_DELAY = 4;

/**
 * Constructor for the GuiGameBrowser class.
 */
GuiGameBrowser::GuiGameBrowser(int w, int h, int selectedGame)
{
    width = w;
    height = h;
    pagesize = thInt("9 - game list browser page size");
    scrollbaron = (gameList.size() > pagesize) ? 1 : 0;
    selectable = true;
    listOffset = selectedGame - (selectedGame % pagesize);
    selectedItem = selectedGame - listOffset;
    focus = 1; // allow focus

    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
    trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A, PAD_BUTTON_A);

    bgGames = Resources::GetImageData("bg_options.png");
    newGames = Resources::GetImageData("new.png");

    bgGameImg = new GuiImage(bgGames);
    bgGameImg->SetParent(this);
    bgGameImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

    maxTextWidth = bgGameImg->GetWidth() - 24 - 4;

    bgGamesEntry = Resources::GetImageData("bg_options_entry.png");

    scrollbar = Resources::GetImageData("scrollbar.png");
    scrollbarImg = new GuiImage(scrollbar);
    scrollbarImg->SetParent(this);
    scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    scrollbarImg->SetPosition(0, 4);

    maxTextWidth -= scrollbarImg->GetWidth() + 4;

    arrowDown = Resources::GetImageData("scrollbar_arrowdown.png");
    arrowDownImg = new GuiImage(arrowDown);
    arrowDownOver = Resources::GetImageData("scrollbar_arrowdown.png");
    arrowDownOverImg = new GuiImage(arrowDownOver);
    arrowUp = Resources::GetImageData("scrollbar_arrowup.png");
    arrowUpImg = new GuiImage(arrowUp);
    arrowUpOver = Resources::GetImageData("scrollbar_arrowup.png");
    arrowUpOverImg = new GuiImage(arrowUpOver);
    scrollbarBox = Resources::GetImageData("scrollbar_box.png");
    scrollbarBoxImg = new GuiImage(scrollbarBox);
    scrollbarBoxOver = Resources::GetImageData("scrollbar_box.png");
    scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

    arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
    arrowUpBtn->SetParent(this);
    arrowUpBtn->SetImage(arrowUpImg);
    arrowUpBtn->SetImageOver(arrowUpOverImg);
    arrowUpBtn->SetImageHold(arrowUpOverImg);
    arrowUpBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    arrowUpBtn->SetPosition(width / 2 - 18 + 7, -18);
    arrowUpBtn->SetSelectable(false);
    arrowUpBtn->SetTrigger(trigA);
    arrowUpBtn->SetEffectOnOver(EFFECT_SCALE, 50, 130);
    arrowUpBtn->SetSoundClick(btnSoundClick);

    arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
    arrowDownBtn->SetParent(this);
    arrowDownBtn->SetImage(arrowDownImg);
    arrowDownBtn->SetImageOver(arrowDownOverImg);
    arrowDownBtn->SetImageHold(arrowDownOverImg);
    arrowDownBtn->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    arrowDownBtn->SetPosition(width / 2 - 18 + 7, 18);
    arrowDownBtn->SetSelectable(false);
    arrowDownBtn->SetTrigger(trigA);
    arrowDownBtn->SetEffectOnOver(EFFECT_SCALE, 50, 130);
    arrowDownBtn->SetSoundClick(btnSoundClick);

    scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
    scrollbarBoxBtn->SetParent(this);
    scrollbarBoxBtn->SetImage(scrollbarBoxImg);
    scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
    scrollbarBoxBtn->SetImageHold(scrollbarBoxOverImg);
    scrollbarBoxBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    scrollbarBoxBtn->SetSelectable(false);
    scrollbarBoxBtn->SetEffectOnOver(EFFECT_SCALE, 50, 120);
    scrollbarBoxBtn->SetMinY(0);
    scrollbarBoxBtn->SetMaxY(height - 30);
    scrollbarBoxBtn->SetHoldable(true);
    scrollbarBoxBtn->SetTrigger(trigHeldA);

    gameIndex = new int[pagesize];
    game = new GuiButton *[pagesize];
    gameTxt = new GuiText *[pagesize];
    gameTxtOver = new GuiText *[pagesize];
    gameBg = new GuiImage *[pagesize];
    newImg = new GuiImage *[pagesize];

    for (int i = 0; i < pagesize; i++)
    {
        gameTxt[i] = new GuiText(GameTitles.GetTitle(gameList[i]), 20, thColor("r=0 g=0 b=0 a=255 - game browser list text color"));
        gameTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        gameTxt[i]->SetPosition(24, 0);
        gameTxt[i]->SetMaxWidth(maxTextWidth, DOTTED);

        gameTxtOver[i] = new GuiText(GameTitles.GetTitle(gameList[i]), 20, thColor("r=0 g=0 b=0 a=255 - game browser list text color over"));
        gameTxtOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        gameTxtOver[i]->SetPosition(24, 0);
        gameTxtOver[i]->SetMaxWidth(maxTextWidth, SCROLL_HORIZONTAL);

        gameBg[i] = new GuiImage(bgGamesEntry);

        newImg[i] = new GuiImage(newGames);
        newImg[i]->SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
        newImg[i]->SetVisible(false);

        game[i] = new GuiButton(width - 28, GAMESELECTSIZE);
        game[i]->SetParent(this);
        game[i]->SetLabel(gameTxt[i]);
        game[i]->SetLabelOver(gameTxtOver[i]);
        game[i]->SetIcon(newImg[i]);
        game[i]->SetImageOver(gameBg[i]);
        game[i]->SetPosition(5, GAMESELECTSIZE * i + 4);
        game[i]->SetRumble(false);
        game[i]->SetTrigger(trigA);
        game[i]->SetSoundClick(btnSoundClick);

        gameIndex[i] = i;
    }
    UpdateListEntries();
}

/**
 * Destructor for the GuiGameBrowser class.
 */
GuiGameBrowser::~GuiGameBrowser()
{
    delete arrowUpBtn;
    delete arrowDownBtn;
    delete scrollbarBoxBtn;
    delete scrollbarImg;
    delete arrowDownImg;
    delete arrowDownOverImg;
    delete arrowUpImg;
    delete arrowUpOverImg;
    delete scrollbarBoxImg;
    delete scrollbarBoxOverImg;
    delete scrollbar;
    delete arrowDown;
    delete arrowDownOver;
    delete arrowUp;
    delete arrowUpOver;
    delete scrollbarBox;
    delete scrollbarBoxOver;
    delete bgGameImg;
    delete bgGames;
    delete bgGamesEntry;
    delete newGames;

    delete trigA;
    delete trigHeldA;

    for (int i = 0; i < pagesize; i++)
    {
        delete gameTxt[i];
        delete gameTxtOver[i];
        delete gameBg[i];
        delete game[i];
        delete newImg[i];
    }
    delete[] gameIndex;
    delete[] game;
    delete[] gameTxt;
    delete[] gameTxtOver;
    delete[] gameBg;
}

void GuiGameBrowser::SetFocus(int f)
{
    LOCK( this );
    if (!gameList.size()) return;

    focus = f;

    for (int i = 0; i < pagesize; i++)
        game[i]->ResetState();

    if (f == 1) game[selectedItem]->SetState(STATE_SELECTED);
}

void GuiGameBrowser::ResetState()
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

int GuiGameBrowser::GetOffset()
{
    return listOffset;
}
int GuiGameBrowser::GetClickedOption()
{
    int found = -1;
    for (int i = 0; i < pagesize; i++)
    {
        if (game[i]->GetState() == STATE_CLICKED)
        {
            game[i]->SetState(STATE_SELECTED);
            found = gameIndex[i];
            break;
        }
    }
    return found;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiGameBrowser::FindMenuItem(int currentItem, int direction)
{
    int nextItem = currentItem + direction;

    if (nextItem < 0 || nextItem >= gameList.size()) return -1;

    if (strlen(GameTitles.GetTitle(gameList[nextItem])) > 0)
        return nextItem;

	return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiGameBrowser::Draw()
{
    LOCK( this );
    if (!this->IsVisible() || !gameList.size()) return;

    bgGameImg->Draw();

    int next = listOffset;

    for (int i = 0; i < pagesize; i++)
    {
        if (next >= 0)
        {
            game[i]->Draw();
            next = this->FindMenuItem(next, 1);
        }
        else break;
    }

    if (scrollbaron == 1)
    {
        scrollbarImg->Draw();
        arrowUpBtn->Draw();
        arrowDownBtn->Draw();
        scrollbarBoxBtn->Draw();
    }
    this->UpdateEffects();
}

void GuiGameBrowser::UpdateListEntries()
{
    int next = listOffset;
    for (int i = 0; i < pagesize; i++)
    {
        if (next >= 0)
        {
            if (game[i]->GetState() == STATE_DISABLED)
            {
                game[i]->SetVisible(true);
                game[i]->SetState(STATE_DEFAULT);
            }
            gameTxt[i]->SetText(GameTitles.GetTitle(gameList[next]));
            gameTxt[i]->SetPosition(24, 0);
            gameTxtOver[i]->SetText(GameTitles.GetTitle(gameList[next]));
            gameTxtOver[i]->SetPosition(24, 0);

            if (Settings.marknewtitles)
            {
                bool isNew = NewTitles::Instance()->IsNew(gameList[next]->id);
                if (isNew)
                {
                    gameTxt[i]->SetMaxWidth(maxTextWidth - (newGames->GetWidth() + 1), DOTTED);
                    gameTxtOver[i]->SetMaxWidth(maxTextWidth - (newGames->GetWidth() + 1), SCROLL_HORIZONTAL);
                }
                else
                {
                    gameTxt[i]->SetMaxWidth(maxTextWidth, DOTTED);
                    gameTxtOver[i]->SetMaxWidth(maxTextWidth, SCROLL_HORIZONTAL);
                }
                newImg[i]->SetVisible(isNew);
            }

            gameIndex[i] = next;
            next = this->FindMenuItem(next, 1);
        }
        else
        {
            game[i]->SetVisible(false);
            game[i]->SetState(STATE_DISABLED);
        }
    }
}

void GuiGameBrowser::Update(GuiTrigger * t)
{
    LOCK( this );
    if (state == STATE_DISABLED || !t || !gameList.size()) return;

    static int pressedChan = -1;
    int next, prev;
    int old_listOffset = listOffset;
    static int position2;
    static u32 scrolldelay = 0;

    if(t->wpad.btns_d)
    {
        pressedChan = t->chan;
    }

    if (scrollbaron == 1)
    {
        // update the location of the scroll box based on the position in the option list
        arrowUpBtn->Update(t);
        arrowDownBtn->Update(t);
        scrollbarBoxBtn->Update(t);
    }

    next = listOffset;

    if(pressedChan == -1 || (pressedChan == t->chan && !(t->wpad.btns_h & WPAD_BUTTON_UP) &&
       !(t->wpad.btns_h & WPAD_BUTTON_DOWN) && !(t->wpad.btns_h & WPAD_BUTTON_B)))
    {
        for (int i = 0; i < pagesize; i++)
        {
            if (next >= 0) next = this->FindMenuItem(next, 1);

            if (focus)
            {
                if (i != selectedItem && game[i]->GetState() == STATE_SELECTED)
                    game[i]->ResetState();
                else if (i == selectedItem && game[i]->GetState() == STATE_DEFAULT)
                    game[selectedItem]->SetState(STATE_SELECTED, -1);
            }

            game[i]->Update(t);

            if (game[i]->GetState() == STATE_SELECTED)
            {
                selectedItem = i;
            }
        }
    }

    // pad and joystick navigation
    if (!focus || !gameList.size()) return; // skip navigation

    if (scrollbaron == 1)
    {
        if (t->Down() || arrowDownBtn->GetState() == STATE_CLICKED || arrowDownBtn->GetState() == STATE_HELD) //down
        {
            if(scrolldelay > 0)
                --scrolldelay;
            else
            {
                if(arrowDownBtn->GetState() == STATE_CLICKED || arrowDownBtn->GetState() == STATE_HELD)
                    scrolldelay = DEFAULT_SCROLL_DELAY;
                next = this->FindMenuItem(gameIndex[selectedItem], 1);

                if (next >= 0)
                {
                    if (selectedItem == pagesize - 1)
                    {
                        // move list down by 1
                        listOffset = this->FindMenuItem(listOffset, 1);
                    }
                    else if (game[selectedItem + 1]->IsVisible())
                    {
                        game[selectedItem]->ResetState();
                        game[selectedItem + 1]->SetState(STATE_SELECTED, t->chan);
                        selectedItem++;
                    }
                }
            }
            if (pressedChan == -1 || (pressedChan == t->chan && !(t->wpad.btns_d & WPAD_BUTTON_A) && !(t->wpad.btns_h & WPAD_BUTTON_A)))
                arrowDownBtn->ResetState();
        }
        else if (t->Up() || arrowUpBtn->GetState() == STATE_CLICKED || arrowUpBtn->GetState() == STATE_HELD) //up
        {
            if(scrolldelay > 0)
                --scrolldelay;
            else
            {
                if(arrowUpBtn->GetState() == STATE_CLICKED || arrowUpBtn->GetState() == STATE_HELD)
                    scrolldelay = DEFAULT_SCROLL_DELAY;
                prev = this->FindMenuItem(gameIndex[selectedItem], -1);

                if (prev >= 0)
                {
                    if (selectedItem == 0)
                    {
                        // move list up by 1
                        listOffset = prev;
                    }
                    else
                    {
                        game[selectedItem]->ResetState();
                        game[selectedItem - 1]->SetState(STATE_SELECTED, t->chan);
                        selectedItem--;
                    }
                }
            }
            if (pressedChan == -1 || (pressedChan == t->chan && !(t->wpad.btns_d & WPAD_BUTTON_A) && !(t->wpad.btns_h & WPAD_BUTTON_A)))
                arrowUpBtn->ResetState();
        }
        int position1 = t->wpad.ir.y;

        if (position2 == 0 && t->wpad.ir.valid)
        {
            position2 = position1;
        }

        if (pressedChan == t->chan && (t->wpad.btns_h & WPAD_BUTTON_B) && t->wpad.ir.valid)
        {
            if(scrolldelay > 0)
                --scrolldelay;
            else
            {
                scrolldelay = DEFAULT_SCROLL_DELAY-2;
                scrollbarBoxBtn->ScrollIsOn(1);
                if (position2 > position1)
                {
                    prev = this->FindMenuItem(gameIndex[selectedItem], -1);

                    if (prev >= 0)
                    {
                        if (selectedItem == 0)
                        {
                            // move list up by 1
                            listOffset = prev;
                        }
                        else
                        {
                            game[selectedItem]->ResetState();
                            game[selectedItem - 1]->SetState(STATE_SELECTED, t->chan);
                            selectedItem--;
                        }
                    }
                }
                else if (position2 < position1)
                {
                    next = this->FindMenuItem(gameIndex[selectedItem], 1);
                    if (next >= 0)
                    {
                        if (selectedItem == pagesize - 1)
                        {
                            // move list down by 1
                            listOffset = this->FindMenuItem(listOffset, 1);
                        }
                        else if (game[selectedItem + 1]->IsVisible())
                        {
                            game[selectedItem]->ResetState();
                            game[selectedItem + 1]->SetState(STATE_SELECTED, t->chan);
                            selectedItem++;
                        }
                    }
                }
            }
        }
        else if (pressedChan == -1 || (pressedChan == t->chan && !(t->wpad.btns_h & WPAD_BUTTON_B)))
        {
            scrollbarBoxBtn->ScrollIsOn(0);
            position2 = 0;
        }

        if (scrollbarBoxBtn->GetState() == STATE_HELD && scrollbarBoxBtn->GetStateChan() == t->chan && t->wpad.ir.valid
                && gameList.size() > pagesize)
        {
            // allow dragging of scrollbar box
            scrollbarBoxBtn->SetPosition(width / 2 - 18 + 7, 0);
            int position = t->wpad.ir.y - 32 - scrollbarBoxBtn->GetTop();

            listOffset = (position * gameList.size()) / (25.2 * pagesize) - selectedItem;

            if (listOffset <= 0)
            {
                listOffset = 0;
                selectedItem = 0;
            }
            else if (listOffset + pagesize >= gameList.size())
            {
                listOffset = gameList.size() - pagesize;
                selectedItem = pagesize - 1;
            }

        }
        int positionbar = (25.2 * pagesize) * (listOffset + selectedItem) / gameList.size();

        if (positionbar > (24 * pagesize)) positionbar = (24 * pagesize);
        scrollbarBoxBtn->SetPosition(width / 2 - 18 + 7, positionbar + 8);

        if (t->Right()) //skip pagesize # of games if right is pressed
        {
            if (listOffset < gameList.size() && gameList.size() > pagesize)
            {
                listOffset = listOffset + pagesize;
                if (listOffset + pagesize >= gameList.size()) listOffset = gameList.size() - pagesize;
            }
        }
        else if (t->Left())
        {
            if (listOffset > 0)
            {
                listOffset = listOffset - pagesize;
                if (listOffset < 0) listOffset = 0;
            }
        }

    }
    else
    {
        if (t->Down()) //if there isn't a scrollbar and down is pressed
        {
            next = this->FindMenuItem(gameIndex[selectedItem], 1);

            if (next >= 0)
            {
                if (selectedItem == pagesize - 1)
                {
                    // move list down by 1
                    listOffset = this->FindMenuItem(listOffset, 1);
                }
                else if (game[selectedItem + 1]->IsVisible())
                {
                    game[selectedItem]->ResetState();
                    game[selectedItem + 1]->SetState(STATE_SELECTED, t->chan);
                    selectedItem++;
                }
            }
        }
        else if (t->Up()) //up
        {
            prev = this->FindMenuItem(gameIndex[selectedItem], -1);

            if (prev >= 0)
            {
                if (selectedItem == 0)
                {
                    // move list up by 1
                    listOffset = prev;
                }
                else
                {
                    game[selectedItem]->ResetState();
                    game[selectedItem - 1]->SetState(STATE_SELECTED, t->chan);
                    selectedItem--;
                }
            }
        }
    }

    if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h)
    {
        pressedChan = -1;
    }

    if (old_listOffset != listOffset) UpdateListEntries();

    if (updateCB) updateCB(this);
}

void GuiGameBrowser::Reload()
{
    LOCK( this );
    scrollbaron = (gameList.size() > pagesize) ? 1 : 0;
    selectedItem = 0;
    listOffset = 0;
    focus = 1;
    UpdateListEntries();

    for (int i = 0; i < pagesize; i++)
        game[i]->ResetState();
}
