/*
 miniunz.c
 Version 1.01e, February 12th, 2005

 Copyright (C) 1998-2005 Gilles Vollant
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
# include <unistd.h>
# include <utime.h>

#include "miniunz.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

static int mymkdir(const char* dirname)
{
	int ret = 0;
	ret = mkdir(dirname, 0775);
	return ret;
}

int makedir(char *newdir)
{
	char *buffer;
	char *p;
	int len = (int) strlen(newdir);

	if (len <= 0) return 0;

	buffer = (char*) malloc(len + 1);
	strcpy(buffer, newdir);

	if (buffer[len - 1] == '/')
	{
		buffer[len - 1] = '\0';
	}
	if (mymkdir(buffer) == 0)
	{
		free(buffer);
		return 1;
	}

	p = buffer + 1;
	while (1)
	{
		char hold;

		while (*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((mymkdir(buffer) == -1) && (errno == ENOENT))
		{
			//			printf("couldn't create directory %s\n",buffer);
			free(buffer);
			return 0;
		}
		if (hold == 0) break;
		*p++ = hold;
	}
	free(buffer);
	return 1;
}

static char *fullfilename(const char *basedir, char *filename)
{
	char *file = (char *) malloc(strlen(basedir) + strlen(filename) + 1);
	if (basedir == NULL)
	{
		strcpy(file, filename);
	}
	else
	{
		if (basedir[strlen(basedir) - 1] == '/')
		{
			sprintf(file, "%s%s", basedir, filename);
		}
		else
		{
			sprintf(file, "%s/%s", basedir, filename);
		}
	}
	return file;
}

static int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite,
		const char* password, const char *basedir)
{
	char filename_inzip[256];
	char* filename_withoutpath;
	char* filename_withpath;
	char* p;
	int err = UNZ_OK;
	FILE *fout = NULL;
	void* buf;
	uInt size_buf;

	unz_file_info file_info;
	err = unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if (err != UNZ_OK)
	{
		//		printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return err;
	}

	size_buf = WRITEBUFFERSIZE;
	buf = (void*) malloc(size_buf);
	if (buf == NULL)
	{
		//		printf("Error allocating memory\n");
		return UNZ_INTERNALERROR;
	}

	p = filename_withoutpath = filename_inzip;
	filename_withpath = fullfilename(basedir, filename_inzip);
	while ((*p) != '\0')
	{
		if (((*p) == '/') || ((*p) == '\\')) filename_withoutpath = p + 1;
		p++;
	}

	if ((*filename_withoutpath) == '\0')
	{
		if ((*popt_extract_without_path) == 0)
		{

			// Fix the path, this will fail if the directoryname is the same as the first filename in the zip
			char *path = (char *) malloc(strlen(filename_withpath));
			strcpy(path, filename_withpath);
			char *ptr = strstr(path, filename_withoutpath);
			*ptr = '\0';

			//			printf("creating directory: %s\n",path);
			mymkdir(path);

			free(path);
		}
	}
	else
	{
		char* write_filename;
		int skip = 0;

		if ((*popt_extract_without_path) == 0)
			write_filename = filename_withpath;
		else write_filename = filename_withoutpath;

		err = unzOpenCurrentFilePassword(uf, password);
		if (err != UNZ_OK)
		{
			//			printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
		}

		if (((*popt_overwrite) == 0) && (err == UNZ_OK))
		{
			char rep = 0;
			FILE* ftestexist;
			ftestexist = fopen(write_filename, "rb");
			if (ftestexist != NULL)
			{
				fclose(ftestexist);
				do
				{
					char answer[128];
					int ret;

					//					printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
					ret = scanf("%1s", answer);
					if (ret != 1)
					{
						exit(EXIT_FAILURE);
					}
					rep = answer[0];
					if ((rep >= 'a') && (rep <= 'z')) rep -= 0x20;
				} while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));
			}

			if (rep == 'N') skip = 1;

			if (rep == 'A') *popt_overwrite = 1;
		}

		if ((skip == 0) && (err == UNZ_OK))
		{
			fout = fopen(write_filename, "wb");

			/* some zipfile don't contain directory alone before file */
			if ((fout == NULL) && ((*popt_extract_without_path) == 0) && (filename_withoutpath
					!= (char*) filename_inzip))
			{
				char c = *(filename_withoutpath - 1);
				*(filename_withoutpath - 1) = '\0';

				// Fix the path, this will fail if the directoryname is the same as the first filename in the zip
				char *path = (char *) malloc(strlen(write_filename));
				strcpy(path, write_filename);
				char *ptr = strstr(path, filename_withoutpath);
				*ptr = '\0';
				makedir(path);
				free(path);

				*(filename_withoutpath - 1) = c;
				fout = fopen(write_filename, "wb");
			}

			if (fout == NULL)
			{
				//				printf("error opening %s\n",write_filename);
			}
		}

		if (fout != NULL)
		{
			//			printf(" extracting: %s\n",write_filename);

			do
			{
				err = unzReadCurrentFile(uf, buf, size_buf);
				if (err < 0)
				{
					//					printf("error %d with zipfile in unzReadCurrentFile\n",err);
					break;
				}
				if (err > 0) if (fwrite(buf, err, 1, fout) != 1)
				{
					//						printf("error in writing extracted file\n");
					err = UNZ_ERRNO;
					break;
				}
			} while (err > 0);
			if (fout) fclose(fout);

		}

		if (err == UNZ_OK)
		{
			err = unzCloseCurrentFile(uf);
			if (err != UNZ_OK)
			{
				//				printf("error %d with zipfile in unzCloseCurrentFile\n",err);
			}
		}
		else unzCloseCurrentFile(uf); /* don't lose the error */
	}
	free(filename_withpath);
	free(buf);
	return err;
}

int extractZip(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char* password, const char *basedir)
{
	uLong i;
	unz_global_info gi;
	int err;

	err = unzGetGlobalInfo(uf, &gi);
	if (err != UNZ_OK)
	//		printf("error %d with zipfile in unzGetGlobalInfo \n",err);

	for (i = 0; i < gi.number_entry; i++)
	{
		if (do_extract_currentfile(uf, &opt_extract_without_path, &opt_overwrite, password, basedir) != UNZ_OK) break;

		if ((i + 1) < gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err != UNZ_OK)
			{
				//				printf("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	return 0;
}

int extractZipOnefile(unzFile uf, const char* filename, int opt_extract_without_path, int opt_overwrite,
		const char* password)
{
	if (unzLocateFile(uf, filename, CASESENSITIVITY) != UNZ_OK)
	{
		//		printf("file %s not found in the zipfile\n",filename);
		return 2;
	}

	if (do_extract_currentfile(uf, &opt_extract_without_path, &opt_overwrite, password, NULL) == UNZ_OK)
		return 0;
	else return 1;
}
