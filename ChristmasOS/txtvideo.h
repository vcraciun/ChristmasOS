#ifndef __TXTVIDEO__
#define __TXTVIDEO__

#include "types.h"

#define PAD_RIGHT 		1
#define PAD_ZERO 		2
#define PRINT_BUF_LEN   12

#define VIDEO_MODE_LIST 0x3000
#define MAXBPPWIDERLIST 0x3400
#define MAXBPPNORMLLIST 0x3800
#define TOTAL_VIDEO_MODES 0x2FFC	

typedef struct _VIDEOINFO{
	word rex;
	word resy;
	byte bpp;
	dword lfb;
	word vmode;
}VideoInfo;

void ClearScreen(byte backColor);
void SetAttr(byte backcolor,byte forecolor);
void GotoXY(int x,int y);
void ClearLine(word backColor,int line);
void ScrollLines(int lines,int skiptop,int skipbot);
word PrepareVideo();

void printchar(char **str, char c);
int prints(char **out, const char *string, int width, int pad);
int printi(char **out, dword i, int b, int sg, int width, int pad, int letbase);
int print(char **out, dword *varg);

int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);

#endif