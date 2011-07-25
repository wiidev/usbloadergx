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
#ifndef CATEGORYLIST_HPP_
#define CATEGORYLIST_HPP_

#include <map>
#include <string>

using namespace std;

class CCategoryList
{
	public:
		CCategoryList();
		bool Load(string filepath);
		bool Save();
		bool AddCategory(const string &name);
		bool SetCategory(unsigned int id, const string &name);
		void RemoveCategory(unsigned int id);
		void RemoveCategory(const string &name);
		bool goToFirst() { listIter = nameList.begin(); return true; }
		bool goToNext() { listIter++; return listIter != nameList.end(); }
		unsigned int getCurrentID() const { return listIter->first; }
		const string &getCurrentName() const { return listIter->second; }
		const char * operator[](unsigned int id);
		const char *at(unsigned int id) { return operator[](id); }
		void goToNextCicle()  { listIter++; if(listIter == nameList.end()) listIter = nameList.begin(); }
		void goToPreviousCicle()  { if(listIter == nameList.begin())  listIter = nameList.end(); listIter--; }
		bool findCategory(const string &name);
		bool findCategory(unsigned int id) { listIter = nameList.find(id); return listIter != nameList.end(); };
		int pos() const { return distance(nameList.begin(), listIter); }
		int size() const { return nameList.size(); }
		void clear();
	private:
		string configPath;
		map<unsigned int, string>::const_iterator listIter;
		map<unsigned int, string> nameList;
};

#endif
