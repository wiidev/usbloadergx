#pragma once

#include <sys/stat.h>
#include <gctypes.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define FILE_MODULE_NAME "file"
#define FILE_MODULE_NAME_LENGTH 4

typedef struct _stats
{
	u64 Identifier; // st_ino; on fat, cluster number.
	u64 Size;
	s32 Device;
	s32 Mode;
} Stats;

typedef enum {
	SD_DISK,
	USB_DISK,
	USB2_DISK,
	DISK_NONE
} disk_phys;

typedef enum {
	FS_FAT,
	FS_ELM,
	FS_NTFS,
	FS_EXT2,
	FS_SMB,
	FS_ISFS,
	FS_RIIFS
} disk_fs;

typedef enum {
	IOCTL_InitDisc =	0x30,
	IOCTL_Mount,
	IOCTL_Unmount,
	IOCTL_MountPoint,
	IOCTL_SetDefault,
	IOCTL_SetLogFS,
	IOCTL_GetLogFS,
	IOCTL_Epoch,
	IOCTL_Stat =		0x40,
	IOCTL_CreateFile,
	IOCTL_Delete,
	IOCTL_Rename,
	SEEK_Tell,
	SEEK_Sync,
	IOCTL_CreateDir =	0x50,
	IOCTL_OpenDir,
	IOCTL_NextDir,
	IOCTL_CloseDir,
	IOCTL_Shorten = 	0x60,
	IOCTL_Log =         0x61,
	IOCTL_Context =     0x62,
	IOCTL_CheckPhys,
	IOCTL_GetFreeSpace,
	IOCTL_SetSlotLED,
} file_ioctl;

typedef enum {
	ERROR_SUCCESS = 		0,
	ERROR_NOTOPENED = 		-0x40,
	ERROR_OUTOFMEMORY = 	-0x41,
	ERROR_UNRECOGNIZED =	-0x80,
	ERROR_NOTMOUNTED =		-0x81,
	ERROR_DISKNOTSTARTED =	-0x90,
	ERROR_DISKNOTINSERTED = -0x91,
	ERROR_DISKNOTMOUNTED =	-0x92
} file_error;

int File_Init();
int File_Deinit();

int File_Mount(disk_fs filesystem, const void* options, int length);
int File_MountDisk(disk_fs filesystem, disk_phys disk);
int File_Unmount(int fs);
int File_SetDefault(int fs);
int File_SetDefaultPath(const char* mountpoint);
int File_GetMountPoint(int fs, char* mountpoint, int length);
int File_SetLogFS(int fs);
int File_GetLogFS();
int File_CheckPhysical(int fs);
void File_SetSlotLED(int enabled);

int File_Stat(const char* path, Stats* st);
int File_CreateFile(const char* path);
int File_Delete(const char* path);
int File_Rename(const char* source, const char* dest);

int File_CreateDir(const char* path);
int File_OpenDir(const char* path);
int File_NextDir(int dir, char* path, Stats* st);
int File_CloseDir(int dir);

int File_Open(const char* path, int mode);
int File_Close(int fd);
int File_Read(int fd, void* buffer, int length);
int File_Write(int fd, const void* buffer, int length);
int File_Seek(int fd, int whence, int where);
int File_Tell(int fd);
int File_Sync(int fd);
int File_Log(const void* buffer, int length);
int File_GetFreeSpace(int fs, u64 *free_bytes);

// Filesystem-specific Prototypes
#define FILE_ID_PATH "/mnt/identifier/"
#define FILE_ID_PATH_LEN 16
int File_Open_ID(u64 id, int mode);
int File_RiiFS_Mount(const char* host, int port);
int File_Fat_Mount(disk_phys disk, const char* name);

#ifdef __cplusplus
	}
#endif
