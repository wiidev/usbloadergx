#include "fwrite.h"
#include "files.h"

#include <stdio.h>

static void* iosioctl_location = NULL;
static void* fwrite_location = NULL;

/* NOTES
 *
 * in function arrays:
 * 0x00000000 is a bl
 * 0x00000001 is an instruction referencing r13
 * 0x00000002 is an instruction referencing rtoc/r2
 *
 *  _length is number of u32s in array
 *
 */

static void* FindFunction(char* buffer, u32 length, const u32* findme, u32 findme_length)
{
	for (u32* location = (u32*)buffer; (u8*)location < (u8*)buffer + length - findme_length * 4; location++)
	{
		u32 i = 0;
		for (u32* check = location; check < location + findme_length; check++, i++) {
			if ((findme[i] > 0x10) && (*check != findme[i]))
				break;
		}
		if (i == findme_length)
			return (void*)location;
	}
	return NULL;
}

static void* FindIosIoctl(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0x39610030,0x00000000,0x34010008,0x7c791b78,0x7c9a2378,
		0x7cbb2b78,0x7cdc3378,0x7cfd3b78,0x7d1e4378,0x3be00000,0x4082000c,0x3be0fffc,0x4800004c,
		0x00000001,0x38800040,0x38a00020,0x00000000,0x2c030000,0x90610008,0x4082000c,0x3be0ffea,
		0x48000028,0x38a00000,0x00000009,0x00000009,0x80810008,0x90a40024,0x80810008,0x90a40028,
		0x90030000,0x93230008,0x2c1f0000,0x40820088,0x80a10008,0x3be00000,0x2c050000,0x4082000c,
		0x3be0fffc,0x48000058,0x2c1d0000,0x9345000c,0x4182000c,0x3c1d8000,0x48000008,0x38000000,
		0x90050018,0x2c1b0000,0x93c5001c,0x4182000c,0x3c1b8000,0x48000008,0x38000000,0x90050010,
		0x7f63db78,0x7f84e378,0x93850014,0x00000000,0x7fa3eb78,0x7fc4f378,0x00000000,0x2c1f0000,
		0x40820014,0x80610008,0x38800000,0x00000000,0x7c7f1b78,0x39610030,0x7fe3fb78,0x00000000,
		0x80010034,0x7c0803a6,0x38210030,0x4e800020
	};
	u32 findme_length = sizeof(findme)/sizeof(findme[0]);

	return FindFunction(buffer, length, findme, findme_length);
}

static void* FindFwrite(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0xbf210014,0x7c9b2378,0x7cdc3378,0x7c7a1b78,0x7cb92b78,
		0x38800000,0x7f83e378,0x00000000,0x2c030000,0x40820010,0x7f83e378,0x00000000,0x00000000,
		0x00000000,0x4182001c,0x881c000a,0x2c000000,0x40820010,0x801c0004,0x00000000,0x4082000c,
		0x38600000,0x48000290,0x28000002,0x40820008,0x00000000,0x807c0004,0x00000000,0x38800000,
		0x00000000,0x41820010,0x54603fbe,0x28000002,0x40820008,0x00000000,0x2c040000,0x40820018,
		0x801c0004,0x54003fbe,0x00000000,0x41820008,0x3be00000,0x801c0008,0x00000000,0x40820054,
		0x807c0004,0x00000000,0x54602f7e,0x41820044,0x00000000,0x41820024,0x7f83e378,0x38800000,
		0x38a00002,0x00000000,0x2c030000,0x4182000c,0x38600000,0x48000200,0x801c0008,0x00000000,
		0x5060e804,0x901c0008,0x7f83e378,0x00000000,0x801c0008,0x54001f7e,0x00000000,0x4182001c,
		0x00000000,0x38000000,0x987c000a,0x38600000,0x901c0028,0x480001c0,0x2c1e0000,0x3ba00000,
		0x41820128,0x807c001c,0x809c0024,0x7c041840,0x4082000c,0x2c1f0000,0x41820110,0x801c0020,
		0x7c632050,0x7c030050,0x901c0028,0x80bc0028,0x3b200000,0x00000009,0x00000009,0x4081000c,
		0x7fc5f378,0x93c10008,0x801c0004,0x54003fbe,0x00000000,0x40820030,0x2c050000,0x41820028,
		0x7f43d378,0x3880000a,0x00000000,0x2c030000,0x7c791b78,0x41820010,0x00000000,0x7cba0050,
		0x90a10008,0x80a10008,0x2c050000,0x41820038,0x807c0024,0x7f44d378,0x00000000,0x80810008,
		0x807c0024,0x801c0028,0x7f5a2214,0x7c632214,0x00000009,0x00000009,0x80610008,0x7c030050,
		0x901c0028,0x801c0028,0x2c000000,0x41820018,0x2c190000,0x40820010,0x801c0004,0x00000000,
		0x40820030,0x7f83e378,0x38800000,0x00000000,0x2c030000,0x4182001c,0x00000000,0x38000000,
		0x987c000a,0x3bc00000,0x901c0028,0x4800001c,0x80010008,0x2c1e0000,0x7fbd0214,0x4182000c,
		0x2c1f0000,0x4082ff08,0x2c1e0000,0x4182006c,0x2c1f0000,0x40820064,0x833c001c,0x7c1af214,
		0x83fc0020,0x7f83e378,0x935c001c,0x38810008,0x93dc0020,0x901c0024,0x00000000,0x2c030000,
		0x41820018,0x00000000,0x38000000,0x987c000a,0x901c0028,0x4800000c,0x80010008,0x7fbd0214,
		0x933c001c,0x7f83e378,0x93fc0020,0x00000000,0x38000000,0x901c0028,0x801c0004,0x54003fbe,
		0x28000002,0x4182000c,0x38000000,0x901c0028,0x7c7ddb96,0xbb210014,0x80010034,0x7c0803a6,
		0x38210030,0x4e800020
	};
	u32 findme_length = sizeof(findme)/sizeof(findme[0]);

	return FindFunction(buffer, length, findme, findme_length);
}

