#include "files.h"

#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#ifdef GEKKO
#include <stdlib.h>
#include <gccore.h>
#define os_open IOS_Open
#define os_close IOS_Close
#define os_read IOS_Read
#define os_write IOS_Write
#define os_ioctl IOS_Ioctl
#define os_ioctlv IOS_Ioctlv
#define os_seek IOS_Seek
// libogc will take care of syncing
#define os_sync_after_write(a,b)
#define os_sync_before_read(a,b)
#define Alloc malloc
#define Realloc(a, b, c) realloc(a, b)
#define Dealloc free
#else
#include <syscalls.h>
#include <mem.h>
#define MEM_VIRTUAL_TO_PHYSICAL(a) (a)
#endif

static u32 ioctlbuffer[0x20];
static ioctlv ioctlvbuffer[0x04];
static int file_fd = -1;
static Stats _st[2] __attribute__((aligned(32)));

int File_Init()
{
	if (file_fd < 0) {
		file_fd = os_open(FILE_MODULE_NAME, 0);
#ifdef GEKKO
		if (file_fd >= 0) {
			time_t epoch = time(NULL);
			memcpy(ioctlbuffer, &epoch, sizeof(time_t));
			os_ioctl(file_fd, IOCTL_Epoch, ioctlbuffer, sizeof(time_t), NULL, 0);
		}
#endif
	}

	return file_fd;
}

int File_Deinit()
{
	if (file_fd >= 0)
		os_close(file_fd);
	file_fd = -1;

	return 0;
}

static int FileMountDisk(disk_phys disk)
{
	ioctlbuffer[0] = disk;
	os_sync_after_write(ioctlbuffer, 0x04);
	return os_ioctl(file_fd, IOCTL_InitDisc, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

int File_MountDisk(disk_fs filesystem, disk_phys disk)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int ret = FileMountDisk(disk);
	if (ret < 0)
		return ret;

	return File_Mount(filesystem, &ret, sizeof(ret));
}

int File_Mount(disk_fs filesystem, const void* options, int optionslen)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	ioctlbuffer[0] = filesystem;
	os_sync_after_write(ioctlbuffer, 0x04);
	os_sync_after_write((void*)options, optionslen);
	return os_ioctl(file_fd, IOCTL_Mount, ioctlbuffer, sizeof(ioctlbuffer), (void*)options, optionslen);
}

