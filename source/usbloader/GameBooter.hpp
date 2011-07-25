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
		static int SetupDisc(u8 *gameID);
		static bool LoadOcarina(u8 *gameID);
		static u32 BootPartition(char * dolpath, u8 videoselected, u8 alternatedol, u32 alternatedoloffset);
};

#endif
