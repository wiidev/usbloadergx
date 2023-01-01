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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CGameCategories.hpp"
#include "GameTitles.h"
#include "settings/CSettings.h"
#include "usbloader/GameList.h"
#include "language/gettext.h"
#include "FileOperations/fileops.h"
#include "prompts/ProgressWindow.h"
#include "xml/GameTDB.hpp"
#include "utils/StringTools.h"
#include "svnrev.h"

#define VALID_CONFIG_REV 1084

CGameCategories GameCategories;

CGameCategories::CGameCategories()
	: defaultCategory(1, 0)
{
}

const std::vector<unsigned int> &CGameCategories::operator[](const char *id) const
{
	if (!id)
		return defaultCategory;

	for (std::map<std::string, std::vector<unsigned int>>::const_iterator itr = List.begin(); itr != List.end(); itr++)
	{
		if (strncasecmp(itr->first.c_str(), id, 6) == 0)
			return itr->second;
	}

	return defaultCategory;
}

bool CGameCategories::Load(std::string filepath)
{
	if (filepath.length() == 0)
		return false;

	if (filepath.back() != '/')
		filepath += '/';
	filepath += "GXGameCategories.xml";
	configPath = filepath;

	clear();

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result result = xmlDoc.load_file(filepath.c_str());
	if (!result)
		return false;

	pugi::xml_node root = xmlDoc.child("USBLoaderGX");
	if (!root)
		root = xmlDoc;

	if (!ValidVersion(root.child("Revision")))
		return false;

	pugi::xml_node categories = root.child("Categories");
	if (!categories)
		return false;

	for (pugi::xml_node category : categories.children("Category"))
	{
		const char *ID = category.attribute("ID").value();
		const char *Name = category.attribute("Name").value();

		if (ID && Name)
			CategoryList.SetCategory(atoi(ID), Name);
	}

	pugi::xml_node gamecategories = root.child("GameCategories");
	if (!gamecategories)
		return false;

	for (pugi::xml_node game : gamecategories.children("Game"))
	{
		const char *gameID = game.attribute("ID").value();

		for (pugi::xml_node category : game.children("Category"))
		{
			const char *categoryID = category.attribute("ID").value();
			if (gameID && categoryID)
				SetCategory(gameID, atoi(categoryID));
		}
	}

	CategoryList.goToFirst();

	return true;
}

bool CGameCategories::Save()
{
	char filepath[300];
	snprintf(filepath, sizeof(filepath), configPath.c_str());

	char *ptr = strrchr(filepath, '/');
	if (ptr)
		ptr[0] = 0;

	CreateSubfolder(filepath);

	StartProgress(tr("Generating GXGameCategories.xml"), tr("Please wait..."), 0, false, true);

	pugi::xml_document xmlDoc;
	pugi::xml_node declaration = xmlDoc.append_child(pugi::node_declaration);
	declaration.append_attribute("version") = "1.0";
	declaration.append_attribute("encoding") = "UTF-8";

	pugi::xml_node root = xmlDoc.append_child("USBLoaderGX");
	pugi::xml_node revision = root.append_child("Revision");
	revision.append_child(pugi::node_pcdata).set_value(GetRev());

	int progressSize = CategoryList.size() + List.size();
	int progress = 0;

	//! Add all categories as an ID map
	pugi::xml_node categories = root.append_child("Categories");

	CategoryList.goToFirst();
	do
	{
		ShowProgress(progress, progressSize);

		pugi::xml_node category = categories.append_child("Category");
		category.append_attribute("ID") = fmt("%02i", CategoryList.getCurrentID());
		category.append_attribute("Name") = CategoryList.getCurrentName().c_str();

		++progress;
	} while (CategoryList.goToNext());

	//! Add game specific categories now
	pugi::xml_node gamecategories = root.append_child("GameCategories");

	for (std::map<std::string, std::vector<unsigned int>>::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		ShowProgress(progress, progressSize);

		pugi::xml_node game = gamecategories.append_child("Game");
		game.append_attribute("ID") = itr->first.c_str();
		game.append_attribute("Title") = GameTitles.GetTitle(itr->first.c_str());

		for (u32 i = 0; i < itr->second.size(); ++i)
		{
			const char *CatName = CategoryList[itr->second[i]];
			if (!CatName)
				CatName = "";

			pugi::xml_node category = game.append_child("Category");
			category.append_attribute("ID") = fmt("%02i", itr->second[i]);
			category.append_attribute("Name") = CatName;
		}
		++progress;
	}

	ShowProgress(tr("Writing GXGameCategories.xml"), tr("Please wait..."), 0, progressSize, progressSize, false, true);

	xmlDoc.save_file(configPath.c_str());
	ProgressStop();
	return true;
}