int File_Unmount(int fs)
{
	ioctlbuffer[0] = fs;
	os_sync_after_write(ioctlbuffer, 0x04);
	return os_ioctl(file_fd, IOCTL_Unmount, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

int File_SetDefault(int fs)
{
	ioctlbuffer[0] = fs;
	os_sync_after_write(ioctlbuffer, 0x04);
	return os_ioctl(file_fd, IOCTL_SetDefault, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

int File_SetDefaultPath(const char* mountpoint)
{
	int size = strlen(mountpoint) + 1;
	os_sync_after_write((void*)mountpoint, size);
	return os_ioctl(file_fd, IOCTL_SetDefault, NULL, 0, (void*)mountpoint, size);
}

int File_SetLogFS(int fs)
{
	ioctlbuffer[0] = fs;
	os_sync_after_write(ioctlbuffer, 0x04);
	return os_ioctl(file_fd, IOCTL_SetLogFS, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

int File_GetLogFS()
{
	if (file_fd >= 0)
		return os_ioctl(file_fd, IOCTL_GetLogFS, NULL, 0, NULL, 0);
	else
		return -1;
}

int File_GetMountPoint(int fs, char* mountpoint, int length)
{
	ioctlbuffer[0] = fs;
	os_sync_after_write(ioctlbuffer, 0x04);
	int ret = os_ioctl(file_fd, IOCTL_MountPoint, ioctlbuffer, sizeof(ioctlbuffer), mountpoint, length);
	os_sync_before_read(mountpoint, length);
	return ret;
}

int File_Stat(const char* path, Stats* st)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int ret;
	int len = strlen(path) + 1;
	os_sync_after_write((void*)path, len);
	ret = os_ioctl(file_fd, IOCTL_Stat, (void*)path, len, _st, sizeof(Stats));

	os_sync_before_read(_st, sizeof(Stats));
	memcpy(st, _st, sizeof(Stats));
	return ret;
}

int File_CreateFile(const char* path)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int len = strlen(path) + 1;
	os_sync_after_write((void*)path, len);
	return os_ioctl(file_fd, IOCTL_CreateFile, (void*)path, len, NULL, 0);
}

int File_Delete(const char* path)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int len = strlen(path) + 1;
	os_sync_after_write((void*)path, len);
	return os_ioctl(file_fd, IOCTL_Delete, (void*)path, len, NULL, 0);
}

int File_Rename(const char* source, const char* dest)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	os_sync_after_write((void*)source, strlen(source) + 1);
	os_sync_after_write((void*)dest, strlen(dest) + 1);
	ioctlbuffer[0] = (int)MEM_VIRTUAL_TO_PHYSICAL(source);
	ioctlbuffer[1] = (int)MEM_VIRTUAL_TO_PHYSICAL(dest);
	os_sync_after_write(ioctlbuffer, 0x08);
	return os_ioctl(file_fd, IOCTL_Rename, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

int File_CreateDir(const char* path)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int len = strlen(path) + 1;
	os_sync_after_write((void*)path, len);
	return os_ioctl(file_fd, IOCTL_CreateDir, (void*)path, len, NULL, 0);
}

int File_OpenDir(const char* path)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int len = strlen(path) + 1;
	os_sync_after_write((void*)path, len);
	return os_ioctl(file_fd, IOCTL_OpenDir, (void*)path, len, NULL, 0);
}

int File_NextDir(int dir, char* path, Stats* st)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	int ret;
	ioctlbuffer[0] = dir;
	os_sync_after_write(ioctlbuffer, 0x04);

	ioctlvbuffer[0].data = ioctlbuffer;
	ioctlvbuffer[0].len = 0x04;
	ioctlvbuffer[1].data = path;
	ioctlvbuffer[1].len = MAXPATHLEN;
	ioctlvbuffer[2].data = _st;
	ioctlvbuffer[2].len = sizeof(Stats);
	os_sync_after_write(ioctlvbuffer, sizeof(ioctlvbuffer));
	ret = os_ioctlv(file_fd, IOCTL_NextDir, 1, 2, ioctlvbuffer);
	os_sync_before_read(path, MAXPATHLEN);
	os_sync_before_read(_st, sizeof(Stats));
	if (st)
		memcpy(st, _st, sizeof(Stats));

	return ret;
}

int File_CloseDir(int dir)
{
	if (file_fd < 0)
		return ERROR_NOTOPENED;

	ioctlbuffer[0] = dir;
	os_sync_after_write(ioctlbuffer, 0x04);
	return os_ioctl(file_fd, IOCTL_CloseDir, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

int File_Open(const char* path, int mode)
{
	static char short_path[0x20] ATTRIBUTE_ALIGN(32);
	char fpath[MAXPATHLEN];

	strcpy(fpath, FILE_MODULE_NAME);
	strcat(fpath, path);

	path = fpath;
	if (strlen(fpath)+1 > IPC_MAXPATH_LEN) {
		os_sync_after_write(fpath, strlen(fpath) + 1);
		if (os_ioctl(file_fd, IOCTL_Shorten, (void*)fpath, strlen(fpath)+1, short_path, sizeof(short_path))>=0) {
			path = short_path;
		}
	}

	os_sync_after_write(path, strlen(path) + 1);
	return os_open(path, mode);
}

int File_Close(int fd)
{
	return os_close(fd);
}

int File_Read(int fd, void* buffer, int length)
{
	int ret = os_read(fd, buffer, length);
	os_sync_before_read(buffer, length);
	return ret;
}

int File_Write(int fd, const void* buffer, int length)
{
	os_sync_after_write((void*)buffer, length);
	return os_write(fd, buffer, length);
}

int File_Seek(int fd, int where, int whence)
{
	return os_seek(fd, where, whence);
}

int File_Tell(int fd)
{
	return os_seek(fd, 0, SEEK_Tell);
}

int File_Sync(int fd)
{
	return os_seek(fd, 0, SEEK_Sync);
}

int File_Log(const void* buffer, int length)
{
	if (file_fd >= 0 && length>0)
	{
		os_sync_after_write(buffer, length);
		return os_ioctl(file_fd, IOCTL_Log, (void*)buffer, length, NULL, 0);
	}
	return 0;
}

int File_GetFreeSpace(int fs, u64 *free_bytes)
{
	int ret;
	if (file_fd<0)
		return -1;

	ioctlbuffer[0] = fs;
	os_sync_after_write(ioctlbuffer, 4);
	ret = os_ioctl(file_fd, IOCTL_GetFreeSpace, ioctlbuffer, 4, ioctlbuffer+8, sizeof(u64));
	os_sync_before_read(ioctlbuffer+8, sizeof(u64));
	memcpy(free_bytes, ioctlbuffer+8, sizeof(u64));

	return ret;
}

int File_CheckPhysical(int fs)
{
	ioctlbuffer[0] = fs;
	os_sync_after_write(ioctlbuffer, 0x04);
	return os_ioctl(file_fd, IOCTL_CheckPhys, ioctlbuffer, sizeof(ioctlbuffer), NULL, 0);
}

void File_SetSlotLED(int enabled)
{
	ioctlbuffer[0] = enabled;
	os_sync_after_write(ioctlbuffer, 0x04);
	os_ioctl(file_fd, IOCTL_SetSlotLED, ioctlbuffer, sizeof(u32), NULL, 0);
}

static const char HEX_CHARS[] = "0123456789abcdef";
static void IntToHex(char* dest, u64 num, int length)
{
	for (int i = length - 1; i >= 0; i--)
		*(dest++) = HEX_CHARS[(num >> (i * 4)) & 0x0F];
}

int File_Open_ID(u64 id, int mode)
{
	char path[0x40];
	strcpy(path, FILE_ID_PATH);
	IntToHex(path + FILE_ID_PATH_LEN, id, 0x10);
	path[FILE_ID_PATH_LEN + 0x10] = '\0';
	return File_Open(path, mode);
}

int File_RiiFS_Mount(const char* host, int port)
{
	char data[MAXPATHLEN];

	memcpy(data, &port, sizeof(int));
	strcpy(data + sizeof(int), host);
	return File_Mount(FS_RIIFS, data, sizeof(int) + strlen(host) + 1);
}

int File_Fat_Mount(disk_phys disk, const char* name)
{
	int ret = FileMountDisk(disk);
	if (ret < 0)
		return ret;

	char options[0x40];
	memcpy(options, &ret, sizeof(u32));
	strncpy(options + 4, name, 0x40 - 4 - 1);
	return File_Mount(FS_FAT, options, 4 + strlen(name) + 1);
}
