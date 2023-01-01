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
#include "network/https.h"
#include "utils/StringTools.h"
#include "xml/pugixml.hpp"
#include "gecko.h"

Wiinnertag::Wiinnertag(const std::string &filepath)
{
	ReadXML(filepath);
}

bool Wiinnertag::ReadXML(const std::string &filepath)
{
	pugi::xml_document xmlDoc;
	pugi::xml_parse_result result = xmlDoc.load_file(filepath.c_str());
	if (!result)
		return false;

	pugi::xml_node node = xmlDoc.child("Tag");

	while (node != NULL)
	{
		const char *URL = node.attribute("URL").value();
		const char *Key = node.attribute("Key").value();

		if (URL && Key)
		{
			int size = tagList.size();
			tagList.resize(size + 1);
			tagList[size].first = URL;
			tagList[size].second = Key;
		}

		node = node.next_sibling();
	}

	return true;
}

bool Wiinnertag::Send(const char *gameID)
{
	if (!IsNetworkInit())
		return false;

	char sendURL[1024];

	for (u32 i = 0; i < tagList.size(); ++i)
	{
		strcpy(sendURL, tagList[i].first.c_str());

		replaceString(sendURL, "{ID6}", gameID);
		replaceString(sendURL, "{KEY}", tagList[i].second.c_str());

		struct download file = {};
		file.skip_response = true;
		downloadfile(sendURL, &file);
	}

	return true;
}

bool Wiinnertag::TagGame(const char *gameID)
{
	std::string fullpath(Settings.WiinnertagPath);
	if (fullpath.length() == 0)
		return false;

	if (fullpath.back() != '/')
		fullpath += '/';
	fullpath += "Wiinnertag.xml";

	Wiinnertag Tag(fullpath);
	return Tag.Send(gameID);
}

bool Wiinnertag::CreateExample(const std::string &filepath)
{
	if (filepath.length() == 0)
		return false;

	CreateSubfolder(filepath.c_str());

	std::string fullpath = filepath;
	if (fullpath.back() != '/')
		fullpath += '/';
	fullpath += "Wiinnertag.xml";

	pugi::xml_document xmlDoc;
	pugi::xml_node declaration = xmlDoc.append_child(pugi::node_declaration);
	declaration.append_attribute("version") = "1.0";
	declaration.append_attribute("encoding") = "UTF-8";

	pugi::xml_node Tag = xmlDoc.append_child("Tag");
	Tag.append_attribute("URL") = "https://tag.rc24.xyz/wii?game={ID6}&key={KEY}";
	Tag.append_attribute("Key") = "1234567890";

	xmlDoc.save_file(fullpath.c_str());

	return true;
}
