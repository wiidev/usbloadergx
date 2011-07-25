#ifndef __WIP_H__
#define __WIP_H__

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		u32 offset;
		u32 srcaddress;
		u32 dstaddress;
	} WIP_Code;

	int load_wip_code(u8 *gameid);
	void do_wip_code(u8 * dst, u32 len);
	bool set_wip_list(WIP_Code * list, int size);
	void wip_reset_counter();
	void free_wip();

#ifdef __cplusplus
}
#endif

#endif //__WIP_H__
