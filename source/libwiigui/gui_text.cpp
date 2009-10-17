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

static int presetSize = 18;
static int presetMaxWidth = 0;
static int presetAlignmentHor = 0;
static int presetAlignmentVert = 0;
static int presetWrapMode = 0;
static u16 presetStyle = FTGX_NULL;
static GXColor presetColor = (GXColor){255, 255, 255, 255};

#define TEXT_SCROLL_DELAY			5
#define	TEXT_SCROLL_INITIAL_DELAY	8

/**
 * Constructor for the GuiText class.
 */
GuiText::GuiText(const char * t, int s, GXColor c)
{
	origText = NULL;
	text = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c.a;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrapMode = 0;
	firstLine = 0;
	linestodraw = 8;
	totalLines = 0;
	widescreen = 0; //added
	LineBreak = NULL;
	textDyn = NULL;
	font = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
        textDynRow[i] = NULL;

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);

        if(currentSize > MAX_FONT_SIZE)
            currentSize = MAX_FONT_SIZE;

        if(!fontSystem[currentSize])
        {
            fontSystem[currentSize] = new FreeTypeGX(currentSize);
        }
		textWidth = fontSystem[currentSize]->getWidth(text);
	}
}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	origText = NULL;
	text = NULL;
	size = presetSize;
	currentSize = size;
	color = presetColor;
	alpha = presetColor.a;
	style = presetStyle;
	maxWidth = presetMaxWidth;
	wrapMode = presetWrapMode;
	firstLine = 0;
	linestodraw = 8;
	totalLines = 0;
	widescreen = 0; //added
	LineBreak = NULL;
	textDyn = NULL;
	font = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = presetAlignmentHor;
	alignmentVert = presetAlignmentVert;

	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
        textDynRow[i] = NULL;

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);

        if(currentSize > MAX_FONT_SIZE)
            currentSize = MAX_FONT_SIZE;

        if(!fontSystem[currentSize])
        {
            fontSystem[currentSize] = new FreeTypeGX(currentSize);
        }
		textWidth = fontSystem[currentSize]->getWidth(text);
	}
}

/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(origText)
		free(origText);
	if(text)
		delete [] text;
	if(textDyn)
		delete [] textDyn;

    if(LineBreak) {
        free(LineBreak);
        LineBreak = NULL;
	}
	if(font)
	{
        delete font;
        font = NULL;
    }
	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
            delete [] textDynRow[i];
            textDynRow[i] = NULL;
	    }
	}
}

void GuiText::SetText(const char * t)
{
    if(t && origText)
        if(strcmp(t, origText) == 0)
            return;

    LOCK(this);

	if(origText)
		free(origText);
	if(text)
		delete [] text;
	if(textDyn)
		delete [] textDyn;
	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
           delete [] textDynRow[i];
            textDynRow[i] = NULL;
	    }
	}

	origText = NULL;
	text = NULL;
	textDyn = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);
		textWidth = fontSystem[currentSize]->getWidth(text);
	}
}

void GuiText::SetText(const wchar_t * t)
{
	LOCK(this);
    if(origText)
		free(origText);
	if(text)
		delete [] text;
	if(textDyn)
		delete [] textDyn;
	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
           delete [] textDynRow[i];
            textDynRow[i] = NULL;
	    }
	}

    origText = NULL;
	text = NULL;
	textDyn = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		int len = wcslen(t);
		text = new wchar_t[len+1];
		if(text) wcscpy(text, t);
	}
}

void GuiText::SetTextf(const char *format, ...)
{
    LOCK(this);

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		this->SetText(tmp);
		free(tmp);
	}
	va_end(va);
}

void GuiText::SetPresets(int sz, GXColor c, int w, u16 s, int h, int v)
{
    LOCK(this);
	presetSize = sz;
	presetColor = c;
	presetStyle = s;
	presetMaxWidth = w;
	presetAlignmentHor = h;
	presetAlignmentVert = v;
}

void GuiText::SetFontSize(int s)
{
    LOCK(this);

	size = s;
}

