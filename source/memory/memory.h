#ifndef __MEMORY_H_
#define __MEMORY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define     Disc_ID         ((u32*) 0x80000000)
#define     Disc_Region     ((u32*) 0x80000003)
#define     Disc_Magic      ((u32*) 0x80000018)
#define     Sys_Magic       ((u32*) 0x80000020)
#define     Version         ((u32*) 0x80000024)
#define     Mem_Size        ((u32*) 0x80000028)
#define     Board_Model     ((u32*) 0x8000002C)
#define     Arena_L         ((u32*) 0x80000030)
#define     Arena_H         ((u32*) 0x80000034)
#define     FST             ((u32*) 0x80000038)
#define     Max_FST         ((u32*) 0x8000003C)
#define     Assembler       ((u32*) 0x80000060)
#define     Video_Mode      ((u32*) 0x800000CC)
#define     Dev_Debugger    ((u32*) 0x800000EC)
#define     Simulated_Mem   ((u32*) 0x800000F0)
#define     BI2             ((u32*) 0x800000F4)
#define     Bus_Speed       ((u32*) 0x800000F8)
#define     CPU_Speed       ((u32*) 0x800000FC)
#define     Online_Check    ((u32*) 0x80003180)
#define     GameID_Address  ((u32*) 0x80003184)

#define allocate_memory(size) memalign(32, (size+31)&(~31))

#ifdef __cplusplus
}
#endif

#endif
