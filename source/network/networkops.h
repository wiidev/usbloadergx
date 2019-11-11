/****************************************************************************
 * Network Operations
 * for USB Loader GX
 *
 * HTTP operations
 * Written by dhewg/bushing modified by dimok
 ****************************************************************************/
#ifndef _NETWORKOPS_H_
#define _NETWORKOPS_H_

#define NETWORKBLOCKSIZE	   5*1024	  //5KB
#define NET_DEFAULT_SOCK	   -0xFFFFFFE

void Initialize_Network(int retries = 0);
void DeinitNetwork(void);
bool IsNetworkInit(void);
char * GetNetworkIP(void);
char * GetIncommingIP(void);
s32 network_read(s32 connection, u8 *buf, u32 len);
void CloseConnection();
void HaltNetworkThread();
void ResumeNetworkWait();
void ResumeNetworkThread();
void InitNetworkThread();
void ShutdownNetworkThread();

#endif
