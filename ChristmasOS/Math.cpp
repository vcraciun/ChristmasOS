#include "types.h"
#include "Math.h"

int abs(int nr)
{
	return (nr>=0)?nr:0-nr;
}

qword pow(qword nr,qword pwr)
{

	return 0;
}

qword logn(qword nr,dword base)
{

	return 0;
}

qword sqrt(qword nr)
{
	qword local;

	__asm{
		fninit
		fnclex
		fild [nr]
		fsqrt
		fstp local
	}
	return local;
}

qword sqrtn(qword nr, dword base)
{
	return 0;
}

qword sin(qword nr)
{
	return 0;
}

qword cos(qword nr)
{
	return 0;
}

dword round(qword nr)
{
	qword local;

	__asm{
		fninit
		fnclex
		fld [nr]
		frndint
		fstp local
	}

	return (dword)local;
}