/****************************************************************************
 * Copyright (C) 2009 r-win
 * Copyright (C) 2012 Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "gui_numpad.h"
#include "main.h"
#include "language/gettext.h"
#include "settings/CSettings.h"
#include "themes/CTheme.h"
/**
 * Constructor for the GuiNumpad class.
 */

GuiNumpad::GuiNumpad(char * t, u32 max)
{
	width = 400;
	height = 370;
	selectable = true;
	alignmentHor = ALIGN_CENTER;
	alignmentVert = ALIGN_MIDDLE;
	kbtextmaxlen = max > sizeof(kbtextstr) ? sizeof(kbtextstr) : max; // limit max up to sizeof(kbtextstr)
	strncpy(kbtextstr, t, kbtextmaxlen); // strncpy is needed to fill the rest with \0
	kbtextstr[sizeof(kbtextstr) - 1] = 0; // terminate with \0

	char thekeys[12] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '0', '.'};
	memcpy(keys, thekeys, sizeof(thekeys));

	keyTextbox = Resources::GetImageData("keyboard_textbox.png");
	keyTextboxImg = new GuiImage(keyTextbox);
	keyTextboxImg->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	keyTextboxImg->SetPosition(0, 0);
	this->Append(keyTextboxImg);

	kbText = new GuiText(kbtextstr, 20, ( GXColor ) thColor("r=0 g=0 b=0 a=255 - numpad text color"));
	kbText->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	kbText->SetPosition(0, 10);
	this->Append(kbText);

	keyMedium = Resources::GetImageData("keyboard_mediumkey_over.png");

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	keyBackImg = new GuiImage(keyMedium);
	keyBackText = new GuiText(tr("Back"), 20, (GXColor) thColor("r=0 g=0 b=0 a=255 - numpad key text color"));

	keyBack = new GuiButton(keyBackImg, keyBackImg, ALIGN_CENTER, ALIGN_MIDDLE, 90, 90, trigA, btnSoundOver, btnSoundClick, 1);
	keyBack->SetLabel(keyBackText);
	keyBack->SetTrigger(trigB);
	this->Append(keyBack);

	keyClearImg = new GuiImage(keyMedium);
	keyClearText = new GuiText(tr("Clear"), 20, ( GXColor ) thColor("r=0 g=0 b=0 a=255 - numpad key text color"));
	keyClear = new GuiButton(keyClearImg, keyClearImg, ALIGN_CENTER, ALIGN_MIDDLE, -90, 90, trigA, btnSoundOver, btnSoundClick, 1);
	keyClear->SetLabel(keyClearText);
	this->Append(keyClear);

	char txt[2] = { 0, 0 };
	for (int i = 0; i < NUMPAD_BUTTONS; i++)
	{
		int col = i % 3;
		int row = i / 3;

		txt[0] = keys[i];
		keyImg[i] = new GuiImage(keyMedium);
		keyTxt[i] = new GuiText(txt, 20, (GXColor) thColor("r=0 g=0 b=0 a=255 - numpad key text color"));
		keyTxt[i]->SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
		keyTxt[i]->SetPosition(0, -10);
		keyBtn[i] = new GuiButton(keyImg[i], keyImg[i], ALIGN_CENTER, ALIGN_MIDDLE, -90 + 90 * col, -110 + 50
				* row, trigA, btnSoundOver, btnSoundClick, 1);
		keyBtn[i]->SetLabel(keyTxt[i]);

		this->Append(keyBtn[i]);
	}
}

/**
 * Destructor for the GuiKeyboard class.
 */
GuiNumpad::~GuiNumpad()
{
	delete kbText;
	delete keyTextbox;
	delete keyTextboxImg;
	delete keyBackText;
	delete keyBackImg;
	delete keyBack;
	delete keyClearText;
	delete keyClearImg;
	delete keyClear;
	delete keyMedium;
	delete trigA;
	delete trigB;

	for (int i = 0; i < NUMPAD_BUTTONS; i++)
	{
		delete keyImg[i];
		delete keyTxt[i];
		delete keyBtn[i];
	}
}

void GuiNumpad::Update(GuiTrigger * t)
{
	GuiWindow::Update(t);

	LOCK( this );

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

	for (int i = 0; i < NUMPAD_BUTTONS; i++)
	{
		if (keyBtn[i]->GetState() == STATE_CLICKED)
		{
			if (strlen(kbtextstr) < kbtextmaxlen - 1) // -1 --> kbtextmaxlen means with term. '\0'
			{
				int len = strlen(kbtextstr);
				kbtextstr[len] = keys[i];
				kbtextstr[len+1] = 0;
				kbText->SetText(kbtextstr);
			}
			keyBtn[i]->SetState(STATE_SELECTED, t->chan);
		}
	}
}
