/****************************************************************************
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _GCGAMES_H_
#define _GCGAMES_H_

#include <string>
#include <vector>
#include <gccore.h>
#include "usbloader/disc.h"
#include "settings/CSettings.h"

using namespace std;

const char *nintendontBuildDate(const char *NIN_loader_path);

class GCGames
{
public:
	static const u32 MAGIC = 0xC2339F3D;

	static GCGames *Instance(void) { if(!instance) instance = new GCGames(); return instance; }
	static void DestroyInstance(void) { delete instance; instance = NULL; }

	static u8 *GetOpeningBnr(const char *gameID);

	u32 LoadAllGames(void);

	void LoadGameList(const string &path, vector<struct discHdr> &headerList, vector<string> &pathList);

	bool RemoveGame(const char *gameID);
	bool RemoveSDGame(const char *gameID);
	float GetGameSize(const char *gameID);

	const char *GetPath(const char *gameID) const;

	vector<struct discHdr> & GetHeaders(void)
	{
		LoadAllGames();

		return HeaderList;
	}

	vector<struct discHdr> & GetSDHeaders(void) {
		return sdGCList;
	}

	bool CopyUSB2SD(const struct discHdr *header);
	bool IsInstalled(const char *gameID, u8 disc_number) const;
private:

	static GCGames *instance;

	vector<string> PathList;
	vector<struct discHdr> HeaderList;
	vector<struct discHdr> sdGCList;
	vector<string> sdGCPathList;
};

#endif
