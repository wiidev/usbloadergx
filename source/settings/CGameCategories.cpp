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

using namespace tinyxml2;

#define VALID_CONFIG_REV	1084

CGameCategories GameCategories;

CGameCategories::CGameCategories()
	: defaultCategory(1, 0)
{
}

const vector<unsigned int> &CGameCategories::operator[](const char *id) const
{
	if(!id) return defaultCategory;

	for(map<string, vector<unsigned int> >::const_iterator itr = List.begin(); itr != List.end(); itr++)
	{
		if(strncasecmp(itr->first.c_str(), id, 6) == 0)
			return itr->second;
	}

	return defaultCategory;
}

bool CGameCategories::Load(string filepath)
{
	if(filepath.size() == 0)
		return false;

	if(filepath[filepath.size()-1] != '/')
		filepath += '/';

	filepath += "GXGameCategories.xml";
	configPath = filepath;

	clear();

	XMLDocument xmlDoc;
	if(xmlDoc.LoadFile(filepath.c_str()) != 0)
		return false;

	if(!ValidVersion(xmlDoc.FirstChildElement("Revision")))
		return false;

	XMLElement * node =  xmlDoc.FirstChildElement("Categories");
	if(!node)
		return false;

	node = node->FirstChildElement("Category");

	while(node != NULL)
	{
		const char * ID = node->Attribute("ID");
		const char * Name = node->Attribute("Name");

		if(ID && Name)
			CategoryList.SetCategory(atoi(ID), Name);

		node = node->NextSiblingElement();
	}

	node =  xmlDoc.FirstChildElement("GameCategories");
	if(!node)
		return false;

	node = node->FirstChildElement("Game");

	while(node != NULL)
	{
		const char * gameID = node->Attribute("ID");

		XMLElement * category = node->FirstChildElement("Category");

		while(category != NULL)
		{
			const char * categoryID = category->Attribute("ID");
			if(gameID && categoryID)
				SetCategory(gameID, atoi(categoryID));

			category = category->NextSiblingElement();
		}

		node = node->NextSiblingElement();
	}

	CategoryList.goToFirst();

	return true;
}

bool CGameCategories::Save()
{
	char filepath[300];
	snprintf(filepath, sizeof(filepath), configPath.c_str());

	char * ptr = strrchr(filepath, '/');
	if(ptr)
		ptr[0] = 0;

	CreateSubfolder(filepath);

	StartProgress(tr("Generating GXGameCategories.xml"), tr("Please wait..."), 0, false, true);
	XMLDocument xmlDoc;

	XMLDeclaration *declaration = xmlDoc.NewDeclaration();
	xmlDoc.InsertEndChild(declaration);
	XMLElement *Revision = xmlDoc.NewElement("Revision");
	XMLText *revText = xmlDoc.NewText(GetRev());
	Revision->InsertEndChild(revText);
	xmlDoc.InsertEndChild(Revision);

	int progressSize = CategoryList.size() + List.size();
	int progress = 0;

	//! Add all categories as an ID map
	{
		//! On LinkEndChild TinyXML owns and deletes the elements allocated here.
		//! This is more memory efficient than making another copy of the elements.
		XMLElement *Categories = xmlDoc.NewElement("Categories");

		CategoryList.goToFirst();
		do
		{
			ShowProgress(progress, progressSize);

			XMLElement *Category = xmlDoc.NewElement("Category");
			Category->SetAttribute("ID", fmt("%02i", CategoryList.getCurrentID()));
			Category->SetAttribute("Name", CategoryList.getCurrentName().c_str());

			Categories->LinkEndChild(Category);

			++progress;
		}
		while(CategoryList.goToNext());

		xmlDoc.LinkEndChild(Categories);
	}

	//! Add game specific categories now
	{
		//! On LinkEndChild TinyXML owns and deletes the elements allocated here.
		//! This is more memory efficient than making another copy of the elements.
		XMLElement *GameCategories = xmlDoc.NewElement("GameCategories");

		for(map<string, vector<unsigned int> >::iterator itr = List.begin(); itr != List.end(); itr++)
		{
			ShowProgress(progress, progressSize);

			XMLElement *Game = xmlDoc.NewElement("Game");
			Game->SetAttribute("ID", itr->first.c_str());
			Game->SetAttribute("Title", GameTitles.GetTitle(itr->first.c_str()));

			for(u32 i = 0; i < itr->second.size(); ++i)
			{
				const char *CatName = CategoryList[itr->second[i]];
				if(!CatName)
					CatName = "";

				XMLElement *Category = xmlDoc.NewElement("Category");
				Category->SetAttribute("ID", fmt("%02i", itr->second[i]));
				Category->SetAttribute("Name", CatName);

				Game->LinkEndChild(Category);
			}

			GameCategories->LinkEndChild(Game);
			++progress;
		}

		xmlDoc.LinkEndChild(GameCategories);
	}

	ShowProgress(tr("Writing GXGameCategories.xml"), tr("Please wait..."), 0, progressSize, progressSize, false, true);

	xmlDoc.SaveFile(configPath.c_str());
	ProgressStop();

	return true;
}

