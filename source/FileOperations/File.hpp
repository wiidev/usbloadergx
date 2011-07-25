#ifndef FILE_HPP_
#define FILE_HPP_

#include <stdio.h>
#include <gctypes.h>

class CFile
{
	public:
		CFile();
		CFile(const char * filepath, const char * mode);
		CFile(const u8 * memory, int memsize);
		~CFile();
		int open(const char * filepath, const char * mode);
		int open(const u8 * memory, int memsize);
		void close();
		int read(u8 * ptr, size_t size);
		int write(const u8 * ptr, size_t size);
		int seek(long int offset, int origin);
		long int tell() { return Pos; };
		long int size() { return filesize; };
		void rewind() { seek(0, SEEK_SET); };
	protected:
		FILE * file_fd;
		const u8 * mem_file;
		u64 filesize;
		long int Pos;
};

#endif
