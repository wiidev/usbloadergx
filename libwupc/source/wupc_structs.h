
#ifndef _WUPC_STRUCTS_H_
#define _WUPC_STRUCTS_H_

struct WUPCStat {
	u32 connected;
	u32 transferstate;
	u32 channel;
	u32 rumble;
	s16 xAxisLmid;
	s16 xAxisRmid;
	s16 yAxisLmid;
	s16 yAxisRmid;
	struct bte_pcb *sock;
	struct bd_addr bdaddr;
};

struct WUPCButtons {
	u32 up;
	u32 down;
	u32 state;
};

#endif
