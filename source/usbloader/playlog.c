/*
	PLAYLOG.C
	This code allows to modify play_rec.dat in order to store the
	game time in Wii's log correctly.

	by Marc
	Thanks to tueidj for giving me some hints on how to do it :)
	Most of the code was taken from here:
	http://forum.wiibrew.org/read.php?27,22130

	Modified by Dimok
*/

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <malloc.h>
#include "gecko.h"
#include "utils/tools.h"

#define SECONDS_TO_2000 946684800LL
#define TICKS_PER_SECOND 60750000LL

//! Should be 32 byte aligned
static const char PLAYRECPATH[] ATTRIBUTE_ALIGN(32) = "/title/00000001/00000002/data/play_rec.dat";

typedef struct _PlayRec
{
	u32 checksum;
	union
	{
		u32 data[31];
		struct
		{
			u16 name[42];
			u64 ticks_boot;
			u64 ticks_last;
			char title_id[6];
			char unknown[18];
		} ATTRIBUTE_PACKED;
	};
} PlayRec;

// Thanks to Dr. Clipper
static u64 getWiiTime(void)
{
	time_t uTime = time(NULL);
	return TICKS_PER_SECOND * (uTime - SECONDS_TO_2000);
}

int Playlog_Create(void)
{
	s32 fd = IOS_Open(PLAYRECPATH, IPC_OPEN_RW);
	if(fd >= 0)
	{
		//exists
		IOS_Close(fd);
		return 0;
	}

	ISFS_Initialize();

	//In case the play_rec.dat wasn´t found create one and try again
	int ret = ISFS_CreateFile(PLAYRECPATH, 0, 3, 3, 3);
	if(ret >= 0)
		ISFS_SetAttr(PLAYRECPATH, 0x1000, 1, 0, 3, 3, 3);

	ISFS_Deinitialize();

	return ret;
}

int Playlog_Update(const char * ID, const u16 * title)
{
	if(!ID || !title)
		return -1;

	//If not started from SystemMenu, create playlog
	Playlog_Create();

	s32 fd = -1, res = -1;
	u32 sum = 0;
	u8 i;

	//Open play_rec.dat
	fd = IOS_Open(PLAYRECPATH, IPC_OPEN_RW);
	if(fd == -106)
	{
		//In case the play_rec.dat wasn´t found create one and try again
		int ret = Playlog_Create();
		if(ret < 0)
			return ret;

		fd = IOS_Open(PLAYRECPATH, IPC_OPEN_RW);
	}

	if(fd < 0)
		return res;

	PlayRec * playrec_buf = memalign(32, ALIGN32(sizeof(PlayRec))); //! Should be 32 byte aligned
	if(!playrec_buf)
	{
		IOS_Close(fd);
		return res;
	}

	memset(playrec_buf, 0, sizeof(PlayRec));

	u64 stime = getWiiTime();
	playrec_buf->ticks_boot = stime;
	playrec_buf->ticks_last = stime;

	//Update channel name and ID
	memcpy(playrec_buf->name, title, 84);
	memcpy(playrec_buf->title_id, ID, 6);

	//Calculate and update checksum
	for(i = 0; i < 31; i++)
		sum += playrec_buf->data[i];

	playrec_buf->checksum = sum;

	//Write play_rec.dat
	if(IOS_Write(fd, playrec_buf, sizeof(PlayRec)) == sizeof(PlayRec))
		res = 0;

	IOS_Close(fd);

	free(playrec_buf);

	return res;
}

int Playlog_Delete(void)
{
	s32 res = -1;

	//Open play_rec.dat
	s32 fd = IOS_Open(PLAYRECPATH, IPC_OPEN_RW);
	if(fd < 0)
		return fd;

	PlayRec * playrec_buf = memalign(32, ALIGN32(sizeof(PlayRec)));
	if(!playrec_buf)
		goto cleanup;

	//Read play_rec.dat
	if(IOS_Read(fd, playrec_buf, sizeof(PlayRec)) != sizeof(PlayRec))
		goto cleanup;

	if(IOS_Seek(fd, 0, 0) < 0)
		goto cleanup;

	// invalidate checksum
	playrec_buf->checksum = 0;

	if(IOS_Write(fd, playrec_buf, sizeof(PlayRec)) != sizeof(PlayRec))
		goto cleanup;

	res = 0;

cleanup:
	free(playrec_buf);
	IOS_Close(fd);
	return res;
}
