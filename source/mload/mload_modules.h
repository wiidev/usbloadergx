#ifndef _MLOAD_MODULES_H_
#define _MLOAD_MODULES_H_

#include "mload.h"

#ifdef __cplusplus
extern "C" {
#endif

int load_modules(const u8 * ehcmodule, int ehcmodule_size, const u8 * dip_plugin, int dip_plugin_size);
void enable_ES_ioctlv_vector(void);
void Set_DIP_BCA_Datas(u8 *bca_data);
void disableIOSReload(void);
u8 *search_for_ehcmodule_cfg(u8 *p, int size);
bool shadow_mload();

#ifdef __cplusplus
}
#endif

#endif


