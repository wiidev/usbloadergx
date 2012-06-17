/****************************************************************************
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
#include "CategorySwitchPrompt.hpp"
#include "settings/CGameCategories.hpp"
#include "settings/CSettings.h"
#include "language/gettext.h"

CategorySwitchPrompt::CategorySwitchPrompt()
	: CategoryPrompt(tr("Show Categories")), oldSettingEnabled(Settings.EnabledCategories),
	oldSettingRequired(Settings.RequiredCategories), oldSettingForbidden(Settings.ForbiddenCategories)
{
	browser->checkBoxClicked.connect(this, &CategorySwitchPrompt::OnCheckboxClick);
	browserRefresh.connect(this, &CategorySwitchPrompt::onBrowserRefresh);
	resetChanges.connect(this, &CategorySwitchPrompt::onResetChanges);

	browserRefresh();
}

void CategorySwitchPrompt::onResetChanges()
{
	Settings.EnabledCategories = oldSettingEnabled;
	Settings.RequiredCategories = oldSettingRequired;
	Settings.ForbiddenCategories = oldSettingForbidden;
	GameCategories.Load(Settings.ConfigPath);
}

void CategorySwitchPrompt::onBrowserRefresh()
{
	browser->Clear();
	GameCategories.CategoryList.goToFirst();
	do
	{
		bool checked = false;
		int style = GuiCheckbox::CHECKSIGN;

		// Verify the Enabled Categories [v]
		for(u32 i = 0; i < Settings.EnabledCategories.size(); ++i)
		{
			if(Settings.EnabledCategories[i] == GameCategories.CategoryList.getCurrentID())
			{
				checked = true;
				break;
			}
		}
		
		// Verify the Forbidden Categories [X]
		if(!checked)
		{
			for(u32 i = 0; i < Settings.ForbiddenCategories.size(); ++i)
			{
				if(Settings.ForbiddenCategories[i] == GameCategories.CategoryList.getCurrentID())
				{
					checked = true;
					style = GuiCheckbox::CROSS;
					break;
				}
			}
		}
		
		// Verify the Required Categories [+]
		if(!checked)
		{
			for(u32 i = 0; i < Settings.RequiredCategories.size(); ++i)
			{
				if(Settings.RequiredCategories[i] == GameCategories.CategoryList.getCurrentID())
				{
					checked = true;
					style = GuiCheckbox::PLUS;
					break;
				}
			}
		}
		
		browser->AddEntrie(tr(GameCategories.CategoryList.getCurrentName().c_str()), checked, style, true);
	}
	while(GameCategories.CategoryList.goToNext());

	GameCategories.CategoryList.goToFirst();
	browser->RefreshList();
}

void CategorySwitchPrompt::OnCheckboxClick(GuiCheckbox *checkBox, int index)
{
	GameCategories.CategoryList.goToFirst();
	for(int i = 0; i < index; ++i)
		GameCategories.CategoryList.goToNext();

	u32 i;
	if(!checkBox->IsChecked())
	{
		// Remove from Required
		for(i = 0; i < Settings.RequiredCategories.size(); ++i)
		{
			if(Settings.RequiredCategories[i] == GameCategories.CategoryList.getCurrentID())
			{
				Settings.RequiredCategories.erase(Settings.RequiredCategories.begin()+i);
				markChanged();
				break;
			}
		}
	}
	else if(checkBox->GetStyle() == GuiCheckbox::CHECKSIGN)
	{
		// Add to Enabled
		Settings.EnabledCategories.push_back(GameCategories.CategoryList.getCurrentID());
		markChanged();
	}
	else if(checkBox->GetStyle() == GuiCheckbox::CROSS)
	{
		// Remove from Enabled
		for(i = 0; i < Settings.EnabledCategories.size(); ++i)
		{
			if(Settings.EnabledCategories[i] == GameCategories.CategoryList.getCurrentID())
			{
				Settings.EnabledCategories.erase(Settings.EnabledCategories.begin()+i);
				break;
			}
		}
		// Add to Forbidden
		Settings.ForbiddenCategories.push_back(GameCategories.CategoryList.getCurrentID());
		markChanged();
	}
	else if(checkBox->GetStyle() == GuiCheckbox::PLUS && index > 0)
	{
		// Remove from Forbidden
		for(i = 0; i < Settings.ForbiddenCategories.size(); ++i)
		{
			if(Settings.ForbiddenCategories[i] == GameCategories.CategoryList.getCurrentID())
			{
				Settings.ForbiddenCategories.erase(Settings.ForbiddenCategories.begin()+i);
				break;
			}
		}
		// Add to Required
		Settings.RequiredCategories.push_back(GameCategories.CategoryList.getCurrentID());
		markChanged();
	}

	// Override Style cycling for category "All"
	if(index == 0 && checkBox->GetStyle() == GuiCheckbox::PLUS)
	{
		checkBox->SetStyle(GuiCheckbox::CHECKSIGN);
		checkBox->SetChecked(false);
		for(i = 0; i < Settings.ForbiddenCategories.size(); ++i)
		{
			if(Settings.ForbiddenCategories[i] == GameCategories.CategoryList.getCurrentID())
			{
				Settings.ForbiddenCategories.erase(Settings.ForbiddenCategories.begin()+i);
				markChanged();
				break;
			}
		}
	}
}
