#include "settings/CSettings.h"
#include "utils/tools.h"
#include "Text.hpp"

Text::Text(const char * t, int s, GXColor c) :
	GuiText(t, s, c)
{
	maxWidth = 400;
	linestodraw = 9;
	curLineStart = 0;
	FirstLineOffset = 0;
	wText = NULL;

	if (!text) return;

	wText = new (std::nothrow) wString(text);
	if (!wText)
	{
		return;
	}

	if (wText->size() == 0)
	{
		wText->push_back(L' ');
		wText->push_back(0);
	}

	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);
	delete[] text;
	text = NULL;

	SetMaxWidth(maxWidth);
}

Text::Text(const wchar_t * t, int s, GXColor c) :
	GuiText((wchar_t *) NULL, s, c)
{
	maxWidth = 400;
	linestodraw = 9;
	curLineStart = 0;
	FirstLineOffset = 0;
	wText = NULL;

	if (!t) return;

	wText = new (std::nothrow) wString(t);
	if (!wText)
	{
		return;
	}

	if (wText->size() == 0)
	{
		wText->push_back(L' ');
		wText->push_back(0);
	}

	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);

	SetMaxWidth(maxWidth);
}

Text::~Text()
{
	if (wText) delete wText;
	wText = NULL;

	TextLines.clear();
	ClearDynamicText();
}

void Text::SetText(const char * t)
{
	wchar_t * tmp = charToWideChar(t);
	if (!tmp) return;

	if (wText) delete wText;

	wText = new (std::nothrow) wString(tmp);
	if (!wText)
	{
		return;
	}

	if (wText->size() == 0)
	{
		wText->push_back(L' ');
		wText->push_back(0);
	}

	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);

	delete[] tmp;

	ClearDynamicText();
	CalcLineOffsets();
}

void Text::SetText(const wchar_t * t)
{
	if (!t) return;

	if (wText) delete wText;

	wText = new wString(t);
	textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);
	CalcLineOffsets();
}

void Text::SetMaxWidth(int w)
{
	maxWidth = w;
	curLineStart = 0;
	Refresh();
}

void Text::SetTextLine(int line)
{
	if (line < 0)
		line = 0;
	else if (line > (int) TextLines.size() - 1) line = TextLines.size() - 1;

	curLineStart = line;

	FillRows();

	while ((int) textDyn.size() < linestodraw && curLineStart > 0)
	{
		PreviousLine();
	}
}

void Text::SetTextPos(int pos)
{
	if (!wText) return;

	int diff = 10000;

	for (u32 i = 0; i < TextLines.size(); i++)
	{
		int curDiff = abs(TextLines[i].LineOffset - pos);
		if (curDiff < diff)
		{
			diff = curDiff;
			curLineStart = i;
		}
	}

	FillRows();

	while ((int) textDyn.size() < linestodraw && curLineStart > 0)
	{
		PreviousLine();
	}
}

const wchar_t * Text::GetText()
{
	return wText->c_str();
}

std::string Text::GetUTF8String(void) const
{
	return wText->toUTF8();
}

int Text::GetLineOffset(int ind)
{
	if (TextLines.size() == 0) return 0;

	if (ind < 0) return TextLines[0].LineOffset;

	if (ind >= (int) TextLines.size() - 1) return TextLines[TextLines.size() - 1].LineOffset;

	return TextLines[ind].LineOffset;
}

const wchar_t * Text::GetTextLine(int ind)
{
	if (filling || textDyn.size() == 0) return NULL;

	if (ind < 0) return textDyn[0];

	if (ind >= (int) textDyn.size()) return textDyn[textDyn.size() - 1];

	return textDyn[ind];
}

void Text::Refresh()
{
	CalcLineOffsets();
	FillRows();
}

void Text::NextLine()
{
	if (!wText || (curLineStart + 1 > ((int) TextLines.size() - linestodraw))) return;

	++curLineStart;

	FillRows();
}

void Text::PreviousLine()
{
	if (!wText || curLineStart - 1 < 0) return;

	--curLineStart;

	FillRows();
}

void Text::FillRows()
{
	if (!wText) return;

	filling = true;

	ClearDynamicText();

	for (int i = 0; i < linestodraw && curLineStart+i < (int) TextLines.size(); i++)
	{
		if (i >= (int) textDyn.size())
		{
			textDyn.resize(i + 1);
			textDyn[i] = new wchar_t[maxWidth];
		}
		int offset = TextLines[curLineStart + i].LineOffset;
		int count = TextLines[curLineStart + i].CharCount + 1;

		for (int n = 0; n < count && offset + n < (int) wText->size(); n++)
			textDyn[i][n] = wText->at(offset + n);

		textDyn[i][count] = 0;
	}

	filling = false;

	return;
}

void Text::CalcLineOffsets()
{
	if (!wText) return;

	TextLines.clear();

	TextLine TmpLine;
	TmpLine.CharCount = 0;
	TmpLine.LineOffset = 0;
	TmpLine.width = 0;

	const wchar_t * origTxt = wText->c_str();
	int ch = 0;
	int lastSpace = -1;
	int currWidth = 0;
	int i = 0;

	while (origTxt[ch])
	{
		currWidth += fontSystem->getCharWidth(origTxt[ch], currentSize, ch > 0 ? origTxt[ch - 1] : 0x0000);

		if (currWidth >= maxWidth)
		{
			if (lastSpace > 0)
			{
				ch = lastSpace;
			}
			TmpLine.CharCount = ch - TmpLine.LineOffset;
			TmpLine.width = currWidth;
			TextLines.push_back(TmpLine);
			currWidth = 0;
			lastSpace = -1;
			i = -1;
			TmpLine.LineOffset = ch + 1;
		}
		else if (origTxt[ch] == '\n')
		{
			TmpLine.CharCount = ch - TmpLine.LineOffset;
			TmpLine.width = currWidth;
			TextLines.push_back(TmpLine);
			currWidth = 0;
			lastSpace = -1;
			i = -1;
			TmpLine.LineOffset = ch + 1;
		}
		else if (origTxt[ch] == ' ')
		{
			lastSpace = ch;
		}

		ch++;
		i++;
	}

	TmpLine.CharCount = ch - TmpLine.LineOffset;
	TmpLine.width = currWidth;

	if (TmpLine.CharCount-1 > 0)
	{
		TmpLine.CharCount -= 1;
		TextLines.push_back(TmpLine);
	}
}

void Text::Draw()
{
	if (textDyn.size() == 0) return;

	if (!this->IsVisible()) return;

	GXColor c = color;
	c.a = this->GetAlpha();

	int newSize = (int) (size * GetScale());

	if (newSize != currentSize)
	{
		currentSize = LIMIT(newSize, 1, 100);

		if (wText) textWidth = (font ? font : fontSystem)->getWidth(wText->data(), currentSize);
	}

	u16 lineheight = newSize + 6;

	for (u32 i = 0; i < textDyn.size(); i++)
	{
		if (!filling) (font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop() + i * lineheight, 0,
				textDyn[i], currentSize, c, style, 0, maxWidth);
	}
}