bool CGameCategories::ValidVersion(pugi::xml_node revisionNode)
{
	if (!revisionNode)
		return false;

	if (!revisionNode.first_child() || !revisionNode.first_child().value())
		return false;

	return atoi(revisionNode.first_child().value()) >= VALID_CONFIG_REV;
}

bool CGameCategories::SetCategory(const char *gameID, unsigned int id)
{
	if (!gameID)
		return false;

	char gameID6[7];
	snprintf(gameID6, sizeof(gameID6), gameID);

	std::string stringGameID(gameID6);

	return SetCategory(stringGameID, id);
}

bool CGameCategories::SetCategory(const std::string &gameID, unsigned int id)
{
	if (List[gameID].empty())
		List[gameID] = defaultCategory;

	std::vector<unsigned int> tmpVect(List[gameID]);

	for (u32 i = 0; i < tmpVect.size(); ++i)
	{
		if (tmpVect[i] == id)
			return false;
	}

	List[gameID].push_back(id);
	return true;
}

bool CGameCategories::ReplaceCategory(const char *gameID, unsigned int id)
{
	if (!gameID)
		return false;

	char gameID6[7];
	snprintf(gameID6, sizeof(gameID6), gameID);

	List[std::string(gameID6)] = defaultCategory;
	List[std::string(gameID6)].push_back(id);
	return true;
}

bool CGameCategories::ReplaceCategory(const std::string &gameID, unsigned int id)
{
	List[gameID] = defaultCategory;
	List[gameID].push_back(id);
	return true;
}

void CGameCategories::RemoveCategory(unsigned int id)
{
	for (std::map<std::string, std::vector<unsigned int>>::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		for (u32 i = 0; i < itr->second.size(); ++i)
		{
			if (itr->second[i] == id)
			{
				itr->second.erase(itr->second.begin() + i);
				--i;
			}
		}
	}
}

void CGameCategories::RemoveGameCategories(const std::string &gameID)
{
	for (std::map<std::string, std::vector<unsigned int>>::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		if (gameID == itr->first)
			List.erase(itr);
	}
}

void CGameCategories::RemoveCategory(const char *gameID, unsigned int id)
{
	if (!gameID)
		return;

	std::string gameID6;
	for (int i = 0; i < 6 && gameID[i] != 0; ++i)
		gameID6.push_back(gameID[i]);

	RemoveCategory(gameID6, id);
}

void CGameCategories::RemoveCategory(const std::string &gameID, unsigned int id)
{
	for (std::map<std::string, std::vector<unsigned int>>::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		if (gameID == itr->first)
		{
			for (u32 i = 0; i < itr->second.size(); ++i)
			{
				if (itr->second[i] == id)
				{
					itr->second.erase(itr->second.begin() + i);
					break;
				}
			}
			break;
		}
	}
}

bool CGameCategories::isInCategory(const char *gameID, unsigned int id)
{
	if (id == 0) //! ID = 0 means category 'All' so it is always true
		return true;

	if (!gameID)
		return false;

	std::string gameID6;
	for (int i = 0; i < 6 && gameID[i] != 0; ++i)
		gameID6.push_back(gameID[i]);

	for (std::map<std::string, std::vector<unsigned int>>::iterator itr = GameCategories.List.begin(); itr != GameCategories.List.end(); itr++)
	{
		if (itr->first == gameID6)
		{
			for (u32 i = 0; i < itr->second.size(); ++i)
			{
				if (itr->second[i] == id)
					return true;
			}
			break;
		}
	}

	return false;
}

bool CGameCategories::ImportFromGameTDB(const std::string &xmlpath)
{
	GameTDB XML_DB;

	if (!XML_DB.OpenFile(xmlpath.c_str()))
		return false;

	StartProgress(tr("Importing categories"), tr("Please wait..."), 0, false, true);

	XML_DB.SetLanguageCode(Settings.db_language);

	std::vector<struct discHdr *> headerlist;
	if (!gameList.GetGameListHeaders(headerlist, MODE_ALL))
		return false;

	for (u32 i = 0; i < headerlist.size(); ++i)
	{
		ShowProgress(i, headerlist.size());

		std::vector<std::string> genreList;
		std::string GameType;

		if (XML_DB.GetGameType((const char *)headerlist[i]->id, GameType))
		{
			if (!CategoryList.findCategory(GameType))
				CategoryList.AddCategory(GameType);

			this->SetCategory(headerlist[i]->id, CategoryList.getCurrentID());
		}

		if (!XML_DB.GetGenreList((const char *)headerlist[i]->id, genreList))
			continue;

		for (u32 n = 0; n < genreList.size(); ++n)
		{
			if (!CategoryList.findCategory(genreList[n]))
				CategoryList.AddCategory(genreList[n]);

			this->SetCategory(headerlist[i]->id, CategoryList.getCurrentID());
		}
	}

	XML_DB.CloseFile();

	ProgressStop();

	return true;
}
