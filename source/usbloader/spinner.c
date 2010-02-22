#include <gccore.h>
#include "libwbfs/libwbfs.h"

static u32 done = 0;
static u32 total = 0;

void WBFS_Spinner(u32 d, u32 t)
{
    done = d;
    total = t;
}

void GetProgressValue(u32 * d, u32 * t)
{
    *d = done;
    *t = total;
}
