#include "txtvideo.h"

word  *textmem =(word*)0xB8000;
byte  attr;
int screenoffset;

void ClearScreen(byte backColor)
{
	int i;
	word data=backColor;
	
	data=(word)((data<<12)|0x0F20);

	for (i=0;i<80*25;i++)
		textmem[i]=data;
}

void SetAttr(byte backcolor,byte forecolor)
{
	attr=(byte)((backcolor<<4)|forecolor);
}

void GotoXY(int x,int y)
{
	screenoffset=x+y*80;
}

void ClearLine(word backColor,int line)
{
	word at=(word)((backColor<<12)|0x0F20);
	int i;

	for (i=0;i<80;i++)
		textmem[line*80+i]=at;
}

void ScrollLines(int lines,int skiptop,int skipbot)
{
	int i;

	if (screenoffset>=(25-skipbot)*80)
	{
		for (i=0;i<(25-skipbot-skiptop)*80;i++)
		{
			textmem[skiptop*80+i]=textmem[(skiptop+lines)*80+i];
		}

		for (i=0;i<lines;i++)
		{
			ClearLine(1,24-skipbot+i);
		}

		screenoffset=(24-skipbot)*80;
	}
}

void printchar(char **str, char c)
{
	word data=attr;
	data=(word)(data<<8);

	if (str) {
		**str = c;
		++(*str);
	}
	else 
		textmem[screenoffset++]=(word)(data|c);
}

int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0;
	register char padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) 
			++len;
		if (len >= width) 
			width = 0;
		else 
			width -= len;
		if (pad & PAD_ZERO) 
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}
	return pc;
}

int printi(char **out, dword i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int neg = 0, pc = 0;
	register char t;
	register dword u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && (long)i < 0) {
		neg = 1;
		u = ~i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (char)(u % b);
		if( t >= 10 )
			t += (char)(letbase - '0' - 10);
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

int print(char **out, dword *varg)
{
	register int width, pad;
	register int pc = 0;
	register char *format = (char*)(*varg++);
	char scr[2];
	//byte old_color,new_color;

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') 
			{
				printchar (out, *format);
				++pc;
				continue;
			}
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = *((char **)varg++);
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)(*varg++);
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
			/*if (*format == 'z') {
				//chage color to this section %z string %z
				if (*(format+1)=='[' && *(format+3)==']')
				{
					old_color=attr;
					if (*(format+2)<='9')
						new_color=*(format+2)-'0';
					else
						if (*(format+2)<='Z')
							new_color=*(format+2)-'A'+10;
						else
							if (*(format+2)<='z')
								new_color=*(format+2)-'a'+10;
					attr=(attr & 0xF0)|(new_color & 0x0F);
					format+=3;
				}
				else
					attr=old_color;
			}*/
		}
		else {
			switch (*format)
			{
			case '\n':
				screenoffset+=80-screenoffset%80;
				ScrollLines(1,1,1);
				break;
			default:
				printchar (out, *format);					
			}
			++pc;
		}
	}
	if (out) 
		**out = '\0';	
	return pc;
}

int printf(const char *format, ...)
{
	register dword *varg = (dword *)(&format);
	return print(0, varg);
}

int sprintf(char *out, const char *format, ...)
{
	int a;
	register dword *varg = (dword *)(&format);
	a=print(&out,varg);
	return a;
}