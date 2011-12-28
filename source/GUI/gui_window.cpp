/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_window.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

GuiWindow::GuiWindow()
{
	width = 0;
	height = 0;
	forceDim = false;
	allowDim = true;
}

GuiWindow::GuiWindow(int w, int h)
{
	width = w;
	height = h;
	forceDim = false;
	allowDim = true;
}

GuiWindow::~GuiWindow()
{
}

void GuiWindow::Append(GuiElement* e)
{
	LOCK( this );
	if (e == NULL) return;

	Remove(e);
	_elements.push_back(e);
	e->SetParent(this);
}

void GuiWindow::Insert(GuiElement* e, u32 index)
{
	LOCK( this );
	if (e == NULL || index > (_elements.size() - 1)) return;

	Remove(e);
	_elements.insert(_elements.begin() + index, e);
	e->SetParent(this);
}

void GuiWindow::Remove(GuiElement* e)
{
	LOCK( this );
	if (e == NULL) return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		if (e == _elements.at(i))
		{
			_elements.erase(_elements.begin() + i);
			break;
		}
	}
}

void GuiWindow::RemoveAll()
{
	LOCK( this );
	_elements.clear();
}

GuiElement* GuiWindow::GetGuiElementAt(u32 index) const
{
	if (index >= _elements.size()) return NULL;
	return _elements.at(index);
}

u32 GuiWindow::GetSize()
{
	return _elements.size();
}

void GuiWindow::Draw()
{
	LOCK( this );
	if (_elements.size() == 0 || !this->IsVisible()) return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			_elements.at(i)->Draw();
		}
		catch (const std::exception& e)
		{
		}
	}

	this->UpdateEffects();

	if ((parentElement && state == STATE_DISABLED && allowDim) || forceDim)
		Menu_DrawRectangle(0, 0, screenwidth, screenheight, (GXColor) {0, 0, 0, 0x70}, 1);
}
void GuiWindow::DrawTooltip()
{
	LOCK( this );
	if (_elements.size() == 0 || !this->IsVisible()) return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			_elements.at(i)->DrawTooltip();
		}
		catch (const std::exception& e)
		{
		}
	}
}
void GuiWindow::ResetState()
{
	LOCK( this );
	if (state != STATE_DISABLED) state = STATE_DEFAULT;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			_elements.at(i)->ResetState();
		}
		catch (const std::exception& e)
		{
		}
	}
}

void GuiWindow::SetState(int s)
{
	LOCK( this );
	state = s;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			_elements.at(i)->SetState(s);
		}
		catch (const std::exception& e)
		{
		}
	}
}

void GuiWindow::SetVisible(bool v)
{
	LOCK( this );
	visible = v;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			_elements.at(i)->SetVisible(v);
		}
		catch (const std::exception& e)
		{
		}
	}
}

int GuiWindow::GetSelected()
{
	// find selected element
	int found = -1;
	for (u8 i = 0; i < _elements.size(); i++)
	{
		try
		{
			if (_elements.at(i)->GetState() == STATE_SELECTED)
			{
				found = i;
				break;
			}
		}
		catch (const std::exception& e)
		{
		}
	}
	return found;
}

// set element to left/right as selected
// there's probably a more clever way to do this, but this way works
void GuiWindow::MoveSelectionHor(int dir)
{
	LOCK( this );
	int found = -1;
	u16 left = 0;
	u16 top = 0;
	u8 i = 0;

	int selected = this->GetSelected();

	if (selected >= 0)
	{
		left = _elements.at(selected)->GetLeft();
		top = _elements.at(selected)->GetTop();
	}

	// look for a button on the same row, to the left/right
	for (i = 0; i < _elements.size(); i++)
	{
		try
		{
			if (_elements.at(i)->IsSelectable())
			{
				if (_elements.at(i)->GetLeft() * dir > left * dir && _elements.at(i)->GetTop() == top)
				{
					if (found == -1)
						found = i;
					else if (_elements.at(i)->GetLeft() * dir < _elements.at(found)->GetLeft() * dir) found = i; // this is a better match
				}
			}
		}
		catch (const std::exception& e)
		{
		}
	}
	if (found >= 0) goto matchfound;

	// match still not found, let's try the first button in the next row
	for (i = 0; i < _elements.size(); i++)
	{
		try
		{
			if (_elements.at(i)->IsSelectable())
			{
				if (_elements.at(i)->GetTop() * dir > top * dir)
				{
					if (found == -1)
						found = i;
					else if (_elements.at(i)->GetTop() * dir < _elements.at(found)->GetTop() * dir)
						found = i; // this is a better match
					else if (_elements.at(i)->GetTop() * dir == _elements.at(found)->GetTop() * dir
							&& _elements.at(i)->GetLeft() * dir < _elements.at(found)->GetLeft() * dir) found = i; // this is a better match
				}
			}
		}
		catch (const std::exception& e)
		{
		}
	}

	// match found
	matchfound: if (found >= 0)
	{
		_elements.at(found)->SetState(STATE_SELECTED);
		if (selected >= 0) _elements.at(selected)->ResetState();
	}
}

void GuiWindow::MoveSelectionVert(int dir)
{
	LOCK( this );
	int found = -1;
	u16 left = 0;
	u16 top = 0;
	u8 i = 0;

	int selected = this->GetSelected();

	if (selected >= 0)
	{
		left = _elements.at(selected)->GetLeft();
		top = _elements.at(selected)->GetTop();
	}

	// look for a button above/below, with the least horizontal difference
	for (i = 0; i < _elements.size(); i++)
	{
		try
		{
			if (_elements.at(i)->IsSelectable())
			{
				if (_elements.at(i)->GetTop() * dir > top * dir)
				{
					if (found == -1)
						found = i;
					else if (_elements.at(i)->GetTop() * dir < _elements.at(found)->GetTop() * dir)
						found = i; // this is a better match
					else if (_elements.at(i)->GetTop() * dir == _elements.at(found)->GetTop() * dir && abs(
							_elements.at(i)->GetLeft() - left) < abs(_elements.at(found)->GetLeft() - left)) found = i;
				}
			}
		}
		catch (const std::exception& e)
		{
		}
	}
	if (found >= 0) goto matchfound;

	// match found
	matchfound: if (found >= 0)
	{
		_elements.at(found)->SetState(STATE_SELECTED);
		if (selected >= 0) _elements.at(selected)->ResetState();
	}
}

void GuiWindow::Update(GuiTrigger * t)
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

	if (updateCB) updateCB(this);
}
