/****************************************************************************
 * Copyright (C) 2026 KakarottoCake
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
#ifndef RIFTWII_HPP_
#define RIFTWII_HPP_

#include <string>
#include <vector>
#include <gctypes.h>
#include "usbloader/disc.h"

//! Riivolution packs through RiftWii (https://github.com/KakarottoCake/Riftwii).
//! A game with a pack picked in its Game Load settings is handed to RiftWii,
//! which applies the pack and boots the game with the settings chosen here.
//! RiftWii's side is documented in its docs/HEADLESS.md.
namespace RiftWii
{
	//! Where RiftWii's boot.dol was found, or an empty string
	std::string LoaderPath();
	//! The packs in sd:/riivolution made for this game (file names)
	void ListPacks(const char *gameID, std::vector<std::string> &packs);
	//! Starts RiftWii with the game, the pack and the resolved game settings.
	//! Only returns on failure.
	int BootGame(const struct discHdr &gameHeader, const std::string &pack, u8 videoChoice, u8 languageChoice,
				 u8 ocarinaChoice, u8 deflicker, u16 videoWidth, u8 privateServer, const char *customAddress,
				 u8 autoIOS, s32 iosChoice, u64 returnTo);
}

#endif
