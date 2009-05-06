/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_tooltip.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

GuiImageData GuiTooltip::tooltipStd(tooltip_png);
GuiImageData GuiTooltip::tooltipMedium(tooltip_medium_png);
GuiImageData GuiTooltip::tooltipLarge(tooltip_large_png);


/**
 * Constructor for the GuiTooltip class. 
 */ 
GuiTooltip::GuiTooltip(const char *t)
{
	text = NULL;
	image.SetParent(this); 
	SetText(t);
}

/*
 * Destructor for the GuiTooltip class.
 */ 
GuiTooltip::~GuiTooltip()
{
	if(text)	delete text;	
}

/* !Sets the text of the GuiTooltip element 
 * !\param t Text 
 */
void GuiTooltip::SetText(const char * t)
{
	LOCK(this);
	if(text)
	{
		delete text;
		text = NULL;
	}
	int t_width = 24;
	if(t && (text = new GuiText(t, 22, (GXColor){0, 0, 0, 255})))
	{
		text->SetParent(this); 
		t_width += text->GetTextWidth();
	}

	if(t_width > tooltipMedium.GetWidth())
		image.SetImage(&tooltipLarge);
	else if(t_width > tooltipStd.GetWidth())
		image.SetImage(&tooltipMedium);
	else
		image.SetImage(&tooltipStd);
	image.SetPosition(0, 0);
	width = image.GetWidth();
	height = image.GetHeight();
}

/*
 * Draw the Tooltip on screen
 */
void GuiTooltip::Draw()
{
	LOCK(this);
	if(!this->IsVisible()) return; 

	image.Draw(); 
	if(text) text->Draw();

	this->UpdateEffects();
}
