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
#include "GCDeleteMenu.h"
#include "GameCube/GCGames.h"
#include "settings/CSettings.h"
#include "settings/GameTitles.h"
#include "language/gettext.h"
#include "themes/gettheme.h"
#include "themes/Resources.h"
#include "menu/menus.h"

GCDeleteMenu::GCDeleteMenu(void)
{
	titleTxt->SetText(tr("Game Cube Games Delete"));
	button1Txt->SetText(tr("Delete"));
	browserRefresh();
}

GCDeleteMenu::~GCDeleteMenu()
{
	for(u32 i = 0; i < sizeTxtList.size(); ++i)
		delete sizeTxtList[i];
}

void GCDeleteMenu::browserRefresh(void)
{
	browser->Clear();
	browser->SetMaxTextWidth(200);

	for(u32 i = 0; i < sizeTxtList.size(); ++i)
		delete sizeTxtList[i];

	sizeTxtList.resize(GCGames::Instance()->GetSDHeaders().size());

	for(u32 i = 0; i < GCGames::Instance()->GetSDHeaders().size(); ++i)
	{
		const struct discHdr *gcDiscHdr = &GCGames::Instance()->GetSDHeaders().at(i);
		float fSize = GCGames::Instance()->GetGameSize((char*)gcDiscHdr->id);

		browser->AddEntrie(GameTitles.GetTitle(gcDiscHdr));

		char size_text[20];
		snprintf(size_text, sizeof(size_text), "(%.2fGB)", fSize);

		sizeTxtList[i] = new GuiText(size_text, 18, thColor("r=0 g=0 b=0 a=255 - checkbox browser text color"));
		sizeTxtList[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
		sizeTxtList[i]->SetPosition(-40, 5);
		browser->GetCheckbox(i)->SetLabel(sizeTxtList[i]);
	}
}

int GCDeleteMenu::Show()
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
				WindowPrompt(tr("Error:"), tr("Nothing selected to delete."), tr("OK"));
			}
			else if(WindowPrompt(tr("Attention!"), tr("Are you really sure you want to delete all selected games from the SD card?"), tr("Yes"), tr("Cancel")))
			{
				DeleteSelectedGames();
				break;
			}

			button1->ResetState();
		}
	}

	return 0;
}

void GCDeleteMenu::DeleteSelectedGames(void)
{
	for(u32 i = 0; i < GCGames::Instance()->GetSDHeaders().size(); ++i)
	{
		if(browser->IsChecked(i))
		{
			GCGames::Instance()->RemoveSDGame((char*)GCGames::Instance()->GetSDHeaders().at(i).id);
		}
	}
	GCGames::Instance()->LoadAllGames();
}
