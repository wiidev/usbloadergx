#include <string.h>
#include <gctypes.h>

bool PatchDOL(u8 * Address, int Size, const u8 * SearchPattern, int SearchSize, const u8 * PatchData, int PatchSize)
{
	u8 * Addr = Address;
	u8 * Addr_end = Address + Size;

	while (Addr <= Addr_end - SearchSize)
	{
		if (memcmp(Addr, SearchPattern, SearchSize) == 0)
		{
			memcpy(Addr, PatchData, PatchSize);
			return true;
		}
		Addr += 4;
	}
	return false;
}