void GuiText::SetMaxWidth(int width, int w)
{
    LOCK(this);

	maxWidth = width;
	wrapMode = w;

	if(w == LONGTEXT && text) {

        int strlen = wcslen(text);
        int i = 0;
        int ch = 0;
        int linenum = 0;
        int lastSpace = -1;
        int lastSpaceIndex = -1;

        wchar_t *tmptext = new wchar_t[maxWidth];

        LineBreak = (u32 *) malloc(sizeof(u32));
        memset(&(LineBreak[linenum]), 0, sizeof(u32));

        LineBreak[linenum] = 0;
        linenum++;

        while(ch < strlen)
        {
            tmptext[i] = text[ch];
            tmptext[i+1] = 0;

            if(text[ch] == ' ' || ch == strlen-1 || fontSystem[currentSize]->getWidth(tmptext) >= maxWidth)
            {
                if(fontSystem[currentSize]->getWidth(tmptext) >= maxWidth)
                {
                    if(lastSpace >= 0)
                    {
                        tmptext[lastSpaceIndex] = 0; // discard space, and everything after
                        ch = lastSpace; // go backwards to the last space
                        lastSpace = -1; // we have used this space
                        lastSpaceIndex = -1;
                    }
                    LineBreak = (u32 *) realloc(LineBreak, (linenum+1)* sizeof(u32));
                    memset(&(LineBreak[linenum]), 0, sizeof(u32));
                    LineBreak[linenum] = ch;
                    linenum++;
                    i = -1;
                }
            }
            if(text[ch] == ' ' && i >= 0)
            {
                lastSpace = ch+1;
                lastSpaceIndex = i;
            }
            if(text[ch] == '\n')
            {
                LineBreak = (u32 *) realloc(LineBreak, (linenum+1)* sizeof(u32));
                memset(&(LineBreak[linenum]), 0, sizeof(u32));
                LineBreak[linenum] = ch+1;
                linenum++;
                i = -1;
                lastSpace = -1;
                lastSpaceIndex = -1;
            }
            ch++;
            i++;

            if(ch == strlen)
            {
                LineBreak = (u32 *) realloc(LineBreak, (linenum+1)* sizeof(u32));
                memset(&(LineBreak[linenum]), 0, sizeof(u32));
                LineBreak[linenum] = ch;
                linenum++;
                break;
            }
        }
        delete [] tmptext;
        totalLines = linenum;
	}

	else if(w == SCROLL_HORIZONTAL) {

        textScrollPos = 0;
        textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
        textScrollDelay = TEXT_SCROLL_DELAY;
	}

    if(textDyn)
    {
        delete [] textDyn;
        textDyn = NULL;
    }

	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
            delete [] textDynRow[i];
            textDynRow[i] = NULL;
	    }
	}
}

void GuiText::SetColor(GXColor c)
{
    LOCK(this);
	color = c;
	alpha = c.a;
}

void GuiText::SetStyle(u16 s)
{
    LOCK(this);
	style = s;
}

void GuiText::SetAlignment(int hor, int vert)
{
    LOCK(this);
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

int GuiText::GetTextWidth()
{
	if(!text)
		return 0;

	return fontSystem[currentSize]->getWidth(text);
}

/**
 * Set the lines to draw
 */
void GuiText::SetFirstLine(int line)
{
    LOCK(this);
	firstLine = line;

	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
            delete [] textDynRow[i];
            textDynRow[i] = NULL;
	    }
	}
}

void GuiText::SetLinesToDraw(int line)
{
    LOCK(this);
	linestodraw = line;

	if(linestodraw >= MAX_LINES_TO_DRAW)
        linestodraw = MAX_LINES_TO_DRAW-1;

	for(int i = 0; i < MAX_LINES_TO_DRAW; i++)
	{
	    if(textDynRow[i])
	    {
            delete [] textDynRow[i];
            textDynRow[i] = NULL;
	    }
	}
}

int GuiText::GetTotalLines()
{
    return totalLines-1;
}

/**
 * Change font
 */
