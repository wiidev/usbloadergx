#ifndef _NAND_H_
#define _NAND_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 'NAND Device' structure */
typedef struct {
	/* Device name */
	char *name;

	/* Mode value */
	u32 mode;

	/* Un/mount command */
	u32 mountCmd;
	u32 umountCmd;
} nandDevice;


#define REAL_NAND	0
#define EMU_SD		1
#define EMU_USB		2

/* Prototypes */
s32 Nand_Mount(nandDevice *);
s32 Nand_Unmount(nandDevice *);
s32 Nand_Enable(nandDevice *);
s32 Nand_Disable(void);
s32 Enable_Emu(int selection);
s32 Disable_Emu();

void Set_Partition(int);
void Set_Path(const char*);
void Set_FullMode(int);
const char* Get_Path(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
