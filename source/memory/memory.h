#ifndef __MEMORY_H_
#define __MEMORY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define	 Disc_ID		((vu32*) 0x80000000)
#define	 Disc_Region	((vu32*) 0x80000003)
#define	 Disc_Magic		((vu32*) 0x80000018)
#define	 Sys_Magic		((vu32*) 0x80000020)
#define	 Sys_Version	((vu32*) 0x80000024)
#define	 Mem_Size		((vu32*) 0x80000028)
#define	 Board_Model	((vu32*) 0x8000002C)
#define	 Arena_L		((vu32*) 0x80000030)
#define	 Arena_H		((vu32*) 0x80000034)
#define	 FST			((vu32*) 0x80000038)
#define	 Max_FST		((vu32*) 0x8000003C)
#define	 Assembler		((vu32*) 0x80000060)
#define	 Video_Mode		((vu32*) 0x800000CC)
#define	 Dev_Debugger	((vu32*) 0x800000EC)
#define	 Simulated_Mem	((vu32*) 0x800000F0)
#define	 BI2			((vu32*) 0x800000F4)
#define	 Bus_Speed		((vu32*) 0x800000F8)
#define	 CPU_Speed		((vu32*) 0x800000FC)
#define	 Online_Check	((vu32*) 0x80003180)
#define	 GameID_Address	((vu32*) 0x80003184)
#define	 HW_PPCSPEED	((vu32*) 0xCD800018)

#define allocate_memory(size) memalign(32, (size+31)&(~31))

#ifdef __cplusplus
}
#endif

#endif
