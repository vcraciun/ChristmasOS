#include "types.h"
#include "System.h"
#include "scancodes.h"
#include "Video.h"
#include "m_mouse.h"
#include "crt.h"

GDT_ENTRY* gdt = (GDT_ENTRY*)GDT_TABLE_ADDR;

void memcpy(void *Dest,void *Src, dword length)
{
	dword i;

	for (i=0;i<length;i++)
		*((byte*)Dest+i)=*((byte*)Src+i);
}

void memset(void *Dest,byte data, dword length)
{
	dword i;

	for (i=0;i<length;i++)
		*((byte*)Dest+i)=data;
}

void memsetSSED(byte *Dest,dword data0, dword length)
{
	dword i,lng=length/16;
	dword data[4]={data0,data0,data0,data0};

	__asm{
		mov ecx,lng
		mov esi,Dest
		movdqu xmm0,data
mmset:
		movdqu [esi],xmm0
		add esi,16
		loop mmset
	}
}

void memsetSSEB(byte *Dest,byte data0, dword length)
{
	dword i,lng=length/16;
	dword data[16]={data0,data0,data0,data0,data0,data0,data0,data0,data0,data0,data0,data0,data0,data0,data0,data0};

	__asm{
		mov ecx,lng
			mov esi,Dest
			movdqu xmm0,data
mmset:
		movdqu [esi],xmm0
			add esi,16
			loop mmset
	}
}

void memcpySSE(byte *Dest,byte *Src, dword length)
{
	dword i;
	dword lng=length/16;

	__asm{
		mov esi,Src
		mov edi,Dest
		mov ecx,lng
move:
		movdqu xmm0,[esi]
		movdqu [edi],xmm0
		add esi,16
		add edi,16
		loop move
	}
}

bool memcmp(void *Dest,void *Src,dword len)
{
	dword i;

	for (i=0;i<len;i++)
		if (((byte*)Dest)[i]!=((byte*)Src)[i])
			return false;

	return true;
}

void outb(word port, byte data)
{
	__asm{
		mov dx,port
		mov al,data
		out dx,al
	}
}

void outw(word port, word data)
{
	__asm{
		mov dx,port
		mov ax,data
		out dx,ax
	}
}

void outd(word port, dword data)
{
	__asm{
		mov dx,port
		mov eax,data
		out dx,eax
	}
}

byte inb(word port)
{
	byte data;

	__asm{
		mov dx,port
		in al,dx
		mov data,al
	}
	return data;
}

word inw(word port)
{
	word data;

	__asm{
		mov dx,port
		in ax,dx
		mov data,ax
	}
	return data;
}

dword ind(word port)
{
	dword data;

	__asm{
		mov dx,port
		in eax,dx
		mov data,eax
	}
	return data;
}

void  insb(word port,byte *data,dword count)
{
	int i;

	for (i=0;i<count;i++)
		data[i]=inb(port);
}

void  insw(word port,word *data,dword count)
{
	int i;

	for (i=0;i<count;i++)
		data[i]=inw(port);
}

void  insd(word port,dword *data,dword count)
{
	int i;

	for (i=0;i<count;i++)
		data[i]=ind(port);
}

void  outsb(word port,byte *data,dword count)
{
	int i;

	for (i=0;i<count;i++)
		outb(port,data[i]);
}

void  outsw(word port,word *data,dword count)
{
	int i;

	for (i=0;i<count;i++)
		outw(port,data[i]);
}

void  outsd(word port,dword *data,dword count)
{
	int i;

	for (i=0;i<count;i++)
		outd(port,data[i]);
}

