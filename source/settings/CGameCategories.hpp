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
#ifndef GAMECATEGORIES_HPP_
#define GAMECATEGORIES_HPP_

#include <gctypes.h>
#include <map>
#include <string>
#include <vector>
#include "xml/tinyxml2.h"
#include "CCategoryList.hpp"

using namespace tinyxml2;

class CGameCategories
{
	public:
		CGameCategories();
		bool Load(string filepath);
		bool Save();
		bool SetCategory(const string &gameID, unsigned int id);
		bool SetCategory(const char *gameID, unsigned int id);
		bool SetCategory(const unsigned char *gameID, int unsigned id) { return SetCategory((const char *) gameID, id); };
		bool ReplaceCategory(const string &gameID, unsigned int id);
		bool ReplaceCategory(const char *gameID, unsigned int id);
		bool ReplaceCategory(const unsigned char *gameID, int unsigned id) { return SetCategory((const char *) gameID, id); };
		void RemoveCategory(unsigned int id);
		void RemoveCategory(const string &gameID, unsigned int id);
		void RemoveCategory(const char *gameID, unsigned int id);
		void RemoveCategory(const unsigned char *gameID, unsigned int id)  { RemoveCategory((const char *) gameID, id); };
		void RemoveGameCategories(const string &gameID);
		const vector<unsigned int> &operator[](const char *gameID) const;
		const vector<unsigned int> &operator[](const unsigned char *gameID) const { return operator[]((const char *) gameID); }
		bool ImportFromGameTDB(const string &xmlpath);
		void clear() { List.clear(); CategoryList.clear(); };
		static bool isInCategory(const char *gameID, unsigned int id);

		CCategoryList CategoryList;
	protected:
		bool ValidVersion(XMLElement *xmlfile);

		string configPath;
		const vector<unsigned int> defaultCategory;
		map<string, vector<unsigned int> > List;
};

extern CGameCategories GameCategories;

#endif
