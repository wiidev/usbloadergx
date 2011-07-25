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
#ifndef WIINNERTAG_H_
#define WIINNERTAG_H_

#include <vector>
#include <string>
#include <gctypes.h>

using namespace std;

class Wiinnertag
{
	public:
		static bool CreateExample(const string &filepath);
		static bool TagGame(const char *gameID);
	private:
		Wiinnertag(const string &filepath);
		bool Send(const char *gameID);
		bool ReadXML(const string &filepath);
		vector<pair<string, string> > tagList;
};

#endif
