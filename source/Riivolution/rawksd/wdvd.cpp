#include <ogc/ipc.h>
#include <string.h>
#include "wdvd.h"

static int di_fd = -1;

// Apparently GCC is too cool for casting.
static union {
	u32 inbuffer[0x10]; //u32 inbuffer[0x10] ATTRIBUTE_ALIGN(32);
	ioctlv iovector[0x08];
} ATTRIBUTE_ALIGN(32);

int WDVD_Init() {
    if (di_fd >= 0)
		return di_fd;

    di_fd = IOS_Open("/dev/do", 0);
    if (di_fd<0)
    	di_fd = IOS_Open("/dev/di", 0);

    return di_fd;
}

bool WDVD_LowClosePartition() {

	if (di_fd<0)
		return false;

	inbuffer[0x00] = 0x8C000000;

	return !!IOS_Ioctl(di_fd, 0x8C, inbuffer, 0x20, 0, 0);
}

bool WDVD_Reset() {
	if (di_fd < 0)
		return false;

	inbuffer[0x00] = 0x8A000000;
	inbuffer[0x01] = 1;
	return (IOS_Ioctl(di_fd, 0x8A, inbuffer, 0x20, inbuffer + 0x08, 0x20)==1);
}

int WDVD_LowUnencryptedRead(void *buf, u32 len, u64 offset) {
	int result;

	if (di_fd<0 || (u32)buf&0x1F)
		return -1;

	inbuffer[0] = 0x8d000000;
	inbuffer[1] = len;
	inbuffer[2] = offset >> 2;

	result = IOS_Ioctl(di_fd, 0x8d, inbuffer, 0x20, buf, len);

	return (result==1) ? 0 : -result;
}


int WDVD_LowRead(void *buf, u32 len, u64 offset) {
	int result;

	if (di_fd<0 || (u32)buf&0x1F)
		return -1;

	memset(inbuffer, 0, 0x20);
	inbuffer[0] = 0x71000000;
	inbuffer[1] = len;
	inbuffer[2] = offset >> 2;

	result = IOS_Ioctl(di_fd, 0x71, inbuffer, 0x20, buf, len);

	return (result==1) ? 0 : -result;
}

int WDVD_LowReadSectors(void *outbuf, u32 len, u32 offset)
{
    // make sure it's a MEM2 buffer
    u32 high_addr = ((u32)outbuf) >> 26;
    if ((high_addr|0x10)!=(0xD0>>2) || (u32)outbuf&0x1F)
      return -1;

	return WDVD_LowUnencryptedRead(outbuf, len<<11, (u64)offset<<11);
}

int WDVD_LowReadDiskId() {
	int result;
	void *outbuf = (void*)0x80000000;

	if (di_fd<0)
		return -1;

	memset(outbuf, 0, 0x20);
	inbuffer[0] = 0x70000000;

	result = IOS_Ioctl(di_fd, 0x70, inbuffer, 0x20, outbuf, 0x20);
	return (result==1) ? 0 : -result;
}

// ugh. Don't even need this tmd shit.
static u8 tmd_data[0x49e4] ATTRIBUTE_ALIGN(32);
static u8 errorbuffer[0x40] ATTRIBUTE_ALIGN(32);
int WDVD_LowOpenPartition(u64 offset) {
	int result;

	if (di_fd<0)
		return -1;

	WDVD_LowClosePartition();

	inbuffer[12] = 0x8B000000;
	inbuffer[13] = offset >> 2;
	inbuffer[14] = inbuffer[15] = 0;

	iovector[0].data = inbuffer+12;
	iovector[0].len = 0x20;
	iovector[1].data = 0;
	iovector[1].len = 0x2a4;
	iovector[2].data = 0;
	iovector[2].len = 0;
	iovector[3].data = tmd_data;
	iovector[3].len = 0x49e4;
	iovector[4].data = errorbuffer;
	iovector[4].len = 0x20;

	result = IOS_Ioctlv(di_fd, 0x8B, 3, 2, iovector);

	return (result==1) ? 0 : -result;
}
tmd* WDVD_GetTMD()
{
	return (tmd*)(tmd_data+sizeof(sig_rsa2048));
}

void WDVD_Close() {
	if (di_fd < 0)
		return;

	IOS_Close(di_fd);
	di_fd = -1;
}

int WDVD_CheckCover() {
	int result;
	if (di_fd<0)
		return -1;

	inbuffer[0] = 0x88000000;
	result = IOS_Ioctl(di_fd, 0x88, inbuffer, 0x20, inbuffer + 0x08, 0x20);
	return (result==1) ? 1 : -result;
}

int WDVD_VerifyCover(bool* cover) {
	*cover = false;
	if (di_fd<0)
		return -1;

	inbuffer[0] = 0xdb000000;
	if (IOS_Ioctl(di_fd, 0xdb, inbuffer, 0x20, inbuffer + 0x08, 0x20) != 1)
		return -1;

	*cover = !inbuffer[0x08];
	return 0;
}

int WDVD_DiscInserted() {
	if (di_fd<0)
		return -1;

	return IOS_Ioctl(di_fd, 0xce, inbuffer, 0x20, inbuffer + 0x08, 0x20);
}

int WDVD_LowReadBCA(void* buffer, u32 length)
{
	int result;
	if (di_fd<0)
		return -1;

	inbuffer[0] = 0xDA000000;
	result = IOS_Ioctl(di_fd, 0xDA, inbuffer, 0x20, errorbuffer, 0x40);
	memcpy(buffer, errorbuffer, length);
	return (result==1) ? 0 : -result;
}

int WDVD_StopMotor(bool eject, bool kill)
{
	if (di_fd<0)
		return -1;

	inbuffer[0] = 0xE3000000;
	inbuffer[1] = eject ? 1 : 0;
	inbuffer[2] = kill ? 1 : 0;
	return IOS_Ioctl(di_fd, 0xE3, inbuffer, 0x20, inbuffer + 0x08, 0x20);
}

int WDVD_SetDVDMode(int enable)
{
	int result = 0;
	if (di_fd<0)
		return -1;

	inbuffer[0] = enable << 24;
	result = IOS_Ioctl(di_fd, 0x8E, inbuffer, 0x20, inbuffer + 0x08, 0x20);
	return (result==1) ? 0 : -result;
}

int WDVD_DVDRead(void *buf, u32 len, u32 lba)
{
	int result = 0;
	if (di_fd<0 || (u32)buf&0x1F)
		return -1;

	inbuffer[0] = 0xD0000000;
	inbuffer[1] = 0;
	inbuffer[2] = 0;
	inbuffer[3] = len;
	inbuffer[4] = lba;

	result = IOS_Ioctl(di_fd, 0xD0, inbuffer, 0x20, buf, len<<11);

	return (result==1) ? 0 : -result;
}


int WDVD_ReportKey(int keytype, u32 lba, void* buf)
{
	int result;
	if (di_fd<0)
		return -1;

	inbuffer[0] = 0xA4000000;
	inbuffer[1] = keytype;
	inbuffer[2] = lba;

	result = IOS_Ioctl(di_fd, 0xA4, inbuffer, 0x20, errorbuffer, 0x20);
	memcpy(buf, errorbuffer, 0x20);
	return (result==1) ? 0 : -result;
}


int WDVD_StartLog()
{
	if (di_fd<0)
		return -1;
	return IOS_Ioctl(di_fd, 0xCF, inbuffer, 0x20, inbuffer + 0x08, 0x20);
}
