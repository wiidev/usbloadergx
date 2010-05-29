#ifndef __WIP_H__
#define __WIP_H__

int load_wip_code(u8 *gameid);
void do_wip_code(u8 * dst, u32 len);
void wip_reset_counter();
void free_wip();

#endif //__WIP_H__
