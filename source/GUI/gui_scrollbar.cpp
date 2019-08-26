/***************************************************************************
 * Copyright (C) 2011
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
#include "themes/Resources.h"
#include "menu/menus.h"
#include "gui_scrollbar.hpp"
#include "utils/tools.h"

GuiScrollbar::GuiScrollbar(int h, u8 m)
{
	SelItem = 0;
	SelInd = 0;
	RowSize = 0;
	PageSize = 0;
	EntrieCount = 0;
	ScrollSpeed = 15;
	ButtonScroll = 0;
	ButtonScrollSpeed = 20;
	ScrollState = 0;
	pressedChan = -1;
	AllowDPad = true;
	MovePointer = false;
	Mode = m;
	listChanged.connect(this, &GuiScrollbar::setScrollboxPosition);

	scrollbarTop = Resources::GetImageData("scrollBarTop.png");
	scrollbarBottom = Resources::GetImageData("scrollBarBottom.png");
	scrollbarTile = Resources::GetImageData("scrollBarTile.png");
	arrowDown = Resources::GetImageData("scrollbar_arrowdown.png");
	arrowDownOver = Resources::GetImageData("scrollbar_arrowdown.png");
	arrowUp = Resources::GetImageData("scrollbar_arrowup.png");
	arrowUpOver = Resources::GetImageData("scrollbar_arrowup.png");
	scrollbarBox = Resources::GetImageData("scrollbar_box.png");
	scrollbarBoxOver = Resources::GetImageData("scrollbar_box.png");
	oneButtonScrollImgData = Resources::GetImageData("oneButtonScroll.png");

	height = h;
	width = MAX(scrollbarBox->GetWidth(), scrollbarTile->GetWidth());

	MinHeight = arrowUp->GetHeight();
	MaxHeight = height-scrollbarBox->GetHeight()-arrowDown->GetHeight();

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	int Tiles = (height-scrollbarTop->GetHeight()-scrollbarBottom->GetHeight())/4;
	int PositionY = 0;
	ButtonPositionX = 0;

	oneButtonScrollImg = new GuiImage(oneButtonScrollImgData);

	scrollbarTopImg = new GuiImage(scrollbarTop);
	scrollbarTopImg->SetParent(this);
	scrollbarTopImg->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	scrollbarTopImg->SetPosition(0, PositionY);
	PositionY += scrollbarTop->GetHeight();

	scrollbarTileImg = new GuiImage(scrollbarTile);
	scrollbarTileImg->SetParent(this);
	scrollbarTileImg->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	scrollbarTileImg->SetPosition(0, PositionY);
	scrollbarTileImg->SetTileVertical(Tiles);
	PositionY += Tiles*scrollbarTile->GetHeight();

	scrollbarBottomImg = new GuiImage(scrollbarBottom);
	scrollbarBottomImg->SetParent(this);
	scrollbarBottomImg->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	scrollbarBottomImg->SetPosition(0, PositionY);

	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOverImg = new GuiImage(arrowUpOver);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	arrowUpBtn->SetPosition(ButtonPositionX, 0);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);
	arrowUpBtn->Held.connect(this, &GuiScrollbar::OnUpButtonHold);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_CENTER, ALIGN_BOTTOM);
	arrowDownBtn->SetPosition(ButtonPositionX, 0);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);
	arrowDownBtn->Held.connect(this, &GuiScrollbar::OnDownButtonHold);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_CENTER, ALIGN_TOP);
	scrollbarBoxBtn->SetPosition(ButtonPositionX, MinHeight);
	scrollbarBoxBtn->SetMinY(MinHeight);
	scrollbarBoxBtn->SetMaxY(MaxHeight);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);
	scrollbarBoxBtn->Held.connect(this, &GuiScrollbar::OnBoxButtonHold);
}

GuiScrollbar::~GuiScrollbar()
{
	delete scrollbarTop;
	delete scrollbarBottom;
	delete scrollbarTile;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;
	delete oneButtonScrollImg;

	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete scrollbarTopImg;
	delete scrollbarBottomImg;
	delete scrollbarTileImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;
	delete oneButtonScrollImgData;

	delete trigHeldA;
}

void GuiScrollbar::ScrollOneUp()
{
	if(Mode == ICONMODE)
	{
		if(SelInd+SelItem-RowSize >= 0)
		{
			SelItem = SelItem-RowSize;
			if(SelItem < 0)
			{
				// move list up by 1
				SelInd = SelInd-RowSize;
				SelItem = SelItem+RowSize;
			}
		}
	}
	else if(Mode == LISTMODE)
	{
		if(SelItem == 0 && SelInd > 0)
		{
			// move list up by 1
			--SelInd;
		}
		else if(SelInd+SelItem > 0)
		{
			--SelItem;
		}
	}
}

void GuiScrollbar::ScrollOneDown()
{
	if(Mode == ICONMODE)
	{
		int i = RowSize;
		while(SelInd+SelItem+RowSize >= EntrieCount && i > 0 && SelItem > 0 && RowSize < EntrieCount)
		{
			--i;
			--SelItem;
		}
		if(SelInd+SelItem+RowSize < EntrieCount)
		{
			SelItem = SelItem+RowSize;
			if(SelItem >= PageSize)
			{
				// move list down by 1
				SelInd += RowSize;
				SelItem = SelItem-RowSize;
			}
		}
	}
	else if(Mode == LISTMODE)
	{
		if(SelInd+SelItem + 1 < EntrieCount)
		{
			if(SelItem == PageSize-1)
			{
				// move list down by 1
				SelInd++;
			}
			else
			{
				SelItem++;
			}
		}
	}
}

void GuiScrollbar::OnUpButtonHold(GuiButton *sender, int pointer, const POINT &p)
{
	if(ScrollState < ScrollSpeed)
		return;

	ScrollOneUp();

	ScrollState = 0;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::OnDownButtonHold(GuiButton *sender, int pointer, const POINT &p)
{
	if(ScrollState < ScrollSpeed)
		return;

	ScrollOneDown();

	ScrollState = 0;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::OnBoxButtonHold(GuiButton *sender, int pointer, const POINT &p)
{
	if(ScrollState < ScrollSpeed)
		return;

	if(!userInput[pointer].wpad.ir.valid)
		return;

	int y = p.y-this->GetTop()-scrollbarBox->GetHeight()/2;

	int positionWiimote = LIMIT(y-MinHeight, 0, MaxHeight-MinHeight);

	int newSelected = (int) ((float) positionWiimote / (float) (MaxHeight-MinHeight) * (float) (EntrieCount-1));

	if(Mode == ICONMODE)
	{
		int rows = (int) floor(((float) (newSelected-SelInd-SelItem)) / ((float) RowSize));

		while(SelInd+rows*RowSize >= EntrieCount-PageSize+RowSize)
			rows--;

		int pageIndex = LIMIT(SelInd+rows*RowSize, 0, EntrieCount-1-RowSize);

		if(newSelected <= 0)
			SelItem = 0;
		else if(newSelected >= EntrieCount-1)
			SelItem = EntrieCount-1-pageIndex;

		SelInd = pageIndex;
	}
	else if(Mode == LISTMODE)
	{
		int diff = newSelected-SelInd-SelItem;

		if(newSelected <= 0)
		{
			SelItem = 0;
			SelInd = 0;
		}
		else if(newSelected >= EntrieCount-1)
		{
			SelItem = (PageSize-1 < EntrieCount-1) ? PageSize-1 : EntrieCount-1;
			SelInd = EntrieCount-PageSize;
		}
		else if(newSelected < PageSize && SelInd == 0 && diff < 0)
		{
			SelItem = MAX(SelItem+diff, 0);
		}
		else if(EntrieCount-newSelected < PageSize && SelInd == EntrieCount-PageSize && diff > 0)
		{
			SelItem = MIN(SelItem+diff, PageSize-1);
		}
		else
		{
			SelInd = LIMIT(SelInd+diff, 0, ((EntrieCount-PageSize < 0) ? 0 : EntrieCount-PageSize));
		}
	}

	ScrollState = 0;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::SetPageSize(int size)
{
	if(PageSize == size)
		return;

	PageSize = size;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::SetRowSize(int size)
{
	if(RowSize == size)
		return;

	RowSize = size;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::SetSelectedItem(int pos)
{
	if(SelItem == pos)
		return;

	SelItem = pos;
}

void GuiScrollbar::SetSelectedIndex(int pos)
{
	if(SelInd == pos)
		return;

	SelInd = pos;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::SetEntrieCount(int cnt)
{
	if(EntrieCount == cnt)
		return;

	EntrieCount = cnt;
	listChanged(SelItem, SelInd);
}

void GuiScrollbar::setScrollboxPosition(int SelItem, int SelInd)
{
	if(Mode == ICONMODE)
	{
		u8 row = (u8) floor((float) SelItem / (float) RowSize);

		int position = MinHeight+(MaxHeight-MinHeight)*(SelInd+row*RowSize)/(EntrieCount-1);

		if(position < MinHeight)
			position = MinHeight;
		else if(position > MaxHeight || ((SelInd+PageSize >= (EntrieCount-1)) && row > 1))
			position = MaxHeight;

		scrollbarBoxBtn->SetPosition(ButtonPositionX, position);
	}
	else if(Mode == LISTMODE)
	{
		int position = MinHeight+(MaxHeight-MinHeight)*(SelInd+SelItem)/(EntrieCount-1);

		if(position < MinHeight)
			position = MinHeight;
		else if(position > MaxHeight || (SelInd+SelItem >= EntrieCount-1))
			position = MaxHeight;

		scrollbarBoxBtn->SetPosition(ButtonPositionX, position);
	}
}

void GuiScrollbar::CheckDPadControls(GuiTrigger *t)
{
	if(t->Up())
	{
		ScrollOneUp();
		listChanged(SelItem, SelInd);
		MovePointer = true;
	}
	else if(t->Down())
	{
		ScrollOneDown();
		listChanged(SelItem, SelInd);
		MovePointer = true;
	}
	else if(t->Left() && Mode == LISTMODE)
	{
		SelInd -= PageSize;
		if(SelInd < 0)
		{
			SelInd = 0;
			SelItem = 0;
		}
		listChanged(SelItem, SelInd);
		MovePointer = true;
	}
	else if(t->Right() && Mode == LISTMODE)
	{
		SelInd += PageSize;
		if(SelInd+PageSize >= EntrieCount)
		{
			SelInd = MAX(EntrieCount-PageSize, 0);
			SelItem = LIMIT(PageSize-1, 0, EntrieCount-1);
		}
		listChanged(SelItem, SelInd);
		MovePointer = true;
	}
	
	if(MovePointer)
	{
		int selHeight = (arrowDownBtn->GetTop()+arrowDownBtn->GetHeight()-arrowUpBtn->GetTop())/PageSize;
		int position = arrowUpBtn->GetTop()+(selHeight*SelItem)+(selHeight/2);
		for (int i = 3; i >= 0; i--)
			pointer[i]->SetPosition(scrollbarBoxBtn->GetLeft()-22, position, 0);
		MovePointer = false;
	}
}

void GuiScrollbar::ScrollByButton(GuiTrigger *t)
{
	static int pressedPosition = -1;

	if(!t->wpad.ir.valid || ScrollState < ButtonScrollSpeed-ButtonScrollSpeed*fabs(pressedPosition-t->wpad.ir.y)/250.f)
		return;

	if(pressedChan == -1 && (t->wpad.btns_d & ButtonScroll) &&
	   parentElement && parentElement->IsInside(t->wpad.ir.x, t->wpad.ir.y))
	{
		pressedPosition = t->wpad.ir.y;
		pressedChan = t->chan;
		oneButtonScrollImg->SetPosition(t->wpad.ir.x-oneButtonScrollImg->GetWidth()/2, t->wpad.ir.y-oneButtonScrollImg->GetHeight()/2);
	}

	if(pressedChan == t->chan && (t->wpad.btns_h & ButtonScroll) )
	{
		if(pressedPosition-oneButtonScrollImg->GetHeight()/2 > t->wpad.ir.y)
			ScrollOneUp();
		else if(pressedPosition+oneButtonScrollImg->GetHeight()/2 < t->wpad.ir.y)
			ScrollOneDown();

		ScrollState = 0;
		listChanged(SelItem, SelInd);
	}

	if(pressedChan == t->chan && !t->wpad.btns_d && !t->wpad.btns_h)
	{
		pressedChan = -1;
		pressedPosition = -1;
	}
}

void GuiScrollbar::Draw()
{
	if(PageSize <= EntrieCount)
	{
		scrollbarTileImg->Draw();
		scrollbarTopImg->Draw();
		scrollbarBottomImg->Draw();
		arrowUpBtn->Draw();
		arrowDownBtn->Draw();
		scrollbarBoxBtn->Draw();
	}

	if(pressedChan >= 0 && userInput[pressedChan].wpad.ir.valid)
		oneButtonScrollImg->Draw();

	UpdateEffects();
}

void GuiScrollbar::Update(GuiTrigger * t)
{
	if(PageSize <= EntrieCount)
	{
		arrowUpBtn->Update(t);
		arrowDownBtn->Update(t);
		scrollbarBoxBtn->Update(t);
	}

	if(AllowDPad)
		CheckDPadControls(t);
	if(ButtonScroll)
		ScrollByButton(t);

	++ScrollState;
}
