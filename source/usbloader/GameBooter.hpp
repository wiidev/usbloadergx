/****************************************************************************
 * Copyright (C) 2009-2011 Dimok
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
#ifndef GAMEBOOTER_HPP_
#define GAMEBOOTER_HPP_

#include <gctypes.h>

class GameBooter
{
	public:
		static int BootGame(const char * gameID);
		static int BootGCMode();
	private:
		static int FindDiscHeader(const char * gameID, struct discHdr &gameHeader);
		static void SetupAltDOL(u8 * gameID, u8 &alternatedol, u32 &alternatedoloffset);
		static void SetupNandEmu(u8 NandEmuMode, const char *NandEmuPath, struct discHdr &gameHeader);
		static int SetupDisc(u8 *gameID);
		static u32 BootPartition(char * dolpath, u8 videoselected, u8 alternatedol, u32 alternatedoloffset);
		static void ShutDownDevices(int gameUSBPort);
};

#endif
