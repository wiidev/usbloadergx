/*
	Time functions were changed within libogc in 2018.
	Since then some Wii systems return the incorrect time.
*/

#include <ogc/conf.h>
#include <ogc/exi.h>
#include <time.h>

static u32 getrtc(u32 *gctime)
{
	u32 ret = 0, time = 0, cmd = 0x20000000;

	if (EXI_Lock(EXI_CHANNEL_0, EXI_DEVICE_1, NULL) == 0)
		return 0;
	if (EXI_Select(EXI_CHANNEL_0, EXI_DEVICE_1, EXI_SPEED8MHZ) == 0)
	{
		EXI_Unlock(EXI_CHANNEL_0);
		return 0;
	}

	if (EXI_Imm(EXI_CHANNEL_0, &cmd, 4, EXI_WRITE, NULL) == 0)
		ret |= 0x01;
	if (EXI_Sync(EXI_CHANNEL_0) == 0)
		ret |= 0x02;
	if (EXI_Imm(EXI_CHANNEL_0, &time, 4, EXI_READ, NULL) == 0)
		ret |= 0x04;
	if (EXI_Sync(EXI_CHANNEL_0) == 0)
		ret |= 0x08;
	if (EXI_Deselect(EXI_CHANNEL_0) == 0)
		ret |= 0x10;

	EXI_Unlock(EXI_CHANNEL_0);
	*gctime = time;
	if (ret)
		return 0;

	return 1;
}

u32 SYS_GetRTC(u32 *gctime)
{
	u32 cnt = 0, ret = 0;
	u32 time1, time2;

	while (cnt < 16)
	{
		if (getrtc(&time1) == 0)
			ret |= 0x01;
		if (getrtc(&time2) == 0)
			ret |= 0x02;
		if (ret)
			return 0;
		if (time1 == time2)
		{
			*gctime = time1;
			return 1;
		}
		cnt++;
	}
	return 0;
}

time_t __wrap_time(time_t *timer)
{
	u32 gctime = 0, bias = 0;

	if (SYS_GetRTC((u32 *)&gctime) == 0)
		return (time_t)0;
	if (CONF_GetCounterBias(&bias) >= 0)
		gctime += bias;
	gctime += 946684800;
	if (timer)
		*timer = gctime;
	return gctime;
}

/*
// This function returns the wrong time on a Brazilian Wii (4.3U)
// Reported by Walwii @ GBAtemp.net
time_t __wrap_time(time_t *timer)
{
	time_t gctime;
	u32 rtctime = 0, bias = 0;

	if (SYS_GetRTC(&rtctime) == 0)
		return (time_t)0;
	gctime = rtctime;
	if (CONF_GetCounterBias(&bias) >= 0)
		gctime += bias;
	gctime += 946684800;
	if (timer)
		*timer = gctime;
	return gctime;
}
*/
