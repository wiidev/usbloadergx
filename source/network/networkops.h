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
#define NET_SIZE_UNKNOWN	   -0xFFFFFFF
#define NET_DEFAULT_SOCK	   -0xFFFFFFE

void Initialize_Network(int retries = 0);
void DeinitNetwork(void);
bool IsNetworkInit(void);
char * GetNetworkIP(void);
char * GetIncommingIP(void);
bool ShutdownWC24();
bool CheckConnection(const char *url, float timeout = 5.0f);
s32 network_request(s32 connection, const char * request, char * filename);
s32 network_read(s32 connection, u8 *buf, u32 len);
s32 download_request(const char * url, char * filename = NULL);
s32 DownloadWithResponse(const char * url, u8 **outbuffer, u32 *outsize);
void CloseConnection();
char * HEAD_Request(const char * url);
void HaltNetworkThread();
void ResumeNetworkWait();
void ResumeNetworkThread();
void InitNetworkThread();
void ShutdownNetworkThread();

#endif
