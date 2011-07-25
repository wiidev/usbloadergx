/***************************************************************************
 * Copyright (C) 2009
 * by USB Loader GX Team
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
 *
 * Theme_List Class
 * for the USB Loader GX
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gctypes.h>

#include "network/networkops.h"
#include "Theme_List.h"
#include "xml/tinyxml.h"

Theme_List::Theme_List(const char * url)
{
	if (!IsNetworkInit())
		return;

	u8 *buffer = NULL;
	u32 size = 0;

	DownloadWithResponse(url, &buffer, &size);

	if(!buffer)
		return;

	const char *xml = strstr((char *) buffer, "<?xml version=");
	if(!xml)
	{
		free(buffer);
		return;
	}

	ParseXML(xml);

	free(buffer);
}

Theme_List::~Theme_List()
{
}

bool Theme_List::ParseXML(const char * xmlfile)
{
	TiXmlDocument xmlDoc;

	if(!xmlDoc.Parse(xmlfile))
		return false;

	TiXmlElement *themesNode =  xmlDoc.FirstChildElement("themes");
	if (!themesNode)
		return false;

	TiXmlElement *theme = themesNode->FirstChildElement("theme");

	while(theme)
	{
		int i = ThemesList.size();
		ThemesList.resize(i+1);

		TiXmlElement *node = NULL;

		node = theme->FirstChildElement("name");
		if(node && node->FirstChild() && node->FirstChild()->Value())
			ThemesList[i].themetitle = node->FirstChild()->Value();

		node = theme->FirstChildElement("creator");
		if(node && node->FirstChild() && node->FirstChild()->Value())
			ThemesList[i].author = node->FirstChild()->Value();

		node = theme->FirstChildElement("thumbpath");
		if(node && node->FirstChild() && node->FirstChild()->Value())
			ThemesList[i].imagelink = node->FirstChild()->Value();

		node = theme->FirstChildElement("downloadpath");
		if(node && node->FirstChild() && node->FirstChild()->Value())
			ThemesList[i].downloadlink = node->FirstChild()->Value();

		node = theme->FirstChildElement("averagerating");
		if(node && node->FirstChild() && node->FirstChild()->Value())
			ThemesList[i].rating = atoi(node->FirstChild()->Value());

		theme = theme->NextSiblingElement();
	}

	return true;
}

const char * Theme_List::GetThemeTitle(int ind) const
{
	if (ind < 0 || ind >= (int) ThemesList.size())
		return NULL;

	else return ThemesList[ind].themetitle.c_str();
}

const char * Theme_List::GetThemeAuthor(int ind) const
{
	if (ind < 0 || ind >= (int) ThemesList.size())
		return NULL;

	return ThemesList[ind].author.c_str();
}

const char * Theme_List::GetImageLink(int ind) const
{
	if (ind < 0 || ind >= (int) ThemesList.size())
		return NULL;

	return ThemesList[ind].imagelink.c_str();
}

const char * Theme_List::GetDownloadLink(int ind) const
{
	if (ind < 0 || ind >= (int) ThemesList.size())
		return NULL;

	return ThemesList[ind].downloadlink.c_str();
}
