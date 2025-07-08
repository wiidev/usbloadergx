/* 
From Swiss, mcp.h
 */

#ifndef __MCP_H__
#define __MCP_H__

#include <gctypes.h>
#include <ogc/dvd.h>
#include "usbloader/disc.h"

#define MCP_RESULT_READY        0
#define MCP_RESULT_BUSY        -1
#define MCP_RESULT_WRONGDEVICE -2
#define MCP_RESULT_NOCARD      -3
#define MCP_RESULT_FATAL_ERROR -128

#ifdef __cplusplus
extern "C" {
#endif

s32 MCP_ProbeEx(s32 chan);
s32 MCP_GetDeviceID(s32 chan, u32 *id);
s32 MCP_SetDiskID(s32 chan, const dvddiskid *diskID);
s32 MCP_SetDiskInfo(s32 chan, const char diskInfo[64]);
void gameID_early_set(const discHdr *header);

#ifdef __cplusplus
}
#endif

#endif