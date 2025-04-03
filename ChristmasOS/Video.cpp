#include "types.h"
#include "Video.h"
#include "Math.h"
#include "txtvideo.h"
#include "System.h"
#include "crt.h"
#include "Memory.h"

VIDEO* VIDEO::instance = 0;

VIDEO* VIDEO::GetInstance(int resy)
{
	if (!instance)
		instance = new VIDEO(resy);
	return instance;
}

void VIDEO::FreeInstance()
{
	delete instance;
}

VIDEO::VIDEO(int _resy)
{
	resy=_resy;

	videomode=PrepareVideo();
	DoRmInt(0x10,0x4F02,videomode|0x4000,0,0);

	second_video_buffer=new byte[GetWidth()*GetHeight()*(GetBPP()>>3)*9];
	sbuf=(dword*)(second_video_buffer+GetWidth()*(GetBPP()>>3)*(3*GetHeight()+1));
}

VIDEO::~VIDEO(){}

void VIDEO::SSE_clrscr(dword color)
{
	memsetSSED((byte*)videomem,color,FB_MAX_SIZE);
}

void VIDEO::set_font(int w,int h,int nchars,byte *tbl)
{
	cfont.width=w;
	cfont.height=h;
	cfont.nrch=nchars;
	cfont.fonttable=tbl;
}

void VIDEO::plot_pixel(int x, int y, dword colour)
{
   int px;
   
   if(x >= FB_WIDTH) 
	   x = FB_WIDTH - 1;
   if(y >= FB_HEIGHT) 
	   y = FB_HEIGHT - 1;
   
   px = x + y * FB_WIDTH;
   
   videomem[px] = colour;
}

dword  VIDEO::get_pixel(int x,int y)
{
	int px;

	px=x+y*FB_WIDTH;
	return videomem[px];
}

void VIDEO::plot_rectangle(int x, int y, int w, int h, bool fill, dword colour)
{
	int w_loop, h_loop;

	if(x > FB_WIDTH) 
		x = FB_WIDTH;
	if(y > FB_HEIGHT) 
		y = FB_HEIGHT;
	if((x + w) > FB_WIDTH) 
		w = FB_WIDTH - x;
	if((y + h) > FB_HEIGHT) 
		h = FB_HEIGHT - y;

	for(h_loop = 0; h_loop < h; h_loop++)
	{
		for(w_loop = 0; w_loop < w; w_loop++)
		{
			if (!fill && (!h_loop || !w_loop || h_loop==h-1 || w_loop==w-1))
				plot_pixel(x + w_loop, y + h_loop, colour);
			else
				if (fill)
					plot_pixel(x + w_loop, y + h_loop, colour);
		}
	}
}

void VIDEO::plot_rectangleSSE(int x, int y, int w, int h, bool fill, dword col)
{
	int w_loop, h_loop;
	dword rsse[4]={col,col,col,col};
	dword *vmm=videomem+y*FB_WIDTH+x;
	dword line=FB_WIDTH*4;

	if(x > FB_WIDTH || x<=0 || w<=0) 
		return;
	if(y > FB_HEIGHT || y<=0 || h<=0) 
		return;
	if((x + w) > FB_WIDTH) 
		return;
	if((y + h) > FB_HEIGHT) 
		return;

	w/=4;

	__asm{
		movdqu xmm0,rsse
		mov esi,vmm		
		mov ecx,h
p1:
		push ecx
		push esi
		mov ecx,w
p2:
		movdqu [esi],xmm0
		add esi,16
		loop p2
		pop esi
		pop ecx
		add esi,line
		loop p1
	}
}

void VIDEO::line_break(void)
{
   txt_x = 0;
   
   if((txt_y + 1) >= FB_WIDTH/cfont.width)
   {
      dword px = 0;
      dword *fb_offset = videomem;
      
      fb_offset += cfont.height * (cfont.width * FB_BYTESPERPIXEL * FB_WIDTH/cfont.width);
      
      while(px < (FB_MAX_SIZE - (dword)fb_offset))
      {
         videomem[px] = fb_offset[px];
         px++;
      }
      
      plot_rectangle(0, FB_HEIGHT - cfont.height, FB_WIDTH, cfont.height, true, FB_BACKGROUND);
   }
   else
      txt_y++;
}

