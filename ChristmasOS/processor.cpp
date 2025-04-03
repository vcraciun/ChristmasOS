#include "types.h"
#include "processor.h"

dword GetProcSpeed()
{
	TIMER* tmr = TIMER::GetInstance();
	qword t1,t2;

	__asm{
		rdtsc
		mov dword ptr t1,eax
		mov dword ptr t1+4,edx
	}

	tmr->timer_wait(1000);

	__asm{
		rdtsc
		mov dword ptr t2,eax
		mov dword ptr t2+4,edx
	}

	return (dword)(t2-t1)/1000000;
}

void GetProcType(dword *d1,dword *d2,dword *d3)
{
	dword r1,r2,r3;
	_asm{
		xor eax,eax
		cpuid
		mov r1,ebx
		mov r2,edx
		mov r3,ecx
	}

	*d1=r1;
	*d2=r2;
	*d3=r3;
}

void GetProcFeatures()
{

}
