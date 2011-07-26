#ifndef _NAND_EMU_H_
#define _NAND_EMU_H_

#define REAL_NAND	0
#define EMU_SD		1
#define EMU_USB		2

/* Prototypes */
s32 Enable_Emu(int selection);
s32 Disable_Emu();
void Set_Partition(int);
void Set_Path(const char*);
void Set_FullMode(int);
const char* Get_Path(void);

#endif
