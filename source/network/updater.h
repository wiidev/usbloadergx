#ifndef _UPDATER_H_
#define _UPDATER_H_

void Initialize_Network(void);
bool IsNetworkInit(void);
char * GetNetworkIP(void);
s32 network_request(const char * request);
s32 network_read(u8 *buf, u32 len);
s32 download_request(const char * url);
void CloseConnection();
int CheckUpdate();

void HaltNetworkThread();
void ResumeNetworkThread();
void InitNetworkThread();
void ShutdownNetworkThread();

#endif
