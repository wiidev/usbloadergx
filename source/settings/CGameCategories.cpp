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
#include "xml/WiiTDB.hpp"
#include "utils/StringTools.h"
#include "svnrev.h"

#define VALID_CONFIG_REV    1084

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

    TiXmlDocument xmlDoc(filepath.c_str());
    if(!xmlDoc.LoadFile())
    	return false;

	if(!ValidVersion(xmlDoc.FirstChildElement("Revision")))
		return false;

	TiXmlElement * node =  xmlDoc.FirstChildElement("Categories");
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

    	TiXmlElement * category = node->FirstChildElement("Category");

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

    TiXmlDocument xmlDoc;

    TiXmlDeclaration declaration("1.0", "UTF-8", "");
    xmlDoc.InsertEndChild(declaration);

    TiXmlElement Revision("Revision");
    TiXmlText revText(GetRev());
    Revision.InsertEndChild(revText);
    xmlDoc.InsertEndChild(Revision);

	//! Add all categories as an ID map
	{
		TiXmlElement Categories("Categories");

		CategoryList.goToFirst();
		do
		{
			TiXmlElement Category("Category");
			Category.SetAttribute("ID", fmt("%02i", CategoryList.getCurrentID()));
			Category.SetAttribute("Name", CategoryList.getCurrentName().c_str());

			Categories.InsertEndChild(Category);
		}
		while(CategoryList.goToNext());

		xmlDoc.InsertEndChild(Categories);
	}

	//! Add game specific categories now
	{
		TiXmlElement GameCategories("GameCategories");

		for(map<string, vector<unsigned int> >::iterator itr = List.begin(); itr != List.end(); itr++)
		{
			TiXmlElement Game("Game");
			Game.SetAttribute("ID", itr->first.c_str());
			Game.SetAttribute("Title", GameTitles.GetTitle(itr->first.c_str()));

			for(u32 i = 0; i < itr->second.size(); ++i)
			{
				TiXmlElement Category("Category");
				Category.SetAttribute("ID", fmt("%02i", itr->second[i]));
				Category.SetAttribute("Name", CategoryList[itr->second[i]]);

				Game.InsertEndChild(Category);
			}

			GameCategories.InsertEndChild(Game);
		}

		xmlDoc.InsertEndChild(GameCategories);
	}

	xmlDoc.SaveFile(configPath);

    return true;
}

bool CGameCategories::ValidVersion(TiXmlElement *revisionNode)
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
                itr->second.erase(itr->second.begin()+ i);
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
    for(int i = 0; i < 6; ++i)
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
    for(int i = 0; i < 6; ++i)
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

bool CGameCategories::ImportFromWiiTDB(const string &xmlpath)
{
    WiiTDB XML_DB;

    if(!XML_DB.OpenFile(xmlpath.c_str()))
        return false;

    XML_DB.SetLanguageCode(Settings.db_language);
    wString filter(gameList.GetCurrentFilter());
    gameList.LoadUnfiltered();

    for(int i = 0; i < gameList.size(); ++i)
    {
        vector<string> genreList;

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

	return true;
}
