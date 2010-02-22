#ifndef _MLOAD_MODULES_H_
#define _MLOAD_MODULES_H_

#include "dip_plugin.h"
#include "mload.h"
#include "fatffs_module_bin.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void *external_ehcmodule;
extern int size_external_ehcmodule;
extern int use_port1;

int load_ehc_module();
int load_fatffs_module(u8 *discid);

void enable_ES_ioctlv_vector(void);
void Set_DIP_BCA_Datas(u8 *bca_data);

void test_and_patch_for_port1();

int enable_ffs(int mode);

#ifdef __cplusplus
}
#endif

#endif
