#include "mem2alloc.hpp"

#include <ogc/system.h>
#include <algorithm>
#include <string.h>

#define IOS_RELOAD_AREA		0x90200000

class LockMutex
{
	mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
};

void CMEM2Alloc::init(unsigned int size)
{
	m_baseAddress = (SBlock *) std::max(((u32)SYS_GetArena2Lo() + 31) & ~31, IOS_RELOAD_AREA);
	m_endAddress = (SBlock *) ((char *)m_baseAddress + std::min(size * 0x100000, SYS_GetArena2Size() & ~31));
	if (m_endAddress > (SBlock *) 0x93300000) //rest is reserved for usb/usb2/network and other stuff... (0xE0000 bytes)
		m_endAddress = (SBlock *) 0x93300000;
	SYS_SetArena2Lo(m_endAddress);
	LWP_MutexInit(&m_mutex, 0);
}

void CMEM2Alloc::init(void *addr, void *end)
{
	m_baseAddress = (SBlock *)(((u32)addr + 31) & ~31);
	m_endAddress = (SBlock *)((u32)end & ~31);
	LWP_MutexInit(&m_mutex, 0);
}

void CMEM2Alloc::cleanup(void)
{
	LWP_MutexDestroy(m_mutex);
	m_mutex = 0;
	m_first = 0;
	// Try to release the range we took through SYS functions
	if (SYS_GetArena2Lo() == m_endAddress)
		SYS_SetArena2Lo(m_baseAddress);
	m_baseAddress = 0;
	m_endAddress = 0;
}

void CMEM2Alloc::clear(void)
{
	m_first = 0;
	memset(m_baseAddress, 0, (u8 *)m_endAddress - (u8 *)m_endAddress);
}

unsigned int CMEM2Alloc::usableSize(void *p)
{
	return p == 0 ? 0 : ((SBlock *)p - 1)->s * sizeof (SBlock);
}

void *CMEM2Alloc::allocate(unsigned int s)
{
	if (s == 0)
		s = 1;
	//
	LockMutex lock(m_mutex);
	//
	s = (s - 1) / sizeof (SBlock) + 1;
	// First block
	if (m_first == 0)
	{
		if (m_baseAddress + s + 1 >= m_endAddress)
			return 0;
		m_first = m_baseAddress;
		m_first->next = 0;
		m_first->prev = 0;
		m_first->s = s;
		m_first->f = false;
		return (void *)(m_first + 1);
	}
	// Search for a free block
	SBlock *i;
	SBlock *j;
	for (i = m_first; i != 0; i = i->next)
	{
		if (i->f && i->s >= s)
			break;
		j = i;
	}
	// Create a new block
	if (i == 0)
	{
		i = j + j->s + 1;
		if (i + s + 1 >= m_endAddress)
			return 0;
		j->next = i;
		i->prev = j;
		i->next = 0;
		i->s = s;
		i->f = false;
		return (void *)(i + 1);
	}
	// Reuse a free block
	i->f = false;
	// Split it
	if (i->s > s + 1)
	{
		j = i + s + 1;
		j->f = true;
		j->s = i->s - s - 1;
		i->s = s;
		j->next = i->next;
		j->prev = i;
		i->next = j;
		if (j->next != 0)
			j->next->prev = j;
	}
	return (void *)(i + 1);
}

void CMEM2Alloc::release(void *p)
{
	if (p == 0)
		return;

	LockMutex lock(m_mutex);
	SBlock *i = (SBlock *)p - 1;
	i->f = true;

	// If there are no other blocks following yet,
	// set the remaining size to free size. - Dimok
	if(i->next == 0)
		i->s = m_endAddress - i - 1;

	// Merge with previous block
	if (i->prev != 0 && i->prev->f)
	{
		i = i->prev;
		i->s += i->next->s + 1;
		i->next = i->next->next;
		if (i->next != 0)
			i->next->prev = i;
	}
	// Merge with next block
	if (i->next != 0 && i->next->f)
	{
		i->s += i->next->s + 1;
		i->next = i->next->next;
		if (i->next != 0)
			i->next->prev = i;
	}
}

void *CMEM2Alloc::reallocate(void *p, unsigned int s)
{
	SBlock *i;
	SBlock *j;
	void *n;

	if (s == 0)
		s = 1;
	if (p == 0)
		return allocate(s);

	i = (SBlock *)p - 1;
	s = (s - 1) / sizeof (SBlock) + 1;
	{
		LockMutex lock(m_mutex);

		//out of memory /* Dimok */
		if (i + s + 1 >= m_endAddress)
		{
			return 0;
		}

		// Last block
		if (i->next == 0 && i + s + 1 < m_endAddress)
		{
			i->s = s;
			return p;
		}
		// Size <= current size + next block
		if (i->next != 0 && i->s < s && i->next->f && i->s + i->next->s + 1 >= s)
		{
			// Merge
			i->s += i->next->s + 1;
			i->next = i->next->next;
			if (i->next != 0)
				i->next->prev = i;
		}
		// Size <= current size
		if (i->s >= s)
		{
			// Split
			if (i->s > s + 1)
			{
				j = i + s + 1;
				j->f = true;
				j->s = i->s - s - 1;
				i->s = s;
				j->next = i->next;
				j->prev = i;
				i->next = j;
				if (j->next != 0)
					j->next->prev = j;
			}
			return p;
		}
	}
	// Size > current size
	n = allocate(s * sizeof (SBlock));
	if (n == 0)
		return 0;
	memcpy(n, p, i->s * sizeof (SBlock));
	release(p);
	return n;
}

unsigned int CMEM2Alloc::FreeSize()
{
	LockMutex lock(m_mutex);

	if (m_first == 0)
		return (const char *) m_endAddress - (const char *) m_baseAddress;

	SBlock *i;
	unsigned int size = 0;

	for(i = m_first; i != 0; i = i->next)
	{
		if(i->f && i->next != 0)
			size += i->s;

		else if(i->f && i->next == 0)
			size += m_endAddress - i - 1;

		else if(!i->f && i->next == 0)
			size += m_endAddress - i - i->s - 1;
	}

	return size*sizeof(SBlock);
}
