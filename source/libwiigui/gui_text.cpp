/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_text.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

static int currentSize = 0;
static int presetSize = 0;
static int presetMaxWidth = 0;
static int presetAlignmentHor = 0;
static int presetAlignmentVert = 0;
static u16 presetStyle = 0;
static GXColor presetColor = (GXColor){255, 255, 255, 255};

/**
 * Constructor for the GuiText class.
 */
GuiText::GuiText(const char * t, int s, GXColor c)
{
	text = NULL;
	size = s;
	color = c;
	alpha = c.a;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

	if(t)
		text = fontSystem->charToWideChar((char *)t);
}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	text = NULL;
	size = presetSize;
	color = presetColor;
	alpha = presetColor.a;
	style = presetStyle;
	maxWidth = presetMaxWidth;

	alignmentHor = presetAlignmentHor;
	alignmentVert = presetAlignmentVert;

	if(t)
		text = fontSystem->charToWideChar((char *)t);
}

/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(text)
	{
		delete text;
		text = NULL;
	}
}

void GuiText::SetText(const char * t)
{
	if(text)
		delete text;

	text = NULL;

	if(t)
		text = fontSystem->charToWideChar((char *)t);
}

void GuiText::SetPresets(int sz, GXColor c, int w, u16 s, int h, int v)
{
	presetSize = sz;
	presetColor = c;
	presetStyle = s;
	presetMaxWidth = w;
	presetAlignmentHor = h;
	presetAlignmentVert = v;
}

void GuiText::SetFontSize(int s)
{
	size = s;
}

void GuiText::SetMaxWidth(int w)
{
	maxWidth = w;
}

void GuiText::SetColor(GXColor c)
{
	color = c;
	alpha = c.a;
}

void GuiText::SetStyle(u16 s)
{
	style = s;
}

void GuiText::SetAlignment(int hor, int vert)
{
	style = 0;

	switch(hor)
	{
		case ALIGN_LEFT:
			style |= FTGX_JUSTIFY_LEFT;
			break;
		case ALIGN_RIGHT:
			style |= FTGX_JUSTIFY_RIGHT;
			break;
		default:
			style |= FTGX_JUSTIFY_CENTER;
			break;
	}
	switch(vert)
	{
		case ALIGN_TOP:
			style |= FTGX_ALIGN_TOP;
			break;
		case ALIGN_BOTTOM:
			style |= FTGX_ALIGN_BOTTOM;
			break;
		default:
			style |= FTGX_ALIGN_MIDDLE;
			break;
	}

	alignmentHor = hor;
	alignmentVert = vert;
}

/**
 * Draw the text on screen
 */
void GuiText::Draw()
{
	if(!text)
		return;

	if(!this->IsVisible())
		return;

	GXColor c = color;
	c.a = this->GetAlpha();

	int newSize = size*this->GetScale();

	if(newSize != currentSize)
	{
		fontSystem->changeSize(newSize);
		currentSize = newSize;
	}

	int voffset = 0;

	if(alignmentVert == ALIGN_MIDDLE)
		voffset = -newSize/2 + 2;

	if(maxWidth > 0) // text wrapping
	{
		int lineheight = newSize + 6;
		int strlen = wcslen(text);
		int i = 0;
		int ch = 0;
		int linenum = 0;
		int lastSpace = -1;
		int lastSpaceIndex = -1;
		wchar_t * tmptext[20];

		while(ch < strlen)
		{
			if(i == 0)
				tmptext[linenum] = new wchar_t[strlen + 1];

			tmptext[linenum][i] = text[ch];
			tmptext[linenum][i+1] = 0;

			if(text[ch] == ' ' || ch == strlen-1)
			{
				if(fontSystem->getWidth(tmptext[linenum]) >= maxWidth)
				{
					if(lastSpace >= 0)
					{
						tmptext[linenum][lastSpaceIndex] = 0; // discard space, and everything after
						ch = lastSpace; // go backwards to the last space
						lastSpace = -1; // we have used this space
						lastSpaceIndex = -1;
					}
					linenum++;
					i = -1;
				}
				else if(ch == strlen-1)
				{
					linenum++;
				}
			}
			if(text[ch] == ' ' && i >= 0)
			{
				lastSpace = ch;
				lastSpaceIndex = i;
			}
			ch++;
			i++;
		}

		if(alignmentVert == ALIGN_MIDDLE)
			voffset = voffset - (lineheight*linenum)/2 + lineheight/2;

		for(i=0; i < linenum; i++)
		{
			fontSystem->drawText(this->GetLeft(), this->GetTop()+voffset+i*lineheight, tmptext[i], c, style);
			delete tmptext[i];
		}
	}
	else
	{
		fontSystem->drawText(this->GetLeft(), this->GetTop()+voffset, text, c, style);
	}
	this->UpdateEffects();
}
