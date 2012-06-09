/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
#include "gui_checkbox.hpp"

#define WHITEBOX_RED_SIZE   4

GuiCheckbox::GuiCheckbox(int s)
	: GuiButton(30, 30), Checked(false), MultiStates(false)
{
	style = s;
	Checksign.SetParent(this);
	Cross.SetParent(this);
	Plus.SetParent(this);
	Blackbox.SetParent(this);
	Whitebox.SetParent(this);
	Checksign.SetColor((GXColor) {0, 0, 0, 255});
	Cross.SetColor((GXColor) {0, 0, 0, 255});
	Plus.SetColor((GXColor) {0, 0, 0, 255});
	Blackbox.SetColor((GXColor) {0, 0, 0, 255});
	Whitebox.SetColor((GXColor) {255, 255, 255, 255});

	SetSize(30, 30);
}

GuiCheckbox::GuiCheckbox(int w, int h, int s)
		: GuiButton(w, h), Checked(false), MultiStates(false)
{
	style = s;
	Checksign.SetParent(this);
	Cross.SetParent(this);
	Plus.SetParent(this);
	Blackbox.SetParent(this);
	Whitebox.SetParent(this);
	Checksign.SetColor((GXColor) {0, 0, 0, 255});
	Cross.SetColor((GXColor) {0, 0, 0, 255});
	Plus.SetColor((GXColor) {0, 0, 0, 255});
	Blackbox.SetColor((GXColor) {0, 0, 0, 255});
	Whitebox.SetColor((GXColor) {255, 255, 255, 255});

	SetSize(w, h);
}

void GuiCheckbox::SetSize(int w, int h)
{
	width = w;
	height = h;
	Checksign.SetSize(w-WHITEBOX_RED_SIZE, h-WHITEBOX_RED_SIZE);
	Checksign.SetPosition(WHITEBOX_RED_SIZE/2, WHITEBOX_RED_SIZE/2);
	Cross.SetSize(w-WHITEBOX_RED_SIZE, h-WHITEBOX_RED_SIZE);
	Cross.SetPosition(WHITEBOX_RED_SIZE/2, WHITEBOX_RED_SIZE/2);
	Plus.SetSize(w-WHITEBOX_RED_SIZE, h-WHITEBOX_RED_SIZE);
	Plus.SetPosition(WHITEBOX_RED_SIZE/2, WHITEBOX_RED_SIZE/2);
	Blackbox.SetSize(w, h);
	Whitebox.SetSize(w-WHITEBOX_RED_SIZE, h-WHITEBOX_RED_SIZE);
	Whitebox.SetPosition(WHITEBOX_RED_SIZE/2, WHITEBOX_RED_SIZE/2);
	SetAlignment(alignmentHor, alignmentVert);
}

void GuiCheckbox::SetClickSize(int w, int h)
{
	width = w;
	height = h;
}

void GuiCheckbox::SetTransparent(bool b)
{
	Blackbox.SetFilled(b);
	Whitebox.SetFilled(b);
}

void GuiCheckbox::SetState(int s, int c)
{
	if(s == STATE_CLICKED)
	{
		if(MultiStates)
		{
			if(!Checked)
				Checked = !Checked;
			else
				style++;
				
			if(style == MAX_CHECKBOX_STYLE)
			{
				Checked = !Checked;
				style = CHECKSIGN;
			}		
		}
		else
			Checked = !Checked;
	}

	GuiButton::SetState(s, c);
}

void GuiCheckbox::SetAlignment(int h, int v)
{
	GuiButton::SetAlignment(h, v);
	Checksign.SetAlignment(h, v);
	Cross.SetAlignment(h, v);
	Plus.SetAlignment(h, v);
	Blackbox.SetAlignment(h, v);
	Whitebox.SetAlignment(h, v);

	if(h == ALIGN_RIGHT)
	{
		Checksign.SetPosition(-WHITEBOX_RED_SIZE/2, Checksign.GetTopPos());
		Cross.SetPosition(-WHITEBOX_RED_SIZE/2, Cross.GetTopPos());
		Plus.SetPosition(-WHITEBOX_RED_SIZE/2, Plus.GetTopPos());
		Whitebox.SetPosition(-WHITEBOX_RED_SIZE/2, Whitebox.GetTopPos());
	}
	else if(h == ALIGN_CENTER)
	{
		Checksign.SetPosition(0, Checksign.GetTopPos());
		Cross.SetPosition(0, Cross.GetTopPos());
		Plus.SetPosition(0, Plus.GetTopPos());
		Whitebox.SetPosition(0, Whitebox.GetTopPos());
	}
	if(v == ALIGN_BOTTOM)
	{
		Checksign.SetPosition(Checksign.GetLeftPos(), -WHITEBOX_RED_SIZE/2);
		Cross.SetPosition(Cross.GetLeftPos(), -WHITEBOX_RED_SIZE/2);
		Plus.SetPosition(Plus.GetLeftPos(), -WHITEBOX_RED_SIZE/2);
		Whitebox.SetPosition(Whitebox.GetLeftPos(), -WHITEBOX_RED_SIZE/2);
	}
	else if(v == ALIGN_MIDDLE)
	{
		Checksign.SetPosition(Checksign.GetLeftPos(), 0);
		Cross.SetPosition(Cross.GetLeftPos(), 0);
		Plus.SetPosition(Plus.GetLeftPos(), 0);
		Whitebox.SetPosition(Whitebox.GetLeftPos(), 0);
	}
}

void GuiCheckbox::Draw()
{
	GuiButton::Draw();
	Blackbox.Draw();
	Whitebox.Draw();
	if(Checked)
	{
		if(style == CHECKSIGN)
			Checksign.Draw();
		else if (style == PLUS)
			Plus.Draw();
		else
			Cross.Draw();
	}
}