bool GuiText::SetFont(const u8 *fontbuffer, const u32 filesize)
{
    if(!fontbuffer || !filesize)
        return false;

    LOCK(this);
    if(font)
    {
        delete font;
        font = NULL;
    }
	font = new FreeTypeGX(currentSize, false, (u8 *) fontbuffer, filesize);
	textWidth = font->getWidth(text);

	return true;
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

	if(newSize > MAX_FONT_SIZE)
		newSize = MAX_FONT_SIZE;

	if(newSize != currentSize)
	{
	    if(font)
	    {
	        font->ChangeFontSize(newSize);
	    }
        else if(!fontSystem[newSize])
            fontSystem[newSize] = new FreeTypeGX(newSize);

        if(text)
            textWidth = (font ? font : fontSystem[newSize])->getWidth(text);
		currentSize = newSize;
	}

	if(maxWidth > 0 && maxWidth <= textWidth)
	{
		if(wrapMode == LONGTEXT) // text wrapping
		{
            int lineheight = newSize + 6;
		    if(!textDynRow[0])
		    {
                int index = 0;
                u32 strlen = (u32) wcslen(text);
                int linenum = firstLine;
                int lineIndex = 0;
                u32 ch = LineBreak[linenum];

                u32 lastch = LineBreak[linenum+linestodraw]+1;

                textDynRow[lineIndex] = new wchar_t[maxWidth];

                textDynRow[lineIndex][index] = 0;

                while((ch < lastch) && (ch < strlen+1))
                {
                    if(ch == LineBreak[linenum+1])
                    {
                        (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop()+lineIndex*lineheight, textDynRow[lineIndex], c, style);
                        linenum++;
                        lineIndex++;
                        index = 0;
                        if(!textDynRow[lineIndex])
                            textDynRow[lineIndex] = new wchar_t[maxWidth];
                        textDynRow[lineIndex][index] = 0;
                    }

                    textDynRow[lineIndex][index] = text[ch];
                    textDynRow[lineIndex][index+1] = 0;

                    index++;
                    ch++;
                }
		    }
		    else
		    {
                for(int i = 0; i < linestodraw; i++)
                    if(textDynRow[i])
                        (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop()+i*lineheight, textDynRow[i], c, style);
		    }
		}
		else if(wrapMode == DOTTED) // text dotted
		{
		    if(!textDyn)
		    {
                int strlen = wcslen(text);
                int i = 0;

                textDyn = new wchar_t[maxWidth];

                while(i < strlen)
                {
                    textDyn[i] = text[i];
                    textDyn[i+1] = 0;

                    if((font ? font : fontSystem[currentSize])->getWidth(textDyn) >= maxWidth)
                    {
                        textDyn[i-3] = '.';
                        textDyn[i-2] = '.';
                        textDyn[i-1] = '.';
                        textDyn[i] = 0;
                        break;
                    }

                    i++;
                }
		    }
		    if(textDyn)
                (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop(), textDyn, c, style);
		}

		else if(wrapMode == SCROLL_HORIZONTAL)
		{
			int strlen = wcslen(text);

            int ch = 0;
            if(!textDyn) {
                textDyn = new wchar_t[maxWidth];

                while(ch < strlen)
                {
                    textDyn[ch] = text[ch];
                    textDyn[ch+1] = 0;
                    ch++;

                    if((font ? font : fontSystem[currentSize])->getWidth(textDyn) >= maxWidth)
                        break;
                }
            }

			if(maxWidth < textWidth && (frameCount % textScrollDelay == 0))
			{
				if(textScrollInitialDelay)
				{
					textScrollInitialDelay--;
				}
				else
				{
					textScrollPos++;
					if(textScrollPos > strlen)
					{
						textScrollPos = 0;
						textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
					}

					int ch = textScrollPos;

					if(textDyn) {
                        delete [] textDyn;
                        textDyn = NULL;
					}
                    textDyn = new wchar_t[maxWidth];

                    int i = 0;

					while(1)
					{
                        if(ch > strlen-1) {
                            textDyn[i++] = ' ';
                            textDyn[i++] = ' ';
                            textDyn[i++] = ' ';
                            textDyn[i+1] = 0;
                            ch = 0;
                        }

					    textDyn[i] = text[ch];
					    textDyn[i+1] = 0;
					    ch++;
					    i++;

					    if((font ? font : fontSystem[currentSize])->getWidth(textDyn) >= maxWidth)
                            break;
					}
				}
			}
			if(textDyn)
				(font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop(), textDyn, c, style);
        }
		else if(wrapMode == WRAP)
		{
            int lineheight = newSize + 6;
		    if(!textDynRow[0])
		    {
                int txtlen = wcslen(text);
                int i = 0;
                int ch = 0;
                int linenum = 0;
                int lastSpace = -1;
                int lastSpaceIndex = -1;

                while(ch < txtlen && linenum < MAX_LINES_TO_DRAW)
                {
                    if(!textDynRow[linenum])
                        textDynRow[linenum] = new wchar_t[maxWidth];

                    textDynRow[linenum][i] = text[ch];
                    textDynRow[linenum][i+1] = 0;

                    if(text[ch] == ' ' || ch == txtlen-1)
                    {
                        if((font ? font : fontSystem[currentSize])->getWidth(textDynRow[linenum]) >= maxWidth)
                        {
                            if(lastSpace >= 0)
                            {
                                textDynRow[linenum][lastSpaceIndex] = 0; // discard space, and everything after
                                ch = lastSpace; // go backwards to the last space
                                lastSpace = -1; // we have used this space
                                lastSpaceIndex = -1;
                            }
                            linenum++;
                            i = -1;
                        }
                        else if(ch == txtlen-1)
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
                linestodraw = linenum;
		    }

            int voffset = 0;
            if(alignmentVert == ALIGN_MIDDLE)
                voffset = -(lineheight*linestodraw)/2 + lineheight/2;

            for(int i=0; i < linestodraw; i++)
            {
                if(textDynRow[i])
                    (font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop()+voffset+i*lineheight, textDynRow[i], c, style);
            }
		}
		else
		{
			(font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop(), text, c, style);
		}
	}
	else
	{
		(font ? font : fontSystem[currentSize])->drawText(this->GetLeft(), this->GetTop(), text, c, style);
	}
	this->UpdateEffects();
}
