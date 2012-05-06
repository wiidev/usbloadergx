/****************************************************************************
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef GCDELETEMENU_H
#define GCDELETEMENU_H

#include "CheckboxBrowserMenu.h"

class GCDeleteMenu : public CheckboxBrowserMenu
{
	public:
		GCDeleteMenu();
		virtual ~GCDeleteMenu();
		int Show();
	private:
		void DeleteSelectedGames(void);
		void browserRefresh();
		std::vector<GuiText *> sizeTxtList;
};

#endif // GCDELETEMENU_H
