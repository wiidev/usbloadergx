/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include "language/gettext.h"
#include "networkops.h"
#include "http.h"
#include "FileOperations/fileops.h"
#include "prompts/PromptWindows.h"
#include "prompts/ProgressWindow.h"
#include "utils/ShowError.h"

/****************************************************************************
 * Download a file from a given url with a Progressbar
 ****************************************************************************/
int DownloadFileToMem(const char *url, u8 **inbuffer, u32 *size)
{
	if(strncasecmp(url, "http://", strlen("http://")) != 0)
	{
		ShowError(tr("Not a valid URL"));
		return -1;
	}
	char *path = strchr(url + strlen("http://"), '/');

	if(!path)
	{
		ShowError(tr("Not a valid URL path"));
		return -2;
	}

	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
		ShowError(tr("Not a valid domain"));
		return -3;
	}

	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';

	int connection = GetConnection(domain);

	if(connection < 0)
	{
		ShowError(tr("Could not connect to the server."));
		return -4;
	}

	char header[1024];
	char * ptr = header;
	ptr += sprintf(ptr, "GET %s HTTP/1.1\r\n", path);
	ptr += sprintf(ptr, "Host: %s\r\n", domain);
	ptr += sprintf(ptr, "Referer: %s\r\n", domain);
	ptr += sprintf(ptr, "User-Agent: USB Loader GX\r\n");
	ptr += sprintf(ptr, "Pragma: no-cache\r\n");
	ptr += sprintf(ptr, "Cache-Control: no-cache\r\n");
	ptr += sprintf(ptr, "Connection: close\r\n\r\n");

	char filename[255];
	memset(filename, 0, sizeof(filename));

	int filesize = network_request(connection, header, filename);

	if(filesize <= 0)
	{
		net_close(connection);
		ShowError(tr("Filesize is 0 Byte."));
		return -5;
	}

	int blocksize = 4*1024;

	u8 * buffer = (u8 *) malloc(filesize);
	if(!buffer)
	{
		net_close(connection);
		ShowError(tr("Not enough memory."));
		return -6;
	}

	ProgressCancelEnable(true);
	StartProgress(tr("Downloading file..."), 0, filename, true, true);

	int done = 0;

	while(done < filesize)
	{
		if(ProgressCanceled())
		{
			done = PROGRESS_CANCELED;
			break;
		}

		ShowProgress(done, filesize);

		if(blocksize > filesize - done)
			blocksize = filesize - done;


		s32 read = network_read(connection, buffer+done, blocksize);

		if(read < 0)
		{
			done = -8;
			ShowError(tr("Transfer failed"));
			break;
		}
		else if(!read)
			break;

		done += read;
	}

	ProgressStop();
	ProgressCancelEnable(false);
	net_close(connection);

	if(done < 0)
	{
		free(buffer);
		return done;
	}

	*inbuffer = buffer;
	*size = filesize;

	return done;
}

/****************************************************************************
 * Download a file from a given url to a given path with a Progressbar
 ****************************************************************************/
int DownloadFileToPath(const char *orig_url, const char *dest, bool UseFilename)
{
	if(!orig_url || !dest)
	{
		ShowError(tr("No URL or Path specified."));
		return -2;
	}

	bool addhttp = false;

	if(strncasecmp(orig_url, "http://", strlen("http://")) != 0)
	{
		addhttp = true;
	}

	char url[strlen(orig_url) + (addhttp ? strlen("http://") : 0) + 1];

	if(addhttp)
		snprintf(url, sizeof(url), "http://%s", orig_url);
	else
		strcpy(url, orig_url);

	char *path = strchr(url + strlen("http://"), '/');

	if(!path)
	{
		ShowError(tr("Not a valid URL path"));
		return -2;
	}

	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
		ShowError(tr("Not a valid domain"));
		return -3;
	}

	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';

	int connection = GetConnection(domain);

	if(connection < 0)
	{
		ShowError(tr("Could not connect to the server."));
		return -4;
	}

	char header[1024];
	char * ptr = header;
	ptr += sprintf(ptr, "GET %s HTTP/1.1\r\n", path);
	ptr += sprintf(ptr, "Host: %s\r\n", domain);
	ptr += sprintf(ptr, "Referer: %s\r\n", domain);
	ptr += sprintf(ptr, "User-Agent: WiiXplorer\r\n");
	ptr += sprintf(ptr, "Pragma: no-cache\r\n");
	ptr += sprintf(ptr, "Cache-Control: no-cache\r\n");
	ptr += sprintf(ptr, "Connection: close\r\n\r\n");

	char filename[255];
	memset(filename, 0, sizeof(filename));

	int filesize = network_request(connection, header, filename);

	if(filesize <= 0)
	{
		net_close(connection);
		ShowError(tr("Filesize is %i Byte."), filesize);
		return -5;
	}

	int blocksize = 4*1024;

	u8 *buffer = (u8 *) malloc(blocksize);
	if(!buffer)
	{
		net_close(connection);
		ShowError(tr("Not enough memory."));
		return -6;
	}

	if(UseFilename)
	{
		if(dest[strlen(dest)-1] != '/')
			strcat((char *) dest, "/");

		CreateSubfolder(dest);

		strcat((char *) dest, filename);
	}

	if(!UseFilename && strcmp(filename, "") == 0)
	{
		const char * ptr = strrchr(dest, '/');
		if(ptr) ptr++;
		else ptr = dest;

		snprintf(filename, sizeof(filename), "%s", ptr);
	}

	FILE *file = fopen(dest, "wb");
	if(!file)
	{
		net_close(connection);
		free(buffer);
		ShowError(tr("Cannot write to destination."));
		return -7;
	}

	ProgressCancelEnable(true);
	StartProgress(tr("Downloading file..."), 0, filename, true, true);

	int done = 0;

	while(done < filesize)
	{
		if(ProgressCanceled())
		{
			done = PROGRESS_CANCELED;
			break;
		}

		ShowProgress(done, filesize);

		if(blocksize > filesize - done)
			blocksize = filesize - done;

		s32 read = network_read(connection, buffer, blocksize);

		if(read < 0)
		{
			done = -8;
			ShowError(tr("Transfer failed"));
			break;
		}
		else if(!read)
			break;

		fwrite(buffer, 1, read, file);

		done += read;
	}

	free(buffer);
	ProgressStop();
	net_close(connection);
	fclose(file);
	ProgressStop();
	ProgressCancelEnable(false);

	return done;
}