void VIDEO::plot_character(int x, int y, char c, dword colour)
{
   int yloop, xloop;
   
   if(x >= (FB_WIDTH - cfont.width)) 
	   x = FB_WIDTH - cfont.width - 1;
   if(y >= (FB_HEIGHT - cfont.height)) 
	   y = FB_HEIGHT - cfont.height - 1;
   if(c >= cfont.nrch) 
	   c = 0; 

   for(yloop = 0; yloop < cfont.height; yloop++)
   {
      for(xloop = 0; xloop < cfont.width; xloop++)
	  {
         if(cfont.fonttable[c*cfont.height+yloop] & (1 << xloop))
            plot_pixel(x + cfont.width - xloop, y + yloop, colour);
	  }
   }
}

void VIDEO::backspace(void)
{
   int x, y;
   
   if(!txt_x)
   {
      if(txt_y)
      {
         txt_y--;
         txt_x = FB_WIDTH/cfont.width - 1;
      }
      else return;
   }
   else
      txt_x--;
 
   x = txt_x * cfont.width;
   y = txt_y * cfont.height;
   
   plot_rectangle(x, y, cfont.width, cfont.height, true, FB_BACKGROUND);
}

void VIDEO::write_character(char c, dword colour)
{
   int x, y;
   
   x = txt_x * cfont.width;
   y = txt_y * cfont.height;
   
   plot_character(x, y, c, colour);
      
   txt_x++;
   if(txt_x >= FB_WIDTH/cfont.width) 
	   line_break();
}

void VIDEO::write_string(char *str, dword colour)
{
   int x, y;
   
   x = txt_x * cfont.width;
   y = txt_y * cfont.height;
   
   while(*str)
   {
      plot_character(x, y, *str, colour);
      
      str++;
      x += cfont.width;
      
      txt_x++;
      if(txt_x >= FB_WIDTH/cfont.width) 
		  line_break();
   }
   
   line_break();
}

void VIDEO::circle(dword x, dword y, int r, dword c)
{
	int i, j;
	qword yu,yd;
	
	for (i=0-r;i<r;i++)
	{
		yu=sqrt(r*r-i*i);
		plot_pixel(x+i,round(yu),c);
		plot_pixel(x+i,round(-yu),c);
	}
}

void VIDEO::line(dword x1,dword y1,dword x2,dword y2)
{
	int deltax,deltay,x,y;
	int xinc1,xinc2,yinc1,yinc2;
	int den,num,numadd,numpixels;
	int curpixel;

	deltax = abs(x2 - x1);    	// The difference between the x's
	deltay = abs(y2 - y1);    	// The difference between the y's
	x = x1;                   	// Start x off at the first pixel
	y = y1;                   	// Start y off at the first pixel

	if (x2 >= x1)             	// The x-values are increasing
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else                      	// The x-values are decreasing
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1)             	// The y-values are increasing
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else                      	// The y-values are decreasing
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay)     	// There is at least one x-value for every y-value
	{
		xinc1 = 0;              	// Don't change the x when numerator >= denominator
		yinc2 = 0;              	// Don't change the y for every iteration
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;     	// There are more x-values than y-values
	}
	else                      	// There is at least one y-value for every x-value
	{
		xinc2 = 0;              	// Don't change the x for every iteration
		yinc1 = 0;              	// Don't change the y when numerator >= denominator
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;     	// There are more y-values than x-values
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		plot_pixel(x,y,color);
		num += numadd;          	// Increase the numerator by the top of the fraction
		if (num >= den)         	// Check if numerator >= denominator
		{
			num -= den;           	// Calculate the new numerator value
			x += xinc1;           	// Change the x as appropriate
			y += yinc1;           	// Change the y as appropriate
		}
		x += xinc2;             	// Change the x as appropriate
		y += yinc2;             	// Change the y as appropriate
	}
}

void VIDEO::GotoXY(dword x,dword y)
{
	xx=x;
	yy=y;
}

void VIDEO::SetAttr(dword fore,dword back,bool transparent)
{
	color=fore;
	cback=back;
	//transp=transparent;
}

