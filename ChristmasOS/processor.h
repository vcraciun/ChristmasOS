#ifndef __PROCESSOR__
#define __PROCESSOR__

#include "types.h"
#include "timer.h"

#define PROCSPEED_ADDRESS 0x2FF8

dword GetProcSpeed();
void GetProcType(dword *d1,dword *d2,dword *d3);
void GetProcFeatures();

#endif