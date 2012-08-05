#include <wctype.h>
#include "gui.h"
#include "gui_searchbar.h"

#include "../wpad.h"
#include "../main.h"
#include "../settings/CSettings.h"
#include "../settings/GameTitles.h"
#include "../themes/CTheme.h"
#include "../usbloader/GameList.h"

extern GuiWindow * mainWindow;

class cSearchButton
{
	public:
		cSearchButton(wchar_t *Char, GuiImageData *keyImageData, GuiImageData *keyOverImageData, int x, int y,
				GuiTrigger* trig, GuiSound* sndOver, GuiSound* sndClick) :
			wchar(*Char), image(keyImageData), imageOver(keyOverImageData), text((char *) NULL, 20, ( GXColor )
			{   0, 0, 0, 0xff}), button(&image, &imageOver, ALIGN_LEFT, ALIGN_TOP, x, y, trig, sndOver, sndClick, 1)
		{
			text.SetText(Char);
			button.SetLabel(&text);
		}
		wchar_t wchar;
		GuiImage image;
		GuiImage imageOver;
		GuiText text;
		GuiButton button;
	private:

};

static wchar_t lastSearchChar = 0;
std::set<wchar_t> GuiSearchBar::SearchChars;

GuiSearchBar::GuiSearchBar() :
	inSide(0), text((char *) NULL, 22, ( GXColor ) {0, 0, 0, 255}), buttons(0),
	keyImageData(Resources::GetFile("keyboard_key.png"), Resources::GetFileSize("keyboard_key.png")),
	keyOverImageData(Resources::GetFile("keyboard_key_over.png"), Resources::GetFileSize("keyboard_key_over.png"))
{
	trig.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	SetAlignment(ALIGN_CENTER, ALIGN_MIDDLE);

	cnt = SearchChars.size();
	buttons = new cSearchButton*[cnt];

	wchar_t charstr[2] = { 0, 0 };
	int lines = (cnt + 9) / 10;
	int buttonsPerLine = (cnt + lines - 1) / lines;
	width = 10 + buttonsPerLine * 42 + 10;
	int i = 0, x_start = 10, x = 0, y_start = 10 + 42, y = 0;
	if (width < 320)
	{
		x_start += (320 - width) >> 1;
		width = 320;
	}
	for (std::set<wchar_t>::iterator it=SearchChars.begin() ; it != SearchChars.end(); it++, i++, x++)
	{
		if (x >= buttonsPerLine) x = 0;
		if (x == 0) y++;
		charstr[0] = *it;
		buttons[i] = new cSearchButton(charstr, &keyImageData, &keyOverImageData, x_start + x * 42, y_start - 42 + y
				* 42, &trig, btnSoundOver, btnSoundClick);
		this->Append(&(buttons[i]->button));
	}
	height = 10 + 42 + y * 42 + 10;

	charstr[0] = Settings.SearchMode == SEARCH_BEGINNING ? L'=' : L'*';
	searchModeBtn = new cSearchButton(charstr,  &keyImageData, &keyOverImageData, 10, 10, &trig, btnSoundOver, btnSoundClick);
	this->Append(&searchModeBtn->button);

	text.SetText(gameList.GetCurrentFilter());
	text.SetPosition(55, 15);
	text.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	text.SetWidescreen(Settings.widescreen);
	text.SetMaxWidth(width - (55 + 2 * 42 + 10), SCROLL_HORIZONTAL);
	this->Append(&text);

	imgBacspaceBtn = Resources::GetImageData("keyboard_backspace_over.png");
	BacspaceBtnImg_Over = new GuiImage(imgBacspaceBtn);
	BacspaceBtnImg = new GuiImage(BacspaceBtnImg_Over);
	BacspaceBtnImg->SetGrayscale();
	BacspaceBtn = new GuiButton(BacspaceBtnImg, BacspaceBtnImg_Over, ALIGN_RIGHT, ALIGN_TOP, -52, 10, &trig, btnSoundOver, btnSoundClick, 1);
	this->Append(BacspaceBtn);

	imgClearBtn = Resources::GetImageData("keyboard_clear_over.png");
	ClearBtnImg_Over = new GuiImage(imgClearBtn);
	ClearBtnImg = new GuiImage(ClearBtnImg_Over);
	ClearBtnImg->SetGrayscale();
	ClearBtn = new GuiButton(ClearBtnImg, ClearBtnImg_Over, ALIGN_RIGHT, ALIGN_TOP, -10, 10, &trig, btnSoundOver, btnSoundClick, 1);
	this->Append(ClearBtn);

	CloseBtn = new GuiButton(0, 0);
	CloseBtn->SetTrigger(&trigB);
	this->Append(CloseBtn);

	//  SetPosition(100,100);

}
GuiSearchBar::~GuiSearchBar()
{
	if (buttons)
	{
		for (int i = 0; i < cnt; i++)
			delete buttons[i];
		delete[] buttons;
	}
	delete ClearBtn;
	delete ClearBtnImg;
	delete ClearBtnImg_Over;
	delete imgClearBtn;

	delete CloseBtn;
	delete searchModeBtn;

	delete BacspaceBtn;
	delete BacspaceBtnImg;
	delete BacspaceBtnImg_Over;
	delete imgBacspaceBtn;
	if (inSide) mainWindow->SetState(STATE_DEFAULT);
}
void GuiSearchBar::Draw()
{
	Menu_DrawRectangle(this->GetLeft(), this->GetTop(), width, height, ( GXColor ) {0, 0, 0, 0xa0}, 1);
	Menu_DrawRectangle(this->GetLeft() + 55, this->GetTop() + 15, width - (55 + 2 * 42 + 10), 22, ( GXColor ) {255, 255, 255, 255}, 1);
	GuiWindow::Draw();
}
void GuiSearchBar::Update(GuiTrigger * t)
{
	LOCK( this );
	if (_elements.size() == 0 || (state == STATE_DISABLED && parentElement)) return;
	// cursor
	if (t->wpad.ir.valid && state != STATE_DISABLED)
	{
		if (this->IsInside(t->wpad.ir.x, t->wpad.ir.y))
		{
			if (inSide == 0)
			{
				mainWindow->SetState(STATE_DISABLED);
				this->SetState(STATE_DEFAULT);
			}
			inSide |= 1 << t->chan;
		}
		else if (inSide)
		{
			inSide &= ~(1 << t->chan);
			if (inSide == 0) mainWindow->SetState(STATE_DEFAULT);
		}
	}
	GuiWindow::Update(t);
}
wchar_t GuiSearchBar::GetClicked()
{
	lastSearchChar = 0;

	if (buttons)
	{
		for (int i = 0; i < cnt; i++)
		{
			if (buttons[i]->button.GetState() == STATE_CLICKED)
			{
				buttons[i]->button.ResetState();
				lastSearchChar = buttons[i]->wchar;
			}
		}
	}

	if (BacspaceBtn->GetState() == STATE_CLICKED) lastSearchChar = 8;
	else if (ClearBtn->GetState() == STATE_CLICKED) lastSearchChar = 7;
	else if (CloseBtn->GetState() == STATE_CLICKED) lastSearchChar = 27;
	else if (searchModeBtn->button.GetState() == STATE_CLICKED) lastSearchChar = 6;

	return lastSearchChar;
}