word VIDEO::PrepareVideo()
{
	dword i,j,k;
	VideoInfo *vmlist=(VideoInfo*)(VIDEO_MODE_LIST);
	VideoInfo *vmmaxbppw=(VideoInfo*)MAXBPPWIDERLIST;
	VideoInfo *vmmaxbppn=(VideoInfo*)MAXBPPNORMLLIST;
	dword modes=*(dword*)(TOTAL_VIDEO_MODES);
	byte maxbpp=0;
	word miny=0;
	dword mode=0;
	word vmode=0;

	for (i=0;i<modes;i++)
	{
		if (vmlist[i].bpp>maxbpp)
			maxbpp=vmlist[i].bpp;
	}

	printf("Max BPP found: %d\n",maxbpp);

	j=0;
	k=0;
	for (i=0;i<modes;i++)
	{
		if (vmlist[i].bpp==maxbpp && vmlist[i].rex>700 && (vmlist[i].rex*10/vmlist[i].resy==16))
		{
			vmmaxbppw[j].bpp=vmlist[i].bpp;
			vmmaxbppw[j].lfb=vmlist[i].lfb;
			vmmaxbppw[j].rex=vmlist[i].rex;
			vmmaxbppw[j].resy=vmlist[i].resy;
			vmmaxbppw[j].vmode=vmlist[i].vmode;
			j++;
		}
		else
		if (vmlist[i].bpp==maxbpp && vmlist[i].rex>700 && (((vmlist[i].rex*10)/vmlist[i].resy==12) || ((vmlist[i].rex*10)/vmlist[i].resy==13)))
		{
			vmmaxbppn[k].bpp=vmlist[i].bpp;
			vmmaxbppn[k].lfb=vmlist[i].lfb;
			vmmaxbppn[k].rex=vmlist[i].rex;
			vmmaxbppn[k].resy=vmlist[i].resy;
			vmmaxbppn[k].vmode=vmlist[i].vmode;
			k++;
		}
	}

	if (j>0)
	{

		printf("Best Wide video modes found:\n");
		for (i=0;i<j;i++)
			printf("%4d x %4d bpp=%2d lfb=0x%08X Mode=0x%04X\n",vmmaxbppw[i].rex,vmmaxbppw[i].resy,vmmaxbppw[i].bpp,vmmaxbppw[i].lfb,vmmaxbppw[i].vmode);
		for (i=0;i<j;i++)
		{
			if (vmmaxbppw[i].resy>miny && vmmaxbppw[i].resy<1100)
			{
				miny=vmmaxbppw[i].resy;
				mode=i;
			}
		}
		FB_WIDTH=vmmaxbppw[mode].rex;
		FB_HEIGHT=vmmaxbppw[mode].resy;
		FB_DEPTH=vmmaxbppw[mode].bpp;
		FB_BYTESPERPIXEL=vmmaxbppw[mode].bpp>>3;
		FB_MAX_SIZE=FB_WIDTH*FB_HEIGHT*FB_BYTESPERPIXEL;
		videomem=(dword*)vmmaxbppw[mode].lfb;
		printf("Selected Video-Mode: %d x %d @ %dBPP LFB=0x%08X Mode=0x%04X\n",FB_WIDTH,FB_HEIGHT,FB_DEPTH,vmmaxbppw[mode].lfb,vmmaxbppw[mode].vmode);
		vmode=vmmaxbppw[mode].vmode;
	}
	else
		if (k>0)
		{
			printf("Best Square video modes found:\n");
			for (i=0;i<k;i++)
				printf("%4d x %4d bpp=%2d lfb=0x%08X Mode=0x%04X\n",vmmaxbppn[i].rex,vmmaxbppn[i].resy,vmmaxbppn[i].bpp,vmmaxbppn[i].lfb,vmmaxbppn[i].vmode);
			for (i=0;i<k;i++)
			{
				if (!resy)
				{
					if (vmmaxbppn[i].resy>miny && vmmaxbppn[i].resy<1100)
					{
						miny=vmmaxbppn[i].resy;
						mode=i;
					}
				}
				else
				{
					if (vmmaxbppn[i].resy==resy)
					{
						miny=vmmaxbppn[i].resy;
						mode=i;
					}
				}
			}
			FB_WIDTH=vmmaxbppn[mode].rex;
			FB_HEIGHT=vmmaxbppn[mode].resy;
			FB_DEPTH=vmmaxbppn[mode].bpp;
			FB_BYTESPERPIXEL=vmmaxbppn[mode].bpp>>3;
			FB_MAX_SIZE=FB_WIDTH*FB_HEIGHT*FB_BYTESPERPIXEL;
			videomem=(dword*)vmmaxbppn[mode].lfb;
			printf("Selected Video-Mode: %d x %d @ %dBPP LFB=0x%08X Mode=0x%04X\n",FB_WIDTH,FB_HEIGHT,FB_DEPTH,vmmaxbppn[mode].lfb,vmmaxbppn[mode].vmode);
			vmode=vmmaxbppn[mode].vmode;
		}
		return vmode;
}

void VIDEO::gprintchar(char **str, int c)
{
	if (str) {
		**str = c;
		++(*str);
	}
	else 
	{
		plot_character(xx,yy,c,color);
		xx+=cfont.width;
	}
}


int VIDEO::gprints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

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
			gprintchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		gprintchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		gprintchar (out, padchar);
		++pc;
	}
	return pc;
}

