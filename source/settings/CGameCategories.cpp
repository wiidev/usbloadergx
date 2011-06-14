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
#include "xml/xml.h"
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

static const char * XMLSaveCallback(mxml_node_t *node, int where)
{
	const char *name = node->value.element.name;

	if (where == MXML_WS_BEFORE_OPEN)
	{
		if(!strcmp(name, "Revision") || !strcmp(name, "Categories") || !strcmp(name, "GameCategories"))
			return "\n";
		else if(!strcmp(name, "Game"))
			return "\n\t";
		else if(!strcmp(name, "Category"))
			return "\n\t\t";
	}
	else if(where == MXML_WS_BEFORE_CLOSE)
	{
		if(!strcmp(name, "Categories") || !strcmp(name, "GameCategories"))
			return "\n";
        else if(!strcmp(name, "Game"))
            return "\n\t";
	}
	return (NULL);
}

bool CGameCategories::Load(string filepath)
{
    if(filepath.size() == 0)
        return false;

    if(filepath[filepath.size()-1] != '/')
        filepath += '/';

    filepath += "GXGameCategories.xml";
    configPath = filepath;

    u8 *buffer = NULL;
    u64 filesize = 0;

    LoadFileToMem(filepath.c_str(), &buffer, &filesize);

    if(!buffer)
        return false;

    clear();

    mxml_node_t *xmlfile = mxmlLoadString(NULL, (const char *) buffer, MXML_OPAQUE_CALLBACK);

    if(!ValidVersion(xmlfile))
    {
        mxmlDelete(xmlfile);
        free(buffer);
        return false;
    }

    mxml_node_t *node = mxmlFindElement(xmlfile, xmlfile, "Categories", NULL, NULL, MXML_DESCEND_FIRST);
    if(!node)
    {
        mxmlDelete(xmlfile);
        free(buffer);
        return false;
    }

    node = mxmlFindElement(node, xmlfile, "Category", NULL, NULL, MXML_DESCEND_FIRST);

    while(node != NULL)
    {
		const char * ID = mxmlElementGetAttr(node, "ID");
		const char * Name = mxmlElementGetAttr(node, "Name");

		if(ID && Name)
            CategoryList.SetCategory(atoi(ID), Name);

        node = mxmlFindElement(node, xmlfile, "Category", NULL, NULL, MXML_NO_DESCEND);
    }

    node = mxmlFindElement(xmlfile, xmlfile, "GameCategories", NULL, NULL, MXML_DESCEND_FIRST);
    if(!node)
    {
        mxmlDelete(xmlfile);
        free(buffer);
        return false;
    }

    node = mxmlFindElement(node, xmlfile, "Game", NULL, NULL, MXML_DESCEND_FIRST);

    while(node != NULL)
    {
		const char * gameID = mxmlElementGetAttr(node, "ID");

		mxml_node_t *category = mxmlFindElement(node, xmlfile, "Category", NULL, NULL, MXML_DESCEND_FIRST);

        while(category != NULL)
        {
		    const char * categoryID = mxmlElementGetAttr(category, "ID");
		    if(gameID && categoryID)
		        SetCategory(gameID, atoi(categoryID));
		    category = mxmlFindElement(category, xmlfile, "Category", NULL, NULL, MXML_NO_DESCEND);
        }

        node = mxmlFindElement(node, xmlfile, "Game", NULL, NULL, MXML_NO_DESCEND);
    }

    mxmlDelete(xmlfile);
    free(buffer);

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

    FILE * f = fopen(configPath.c_str(), "wb");
    if(!f)
        return false;

    mxml_node_t *xmlfile = mxmlNewXML("1.0");
    mxmlSetWrapMargin(0);

    mxml_node_t	*node = mxmlNewElement(xmlfile, "Revision");
    mxmlNewInteger(node, atoi(GetRev()));

    node = mxmlNewElement(xmlfile, "Categories");

    CategoryList.goToFirst();
    do
    {
        mxml_node_t	*category = mxmlNewElement(node, "Category");
        mxmlElementSetAttrf(category, "ID", "%02u", CategoryList.getCurrentID());
        mxmlElementSetAttr(category, "Name", CategoryList.getCurrentName().c_str());
    }
    while(CategoryList.goToNext());

    node = mxmlNewElement(xmlfile, "GameCategories");

    for(map<string, vector<unsigned int> >::iterator itr = List.begin(); itr != List.end(); itr++)
    {
        mxml_node_t	*game = mxmlNewElement(node, "Game");
        mxmlElementSetAttr(game, "ID", itr->first.c_str());
        mxmlElementSetAttr(game, "Title", GameTitles.GetTitle(itr->first.c_str()));

        for(u32 i = 0; i < itr->second.size(); ++i)
        {
            mxml_node_t	*category = mxmlNewElement(game, "Category");
            mxmlElementSetAttrf(category, "ID", "%02u", itr->second[i]);
            mxmlElementSetAttr(category, "Name", CategoryList[itr->second[i]]);
        }
    }

    mxmlSaveFile(xmlfile, f, XMLSaveCallback);
    fclose(f);

    mxmlDelete(xmlfile);

    return true;
}

bool CGameCategories::ValidVersion(mxml_node_t *xmlfile)
{
    if(!xmlfile) return false;

    mxml_node_t *node = mxmlFindElement(xmlfile, xmlfile, "Revision", NULL, NULL, MXML_DESCEND_FIRST);
    if(!node || !node->child || !node->child->value.opaque)
        return false;

    return atoi(node->child->value.opaque) >= VALID_CONFIG_REV;
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

	return true;
}
