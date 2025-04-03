#ifndef __PIT__
#define __PIT__

#include "types.h"

#define PIT_FREQ 0x1234DD

#define DATA_BYTE 0
#define DATA_STRING 1
#define DATA_BUFFER 2
#define DATA_ARRAY  3

void play_sound(dword nFrequence);
void nosound();
void DelayUs(dword MicroSecs);
void SendPacket(byte tip, dword size, byte* buffer);
void SendArrayBuffer(BYTE *buffer,DWORD packets,DWORD eachsize);

#endif