/****************************************************************************
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
#ifndef _WAD_H_
#define _WAD_H_

#include <gccore.h>

/* 'WAD Header' structure */
typedef struct
{
		/* Header length */
		u32 header_len;
		/* WAD type */
		u16 type;
		u16 padding;
		/* Data length */
		u32 certs_len;
		u32 crl_len;
		u32 tik_len;
		u32 tmd_len;
		u32 data_len;
		u32 footer_len;
} ATTRIBUTE_PACKED wadHeader;

class Wad
{
public:
	Wad(const char *wadpath = 0);
	virtual ~Wad();
	bool Open(const char *wadpath);
	void Close(void);
	bool Install(const char *installpath);
	bool UnInstall(const char *installpath);
private:
	bool InstallContents(const char *installpath);
	int CheckContentMap(const char *installpath, tmd_content *content, char *filepath);
	bool WriteFile(const char *filepath, u8 *buffer, u32 len);
	bool SetTitleUID(const char *intallpath, const u64 &tid);

	FILE *pFile;
	wadHeader *header;
	u8 *p_tik, *p_tmd;
	u8 *content_map;
	u32 content_map_size;
	u32 content_start;
};

#endif
