/****************************************************************************
 * libwiigui
 *
 * gui_customoptionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../wpad.h"
#include "../main.h"
#include "../gecko.h"
#include "../settings/CSettings.h"
#include "gui_optionbrowser.h"
#include "themes/CTheme.h"
#include "utils/tools.h"
#include "menu.h"

#include <unistd.h>

#define GAMESELECTSIZE      30

/**
GuiOptionBrowser * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l, const char * custombg)
    : scrollBar(h-10)
{
    width = w;
    height = h;
    options = l;
    selectable = true;
    selectedItem = 0;
    focus = 1; // allow focus
    coL2 = 50;
    scrollbaron = false;
    listChanged = true;
    listOffset = 0;

    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    bgOptions = Resources::GetImageData(custombg);

    bgOptionsImg = new GuiImage(bgOptions);
    bgOptionsImg->SetParent(this);
    bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

    bgOptionsEntry = Resources::GetImageData("bg_options_entry.png");

    scrollBar.SetParent(this);
    scrollBar.SetAlignment(thAlign("right - options browser scrollbar align hor"), thAlign("top - options browser scrollbar align ver"));
    scrollBar.SetPosition(thInt("0 - options browser scrollbar pos x"), thInt("5 - options browser scrollbar pos y"));
    scrollBar.listChanged.connect(this, &GuiOptionBrowser::onListChange);

    for (int i = 0; i < PAGESIZE; i++)
    {
        optionTxt[i] = new GuiText((wchar_t *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - settings text color"));
        optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        optionTxt[i]->SetPosition(24, 0);
        optionTxt[i]->SetMaxWidth(bgOptionsImg->GetWidth()-scrollBar.GetWidth()-40, DOTTED);

        optionBg[i] = new GuiImage(bgOptionsEntry);

        optionVal[i] = new GuiText((wchar_t *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - settings text color"));
        optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

        optionValOver[i] = new GuiText((wchar_t *) NULL, 20, thColor("r=0 g=0 b=0 a=255 - settings text color"));
        optionValOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

        optionBtn[i] = new GuiButton(width - scrollBar.GetWidth(), GAMESELECTSIZE);
        optionBtn[i]->SetParent(this);
        optionBtn[i]->SetLabel(optionTxt[i], 0);
        optionBtn[i]->SetLabel(optionVal[i], 1);
        optionBtn[i]->SetLabelOver(optionValOver[i], 1);
        optionBtn[i]->SetImageOver(optionBg[i]);
        optionBtn[i]->SetPosition(10, GAMESELECTSIZE * i + 4);
        optionBtn[i]->SetRumble(false);
        optionBtn[i]->SetTrigger(trigA);
        optionBtn[i]->SetSoundClick(btnSoundClick);
    }
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
    delete bgOptionsImg;
    delete bgOptions;
    delete bgOptionsEntry;

    delete trigA;

    for (int i = 0; i < PAGESIZE; i++)
    {
        delete optionTxt[i];
        delete optionVal[i];
        delete optionValOver[i];
        delete optionBg[i];
        delete optionBtn[i];
    }
}

void GuiOptionBrowser::SetFocus(int f)
{
    focus = f;

    for (int i = 0; i < PAGESIZE; i++)
        optionBtn[i]->ResetState();

    if (f == 1) optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiOptionBrowser::ResetState()
{
    if (state != STATE_DISABLED)
    {
        state = STATE_DEFAULT;
        stateChan = -1;
    }

    for (int i = 0; i < PAGESIZE; i++)
    {
        optionBtn[i]->ResetState();
    }
}

int GuiOptionBrowser::GetClickedOption()
{
    for (int i = 0; i < PAGESIZE; i++)
    {
        if (optionBtn[i]->GetState() == STATE_CLICKED)
        {
            optionBtn[i]->SetState(STATE_SELECTED);
            return optionIndex[i];
        }
    }

    return -1;
}

int GuiOptionBrowser::GetSelectedOption()
{
    for (int i = 0; i < PAGESIZE; i++)
    {
        if (optionBtn[i]->GetState() == STATE_SELECTED)
        {
            return optionIndex[i];
        }
    }
    return -1;
}

void GuiOptionBrowser::SetClickable(bool enable)
{
    for (int i = 0; i < PAGESIZE; i++)
    {
        optionBtn[i]->SetClickable(enable);
    }
}

void GuiOptionBrowser::SetOffset(int optionnumber)
{
    listOffset = optionnumber;
    selectedItem = optionnumber;
}

void GuiOptionBrowser::onListChange(int SelItem, int SelInd)
{
    selectedItem = SelItem;
    listOffset = SelInd;
    UpdateListEntries();
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiOptionBrowser::FindMenuItem(int currentItem, int direction)
{
    int nextItem = currentItem + direction;

    if (nextItem < 0 || nextItem >= options->GetLength()) return -1;

    if (strlen(options->GetName(nextItem)) > 0)
        return nextItem;

    return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
    if (!this->IsVisible()) return;

    bgOptionsImg->Draw();

    int next = listOffset;

    for (int i = 0; i < PAGESIZE; i++)
    {
        if (next >= 0)
        {
            optionBtn[i]->Draw();
            next = this->FindMenuItem(next, 1);
        }
        else break;
    }

    if (scrollbaron)
        scrollBar.Draw();

    this->UpdateEffects();
}

void GuiOptionBrowser::UpdateListEntries()
{
    LOCK(this);
    scrollbaron = options->GetLength() > PAGESIZE;
    if (listOffset < 0) listOffset = this->FindMenuItem(-1, 1);
    int next = listOffset;

    int maxNameWidth = 0;
    for (int i = 0; i < PAGESIZE; i++)
    {
        if (next >= 0)
        {
            if (optionBtn[i]->GetState() == STATE_DISABLED)
            {
                optionBtn[i]->SetVisible(true);
                optionBtn[i]->SetState(STATE_DEFAULT);
            }

            optionTxt[i]->SetText(options->GetName(next));
            if (maxNameWidth < optionTxt[i]->GetTextWidth()) maxNameWidth = optionTxt[i]->GetTextWidth();
            optionVal[i]->SetText(options->GetValue(next));
            optionValOver[i]->SetText(options->GetValue(next));

            optionIndex[i] = next;
            next = this->FindMenuItem(next, 1);
        }
        else
        {
            optionBtn[i]->SetVisible(false);
            optionBtn[i]->SetState(STATE_DISABLED);
        }
    }

    if (coL2 < (24 + maxNameWidth + 16))
        coL2 = 24 + maxNameWidth + 16;

    for (int i = 0; i < PAGESIZE; i++)
    {
        if (optionBtn[i]->GetState() != STATE_DISABLED)
        {
            optionVal[i]->SetPosition(coL2, 0);
            optionVal[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2 + scrollBar.GetWidth()+10), DOTTED);

            optionValOver[i]->SetPosition(coL2, 0);
            optionValOver[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2 + scrollBar.GetWidth()+10), SCROLL_HORIZONTAL);
        }
    }
}

void GuiOptionBrowser::Update(GuiTrigger * t)
{
    if (state == STATE_DISABLED || !t) return;

    static int pressedChan = -1;

    if((t->wpad.btns_d & (WPAD_BUTTON_B | WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT |
                          WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT)) ||
       (t->pad.btns_d & (PAD_BUTTON_UP | PAD_BUTTON_DOWN)))
        pressedChan = t->chan;

    if (scrollbaron)
        // update the location of the scroll box based on the position in the option list
        scrollBar.Update(t);

    int next = listOffset;

    if(pressedChan == -1 || (!t->wpad.btns_h && !t->pad.btns_h))
    {
        for(int i = 0; i < PAGESIZE; i++)
        {
            if (next >= 0) next = this->FindMenuItem(next, 1);

            if (i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
            {
                optionBtn[i]->ResetState();
            }
            else if (i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
            {
                optionBtn[selectedItem]->SetState(STATE_SELECTED);
            }

            optionBtn[i]->Update(t);

            if (optionBtn[i]->GetState() == STATE_SELECTED)
                selectedItem = i;
        }
    }

    if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h)
        pressedChan = -1;

    scrollBar.SetPageSize(PAGESIZE);
    scrollBar.SetSelectedItem(selectedItem);
    scrollBar.SetSelectedIndex(listOffset);
    scrollBar.SetEntrieCount(options->GetLength());

    if (options->IsChanged())
    {
        UpdateListEntries();
        listChanged = false;
    }

    if (updateCB) updateCB(this);
}