static void* FindFwriteShort(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x7fdbc9d7,0x4182001c,0x881c000a,0x2c000000,0x40820010,0x801c0004,0x00000000,0x4082000c,
		0x38600000,0x00000008,0x28000002,0x40820008
	};
	u32 findme_length = sizeof(findme)/sizeof(findme[0]);

	void * check = NULL;

	check= FindFunction(buffer, length, findme, findme_length);
	if(check)
		check = (void*)( (u32)check - 0x40 );
	return  check;
}

// Can call this on every chunk write when loading main.dol into memory
int Fwrite_FindPatchLocation(char* buffer, u32 length)
{
	if (File_GetLogFS() < 0)
		return -1;

	if (!iosioctl_location)
		iosioctl_location = FindIosIoctl(buffer, length);
	if (!fwrite_location)
		fwrite_location = FindFwrite(buffer, length);
	if (!fwrite_location)
		fwrite_location = FindFwriteShort(buffer, length);
	return 0;
}

// Can call this after all memory has been setup
int Fwrite_Patch()
{
	if (File_GetLogFS() < 0 || !iosioctl_location || !fwrite_location)
		return -1;

	u32 patch[] = {
	0x38800061,			// ioctl = 0x61
	0x7CA62B78,			// in = buffer
	0x7C651B78,			// in_len = len
	0x38E00000,			// out = 0
	0x39000000,			// out_len = 0
	0x38600004,			// fd = 4
	0x4BF4702C			// b IOS_Ioctl( fd , ioctl , in , in_len , out , out_len );
	};

	// change fd
	patch[5] = 0x38600000 | File_Init();
	// change branch
	patch[6] = (0x48 << 24) | (((u8*)iosioctl_location - ((u8*)fwrite_location + 0x18)) & 0x3ffffff);

	memcpy(fwrite_location, patch, sizeof(patch));

	return 0;
}

// Can call this after all memory has been setup to output xml memory patch
// Not sure this works, seems it would though
int Fwrite_GetMemoryPatch()
{
	u32 patch[] = {
	0x38800061,			// ioctl = 0x61
	0x7CA62B78,			// in = buffer
	0x7C651B78,			// in_len = len
	0x38E00000,			// out = 0
	0x39000000,			// out_len = 0
	0x38600004,			// fd = 4
	0x4BF4702C			// b IOS_Ioctl( fd , ioctl , in , in_len , out , out_len );
	};
	if (File_GetLogFS() < 0 || !fwrite_location || !iosioctl_location)
		return -1;

	// change fd
	patch[5] = 0x38600000 | File_Init();
	// change branch
	patch[6] = (0x48 << 24) | (((u8*)iosioctl_location - ((u8*)fwrite_location + 0x18)) & 0x3ffffff);

	char outbuffer[0x80];
	sprintf(outbuffer, "<memory offset=\"0x%08X\" value=\"388000617CA62B787C651B7838E0000039000000%08X%08X\" />\n", (u32)fwrite_location, patch[5], patch[6]);
	return File_Log(outbuffer, strlen(outbuffer));

}

