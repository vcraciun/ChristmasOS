#ifndef __VIDEO__
#define __VIDEO__

#include "types.h"

#define FB_BACKGROUND 0x00FFFFFF
#define FB_FOREGROUND 0x00000000

#define PAD_RIGHT 		1
#define PAD_ZERO 		2
#define PRINT_BUF_LEN   12

class VIDEO{
	private:
		dword *videomem;
		int   FB_WIDTH;			
		int   FB_HEIGHT;			
		byte  FB_DEPTH;			
		int   FB_MAX_SIZE;		
		int   FB_BYTESPERPIXEL;
		int   txt_x,txt_y;
		FONT  cfont;
		dword xx,yy;
		dword color, cback;
		//bool  transp;
		int resy;
		int videomode;

		int gprints(char **out, const char *string, int width, int pad);
		void gprintchar(char **str, int c);
		int gprinti(char **out, int i, int b, int sg, int width, int pad, int letbase);
		int gprint(char **out, int *varg);

		VIDEO(int _resy=0);
		~VIDEO();

		static VIDEO* instance;

	public:
		static VIDEO* GetInstance(int resy = 0);
		static void FreeInstance();

		dword *getlfb();
		dword GetWidth();
		dword GetHeight();
		byte  GetBPP();
		FONT  GetFont();
		dword GetX();
		dword GetY();
		void  GotoXY(dword x,dword y);
		void  SetTextLimitLeft(dword limit);
		void  SetAttr(dword fore,dword back,bool transparent);
		void  SSE_clrscr(dword color);
		void  plot_pixel(int x, int y, dword colour);
		dword get_pixel(int x,int y);
		void  plot_rectangle(int x, int y, int w, int h, bool fill, dword colour);
		void  plot_rectangleSSE(int x, int y, int w, int h, bool fill, dword colour);
		void  line_break(void);
		void  plot_character(int x, int y, char c, dword colour);
		void  backspace(void);
		void  write_character(char c, dword colour);
		void  write_string(char *str, dword colour);
		void  circle(dword x, dword y, int r, dword c);
		void  line(dword x1,dword y1,dword x2,dword y2);
		void  set_font(int w,int h,int nchars,byte *tbl);
		void  DumpMem(dword start_addr,dword row_length,dword max_lines,dword x,dword y,dword col);
		word  PrepareVideo();
		int   gprintf(const char *format, ...);
		void  copyScreenArea(byte *Dest, dword xpos, dword ypos,dword width, dword height);
		void  restoreScreenArea(byte *Src, dword xpos, dword ypos,dword width, dword height,bool use_transp);
		void  GetBMPDims(byte *buffer,dword *width,dword *height);
		void  PutBMP(dword x,dword y,byte *buffer);

		void stress_rect();  
		void stress_string();

		byte  *second_video_buffer;
		dword *sbuf;
		dword text_limit_left;
};

#endif