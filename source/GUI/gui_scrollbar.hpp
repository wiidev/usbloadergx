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
#ifndef GUISCROLLBAR_HPP_
#define GUISCROLLBAR_HPP_

#include "gui.h"

class GuiScrollbar : public GuiElement, public sigslot::has_slots<>
{
	public:
		GuiScrollbar(int height, u8 mode = LISTMODE);
		virtual ~GuiScrollbar();
		void SetDPadControl(bool a) { AllowDPad = a; }
		void SetButtonScroll(u32 button) { ButtonScroll = button; }
		void ScrollOneUp();
		void ScrollOneDown();
		int GetSelectedItem() { return SelItem; };
		int GetSelectedIndex() { return SelInd; };
		void SetScrollSpeed(u16 speed) { ScrollSpeed = speed; };
		void SetButtonScrollSpeed(u16 speed) { ButtonScrollSpeed = speed; };
		void Draw();
		void Update(GuiTrigger * t);
		enum
		{
			ICONMODE = 0,
			LISTMODE,
		};
		//! Signals
		sigslot::signal2<int, int> listChanged;
		//! Slots
		void SetPageSize(int size);
		void SetRowSize(int size);
		void SetSelectedItem(int pos);
		void SetSelectedIndex(int pos);
		void SetEntrieCount(int cnt);
	protected:
		void setScrollboxPosition(int SelItem, int SelInd);
		void OnUpButtonHold(GuiButton *sender, int pointer, const POINT &p);
		void OnDownButtonHold(GuiButton *sender, int pointer, const POINT &p);
		void OnBoxButtonHold(GuiButton *sender, int pointer, const POINT &p);
		void CheckDPadControls(GuiTrigger *t);
		void ScrollByButton(GuiTrigger *t);

		u8 Mode;
		u32 ScrollState;
		u16 ScrollSpeed;
		u16 ButtonScrollSpeed;
		u32 ButtonScroll;
		bool AllowDPad;
		bool MovePointer;

		int MinHeight;
		int MaxHeight;
		int SelItem;
		int SelInd;
		int RowSize;
		int PageSize;
		int EntrieCount;
		int ButtonPositionX;
		int pressedChan;
		bool listchanged;

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;
		GuiImage * scrollbarTopImg;
		GuiImage * scrollbarBottomImg;
		GuiImage * scrollbarTileImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowDownOverImg;
		GuiImage * arrowUpImg;
		GuiImage * arrowUpOverImg;
		GuiImage * scrollbarBoxImg;
		GuiImage * scrollbarBoxOverImg;
		GuiImage * oneButtonScrollImg;
		GuiImageData * scrollbarTop;
		GuiImageData * scrollbarBottom;
		GuiImageData * scrollbarTile;
		GuiImageData * arrowDown;
		GuiImageData * arrowDownOver;
		GuiImageData * arrowUp;
		GuiImageData * arrowUpOver;
		GuiImageData * scrollbarBox;
		GuiImageData * scrollbarBoxOver;
		GuiImageData * oneButtonScrollImgData;
		GuiTrigger * trigHeldA;
};

#endif
