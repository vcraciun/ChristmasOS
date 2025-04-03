#ifndef __MEM_MANAGER__
#define __MEM_MANAGER__

#include "types.h"

//TABLE for ALLOCATOR
#define MEM_ALLOC		0x100000

//ALLOCATOR TABLE SIZE
#define MEM_TABLE_LEN   0x100000
//#define MEM_KERNEL_STACK 0x200000
#define MEM_FREE_BASE   0x300000

typedef struct _ALLOCATION_UNIT{
	dword start_addr;
	dword size;
	dword avail;
	struct _ALLOCATION_UNIT *next_chunk;
}ALLOCATION_UNIT;

void *operator new(size_t size);
void *operator new[](size_t size);
void  operator delete(void *ptr);
void  operator delete[](void *ptr);
void operator delete(void*, unsigned int);

void InitAllocator();
dword GetRAMAmmount();
dword GetVideoRAMAmmont(dword *start_addr);
dword GetFreeRamSize();
dword GetUsedRamSize();
void *malloc(dword need_size);
void free(void *mem_allocated);


#endif