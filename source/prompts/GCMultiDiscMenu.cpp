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
#include <unistd.h>
#include "GCMultiDiscMenu.h"
#include "GameCube/GCGames.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "language/gettext.h"
#include "themes/gettheme.h"
#include "themes/Resources.h"
#include "menu/menus.h"

GCMultiDiscMenu::GCMultiDiscMenu(const std::vector<struct discHdr> &List)
	: gcGameList(List)
{
	titleTxt->SetText(tr("Game Cube Install Menu"));
	button1Txt->SetText(tr("Install"));
	browserRefresh();
}

void GCMultiDiscMenu::browserRefresh(void)
{
	browser->Clear();

	for(u32 i = 0; i < gcGameList.size(); ++i)
	{
		browser->AddEntrie(gcGameList[i].title);
	}
}

int GCMultiDiscMenu::ShowSelection()
{
	while(true)
	{
		usleep(10000);

		if (shutdown)
			Sys_Shutdown();
		else if (reset)
			Sys_Reboot();

		else if(backBtn->GetState() == STATE_CLICKED)
		{
			if(!changed || WindowPrompt(tr("Do you want to discard changes?"), 0, tr("Yes"), tr("No")))
				break;

			backBtn->ResetState();
		}

		else if (homeButton->GetState() == STATE_CLICKED)
		{
			gprintf("\thomeButton clicked\n");
			WindowExitPrompt();
			mainWindow->SetState(STATE_DISABLED);
			SetState(STATE_DEFAULT);
			homeButton->ResetState();
		}

		else if(button1->GetState() == STATE_CLICKED)
		{
			if(!changed)
			{
				WindowPrompt(tr("Error:"), tr("Nothing selected to install."), tr("OK"));
			}
			else if(WindowPrompt(tr("Do you want to install selected games?"), 0, tr("Yes"), tr("Cancel")))
			{
				return 1;
			}

			button1->ResetState();
		}
	}

	return 0;
}

std::vector<u32> GCMultiDiscMenu::GetSelectedGames(void)
{
	std::vector<u32> selectedGames;

	for(u32 i = 0; i < gcGameList.size(); ++i)
	{
		if(browser->IsChecked(i))
		{
			selectedGames.push_back(i);
		}
	}
	return selectedGames;
}
