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
#include "CCategoryList.hpp"
#include "language/gettext.h"
#include "FileOperations/fileops.h"
#include "svnrev.h"

#define VALID_CONFIG_REV	1085

CCategoryList CategoryList;

CCategoryList::CCategoryList()
{
	clear();
	goToFirst();
}

void CCategoryList::clear()
{
	nameList.clear();
	nameList[0] = tr("All");
}

const char * CCategoryList::operator[](unsigned int id)
{
	map<unsigned int, string>::iterator itr = nameList.find(id);
	if(itr == nameList.end())
		return NULL;

	return nameList[id].c_str();
}

bool CCategoryList::AddCategory(const string &name)
{
	if(findCategory(name))
		return false;

	unsigned int i = 1;
	map<unsigned int, string>::iterator itr;

	//! Find next free key
	while((itr = nameList.find(i)) != nameList.end())
		i++;

	nameList[i] = name;
	listIter = nameList.find(i);

	return true;
}

bool CCategoryList::SetCategory(unsigned int id, const string &name)
{
	RemoveCategory(name);
	nameList[id] = name;
	listIter = nameList.find(id);
	return true;
}

void CCategoryList::RemoveCategory(const string &name)
{
	for (map<unsigned int, string>::iterator itr = nameList.begin(); itr != nameList.end(); itr++)
	{
		if(strcasecmp(name.c_str(), itr->second.c_str()) == 0)
		{
			nameList.erase(itr);
			break;
		}
	}
}

void CCategoryList::RemoveCategory(unsigned int id)
{
	map<unsigned int, string>::iterator itr = nameList.find(id);

	if(itr != nameList.end())
		nameList.erase(itr);
}

bool CCategoryList::findCategory(const string &name)
{
	for (listIter = nameList.begin(); listIter != nameList.end(); listIter++)
	{
		if(strcasecmp(name.c_str(), listIter->second.c_str()) == 0)
			return true;
	}
	return false;
}
