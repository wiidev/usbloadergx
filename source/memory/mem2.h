// 2 MEM2 allocators, one for general purpose, one for covers
// Aligned and padded to 32 bytes, as required by many functions

#ifndef __MEM2_H_
#define __MEM2_H_

#ifdef __cplusplus
extern "C" {
#endif

#define isMEM2Buffer(p) (((u32) p & 0x10000000) != 0)

void *MEM1_alloc(unsigned int s);
void *MEM1_memalign(unsigned int a, unsigned int s);
void *MEM1_realloc(void *p, unsigned int s);
void MEM1_free(void *p);

void MEM2_init(unsigned int mem2Size);
void MEM2_cleanup(void);
void MEM2_takeBigOnes(bool b);
void *MEM2_alloc(unsigned int s);
void *MEM2_realloc(void *p, unsigned int s);
void MEM2_free(void *p);
unsigned int MEM2_usableSize(void *p);
unsigned int MEM2_freesize();

#ifdef __cplusplus
}
#endif

#endif // !defined(__MEM2_H_)