bool CGameCategories::ValidVersion(XMLElement *revisionNode)
{
	if(!revisionNode) return false;

	if(!revisionNode->FirstChild() || !revisionNode->FirstChild()->Value())
		return false;

	return atoi(revisionNode->FirstChild()->Value()) >= VALID_CONFIG_REV;
}

bool CGameCategories::SetCategory(const char *gameID, unsigned int id)
{
	if(!gameID) return false;

	char gameID6[7];
	snprintf(gameID6, sizeof(gameID6), gameID);

	string stringGameID(gameID6);

	return SetCategory(stringGameID, id);
}

bool CGameCategories::SetCategory(const string &gameID, unsigned int id)
{
	if(List[gameID].empty())
		List[gameID] = defaultCategory;

	vector<unsigned int> tmpVect(List[gameID]);

	for(u32 i = 0; i < tmpVect.size(); ++i)
	{
		if(tmpVect[i] == id)
			return false;
	}

	List[gameID].push_back(id);
	return true;
}

bool CGameCategories::ReplaceCategory(const char *gameID, unsigned int id)
{
	if(!gameID) return false;

	char gameID6[7];
	snprintf(gameID6, sizeof(gameID6), gameID);

	List[string(gameID6)] = defaultCategory;
	List[string(gameID6)].push_back(id);
	return true;
}


bool CGameCategories::ReplaceCategory(const string &gameID, unsigned int id)
{
	List[gameID] = defaultCategory;
	List[gameID].push_back(id);
	return true;
}

void CGameCategories::RemoveCategory(unsigned int id)
{
	for(map<string, vector<unsigned int> >::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		for(u32 i = 0; i < itr->second.size(); ++i)
		{
			if(itr->second[i] == id)
			{
				itr->second.erase(itr->second.begin()+ i);
				--i;
			}
		}
	}
}

void CGameCategories::RemoveGameCategories(const string &gameID)
{
	for (map<string, vector<unsigned int> >::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		if(gameID == itr->first)
		{
			List.erase(itr);
		}
	}
}

void CGameCategories::RemoveCategory(const char *gameID, unsigned int id)
{
	if(!gameID) return;

	string gameID6;
	for(int i = 0; i < 6 && gameID[i] != 0; ++i)
		gameID6.push_back(gameID[i]);

	RemoveCategory(gameID6, id);
}

void CGameCategories::RemoveCategory(const string &gameID, unsigned int id)
{
	for (map<string, vector<unsigned int> >::iterator itr = List.begin(); itr != List.end(); itr++)
	{
		if(gameID == itr->first)
		{
			for(u32 i = 0; i < itr->second.size(); ++i)
			{
				if(itr->second[i] == id)
				{
					itr->second.erase(itr->second.begin()+ i);
					break;
				}
			}
			break;
		}
	}
}

bool CGameCategories::isInCategory(const char *gameID, unsigned int id)
{
	if(id == 0) //! ID = 0 means category 'All' so it is always true
		return true;

	if(!gameID) return false;

	string gameID6;
	for(int i = 0; i < 6 && gameID[i] != 0; ++i)
		gameID6.push_back(gameID[i]);

	for (map<string, vector<unsigned int> >::iterator itr = GameCategories.List.begin(); itr != GameCategories.List.end(); itr++)
	{
		if(itr->first == gameID6)
		{
			for(u32 i = 0; i < itr->second.size(); ++i)
			{
				if(itr->second[i] == id)
					return true;
			}
			break;
		}
	}

	return false;
}

bool CGameCategories::ImportFromGameTDB(const string &xmlpath)
{
	GameTDB XML_DB;

	if(!XML_DB.OpenFile(xmlpath.c_str()))
		return false;

	StartProgress(tr("Importing categories"), tr("Please wait..."), 0, false, true);

	XML_DB.SetLanguageCode(Settings.db_language);
	wString filter(gameList.GetCurrentFilter());
	gameList.LoadUnfiltered();

	for(int i = 0; i < gameList.size(); ++i)
	{
		ShowProgress(i, gameList.size());

		vector<string> genreList;
		string GameType;

		if(XML_DB.GetGameType((const char *) gameList[i]->id, GameType))
		{
			if(!CategoryList.findCategory(GameType))
				CategoryList.AddCategory(GameType);

			this->SetCategory(gameList[i]->id, CategoryList.getCurrentID());
		}

		if(!XML_DB.GetGenreList((const char *) gameList[i]->id, genreList))
			continue;

		for(u32 n = 0; n < genreList.size(); ++n)
		{
			if(!CategoryList.findCategory(genreList[n]))
				CategoryList.AddCategory(genreList[n]);

			this->SetCategory(gameList[i]->id, CategoryList.getCurrentID());
		}

	}

	XML_DB.CloseFile();
	gameList.FilterList(filter.c_str());

	ProgressStop();

	return true;
}