int VIDEO::gprinti(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return gprints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			gprintchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + gprints (out, s, width, pad);
}

void VIDEO::SetTextLimitLeft(dword limit)
{
	text_limit_left=limit;
}

int VIDEO::gprint(char **out, int *varg)
{
	register int width, pad;
	register int pc = 0;
	register char *format = (char *)(*varg++);
	char scr[2];
	dword old_color,new_color;
	int i;

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') 
			{
				gprintchar (out, *format);
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
				pc += gprints (out, s?s:"(null)", width, pad);	
				continue;
			}
			if( *format == 'd' ) {
				pc += gprinti (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += gprinti (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += gprinti (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += gprinti (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += gprints (out, scr, width, pad);
				continue;
			}
			if (*format == 'z') {
				//chage color to this section %z string %z
				if (*(format+1)=='[' && *(format+8)==']')
				{
					old_color=color;
					new_color=0;
					for (i=0;i<6;i++)
					{
						new_color<<=4;
						if (*(format+2+i)<='9')
							new_color|=*(format+2+i)-'0';
						else
							if (*(format+2+i)<='Z')
								new_color|=*(format+2+i)-'A'+10;
							else
								if (*(format+2+i)<='z')
									new_color|=*(format+2+i)-'a'+10;						
					}
					color=new_color;
					format+=8;
				}
				else
					color=old_color;
			}
		}
		else {
			switch (*format)
			{
				case '\n':
					xx=text_limit_left;
					yy+=cfont.height;
					break;
				default:
					gprintchar (out, *format);	
			}
			++pc;
		}
	}
	if (out) 
		**out = '\0';	
	return pc;
}

int VIDEO::gprintf(const char *format, ...)
{
	int rtrn;
	register int *varg = (int *)(&format);
	rtrn = gprint(0, varg);
	//SwapVideoBuffers();
	return rtrn;
}

dword *VIDEO::getlfb(void)
{
	return videomem;
}

dword VIDEO::GetWidth()
{
	return FB_WIDTH;
}

dword VIDEO::GetHeight()
{
	return FB_HEIGHT;
}

FONT VIDEO::GetFont()
{
	return cfont;
}

byte VIDEO::GetBPP()
{
	return FB_DEPTH;
}

dword VIDEO::GetX()
{
	return xx;
}

dword VIDEO::GetY()
{
	return yy;
}

//trebe facut managerul de memorie pentru alocari si double-buffering
//trebe totul scris in buffer secundar, si dat repaint la componenta care necesita, sau intreg ecranul (teste)

void VIDEO::stress_rect()
{
	dword x,y,w,h;
	dword col;

	while (true)
	{
		__asm{
			rdtsc
			mov x,eax
			rdtsc
			mov y,eax
			rdtsc
			mov w,eax
			rdtsc
			mov h,eax
			rdtsc
			mov col,eax
		}

		x=x%FB_WIDTH;
		y=y%FB_HEIGHT;
		w=w%(FB_WIDTH-x);
		w/=4;
		w*=4;
		h=h%(FB_HEIGHT-y);
		//plot_rectangle(x,y,w,h,true,col);
		plot_rectangleSSE(x,y,w,h,true,col);
	}
}

void VIDEO::stress_string()
{
	dword x=0,y=0;
	dword col=0xFFFFFF;
	char ch;
	dword i,j,k=0;

	SSE_clrscr(0x30);

	while (true)
	{
		for (i=0;i<FB_WIDTH;i+=cfont.width)
		{
			if (k%2==1)
				plot_character(i,y,'0',col);
			else
				plot_character(i,y,'1',col);
		}

		y+=cfont.height;
		if (y>=FB_HEIGHT)
		{
			//memcpy((byte*)videomem,(byte*)(videomem+FB_WIDTH*cfont.height),FB_WIDTH*4*(FB_HEIGHT-1));
			memcpySSE((byte*)videomem,(byte*)(videomem+FB_WIDTH*cfont.height),FB_WIDTH*4*(FB_HEIGHT-cfont.height));
			//memset((byte*)(videomem+FB_WIDTH*(FB_HEIGHT-cfont.height)),0x30,FB_WIDTH*4*cfont.height);
			memsetSSED((byte*)(videomem+FB_WIDTH*(FB_HEIGHT-cfont.height)),0x30,FB_WIDTH*4*cfont.height);
			y=FB_HEIGHT-cfont.height;
		}
		k++;
	}
}

void VIDEO::DumpMem(dword start_addr,dword row_length,dword max_lines,dword x,dword y,dword col)
{
	dword i,j;
	dword l,c;

	xx=x;
	yy=y+16;
	color=0xFF0000;
	//line(8*(row_length*3+12),yy);

	xx=x;
	yy=y;
	color=col;

	l=0;
	c=0;
	printf("            ");
	for (i=0;i<row_length;i++)
		printf("%02X ",i);
	yy+=16;
	xx=x;
	for (i=start_addr;i<start_addr+row_length*max_lines;i++)
	{
		if (c%row_length==0)
			gprintf("0x%08X: ",i);
		gprintf("%02X ",*((byte*)i));
		c++;
		if (c==row_length)
		{
			c=0;
			l++;
			xx=x;
			yy+=16;
			if (l==max_lines)
				break;
		}
	}
}

void VIDEO::restoreScreenArea(byte *Src, dword xpos, dword ypos,dword width, dword height,bool use_transp)
{
	dword *SrcD=(dword*)Src;
	dword oldx=xx;
	dword oldy=yy;
	dword w=width;
	dword *vmm=videomem+ypos*FB_WIDTH+xpos;
	dword line=FB_WIDTH*4;

	w/=4;

	__asm{
		mov esi,SrcD		
		mov edi,vmm
		mov ecx,height
_restore1:
		push ecx
		push edi
		mov ecx,w
_restore2:
		movdqu xmm0,[esi]		
		cmp [use_transp],1
		je _xored
		movdqu [edi],xmm0
		jmp _over
_xored:
		movdqu xmm1,[edi]		
		xorps xmm1,xmm0
		movdqu [edi],xmm1
_over:
		add esi,16
		add edi,16
		loop _restore2
		pop edi
		pop ecx
		add edi,line
		loop _restore1
	}

	xx=oldx;
	yy=oldy;	
}

void VIDEO::copyScreenArea(byte *Dest, dword xpos, dword ypos,dword width, dword height)
{
	dword *DestD=(dword*)Dest;
	dword *vmm=videomem+ypos*FB_WIDTH+xpos;
	dword oldx=xx;
	dword oldy=yy;
	dword w=width;
	dword line=FB_WIDTH*4;

	w/=4;

	__asm{
		mov esi,vmm		
		mov edi,DestD
		mov ecx,height
_copy1:
		push ecx
		push esi
		mov ecx,w
_copy2:
		movdqu xmm0,[esi]
		movdqu [edi],xmm0
		add esi,16
		add edi,16
		loop _copy2
		pop esi
		pop ecx
		add esi,line
		loop _copy1
	}

	xx=oldx;
	yy=oldy;
}

void VIDEO::GetBMPDims(byte *buffer,dword *width,dword *height)
{
	*width=*((dword*)(buffer+0x12));
	*height=*((dword*)(buffer+0x16));
}

void VIDEO::PutBMP(dword x,dword y,byte *buffer)
{
	dword width,height;
	word bpp;
	dword linesize,pos,start_pos,raw_bitmapsz;
	dword i,j;
	dword rest,padding;
	dword px;

	dword bcolor = color;
	dword bback = cback;
	//bool btransp = transp;

	width=*((dword*)(buffer+0x12));
	height=*((dword*)(buffer+0x16));
	bpp=*((word*)(buffer+0x1C));
	raw_bitmapsz=*((dword*)(buffer+0x22));
	start_pos=*((dword*)(buffer+0x0A));

	linesize=width*(bpp/8);
	rest=linesize%4;
	if (rest)
	{
		padding=4-rest;
		linesize+=padding;
	}
	else 
		padding=0;	
	pos=raw_bitmapsz-linesize;
	
	Mouse::GetInstance()->HideMouse();
	for (yy=y;yy<y+height-2;yy++)
	{
		for (xx=x;xx<x+width;xx++)
		{
			switch (bpp/8)
			{
				case 3:
					color=*((byte*)(buffer+start_pos+pos+2));
					color<<=16;
					color|=*((word*)(buffer+start_pos+pos));				
					break;
				case 4:
					color=*((dword*)(buffer+start_pos+pos));
					break;
			}
			color&=0x00FFFFFF;
			//px=get_pixel(xx,yy);
			//color = px;
			plot_pixel(xx, yy, color);
			if (bpp/8==3)
			{
				if ((pos+padding)%linesize==0)
				{
					pos+=padding;					
					pos-=2*linesize;	
					pos+=3;
					continue;
				}				
			}
			pos+=bpp/8;
		}		
	}
	Mouse::GetInstance()->ShowMouse();

	color = bcolor;
	cback = bback;
	//transp = btransp;
}
