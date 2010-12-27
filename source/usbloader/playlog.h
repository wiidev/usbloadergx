#ifndef PLAYLOG_H_
#define PLAYLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

int Playlog_Create(void);
int Playlog_Update(const char * ID, const u16 * title);
int Playlog_Delete(void);

#ifdef __cplusplus
}
#endif

#endif
