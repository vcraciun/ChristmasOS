#include "pit.h"
#include "System.h"
#include "TextArea.h"

TEXTAREA *pittext;

void play_sound(dword nFrequence) 
{
	dword Div;
	byte tmp;

	Div = PIT_FREQ / nFrequence;
	outb(0x43, 0xb6);
	outb(0x42, (byte) (Div) );
	outb(0x42, (byte) (Div >> 8));

	tmp = inb(0x61);
	if (tmp != (tmp | 3)) {
		outb(0x61, tmp | 3);
	}
}

void nosound() 
{
	byte tmp = (inb(0x61) & 0xFC);
	outb(0x61, tmp);
}

void DelayUs(dword MicroSecs)
{
	while (MicroSecs > 0)
	{
		outb(0x80, 0);
		MicroSecs--;
	}
}

void Write1Bit()
{
	outb(0x61, inb(0x61) & 0xFC);
	pittext->AddChar('0');
	DelayUs(2);
	outb(0x61, inb(0x61) | 3);
	pittext->AddChar('1');
	DelayUs(2);
}

void Write0Bit()
{
	outb(0x61, inb(0x61) | 3);
	pittext->AddChar('1');
	DelayUs(2);
	outb(0x61, inb(0x61) & 0xFC);
	pittext->AddChar('0');
	DelayUs(2);
}

void SendPacket(byte tip, dword size, byte* buffer)
{
	dword nFrequence = 500000;

	dword Div, i;
	byte tmp, j;
	byte data = tip << 6;
	dword dword_data = size;

	//play sound
	Div = PIT_FREQ / nFrequence;
	outb(0x43, 0xb6);
	outb(0x42, (byte) (Div) );
	outb(0x42, (byte) (Div >> 8));

	//calculez numarul total de biti afisati
	pittext->AllocPitArray(4 + 64 + size * 16);
	pittext->Repaint();

	//trimite markerul de inceput de transmisie
	outb(0x61, inb(0x61) | 3);
	DelayUs(50);

	//write the type
	for (j=0;j<2;++j)
	{
		switch (data&0x80)
		{
			case 0x80:
				Write1Bit();
				break;
			case 0:
				Write0Bit();
				break;
			
		}
		data<<=1;
	}
	
	//write the size
	for (j=0;j<32;++j)
	{
		switch (dword_data&0x80000000)
		{
			case 0x80000000:
				Write1Bit();
				break;
			case 0:
				Write0Bit();
				break;
		}
		dword_data<<=1;
	}

	//write the content
	for (i=0;i<size;++i)
	{
		data = buffer[i];
		for (j=0;j<8;++j)
		{
			switch (data&0x80)
			{
				case 0x80:
					Write1Bit();
					break;
				case 0:
					Write0Bit();
					break;			
			}
			data<<=1;
		}
	}
	outb(0x61, inb(0x61) | 3);
	DelayUs(50);
	outb(0x61, inb(0x61) & 0xFC);

	//pittext->RepaintPitArray();
}

void SendArrayBuffer(BYTE *buffer,DWORD packets,DWORD eachsize)
{
	DWORD i;
	DWORD header_stuff=0; //packet_index | packets_count | size

	//for (i=0;i<packets;i++)
	//{
		header_stuff=(i<<24)|(packets<<16)|eachsize;
		SendPacket(DATA_ARRAY,header_stuff,buffer);
	//}
}