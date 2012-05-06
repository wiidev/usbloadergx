/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_keyboard.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../main.h"
#include "../settings/CSettings.h"
#include <stdio.h>
#include <string.h>
#include "themes/CTheme.h"
#include "menu.h"
/**
 * Constructor for the GuiKeyboard class.
 */
//const Key thekeys;
GuiKeyboard::GuiKeyboard(char * t, u32 max, int minimum, int lang)
{
	width = 540;
	height = 400;
	shift = 0;
	caps = 0;
	alt = 0;
	alt2 = 0;
	min = minimum;
	int mode = lang;
	textVisible = true;
	selectable = true;
	alignmentHor = ALIGN_CENTER;
	alignmentVert = ALIGN_MIDDLE;
	kbtextmaxlen = max > sizeof(kbtextstr) ? sizeof(kbtextstr) : max; // limit max up to sizeof(kbtextstr)
	memset(kbtextstr, 0, sizeof(kbtextstr));
	strncpy(kbtextstr, t, kbtextmaxlen); // strncpy is needed to fill the rest with \0
	kbtextstr[sizeof(kbtextstr) - 1] = 0; // terminate with \0
	//QWERTY//
	if (mode == 0)
	{
		Key thekeys[4][11] = {
			{
				{ '1', '!', '\0', '\0' },
				{ '2', '@', '\0', '\0' },
				{ '3', '#', '\0', '\0' },
				{ '4', '$', '\0', '\0' },
				{ '5', '%', '\0', '\0' },
				{ '6', '^', '\0', '\0' },
				{ '7', '&', '\0', '\0' },
				{ '8', '*', '\0', '\0' },
				{ '9', '(', '\0', '\0' },
				{ '0', ')', '\0', '\0' },
				{ '\0', '\0', '\0', '\0' }
			},
			{
				{ 'q', 'Q', '\0', '\0' },
				{ 'w', 'W', '\0', '\0' },
				{ 'e', 'E', '\0', '\0' },
				{ 'r', 'R', '\0', '\0' },
				{ 't', 'T', '\0', '\0' },
				{ 'y', 'Y', '\0', '\0' },
				{ 'u', 'U', '\0', '\0' },
				{ 'i', 'I', '\0', '\0' },
				{ 'o', 'O', '\0', '\0' },
				{ 'p', 'P', '\0', '\0' },
				{ '-', '_', '\0', '\0' }
			},
			{
				{ 'a', 'A', '\0', '\0' },
				{ 's', 'S', '\0', '\0' },
				{ 'd', 'D', '\0', '\0' },
				{ 'f', 'F', '\0', '\0' },
				{ 'g', 'G', '\0', '\0' },
				{ 'h', 'H', '\0', '\0' },
				{ 'j', 'J', '\0', '\0' },
				{ 'k', 'K', '\0', '\0' },
				{ 'l', 'L', '\0', '\0' },
				{ ':', ';', '\0', '\0' },
				{ '\'', '"', '\0', '\0' }
			},
			{
				{ 'z', 'Z', '\0', '\0' },
				{ 'x', 'X', '\0', '\0' },
				{ 'c', 'C', '\0', '\0' },
				{ 'v', 'V', '\0', '\0' },
				{ 'b', 'B', '\0', '\0' },
				{ 'n', 'N', '\0', '\0' },
				{ 'm', 'M', '\0', '\0' },
				{ ',', '<', '\0', '\0' },
				{ '.', '>', '\0', '\0' },
				{ '/', '?', '\0', '\0' },
				{ '\0', '\0', '\0', '\0' }
			}
		};

		memcpy(keys, thekeys, sizeof(thekeys));
	}
	//DVORAK//
	if (mode == 1)
	{
		Key thekeys[4][11] = {
			{
				{ '1', '!', '\0', '\0' },
				{ '2', '@', '\0', '\0' },
				{ '3', '#', '\0', '\0' },
				{ '4', '$', '\0', '\0' },
				{ '5', '%', '\0', '\0' },
				{ '6', '^', '\0', '\0' },
				{ '7', '&', '\0', '\0' },
				{ '8', '*', '\0', '\0' },
				{ '9', '(', '\0', '\0' },
				{ '0', ')', '\0', '\0' },
				{ '\0', '\0', '\0', '\0' }
			},
			{
				{ '\'', '"', '\0', '\0' },
				{ ',', '<', '\0', '\0' },
				{ '.', '>', '\0', '\0' },
				{ 'p', 'P', '\0', '\0' },
				{ 'y', 'Y', '\0', '\0' },
				{ 'f', 'F', '\0', '\0' },
				{ 'g', 'G', '\0', '\0' },
				{ 'c', 'C', '\0', '\0' },
				{ 'r', 'R', '\0', '\0' },
				{ 'l', 'L', '\0', '\0' },
				{ '/', '?', '\0', '\0' }
			},
			{
				{ 'a', 'A', 'm', '\0' },
				{ 'o', 'O', 'm', '\0' },
				{ 'e', 'E', 'm', '\0' },
				{ 'u', 'U', 'm', '\0' },
				{ 'i', 'I', 'm', '\0' },
				{ 'd', 'D', 'm', '\0' },
				{ 'h', 'H', 'm', '\0' },
				{ 't', 'T', 'm', '\0' },
				{ 'n', 'N', 'm', '\0' },
				{ 's', 'S', 'm', '\0' },
				{ '-', '_', 'm', '\0' }
			},
			{
				{ ';', ':', '\0', '\0' },
				{ 'q', 'Q', '\0', '\0' },
				{ 'j', 'J', '\0', '\0' },
				{ 'k', 'K', '\0', '\0' },
				{ 'x', 'X', '\0', '\0' },
				{ 'b',  'B', '\0', '\0' },
				{ 'm', 'M', '\0', '\0' },
				{ 'w', 'W', '\0', '\0' },
				{ 'v', 'V', '\0', '\0' },
				{ 'z', 'Z', '\0', '\0' },
				{ '\0', '\0', '\0', '\0' }
			}
		};
		memcpy(keys, thekeys, sizeof(thekeys));
	}
	//QWETRZ//
	if (mode == 2)
	{
		Key thekeys[4][11] = { { { '1', '!', '^', 'À' }, { '2', '"', '²', 'à' }, { '3', '#', '³', 'È' }, { '4', '$',
				'«', 'è' }, { '5', '%', '»', 'Ì' }, { '6', '&', '„', 'ì' }, { '7', '/', '”', 'Ò' }, { '8', '(', '[',
				'ò' }, { '9', ')', ']', 'Ù' }, { '0', '=', '§', 'ù' }, { 'ß', '?', '\'', 'Ý' } }, { { 'q', 'Q', '@',
				'Á' }, { 'w', 'W', '\0', 'á' }, { 'e', 'E', '€', 'É' }, { 'r', 'R', '\0', 'é' },
				{ 't', 'T', '\0', 'Í' }, { 'z', 'Z', '\0', 'í' }, { 'u', 'U', '\0', 'Ó' }, { 'i', 'I', '\0', 'ó' }, {
						'o', 'O', '\0', 'Ú' }, { 'p', 'P', '\0', 'ú' }, { 'ü', 'Ü', '\0', 'ý' } }, { { 'a', 'A', '\0',
				'Â' }, { 's', 'S', '\0', 'â' }, { 'd', 'D', '\0', 'Ê' }, { 'f', 'F', '\0', 'ê' },
				{ 'g', 'G', '\0', 'Î' }, { 'h', 'H', '\0', 'î' }, { 'j', 'J', '\0', 'Ô' }, { 'k', 'K', '\0', 'ô' }, {
						'l', 'L', '\0', 'Û' }, { 'ö', 'Ö', '\0', 'û' }, { 'ä', 'Ä', '\0', 'Ÿ' } }, { { '<', '>', '|',
				'Ã' }, { 'y', 'Y', '\0', 'ã' }, { 'x', 'X', '\0', 'Ñ' }, { 'c', 'C', 'ç', 'ñ' },
				{ 'v', 'V', '©', 'Ï' }, { 'b', 'B', '\0', 'ï' }, { 'n', 'N', '\0', 'Õ' }, { 'm', 'M', 'µ', 'õ' }, {
						',', ';', '\0', 'ÿ' }, { '.', ':', '\0', '\0' }, { '-', '_', '\0', '\0' } } };
		memcpy(keys, thekeys, sizeof(thekeys));
	}
	//AZERTY//
	if (mode == 3)
	{
		Key thekeys[4][11] = { { { '1', '&', '²', 'À' }, { '2', '~', '³', 'é' }, { '3', '"', '#', 'È' }, { '4', '`',
				'«', 'ù' }, { '5', '(', '[', 'Ì' }, { '6', '-', '|', 'ì' }, { '7', 'µ', '»', 'è' }, { '8', '_', '\'',
				'ò' }, { '9', '+', '^', 'ç' }, { '0', '=', '@', 'à' }, { '°', ')', ']', 'Ý' } }, {
				{ 'a', 'A', 'Æ', 'Á' }, { 'z', 'Z', 'Œ', 'á' }, { 'e', 'E', '€', 'É' }, { 'r', 'R', '®', 'ë' }, { 't',
						'T', '†', 'Í' }, { 'y', 'Y', 'ÿ', 'í' }, { 'u', 'U', 'Õ', 'Ó' }, { 'i', 'I', 'õ', 'Ò' }, { 'o',
						'O', 'Ø', 'Ú' }, { 'p', 'P', 'ø', 'ú' }, { '$', '£', '¤', 'ý' } }, { { 'q', 'Q', 'æ', 'Â' }, {
				's', 'S', 'œ', 'â' }, { 'd', 'D', '\0', 'Ê' }, { 'f', 'F', 'ß', 'ê' }, { 'g', 'G', '\0', 'Î' }, { 'h',
				'H', '\0', 'î' }, { 'j', 'J', '\0', 'Ô' }, { 'k', 'K', '\0', 'ô' }, { 'l', 'L', '\0', 'Û' }, { 'm',
				'M', '\0', 'û' }, { '*', '%', '¬', 'Ù' } }, { { '<', '>', '\0', 'Ã' }, { 'w', 'W', '\0', 'Ä' }, { 'x',
				'X', '\0', 'Ë' }, { 'c', 'C', '©', 'Ç' }, { 'v', 'V', '“', 'Ï' }, { 'b', 'B', '”', 'ï' }, { 'n', 'N',
				'\0', 'Ñ' }, { '?', ',', '?', 'ñ' }, { '.', ';', '.', 'ó' }, { '/', ':', '/', 'ö' }, { '§', '!', '!',
				'Ö' } } };
		memcpy(keys, thekeys, sizeof(thekeys));
	}
	//QWERTY 2//
	if (mode == 4)
	{
		Key thekeys[4][11] = { { { '1', '!', '|', 'Á' }, { '2', '"', '@', 'á' }, { '3', '·', '#', 'À' }, { '4', '$',
				'£', 'à' }, { '5', '%', '~', 'É' }, { '6', '&', '¬', 'é' }, { '7', '/', '\'', 'È' }, { '8', '(', '[',
				'è' }, { '9', ')', ']', 'Í' }, { '0', '=', '¤', 'í' }, { '¡', '?', '¿', 'Ï' } }, { { 'q', 'Q', '\0',
				'ï' }, { 'w', 'W', '\0', 'Ó' }, { 'e', 'E', '€', 'ó' }, { 'r', 'R', '®', 'Ò' }, { 't', 'T', '†', 'ò' },
				{ 'y', 'Y', 'ÿ', 'Ú' }, { 'u', 'U', '“', 'ú' }, { 'i', 'I', '”', 'Ü' }, { 'o', 'O', 'Ø', 'ü' }, { 'p',
						'P', 'ø', 'Ù' }, { '+', '*', '\0', 'ù' } }, { { 'a', 'A', '^', 'Ã' }, { 's', 'S', '²', 'ã' }, {
				'd', 'D', '³', 'Õ' }, { 'f', 'F', '«', 'õ' }, { 'g', 'G', '»', 'Ñ' }, { 'h', 'H', '§', 'ñ' }, { 'j',
				'J', 'µ', 'Ç' }, { 'k', 'K', '¤', 'ç' }, { 'l', 'L', '„', '\0' }, { 'ñ', 'Ñ', '+', '\0' }, { 'ç', 'Ç',
				'°', '\0' } }, { { '<', '>', '\0', 'Ä' }, { 'z', 'Z', '\0', 'ä' }, { 'x', 'X', '\0', 'Â' }, { 'c', 'C',
				'©', 'â' }, { 'v', 'V', '\0', 'å' }, { 'b', 'B', 'ß', 'Ë' }, { 'n', 'N', '\0', 'ë' }, { 'm', 'M', '\0',
				'Ê' }, { ',', ';', '\0', 'ê' }, { '.', ':', '\0', '\0' }, { '-', '_', '\0', '\0' } } };
		memcpy(keys, thekeys, sizeof(thekeys));
	}

	keyTextbox = Resources::GetImageData("keyboard_textbox.png");
	keyTextboxImg = new GuiImage(keyTextbox);
	keyTextboxImg->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	keyTextboxImg->SetPosition(0, 40);//(0,0);
	this->Append(keyTextboxImg);

	kbText = new GuiText(kbtextstr, 20, thColor("r=0 g=0 b=0 a=255 - keyboard text color"));
	kbText->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	kbText->SetPosition(0, 53);//(0, 13);
	this->Append(kbText);

	key = Resources::GetImageData("keyboard_key.png");
	keyOver = Resources::GetImageData("keyboard_key_over.png");
	keyMedium = Resources::GetImageData("keyboard_mediumkey_over.png");
	keyLarge = Resources::GetImageData("keyboard_largekey_over.png");

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	int eurocheck = 0;
	if (mode > 1)
	{
		eurocheck = -20;
	}

	keyBackImg = new GuiImage(keyMedium);
	keyBackOverImg = new GuiImage(keyMedium);
	if (mode == 3)
	{
		keyBackText = new GuiText("Retour", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	}
	else
	{
		keyBackText = new GuiText("Back", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	}
	//keyBack = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyBack = new GuiButton(keyBackImg, keyBackOverImg, 0, 3, 11 * 42 + 40 + eurocheck, 0 * 42 + 120, trigA,
			btnSoundOver, btnSoundClick, 1);
	//keyBack->SetImage(keyBackImg);
	//keyBack->SetImageOver(keyBackOverImg);
	keyBack->SetLabel(keyBackText);
	//keyBack->SetSoundOver(btnSoundOver);
	//keyBack->SetSoundClick(btnSoundClick);
	//keyBack->SetTrigger(trigA);
	keyBack->SetTrigger(trigB);
	if (mode > 1)
	{
		keyBack->SetPosition(11 * 42 + 40 + eurocheck, 0 * 42 + 120);
	}
	else
	{
		keyBack->SetPosition(10 * 42 + 40 + eurocheck, 0 * 42 + 120);
	}//(10*42+40, 0*42+80);
	//keyBack->SetEffectGrow();
	this->Append(keyBack);

	keyClearImg = new GuiImage(keyMedium);
	keyClearOverImg = new GuiImage(keyMedium);
	if (mode == 3)
	{
		keyClearText = new GuiText("Effacer", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	}
	else
	{
		keyClearText = new GuiText("Clear", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	}
	keyClear = new GuiButton(keyClearImg, keyClearOverImg, 0, 3, (10 * 42 + 40) + eurocheck, 4 * 42 + 120, trigA,
			btnSoundOver, btnSoundClick, 1);
	//keyClear = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	//keyClear->SetImage(keyClearImg);
	//keyClear->SetImageOver(keyClearOverImg);
	keyClear->SetLabel(keyClearText);
	//keyClear->SetSoundOver(btnSoundOver);
	//keyClear->SetSoundClick(btnSoundClick);
	//keyClear->SetTrigger(trigA);
	//keyClear->SetPosition((10*42+40)+eurocheck, 4*42+120);//(10*42+40, 0*42+80);
	//keyClear->SetEffectGrow();
	this->Append(keyClear);

	keyAltImg = new GuiImage(keyMedium);
	keyAltOverImg = new GuiImage(keyMedium);
	keyAltText = new GuiText("Alt Gr", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	keyAlt = new GuiButton(keyAltImg, keyAltOverImg, 0, 3, 84 + eurocheck, 4 * 42 + 120, trigA, btnSoundOver,
			btnSoundClick, 1);
	//keyAlt = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	//keyAlt->SetImage(keyAltImg);
	//keyAlt->SetImageOver(keyAltOverImg);
	keyAlt->SetLabel(keyAltText);
	//keyAlt->SetSoundOver(btnSoundOver);
	//keyAlt->SetSoundClick(btnSoundClick);
	//keyAlt->SetTrigger(trigA);
	//keyAlt->SetPosition(84+eurocheck, 4*42+120);//(10*42+40, 4*42+120);
	//keyAlt->SetEffectGrow();
	if (mode > 1)
	{
		this->Append(keyAlt);
	}

	keyAlt2Img = new GuiImage(keyMedium);
	keyAlt2OverImg = new GuiImage(keyMedium);
	keyAlt2Text = new GuiText("Accent", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	keyAlt2 = new GuiButton(keyAlt2Img, keyAlt2OverImg, 0, 3, (8 * 42 + 40) + eurocheck, 4 * 42 + 120, trigA,
			btnSoundOver, btnSoundClick, 1);
	//keyAlt2 = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	//keyAlt2->SetImage(keyAlt2Img);
	//keyAlt2->SetImageOver(keyAlt2OverImg);
	keyAlt2->SetLabel(keyAlt2Text);
	//keyAlt2->SetSoundOver(btnSoundOver);
	//keyAlt2->SetSoundClick(btnSoundClick);
	//keyAlt2->SetTrigger(trigA);
	//keyAlt2->SetPosition((8*42+40)+eurocheck, 4*42+120);//(10*42+40, 4*42+120);
	//keyAlt2->SetEffectGrow();
	if (mode > 1)
	{
		this->Append(keyAlt2);
	}

	keyCapsImg = new GuiImage(keyMedium);
	keyCapsOverImg = new GuiImage(keyMedium);
	keyCapsText = new GuiText("Caps", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	keyCaps = new GuiButton(keyCapsImg, keyCapsOverImg, 0, 3, 0 + eurocheck, 2 * 42 + 120, trigA, btnSoundOver,
			btnSoundClick, 1);
	//keyCaps = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	//keyCaps->SetImage(keyCapsImg);
	//keyCaps->SetImageOver(keyCapsOverImg);
	keyCaps->SetLabel(keyCapsText);
	//keyCaps->SetSoundOver(btnSoundOver);
	//keyCaps->SetSoundClick(btnSoundClick);
	//keyCaps->SetTrigger(trigA);
	//keyCaps->SetPosition(0+eurocheck, 2*42+120);//(0, 2*42+80);
	//keyCaps->SetEffectGrow();
	this->Append(keyCaps);

	keyShiftImg = new GuiImage(keyMedium);
	keyShiftOverImg = new GuiImage(keyMedium);
	keyShiftText = new GuiText("Shift", 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
	keyShift = new GuiButton(keyShiftImg, keyShiftOverImg, 0, 3, 21 + eurocheck, 3 * 42 + 120, trigA, btnSoundOver,
			btnSoundClick, 1);
	//keyShift = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	//keyShift->SetImage(keyShiftImg);
	//keyShift->SetImageOver(keyShiftOverImg);
	keyShift->SetLabel(keyShiftText);
	//keyShift->SetSoundOver(btnSoundOver);
	//keyShift->SetSoundClick(btnSoundClick);
	//keyShift->SetTrigger(trigA);
	//keyShift->SetPosition(21+eurocheck, 3*42+120);//(21, 3*42+80);
	//keyShift->SetEffectGrow();
	this->Append(keyShift);

	keySpaceImg = new GuiImage(keyLarge);
	keySpaceOverImg = new GuiImage(keyLarge);
	keySpace = new GuiButton(keySpaceImg, keySpaceOverImg, 2, 3, 0 + eurocheck, 4 * 42 + 120, trigA, btnSoundOver,
			btnSoundClick, 1);
	//keySpace = new GuiButton(keyLarge->GetWidth(), keyLarge->GetHeight());
	//keySpace->SetImage(keySpaceImg);
	//keySpace->SetImageOver(keySpaceOverImg);
	//keySpace->SetSoundOver(btnSoundOver);
	//keySpace->SetSoundClick(btnSoundClick);
	//keySpace->SetTrigger(trigA);
	//keySpace->SetPosition(0+eurocheck, 4*42+120);//(0, 4*42+80);
	//keySpace->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	//keySpace->SetEffectGrow();
	this->Append(keySpace);

	char txt[2] = { 0, 0 };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			if (keys[i][j].ch != '\0')
			{
				keyImg[i][j] = new GuiImage(key);
				keyImgOver[i][j] = new GuiImage(keyOver);
				txt[0] = keys[i][j].ch;
				keyTxt[i][j] = new GuiText(txt, 20, thColor("r=0 g=0 b=0 a=255 - keyboard key text color"));
				keyTxt[i][j]->SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
				keyTxt[i][j]->SetPosition(0, -10);
				keyBtn[i][j] = new GuiButton(keyImg[i][j], keyImgOver[i][j], 0, 3, (j * 42 + 21 * i + 40) + eurocheck,
						i * 42 + 120, trigA, btnSoundOver, btnSoundClick, 1);
				//keyBtn[i][j] = new GuiButton(key->GetWidth(), key->GetHeight());
				//keyBtn[i][j]->SetImage(keyImg[i][j]);
				//keyBtn[i][j]->SetImageOver(keyImgOver[i][j]);
				//keyBtn[i][j]->SetSoundOver(btnSoundOver);
				//keyBtn[i][j]->SetSoundClick(btnSoundClick);
				//keyBtn[i][j]->SetTrigger(trigA);
				keyBtn[i][j]->SetLabel(keyTxt[i][j]);
				//keyBtn[i][j]->SetPosition((j*42+21*i+40)+eurocheck, i*42+120);//SetPosition(j*42+21*i+40, i*42+80);
				//keyBtn[i][j]->SetEffectGrow();
				this->Append(keyBtn[i][j]);
			}
		}
	}
}

/**
 * Destructor for the GuiKeyboard class.
 */
GuiKeyboard::~GuiKeyboard()
{
	delete kbText;
	delete keyTextbox;
	delete keyTextboxImg;
	delete keyCapsText;
	delete keyCapsImg;
	delete keyCapsOverImg;
	delete keyCaps;
	delete keyShiftText;
	delete keyShiftImg;
	delete keyShiftOverImg;
	delete keyShift;
	if (keyAlt)
	{
		delete keyAlt;
	}
	if (keyAlt2)
	{
		delete keyAlt2;
	}
	delete keyBackText;
	delete keyBackImg;
	delete keyBackOverImg;
	delete keyBack;
	delete keySpaceImg;
	delete keySpaceOverImg;
	delete keySpace;
	delete key;
	delete keyOver;
	delete keyMedium;
	delete keyLarge;
	delete trigA;
	delete trigB;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			if (keys[i][j].ch != '\0')
			{
				delete keyImg[i][j];
				delete keyImgOver[i][j];
				delete keyTxt[i][j];
				delete keyBtn[i][j];
			}
		}
	}
}

void GuiKeyboard::SetDisplayText(const char *text)
{
	if(!text || textVisible)
	{
		kbText->SetText(text);
	}
	else
	{
		std::string asterix(strlen(text), '*');
		kbText->SetText(asterix.c_str());
	}
}

void GuiKeyboard::Update(GuiTrigger * t)
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

	bool changedShiftKey = false;

	if (keySpace->GetState() == STATE_CLICKED)
	{
		if (strlen(kbtextstr) < kbtextmaxlen - 1) // -1 --> kbtextmaxlen means with terminating '\0'
		{
			kbtextstr[strlen(kbtextstr)] = ' ';
			SetDisplayText(kbtextstr);
		}
		keySpace->SetState(STATE_SELECTED, t->chan);
	}
	else if (keyBack->GetState() == STATE_CLICKED)
	{
		if (strlen(kbtextstr) > min)
		{
			kbtextstr[strlen(kbtextstr) - 1] = 0;
			SetDisplayText(kbtextstr);
		}
		keyBack->SetState(STATE_SELECTED, t->chan);
	}
	else if (keyClear->GetState() == STATE_CLICKED)
	{
		while (strlen(kbtextstr) > min)
		{
			kbtextstr[strlen(kbtextstr) - 1] = 0;
		}
		SetDisplayText(kbtextstr);
		keyClear->SetState(STATE_SELECTED, t->chan);
	}
	else if (keyShift->GetState() == STATE_CLICKED)
	{
		changedShiftKey = true;
		shift ^= 1;
		if (alt) alt ^= 1;
		if (alt2) alt2 ^= 1;
		keyShift->SetState(STATE_SELECTED, t->chan);
	}
	else if (keyAlt->GetState() == STATE_CLICKED)
	{
		changedShiftKey = true;
		alt ^= 1;
		if (shift) shift ^= 1;
		if (alt2) alt2 ^= 1;
		keyAlt->SetState(STATE_SELECTED, t->chan);
	}
	else if (keyAlt2->GetState() == STATE_CLICKED)
	{
		changedShiftKey = true;
		alt2 ^= 1;
		if (shift) shift ^= 1;
		if (alt) alt ^= 1;
		keyAlt2->SetState(STATE_SELECTED, t->chan);
	}
	else if (keyCaps->GetState() == STATE_CLICKED)
	{
		changedShiftKey = true;
		caps ^= 1;
		keyCaps->SetState(STATE_SELECTED, t->chan);
	}

	bool update = false;

	char txt[2] = { 0, 0 };

	do
	{
		update = false;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 11; j++)
			{
				if (keys[i][j].ch != '\0')
				{
					if (shift || caps)
						txt[0] = keys[i][j].chShift;
					else if (alt)
						txt[0] = keys[i][j].chalt;
					else if (alt2)
						txt[0] = keys[i][j].chalt2;
					else txt[0] = keys[i][j].ch;

					if (changedShiftKey) // change text only if needed
					keyTxt[i][j]->SetText(txt);

					if (keyBtn[i][j]->GetState() == STATE_CLICKED)
					{
						if (strlen(kbtextstr) < kbtextmaxlen - 1) // -1 --> kbtextmaxlen means with term. '\0'
						{
							kbtextstr[strlen(kbtextstr)] = txt[0];
							SetDisplayText(kbtextstr);
						}
						keyBtn[i][j]->SetState(STATE_SELECTED, t->chan);

						if (shift || alt || alt2)
						{
							if (shift) shift ^= 1;
							if (alt) alt ^= 1;
							if (alt2) alt2 ^= 1;
							update = true;
							changedShiftKey = true;
						}
					}
				}
			}
		}
	} while (update);
}