void GuiSearchBar::FilterList(std::vector<struct discHdr *> &List, wString &GameFilter)
{
	bool endOfGameName = false;  // endOfGameName is disabled by default

	SearchChars.clear();

	for (u32 i = 0; i < List.size(); ++i)
	{
		struct discHdr *header = List.at(i);

		wchar_t *gameName = charToWideChar(GameTitles.GetTitle(header));
		if (!gameName)
		{
			List.erase(List.begin()+i);
			i--;
			continue;
		}

		if(Settings.SearchMode == SEARCH_BEGINNING)
		{
			if (GameFilter.size() > 0 && wcsncasecmp(gameName, GameFilter.c_str(), GameFilter.size()) != 0)
			{
				delete [] gameName;
				List.erase(List.begin()+i);
				i--;
				continue;
			}

			if (   wcslen(gameName) > GameFilter.size()
				&& SearchChars.find(towupper(gameName[GameFilter.size()])) == SearchChars.end()
				&& SearchChars.find(towlower(gameName[GameFilter.size()])) == SearchChars.end())
			{
				SearchChars.insert(gameName[GameFilter.size()]);
			}
			else if (wcslen(gameName) == GameFilter.size())  // The end of the game name was reached
				endOfGameName = true;
		}
		else if(Settings.SearchMode == SEARCH_CONTENT)
		{
			if(GameFilter.size() > 0)
			{
				if (wcscasestr(gameName, GameFilter.c_str()) == 0)
				{
					delete [] gameName;
					List.erase(List.begin()+i);
					i--;
					continue;
				}

				const wchar_t *found = gameName;
				while((found = wcscasestr(found, GameFilter.c_str())) != 0)
				{
					found += GameFilter.size();
					wchar_t ch = towupper(*found);
					if(ch)
						SearchChars.insert(ch);
					else      // The end of the game name was reached
						endOfGameName = true;
				}
			}
			else
			{
				for(const wchar_t *wPtr = gameName; *wPtr != 0; wPtr++)
				{
					wchar_t ch = towupper(*wPtr);
					if(ch > L'@')
						SearchChars.insert(ch);
				}
			}
		}

		delete [] gameName;
	}

	if(List.size() < 2)
		SearchChars.clear();

	// If the last character was not backslash try autocomplete
	if(SearchChars.size() == 1 && GameFilter.size() > 0 && lastSearchChar != 8 && !endOfGameName)
	{
		GameFilter += *SearchChars.begin();
		FilterList(List, GameFilter);
	}
}

/*
 private:
 SearchButtons *buttons;
 }*/

