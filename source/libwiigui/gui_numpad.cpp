/****************************************************************************
 * USB Loader GX
 *
 * r-win 2009
 *
 * gui_numpad.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../main.h"
#include "../settings/CSettings.h"
#include <stdio.h>
#include <string.h>
/**
 * Constructor for the GuiNumpad class.
 */

#define SAFEFREE(p) if(p){free(p);p=NULL;}

GuiNumpad::GuiNumpad(char * t, u32 max)
{
    width = 400;
    height = 370;
    selectable = true;
    focus = 0; // allow focus
    alignmentHor = ALIGN_CENTRE;
    alignmentVert = ALIGN_MIDDLE;
    kbtextmaxlen = max > sizeof(kbtextstr) ? sizeof(kbtextstr) : max; // limit max up to sizeof(kbtextstr)
    //  strlcpy(kbtextstr, t, kbtextmaxlen);
    strncpy(kbtextstr, t, kbtextmaxlen); // strncpy is needed to fill the rest with \0
    kbtextstr[sizeof(kbtextstr) - 1] = 0; // terminate with \0

    char thekeys[11] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0', '0' };
    memcpy(keys, thekeys, sizeof(thekeys));

    keyTextbox = new GuiImageData(keyboard_textbox_png);
    keyTextboxImg = new GuiImage(keyTextbox);
    keyTextboxImg->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    keyTextboxImg->SetPosition(0, 40);//(0,0);
    this->Append(keyTextboxImg);

    kbText = new GuiText(kbtextstr, 20, ( GXColor )
    {   0, 0, 0, 0xff});
    kbText->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    kbText->SetPosition(0, 53);//(0, 13);
    kbText->SetPassChar('*');
    this->Append(kbText);

    keyMedium = new GuiImageData(keyboard_mediumkey_over_png);
    keyMediumOver = new GuiImageData(keyboard_mediumkey_over_png);

    keySoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);
    keySoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, Settings.sfxvolume);
    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigB = new GuiTrigger;
    trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

    keyBackImg = new GuiImage(keyMedium);
    keyBackOverImg = new GuiImage(keyMediumOver);
    keyBackText = new GuiText("Back", 20, ( GXColor )
    {   0, 0, 0, 0xff});

    keyBack = new GuiButton(keyBackImg, keyBackOverImg, ALIGN_CENTRE, ALIGN_MIDDLE, 90, 80, trigA, keySoundOver,
            keySoundClick, 1);
    keyBack->SetLabel(keyBackText);
    keyBack->SetTrigger(trigB);
    this->Append(keyBack);

    keyClearImg = new GuiImage(keyMedium);
    keyClearOverImg = new GuiImage(keyMediumOver);
    keyClearText = new GuiText("Clear", 20, ( GXColor )
    {   0, 0, 0, 0xff});
    keyClear = new GuiButton(keyClearImg, keyClearOverImg, ALIGN_CENTRE, ALIGN_MIDDLE, -90, 80, trigA, keySoundOver,
            keySoundClick, 1);
    keyClear->SetLabel(keyClearText);
    this->Append(keyClear);

    char txt[2] = { 0, 0 };
    for (int i = 0; i < 11; i++)
    {
        if (keys[i] != '\0')
        {
            int col = i % 3;
            int row = i / 3;

            keyImg[i] = new GuiImage(keyMedium);
            keyImgOver[i] = new GuiImage(keyMediumOver);
            txt[0] = keys[i];
            keyTxt[i] = new GuiText(txt, 20, ( GXColor )
            {   0, 0, 0, 0xff});
            keyTxt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
            keyTxt[i]->SetPosition(0, -10);
            keyBtn[i] = new GuiButton(keyImg[i], keyImgOver[i], ALIGN_CENTRE, ALIGN_MIDDLE, -90 + 90 * col, -70 + 50
                    * row, trigA, keySoundOver, keySoundClick, 1);
            keyBtn[i]->SetLabel(keyTxt[i]);

            this->Append(keyBtn[i]);
        }
    }
}

/**
 * Destructor for the GuiKeyboard class.
 */
GuiNumpad::~GuiNumpad()
{
    SAFEFREE( kbText )
    SAFEFREE( keyTextbox )
    SAFEFREE( keyTextboxImg )
    SAFEFREE( keyBackText )
    SAFEFREE( keyBackImg )
    SAFEFREE( keyBackOverImg )
    SAFEFREE( keyBack )
    SAFEFREE( keyClear )
    SAFEFREE( keyClearImg )
    SAFEFREE( keyClearOverImg )
    SAFEFREE( keyClearText )
    SAFEFREE( keyMedium )
    SAFEFREE( keyMediumOver )
    SAFEFREE( keySoundOver )
    SAFEFREE( keySoundClick )
    SAFEFREE( trigA )
    SAFEFREE( trigB )

    for (int i = 0; i < 11; i++)
    {
        if (keys[i] != '\0')
        {
            SAFEFREE( keyImg[i] )
            SAFEFREE( keyImgOver[i] )
            SAFEFREE( keyTxt[i] )
            SAFEFREE( keyBtn[i] )
        }
    }
}

void GuiNumpad::Update(GuiTrigger * t)
{
    LOCK( this );
    if (_elements.size() == 0 || (state == STATE_DISABLED && parentElement)) return;

    for (u8 i = 0; i < _elements.size(); i++)
    {
        try
        {
            _elements.at(i)->Update(t);
        }
        catch (const std::exception& e)
        {
        }
    }

    if (keyBack->GetState() == STATE_CLICKED)
    {
        if (strlen(kbtextstr) > 0)
        {
            kbtextstr[strlen(kbtextstr) - 1] = 0;
            kbText->SetText(kbtextstr);
        }
        keyBack->SetState(STATE_SELECTED, t->chan);
    }
    else if (keyClear->GetState() == STATE_CLICKED)
    {
        memset(kbtextstr, 0, sizeof(kbtextstr));
        kbText->SetText(kbtextstr);
        keyClear->SetState(STATE_SELECTED, t->chan);
    }

    char txt[2] = { 0, 0 };
    for (int i = 0; i < 11; i++)
    {
        if (keys[i] != '\0')
        {
            if (keyBtn[i]->GetState() == STATE_CLICKED)
            {
                txt[0] = keys[i];
                if (strlen(kbtextstr) < kbtextmaxlen - 1) // -1 --> kbtextmaxlen means with term. '\0'
                {
                    kbtextstr[strlen(kbtextstr)] = txt[0];
                    kbText->SetText(kbtextstr);
                }
                keyBtn[i]->SetState(STATE_SELECTED, t->chan);
            }
        }
    }

    kbText->SetPosition(0, 53);

    this->ToggleFocus(t);

    if (focus) // only send actions to this window if it's in focus
    {
        // pad/joystick navigation
        if (t->Right())
            this->MoveSelectionHor(1);
        else if (t->Left())
            this->MoveSelectionHor(-1);
        else if (t->Down())
            this->MoveSelectionVert(1);
        else if (t->Up()) this->MoveSelectionVert(-1);
    }
}
