#pragma once

#include <ogc/es.h>

#ifdef __cplusplus
extern "C" {
#endif

int WDVD_Init();
void WDVD_Close();
bool WDVD_Reset();
//bool WDVD_LowClosePartition();
int WDVD_LowRead(void *buf, u32 len, u64 offset);
int WDVD_LowUnencryptedRead(void *buf, u32 len, u64 offset);
int WDVD_LowReadSectors(void *buf, u32 len, u32 offset);
int WDVD_LowReadDiskId();
int WDVD_LowOpenPartition(u64 offset);
int WDVD_LowReadBCA(void* buffer, u32 length);
int WDVD_ReportKey(int keytype, u32 lba, void* buf);
int WDVD_StopMotor(bool eject, bool kill);
int WDVD_CheckCover();
int WDVD_VerifyCover(bool* cover);
int WDVD_DiscInserted();
int WDVD_SetDVDMode(int enable);
int WDVD_DVDRead(void *outbuf, u32 len, u32 lba);
tmd* WDVD_GetTMD();

int WDVD_StartLog();

#ifdef __cplusplus
}
#endif
