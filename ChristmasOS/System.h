#ifndef __SYSTEM__
#define __SYSTEM__

#include "types.h"
#include "timer.h"
#include "m_mouse.h"

#define CODE_SEGMENT    0x9A
#define DATA_SEGMENT    0x92
#define SPAWN32BIT_S    0xCF
#define SPAWN16BIT_S    0x00
#define GRAN4G          0xFFFFFFFF
#define GRAN64K         0x0000FFFF
#define BASE_MEM0       0x00000000
#define GDT_PTR_ADDR    0x820
#define GDT_TABLE_ADDR  0x840
#define GDT_DESCRIPTORS 0xF8

struct GDT_ENTRY {
	word limit_low;
	word base_low;
	byte base_middle;
	byte access;
	byte granularity;
	byte base_high;
};

void gdt_set_gate(int num, dword base, dword limit, byte access, byte gran);
void gdt_install();

void memcpy(void *Dest,void *Src, dword length);
void memcpySSE(byte *Dest,byte *Src, dword length);
void memset(void *Dest,byte data, dword length);
void memsetSSED(byte *Dest,dword data0, dword length);
void memsetSSEB(byte *Dest,byte data0, dword length);
bool memcmp(void *Dest,void *Src,dword len);

int strlen(char *data);
char *strstr(char *data,char *subdata);
int strncmp(char *dest,char *src,int len,bool ignorecase);
char *strcpy(char *dst,char *src);
char *strncpy(char *dst,char*src,int n);
char *strcat(char *dest,char *src);
char *strncat(char *dest,char *src,int n);

void  outb(word port, byte data);
void  outw(word port, word data);
void  outd(word port, dword data);
byte  inb(word port);
word  inw(word port);
dword ind(word port);
void  insb(word port,byte *data,dword count);
void  insw(word port,word *data,dword count);
void  insd(word port,dword *data,dword count);
void  outsb(word port,byte *data,dword count);
void  outsw(word port,word *data,dword count);
void  outsd(word port,dword *data,dword count);

void SystemReset();
void halt();

#endif