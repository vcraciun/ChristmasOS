#include "MemoryManager.h"
#include "System.h"

ALLOCATION_UNIT *vector_table;
dword allocated_chunks_total;
dword all_allocated_mem;
dword next_avail;

void* operator new(size_t size)
{
	void *p=malloc(size);
	return p;
}

void *operator new[](size_t size)
{
	void *p=malloc(size);
	return p;
}

void  operator delete[](void *ptr)
{
	free(ptr);
}

void  operator delete(void *ptr)
{
	free(ptr);
}

void operator delete(void* ptr, unsigned int)
{
	delete ptr;
}

dword GetRAMAmmount()
{
	dword mask=0xAA1111AA;
	dword backup;
	dword i;

	for (i=0x100000;i<0xffffffff;i+=0x100000)
	{
		backup=*(dword*)i;
		*(dword*)i=mask;
		if (*(dword*)i!=mask)
			break;
		*(dword*)i=backup;
	}

	return i;
}

dword GetVideoRAMAmmont(dword *start_addr)
{
	dword mask=0xAA1111AA;
	dword backup;
	dword i;

	for (i=(dword)start_addr;i<0xffffffff;i+=0x100000)
	{
		backup=*(dword*)i;
		*(dword*)i=mask;
		if (*(dword*)i!=mask)
			break;
		*(dword*)i=backup;
	}

	return (i-(dword)start_addr);
}

void InitAllocator()
{
	all_allocated_mem=0;
	allocated_chunks_total=0;
	memsetSSEB((byte*)MEM_ALLOC,0,MEM_TABLE_LEN);
	vector_table=(ALLOCATION_UNIT*)MEM_ALLOC;
}

dword GetFreeRamSize()
{
	return GetRAMAmmount()-all_allocated_mem;
}

dword GetUsedRamSize()
{
	return all_allocated_mem;
}

void *malloc(dword need_size)
{
	dword i;
	ALLOCATION_UNIT *traverse=vector_table;

	if (!allocated_chunks_total)
	{
		vector_table->start_addr=MEM_FREE_BASE;
		vector_table->size=need_size;
		vector_table->next_chunk=(ALLOCATION_UNIT*)(MEM_ALLOC+sizeof(ALLOCATION_UNIT));
		vector_table->avail=1;
		allocated_chunks_total++;
		all_allocated_mem+=need_size;
		next_avail=MEM_FREE_BASE+need_size;
		//clear allocator table
		memsetSSEB((byte*)MEM_ALLOC,0,MEM_TABLE_LEN);
		return (void*)MEM_FREE_BASE;
	}
	else
	do{
		if (traverse->avail==0 && traverse->start_addr>0)
		{
			if (traverse->size>=need_size)
			{
				traverse->avail=1;
				traverse->size=need_size;
				allocated_chunks_total++;
				all_allocated_mem+=need_size;
				return (void*)traverse->start_addr;
			}
		}
		if (traverse->start_addr==0)
		{
			traverse->start_addr=next_avail;
			traverse->size=need_size;
			traverse->avail=1;
			traverse->next_chunk=traverse+sizeof(ALLOCATION_UNIT);
			allocated_chunks_total++;
			all_allocated_mem+=need_size;
			next_avail+=need_size;
			return (void*)traverse->start_addr;
		}
		traverse=traverse->next_chunk;
	}while(1);	
}

void  free(void *mem_allocated)
{
	dword linear_buf=(dword)mem_allocated;
	ALLOCATION_UNIT *traverse=vector_table;
	dword max_count=0;

	do{
		if (traverse->start_addr==linear_buf)
		{
			traverse->avail=0;
			allocated_chunks_total--;
			all_allocated_mem-=traverse->size;
			break;
		}
		if (traverse->avail==1)
			max_count++;
		if (traverse->next_chunk)
			traverse=traverse->next_chunk;
		else
			break;
	}while(max_count<allocated_chunks_total);
}
