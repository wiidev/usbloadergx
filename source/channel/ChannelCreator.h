#ifndef CHANNEL_CREATOR_H_
#define CHANNEL_CREATOR_H_

#include "usbloader/disc.h"

//! "Create Wii Menu Channel": a channel with the game's banner that starts this game
//! through USB Loader GX (forwarder DOL + game ID). Asks, builds and installs to NAND.
void CreateGameChannel(const struct discHdr *header);

//! True if this game's forwarder channel is installed (shows "Remove Wii Menu Channel").
bool GameChannelInstalled(const struct discHdr *header);

//! True for a shortcut channel made by this loader (forwarder DOL with ULGXFWD1 block);
//! the game list hides those so they do not duplicate their game.
bool IsGameShortcutChannel(u64 tid);

//! Asks and deletes the channel (title + ticket). The game and its saves stay.
void RemoveGameChannel(const struct discHdr *header);

#endif
