#ifndef FWDINSTALL_H_
#define FWDINSTALL_H_

#include <gctypes.h>
#include "fwdchannel.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { FWD_STEP_MEMORY, FWD_STEP_TICKET, FWD_STEP_TITLE, FWD_STEP_CONTENT, FWD_STEP_FINISH, FWD_STEP_DONE };

typedef void (*fwd_progress_cb)(u32 done, u32 total);

/* Returns 0 or a negative ES error; *failed_step says where it stopped. */
s32 fwd_install(const fwd_channel *ch, fwd_progress_cb progress, int *failed_step);
int fwd_title_installed(const u8 title_id[8]);
int fwd_is_forwarder_channel(const u8 title_id[8]);
/* Delete the channel and its tickets; game saves stay. 0 or a negative ES error. */
s32 fwd_uninstall(const u8 title_id[8]);
const char *fwd_step_text(int step);
/* Boot content (nandloader) of the installed 3-content UNEO forwarder; 0 or negative. */
int fwd_read_nandloader(u8 **out, u32 *size);

#ifdef __cplusplus
}
#endif

#endif
