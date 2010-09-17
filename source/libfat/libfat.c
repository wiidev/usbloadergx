/*
	libfat.c
	Simple functionality for startup, mounting and unmounting of FAT-based devices.

 Copyright (c) 2006 Michael "Chishm" Chisholm

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/iosupport.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "partition.h"
#include "fatfile.h"
#include "fatdir.h"
#include "lock.h"
#include "mem_allocate.h"
#include "disc_fat.h"

static const devoptab_t dotab_fat = {
	"fat",
	sizeof (FILE_STRUCT),
	_FAT_open_r,
	_FAT_close_r,
	_FAT_write_r,
	_FAT_read_r,
	_FAT_seek_r,
	_FAT_fstat_r,
	_FAT_stat_r,
	_FAT_link_r,
	_FAT_unlink_r,
	_FAT_chdir_r,
	_FAT_rename_r,
	_FAT_mkdir_r,
	sizeof (DIR_STATE_STRUCT),
	_FAT_diropen_r,
	_FAT_dirreset_r,
	_FAT_dirnext_r,
	_FAT_dirclose_r,
	_FAT_statvfs_r,
	_FAT_ftruncate_r,
	_FAT_fsync_r,
	NULL	/* Device data */
};

bool fatMount (const char* name, const DISC_INTERFACE* interface, sec_t startSector, uint32_t cacheSize, uint32_t SectorsPerPage) {
	PARTITION* partition;
	devoptab_t* devops;
	char* nameCopy;

	if(!interface->startup())
		return false;

	if(!interface->isInserted()) {
		interface->shutdown();
		return false;
	}

	devops = _FAT_mem_allocate (sizeof(devoptab_t) + strlen(name) + 1);
	if (!devops) {
		interface->shutdown();
		return false;
	}
	// Use the space allocated at the end of the devoptab struct for storing the name
	nameCopy = (char*)(devops+1);

	// Initialize the file system
	partition = _FAT_partition_constructor (interface, cacheSize, SectorsPerPage, startSector);
	if (!partition) {
		_FAT_mem_free (devops);
		interface->shutdown();
		return false;
	}

	// Add an entry for this device to the devoptab table
	memcpy (devops, &dotab_fat, sizeof(dotab_fat));
	strcpy (nameCopy, name);
	devops->name = nameCopy;
	devops->deviceData = partition;

	AddDevice (devops);

	return true;
}

bool fatMountSimple (const char* name, const DISC_INTERFACE* interface) {
	return fatMount (name, interface, 0, DEFAULT_CACHE_PAGES, DEFAULT_SECTORS_PAGE);
}

void fatUnmount (const char* name) {
	devoptab_t *devops;
	PARTITION* partition;
	const DISC_INTERFACE *disc;

	devops = (devoptab_t*)GetDeviceOpTab (name);
	if (!devops) {
		return;
	}

	// Perform a quick check to make sure we're dealing with a libfat controlled device
	if (devops->open_r != dotab_fat.open_r) {
		return;
	}

	if (RemoveDevice (name) == -1) {
		return;
	}

	partition = (PARTITION*)devops->deviceData;
	disc = partition->disc;
	_FAT_partition_destructor (partition);
	_FAT_mem_free (devops);
	disc->shutdown();
}

bool fatInit (uint32_t cacheSize, bool setAsDefaultDevice) {
	int i;
	int defaultDevice = -1;
	const DISC_INTERFACE *disc;

	for (i = 0;
		_FAT_disc_interfaces[i].name != NULL && _FAT_disc_interfaces[i].getInterface != NULL;
		i++)
	{
		disc = _FAT_disc_interfaces[i].getInterface();
		if (fatMount (_FAT_disc_interfaces[i].name, disc, 0, cacheSize, DEFAULT_SECTORS_PAGE)) {
			// The first device to successfully mount is set as the default
			if (defaultDevice < 0) {
				defaultDevice = i;
			}
		}
	}

	if (defaultDevice < 0) {
		// None of our devices mounted
		return false;
	}

	if (setAsDefaultDevice) {
		char filePath[MAXPATHLEN * 2];
		strcpy (filePath, _FAT_disc_interfaces[defaultDevice].name);
		strcat (filePath, ":/");
#ifdef ARGV_MAGIC
		if ( __system_argv->argvMagic == ARGV_MAGIC && __system_argv->argc >= 1 && strrchr( __system_argv->argv[0], '/' )!=NULL ) {
			// Check the app's path against each of our mounted devices, to see
			// if we can support it. If so, change to that path.
			for (i = 0;
				_FAT_disc_interfaces[i].name != NULL && _FAT_disc_interfaces[i].getInterface != NULL;
				i++)
			{
				if ( !strncasecmp( __system_argv->argv[0], _FAT_disc_interfaces[i].name,
					strlen(_FAT_disc_interfaces[i].name)))
				{
					char *lastSlash;
					strcpy(filePath, __system_argv->argv[0]);
					lastSlash = strrchr( filePath, '/' );

					if ( NULL != lastSlash) {
						if ( *(lastSlash - 1) == ':') lastSlash++;
						*lastSlash = 0;
					}
				}
			}
		}
#endif
		chdir (filePath);
	}

	return true;
}

bool fatInitDefault (void) {
	return fatInit (DEFAULT_CACHE_PAGES, true);
}


