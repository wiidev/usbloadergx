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
#include "Wiinnertag.h"
#include "FileOperations/fileops.h"
#include "settings/CSettings.h"
#include "network/networkops.h"
#include "utils/StringTools.h"
#include "xml/tinyxml2.h"
#include "gecko.h"

using namespace tinyxml2;

Wiinnertag::Wiinnertag(const string &filepath)
{
	ReadXML(filepath);
}

bool Wiinnertag::ReadXML(const string &filepath)
{
	XMLDocument xmlDoc; 
	if(xmlDoc.LoadFile(filepath.c_str()) != 0)
		return false;

	XMLElement * node =  xmlDoc.FirstChildElement("Tag");

	while(node != NULL)
	{
		const char * URL = node->Attribute("URL");
		const char * Key = node->Attribute("Key");

		if(URL && Key)
		{
			int size = tagList.size();
			tagList.resize(size+1);
			tagList[size].first = URL;
			tagList[size].second = Key;
		}

		node = node->NextSiblingElement();
	}

	return true;
}

bool Wiinnertag::Send(const char *gameID)
{
	if(!IsNetworkInit())
		return false;

	char sendURL[1024];

	for(u32 i = 0; i < tagList.size(); ++i)
	{
		strcpy(sendURL, tagList[i].first.c_str());

		replaceString(sendURL, "{ID6}", gameID);
		replaceString(sendURL, "{KEY}", tagList[i].second.c_str());

		download_request(sendURL);
		CloseConnection();
	}

	return true;
}

bool Wiinnertag::TagGame(const char *gameID)
{
	string fullpath = Settings.WiinnertagPath;
	if(fullpath.size() == 0)
		return false;

	if(fullpath[fullpath.size()-1] != '/')
		fullpath += '/';
	fullpath += "Wiinnertag.xml";

	Wiinnertag Tag(fullpath);
	return Tag.Send(gameID);
}

bool Wiinnertag::CreateExample(const string &filepath)
{
	if(filepath.size() == 0)
		return false;

	CreateSubfolder(filepath.c_str());

	string fullpath = filepath;
	if(fullpath[fullpath.size()-1] != '/')
		fullpath += '/';
	fullpath += "Wiinnertag.xml";

	XMLDocument xmlDoc;

	XMLDeclaration * declaration = xmlDoc.NewDeclaration();
	xmlDoc.InsertEndChild(declaration);

	XMLElement *Tag = xmlDoc.NewElement("Tag");
	Tag->SetAttribute("URL", "http://www.wiinnertag.com/wiinnertag_scripts/update_sign.php?key={KEY}&game_id={ID6}");
	Tag->SetAttribute("Key", "1234567890");
	xmlDoc.InsertEndChild(Tag);

	xmlDoc.SaveFile(fullpath.c_str());

	return true;
}
