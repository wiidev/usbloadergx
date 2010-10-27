#ifndef _CFG_H_
#define _CFG_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "usbloader/disc.h"

    char *get_title(struct discHdr *header);
    char *cfg_get_title(u8 *id);
    void title_set(char *id, char *title);
    void titles_default();
    u8 get_block(struct discHdr *header);
    s8 get_pegi_block(struct discHdr *header);

    void CFG_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