void SystemReset()
{
	VIDEO* screen = VIDEO::GetInstance();
	int timeout = 5;

	TIMER::GetInstance()->SetCountDown(timeout);
	Mouse::GetInstance()->HideMouse();
	screen->plot_rectangle(screen->GetWidth()/2-120,screen->GetHeight()/2-32,240,64,true,0);
	screen->plot_rectangle(screen->GetWidth()/2-120,screen->GetHeight()/2-32,240,16,true,0x80);
	screen->plot_rectangle(screen->GetWidth()/2-120,screen->GetHeight()/2-32,240,64,false,0x909090);
	screen->plot_rectangle(screen->GetWidth()/2-120,screen->GetHeight()/2-32,240,16,false,0x909090);		
	screen->GotoXY(screen->GetWidth()/2-(strlen("System Reset Request!")/2)*screen->GetFont().width,screen->GetHeight()/2-32);
	screen->SetAttr(0xFFFFFF,0x80,true);
	screen->gprintf("System Reset Request!");
	screen->GotoXY(screen->GetWidth()/2-(strlen("Shuting Down in [5] seconds")/2)*screen->GetFont().width,screen->GetHeight()/2);
	screen->SetAttr(0xFFFFFF,0x000000,true);	
	Mouse::GetInstance()->ShowMouse();

	int initial = 5;
	while (timeout)
	{
		timeout = TIMER::GetInstance()->GetCountDown();
		if (timeout != initial)
		{
			initial = timeout;
			Mouse::GetInstance()->HideMouse();
			screen->plot_rectangle(screen->GetWidth()/2-(strlen("Shuting Down in [5] seconds")/2)*screen->GetFont().width,screen->GetHeight()/2,strlen("Shuting Down in [5] seconds")*screen->GetFont().width,screen->GetFont().height,true,0);
			screen->GotoXY(screen->GetWidth()/2-(strlen("Shuting Down in [5] seconds")/2)*screen->GetFont().width,screen->GetHeight()/2);
			screen->SetAttr(0xFFFFFF,0x000000,true);			
			screen->gprintf("Shuting Down in [%d] seconds",timeout);
			Mouse::GetInstance()->ShowMouse();
		}
	}

	while (inb(STATUS_PORT)&STATUS_INBUF==STATUS_INBUF)
		;
	outb(COMMAND_PORT,CMD_RESENDRESULT);
}

int strlen(char *data)
{
	int i=0;

	while (data[i++])
		;

	if (i)
		return i-1;

	return 0;
}

char *strstr(char *data,char *subdata)
{
	int i,j,k;
	int l1,l2;

	l1=strlen(data);
	l2=strlen(subdata);

	for (i=0;i<l1-l2;i++)
	{
		k=0;
		for (j=0;j<l2;j++)
		{
			if (data[i+j]==subdata[j])
				k++;
		}
		if (k==l2)
			return (data+i);
	}

	return 0;
}

char *strcpy(char *dst,char *src)
{
	int i;

	for (i=0;i<strlen(src);i++)
		dst[i]=src[i];

	dst[strlen(src)]=0;
	return dst;
}

char *strncpy(char *dst,char*src,int n)
{
	int i;

	if (n>strlen(src))
		return 0;

	for (i=0;i<n;i++)
		dst[i]=src[i];

	dst[n]=0;
	return dst;
}

int strncmp(char *dest,char *src,int len,bool ignorecase)
{
	int i,k;
	int l1,l2;

	l1=strlen(dest);
	l2=strlen(src);

	if (l1!=l2)
		return l1-l2;

	for (i=0;i<len;i++)
	{
		if (((dest[i]>='a' && dest[i]<='z') || (dest[i]>='A' && dest[i]<='Z')) && ignorecase)
			k=((dest[i]|0x20) - (src[i]|0x20));
		else
			k=dest[i]-src[i];
		if (k)
			return k;
	}

	return 0;
}

char *strcat(char *dest,char *src)
{
	int i,l;

	l=strlen(dest);
	for (i=0;i<strlen(src);i++)
		dest[l+i]=src[i];
	dest[l+strlen(src)]=0;
	return dest;
}

char *strncat(char *dest,char *src,int n)
{
	int i,l;

	if (strlen(src)<n)
		return 0;
	l=strlen(dest);
	for (i=0;i<n;i++)
		dest[l+i]=src[i];
	dest[l+n]=0;
	return dest;
}

void halt()
{
	__asm{
		hlt
	}
}

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

void gdt_install()
{
	*(word*)GDT_PTR_ADDR = GDT_TABLE_ADDR + GDT_DESCRIPTORS * 8 - 1;
	*(dword*)(GDT_PTR_ADDR + 2) = GDT_TABLE_ADDR;

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, BASE_MEM0, GRAN4G, CODE_SEGMENT, SPAWN32BIT_S);
	gdt_set_gate(2, BASE_MEM0, GRAN4G, DATA_SEGMENT, SPAWN32BIT_S);
	gdt_set_gate(3, BASE_MEM0, GRAN64K, CODE_SEGMENT, SPAWN16BIT_S);
	gdt_set_gate(4, BASE_MEM0, GRAN64K, DATA_SEGMENT, SPAWN16BIT_S);

	gdt_flush();
}
