#ifndef _UPDATER_H_
#define _UPDATER_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern bool netcheck;

int Net_Init(char *ip);
s32 network_request(const char * request);
s32 network_read(void *buf, u32 len);
s32 downloadrev(const char * url);
void CloseConnection();

#ifdef __cplusplus
}
#endif

#endif
