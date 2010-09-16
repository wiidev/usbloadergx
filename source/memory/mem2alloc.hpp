// MEM2 allocator
// Made as a class so i can have 2 sections, one being dedicated to the covers

#ifndef __MEM2ALLOC_HPP
#define __MEM2ALLOC_HPP

#include <ogc/mutex.h>

class CMEM2Alloc
{
public:
	void *allocate(unsigned int s);
	void release(void *p);
	void *reallocate(void *p, unsigned int s);
	void init(unsigned int size);
	void init(void *addr, void *end);
	void cleanup(void);
	void clear(void);
	static unsigned int usableSize(void *p);
	void forceEndAddress(void *newAddr) { m_endAddress = (SBlock *)newAddr; }
	void *getEndAddress(void) const { return m_endAddress; }
	void info(void *&address, unsigned int &size) const { address = m_baseAddress; size = (const char *)m_endAddress - (const char *)m_baseAddress; }
	unsigned int FreeSize();
	//
	CMEM2Alloc(void) : m_baseAddress(0), m_endAddress(0), m_first(0), m_mutex(0) { }
private:
	struct SBlock
	{
		unsigned int s;
		SBlock *next;
		SBlock *prev;
		bool f;
	} __attribute__((aligned(32)));
	SBlock *m_baseAddress;
	SBlock *m_endAddress;
	SBlock *m_first;
	mutex_t m_mutex;
private:
	CMEM2Alloc(const CMEM2Alloc &);
};

#endif // !defined(__MEM2ALLOC_HPP)
