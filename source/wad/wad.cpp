/****************************************************************************
 * Copyright (C) 2013 Cyan
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "prompts/ProgressWindow.h"
#include "FileOperations/fileops.h"
#include "language/gettext.h"
#include "utils/ShowError.h"
#include "utils/tools.h"
#include "wad.h"

extern "C"
{
	void aes_set_key(u8 *key);
	void aes_decrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len);
	void _decrypt_title_key(u8 *tik, u8 *title_key);
}

typedef struct map_entry
{
	char name[8];
	u8 hash[20];
} __attribute__((packed)) map_entry_t;

typedef struct uid_entry {
	u64 title_id;
	u32 uid;
} __attribute__((packed)) uid_entry_t;

Wad::Wad(const char *wadpath, bool prompt)
	: pFile(0), header(0),
	  p_tik(0), p_tmd(0),
	  content_map(0), content_map_size(0),
	  content_start(0)
{
	showPrompt = prompt;
	Open(wadpath);
}

Wad::~Wad()
{
	Close();
}

void Wad::Close(void)
{
	if(pFile)
		fclose(pFile);
	if(header)
		free(header);
	if(p_tik)
		free(p_tik);
	if(p_tmd)
		free(p_tmd);
	if(content_map)
		free(content_map);

	pFile = 0;
	header = 0;
	p_tik = 0;
	p_tmd = 0;
	content_map = 0;
	content_map_size = 0;
}

bool Wad::Open(const char *wadpath)
{
	if(!wadpath)
		return false;

	// Close if another file is opened already
	Close();

	// Open file
	pFile = fopen(wadpath, "rb");
	if(!pFile)
	{
		if(showPrompt)
			ShowError(tr("Can't open file: %s"), wadpath);
		return false;
	}

	// Read wad header
	header = (wadHeader *) malloc(sizeof(wadHeader));
	if(!header)
	{
		if(showPrompt)
			ShowError(tr("Not enough memory."));
		return false;
	}

	if(fread(header, 1, sizeof(wadHeader), pFile) != sizeof(wadHeader))
	{
		if(showPrompt)
			ShowError(tr("Failed to read wad header."));
		return false;
	}

	// Check for sanity
	if(header->header_len != sizeof(wadHeader))
	{
		if(showPrompt)
			ShowError(tr("Invalid wad file."));
		return false;
	}

	u32 offset = round_up( header->header_len, 64 );
	offset += round_up( header->certs_len, 64 );
	if (header->crl_len)
		offset += round_up( header->crl_len, 64 );

	// Read title ticket
	p_tik = (u8 *) malloc(header->tik_len);
	if(!p_tik)
	{
		if(showPrompt)
			ShowError(tr("Not enough memory."));
		return false;
	}

	fseek(pFile, offset, SEEK_SET);

	if(fread(p_tik, 1, header->tik_len, pFile) != header->tik_len)
	{
		if(showPrompt)
			ShowError(tr("Failed to read ticket."));
		return false;
	}

	offset += round_up( header->tik_len, 64 );

	// Read title tmd
	p_tmd = (u8 *) malloc(header->tmd_len);
	if(!p_tik)
	{
		if(showPrompt)
			ShowError(tr("Not enough memory."));
		return false;
	}

	fseek(pFile, offset, SEEK_SET);

	if(fread(p_tmd, 1, header->tmd_len, pFile) != header->tmd_len)
	{
		if(showPrompt)
			ShowError(tr("Failed to read tmd file."));
		return false;
	}

	offset += round_up( header->tmd_len, 64 );

	// Prepare offset for install
	content_start = offset;

	return true;
}

bool Wad::UnInstall(const char *installpath)
{
	if(!installpath || !pFile || !header || !p_tmd || !p_tik)
		return false;

	char filepath[1024];
	tmd *tmd_data = (tmd *) SIGNATURE_PAYLOAD((signed_blob *) p_tmd);

	// trim ending slash
	while(installpath[strlen(installpath)-1] == '/')
	{
		char *pathPtr = strrchr(installpath, '/');
		if(pathPtr) *pathPtr = 0;
	}
	
	int result = true;

	// Remove ticket
	snprintf(filepath, sizeof(filepath), "%s/ticket/%08x/%08x.tik", installpath, (unsigned int)(tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id);
	if(!RemoveFile(filepath))
		result = false;

	// Remove contents / data
	snprintf(filepath, sizeof(filepath), "%s/title/%08x/%08x/", installpath, (unsigned int) (tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id);
	if(!RemoveDirectory(filepath))
		result = false;

	return result;
}

bool Wad::Install(const char *installpath)
{
	if(!installpath || !pFile || !header || !p_tmd || !p_tik)
		return false;

	char filepath[1024];
	u8 title_key[16];
	tmd *tmd_data = (tmd *) SIGNATURE_PAYLOAD((signed_blob *) p_tmd);

	// Create necessary folders if not existing
	snprintf(filepath, sizeof(filepath), "%s/ticket/%08x/", installpath, (unsigned int) (tmd_data->title_id >> 32));
	CreateSubfolder(filepath);

	snprintf(filepath, sizeof(filepath), "%s/title/%08x/%08x/content/", installpath, (unsigned int) (tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id);
	CreateSubfolder(filepath);

	snprintf(filepath, sizeof(filepath), "%s/title/%08x/%08x/data/", installpath, (unsigned int) (tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id);
	CreateSubfolder(filepath);

	// Write ticket file
	snprintf(filepath, sizeof(filepath), "%s/ticket/%08x/%08x.tik", installpath, (unsigned int) (tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id);
	if(!WriteFile(filepath, p_tik, header->tik_len))
		return false;

	// Write tmd file
	snprintf(filepath, sizeof(filepath), "%s/title/%08x/%08x/content/title.tmd", installpath, (unsigned int) (tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id);
	if(!WriteFile(filepath, p_tmd, header->tmd_len))
		return false;

	// Get title key and prepare decryption
	_decrypt_title_key(p_tik, title_key);
	aes_set_key(title_key);

	// Start progress
	ProgressCancelEnable(true);
	StartProgress(0, 0, 0, true, true);

	// Install contents
	bool result = InstallContents(installpath);

	// Stop progress
	ProgressStop();
	ProgressCancelEnable(false);

	if(!result)
		return false;

	// Update /sys/uid.sys
	if(!SetTitleUID(installpath, tmd_data->title_id))
		return false;

	return true;
}

bool Wad::WriteFile(const char *filepath, u8 *buffer, u32 len)
{
	FILE *f = fopen(filepath, "wb");
	if(!f)
	{
		if(showPrompt)
			ShowError(tr("Can't create file: %s"), filepath);
		return false;
	}

	u32 write = fwrite(buffer, 1, len, f);
	fclose(f);

	if(write != len && showPrompt)
		ShowError(tr("Write error on file: %s"), filepath);

	return (write == len);
}

bool Wad::InstallContents(const char *installpath)
{
	const u32 blocksize = 50 * 1024;
	u16 cnt;
	u32 totalDone = 0;
	u32 totalSize = 0;
	u32 offset = content_start;
	char filepath[1024];
	char progressTxt[80];
	u8 iv[16];
	bool userCanceled = false;

	// tmd
	tmd *tmd_data = (tmd *) SIGNATURE_PAYLOAD((signed_blob *) p_tmd);

	// Get total size for progress bar
	for (cnt = 0; cnt < tmd_data->num_contents; cnt++)
	{
		if(tmd_data->contents[cnt].type == 0x8001) {
			// shared content
			int result = CheckContentMap(installpath, &tmd_data->contents[cnt], filepath);
			if(result == 1) // exists already, skip file
				continue;
		}
		totalSize += round_up( tmd_data->contents[cnt].size, 64 );
	}

	for (cnt = 0; cnt < tmd_data->num_contents; cnt++)
	{
		if(ProgressCanceled())
			break;

		if(cnt > 0)
			offset += round_up( tmd_data->contents[cnt-1].size, 64);

		u32 done = 0, len;
		tmd_content *content = &tmd_data->contents[cnt];

		// Encrypted content size
		len = round_up( content->size, 64 );

		// Prepare iv for decryption
		memset(iv, 0, sizeof(iv));
		memcpy(iv, &cnt, 2);

		// Install content
		if(content->type == 0x8001) {
			// shared content
			int result = CheckContentMap(installpath, content, filepath);
			if(result == 1) // exists already, skip file
				continue;

			else if(result < 0) // failure
				return false;
			// else it does not exist...install it
		}
		else {
			// private content
			snprintf(filepath, sizeof(filepath), "%s/title/%08x/%08x/content/%08x.app", installpath, (unsigned int) (tmd_data->title_id >> 32), (unsigned int) tmd_data->title_id, (unsigned int) content->cid);
		}

		// Create file
		FILE *fp = fopen(filepath, "wb");
		if(!fp)
		{
			if(showPrompt)
				ShowError(tr("Can't create file: %s"), filepath);
			return false;
		}

		u8 * inbuf = (u8 *) malloc(blocksize);
		u8 * outbuf = (u8 *) malloc(blocksize);
		if(!inbuf || !outbuf)
		{
			if(showPrompt)
				ShowError(tr("Not enough memory."));
			if(inbuf) free(inbuf);
			if(outbuf) free(outbuf);
			fclose(fp);
			return false;
		}

		snprintf(progressTxt, sizeof(progressTxt), "%s %08x.app", tr("Installing content"), (unsigned int) content->cid);

		// Go to position
		fseek(pFile, offset, SEEK_SET);

		// Install content data
		while (done < len)
		{
			if(ProgressCanceled())
			{
				userCanceled = true;
				break;
			}

			ShowProgress(tr("Installing title..."), progressTxt, 0, totalDone + done, totalSize, true, true);

			// Encrypted data length
			u32 size = (len - done);
			if (size > blocksize)
				size = blocksize;
			
			// Decryted data length
			u32 dec_size = (content->size - done); // Content size not round up to 64
			if (dec_size > blocksize)
				dec_size = blocksize;

			// Read data
			if(fread(inbuf, 1, size, pFile) != size)
				break;

			// Decrypt data
			aes_decrypt(iv, inbuf, outbuf, size);

			// Write data
			if(fwrite(outbuf, 1, dec_size, fp) != dec_size)
				break;

			// Set new iv for next read chunk
			memcpy(iv, inbuf + blocksize - 16, 16);

			// Increase variables
			done += size;
		}

		// done
		free(inbuf);
		free(outbuf);
		fclose(fp);

		// update progress variable
		totalDone += len;

		// Check if the user canceled the install manually
		if(userCanceled)
			return false;
		
		// Check if the read/write process stopped before finishing
		if(done < len)
		{
			if(showPrompt)
				ShowError(tr("File read/write error."));
			return false;
		}
	}

	return true;
}

int Wad::CheckContentMap(const char *installpath, tmd_content *content, char *filepath)
{
	if(!content_map)
	{
		// Get and keep content map in memory
		snprintf(filepath, 1024, "%s/shared1/content.map", installpath);
		if(LoadFileToMem(filepath, &content_map, &content_map_size) < 0 || content_map_size < sizeof(map_entry_t))
		{
			if(showPrompt)
				ShowError(tr("Can't read file: %s"), filepath);
			return -1;
		}

		content_map_size /=  sizeof(map_entry_t);
	}

	map_entry_t *map = (map_entry_t *) content_map;

	for(u32 n = 0; n < content_map_size; n++)
	{
		if(memcmp(map[n].hash, content->hash, 20) == 0)
			return 1; // content exists already
	}

	// Content does not exists, append it.
	u32 next_entry = content_map_size;
	u8 *tmp = (u8 *) realloc(content_map, (next_entry + 1) * sizeof(map_entry_t));
	if(!tmp)
	{
		if(showPrompt)
			ShowError(tr("Not enough memory."));
		return -1;
	}

	content_map = tmp;
	content_map_size++;

	map = (map_entry_t *) content_map;
	sprintf(map[next_entry].name, "%08x", (unsigned int)next_entry);
	memcpy(map[next_entry].hash, content->hash, 20);

	// write new content.map
	snprintf(filepath, 1024, "%s/shared1/content.map", installpath);
	if(!WriteFile(filepath, content_map, content_map_size * sizeof(map_entry_t)))
		return -1;

	snprintf(filepath, 1024, "%s/shared1/%08x.app", installpath, (unsigned int)next_entry);

	return 0;
}

bool Wad::SetTitleUID(const char *installpath, const u64 &tid)
{
	char filepath[1024];
	u8 *uid_sys = NULL;
	u32 uid_sys_size = 0;

	// Read in uid.sys file
	snprintf(filepath, sizeof(filepath), "%s/sys/uid.sys", installpath);

	if(LoadFileToMem(filepath, &uid_sys, &uid_sys_size) < 0 || uid_sys_size < sizeof(uid_entry_t))
	{
		if(showPrompt)
			ShowError(tr("Can't read file: %s"), filepath);
		if(uid_sys) free(uid_sys);
		return false;
	}

	uid_sys_size /=  sizeof(uid_entry_t);
	uid_entry_t *map = (uid_entry_t *) uid_sys;

	for(u32 i = 0; i < uid_sys_size; ++i)
	{
		if(map[i].title_id == tid)
		{
			// title entry exists already
			free(uid_sys);
			return true;
		}
	}

	// title entry does not exists, append it
	u32 next_entry = uid_sys_size;
	u8 *tmp = (u8 *) realloc(uid_sys, (next_entry + 1) * sizeof(uid_entry_t));
	if(!tmp)
	{
		if(showPrompt)
			ShowError(tr("Not enough memory."));
		free(uid_sys);
		return -1;
	}

	uid_sys = tmp;
	uid_sys_size++;

	map = (uid_entry_t *) uid_sys;
	map[next_entry].title_id = tid;
	map[next_entry].uid = map[next_entry-1].uid + 1;

	// write new uid.sys
	bool result = WriteFile(filepath, uid_sys, uid_sys_size * sizeof(uid_entry_t));

	free(uid_sys);

	return result;
}
