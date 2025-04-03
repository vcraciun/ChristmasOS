#include "MemoryManager.h"
#include "System.h"
#include "pit.h"
#include "Disk.h"
#include "TextArea.h"

 int _purecall(){
	return 0;
}

TEXTAREA::TEXTAREA(dword left,dword top,dword width,dword height,dword totallines)
{
	static dword id=1;

	x=left;
	y=top;
	w=width;
	h=height;
	caption=0;
	screen_under=0;
	ID=id;
	id++;
	active=false;
	first_time=0;
	lines=totallines;
	start_line=0;
	end_line=0;
	text=0;
	cline=0;
	colon_index=0;
}

TEXTAREA::~TEXTAREA(){}

void TEXTAREA::ClearAll()
{
	int i;

	start_line=0;
	end_line=0;
	for (i=0;i<cline;i++)
	{
		if (text[i])
			memset((byte*)text[i],0,MAX_LINE_SIZE);
	}
	cline=0;
	colon_index=0;
}

void TEXTAREA::AllocPitArray(dword chars)
{
	if (pitArray != 0)
	{
		free(pitArray);
	}	
	pitArray=(char*)malloc(chars);
	pitArrNoOfElems = 0;
}


void TEXTAREA::AddLine(char *line)
{
	int i,l;

	l=strlen(line);

	if (text==0)
	{
		start_line=0;
		end_line=0;
		//text=(char**)memalloc(lines*4);
		text=(char**)malloc(lines*4);
		for (i=0;i<lines;i++)
		{
			text[i]=(char*)malloc(MAX_LINE_SIZE);
			memset((byte*)text[i],0,MAX_LINE_SIZE);
		}

		memcpy((byte*)text[cline],(byte*)line,(l>(w/cfont.width))?w/cfont.width:l);
		cline++;
	}
	else
	{
		memcpy((byte*)text[cline],(byte*)line,(l>(w/cfont.width))?w/cfont.width:l);
		cline++;
		if (cline<(h/cfont.height))
			end_line=cline;
		else
		{
			start_line++;
			end_line=cline;
		}
	}
	Repaint();
}

void TEXTAREA::AddChar(char data)
{
	pitArray[pitArrNoOfElems++] = data;
}

void TEXTAREA::SetColors(dword bc,dword fc,dword gc,dword btc,dword ftc)
{
	backcolor=bc;
	forecolor=fc;
	gridcolor=gc;
	backtitlecolor=btc;
	foretitlecolor=ftc;
}

void TEXTAREA::SetCaption(char *title)
{
	caption=(char*)malloc(strlen(title)+1);
	memcpy((byte*)caption,(byte*)title,strlen(title));
}

void TEXTAREA::SetActive(bool isact)
{
	active=isact;
}

bool TEXTAREA::IsActive()
{
	return active;
}

dword TEXTAREA::GetID()
{
	return ID;
}

void TEXTAREA::set_font(dword width,dword height,dword chars,byte *tbl)
{
	cfont.width=width;
	cfont.height=height;
	cfont.nrch=chars;
	cfont.fonttable=tbl;	
	lineNoOfChars = w / cfont.width;
	noOfLines = h / cfont.height;
}

void TEXTAREA::Destroy()
{
	VIDEO::GetInstance()->restoreScreenArea(screen_under, x, y, w, h, 0);
	active=0;
}

dword TEXTAREA::GetStartLine()
{
	return start_line;
}

dword TEXTAREA::GetEndLine()
{
	return end_line;
}

dword TEXTAREA::GetCurrentLine()
{
	return cline;
}

void TEXTAREA::Repaint()
{
	int i;
	FONT backup;
	dword oldx,oldy;
	VIDEO* screen = VIDEO::GetInstance();

	Mouse::GetInstance()->HideMouse();

	if (active)
	{
		backup=screen->GetFont();
		oldx=screen->GetX();
		oldy=screen->GetY();
		screen->set_font(cfont.width,cfont.height,cfont.nrch,cfont.fonttable);
		if (first_time==0)
		{
			screen_under=(byte*)malloc(w*h*(screen->GetBPP()>>3));
			screen->copyScreenArea(screen_under,x,y,w,h);
			screen->plot_rectangle(x-2,y-cfont.height-4,w+4,cfont.height+4,true,backtitlecolor);
			screen->plot_rectangle(x-2,y,w+4,h+4,true,backcolor);
			screen->plot_rectangle(x-2,y-cfont.height-4,w+4,cfont.height+4,true,gridcolor);
			screen->plot_rectangle(x-2,y,w+4,h+4,true,gridcolor);
			screen->GotoXY(x-2+w/2-(strlen(caption)/2)*cfont.width,y-cfont.height-2);
			screen->SetAttr(foretitlecolor,backtitlecolor,false);
			screen->gprintf(caption);
			if (cline)
			{
				for (i=0;i<cline;i++)
				{
					screen->GotoXY(x,y+2+cfont.height*i);
					screen->SetAttr(forecolor,backcolor,false);
					screen->gprintf(text[start_line+i]);
				}
			}
			first_time++;
		}
		else
		{
			/*screen->plot_rectangle(x-2,y-cfont.height-4,w+4,cfont.height+4,true,backtitlecolor);
			screen->plot_rectangle(x-2,y,w+4,h+4,true,backcolor);
			screen->plot_rectangle(x-2,y-cfont.height-4,w+4,cfont.height+4,false,gridcolor);
			screen->plot_rectangle(x-2,y,w+4,h+4,false,gridcolor);
			screen->GotoXY(x-2+w/2-(strlen(caption)/2)*cfont.width,y-cfont.height-2);
			screen->SetAttr(foretitlecolor,backtitlecolor,true);
			screen->gprintf(caption);*/
			if (cline<h/cfont.height && cline>0)
			{
				screen->GotoXY(x,y+2+cfont.height*cline);
				screen->SetAttr(forecolor,backcolor, false);
				screen->gprintf(text[cline-1]);
			}
			else
			if (cline>=h/cfont.height)
			{
				screen->plot_rectangle(x-1,y+1,w+2,h+2,true,backcolor);				
				for (i=0;i<end_line-start_line;i++)
				{
					screen->GotoXY(x,y+2+cfont.height*i);
					screen->SetAttr(forecolor,backcolor, false);
					screen->gprintf(text[start_line+i]);
				}
			}
			else
			if (cline==0)
				screen->plot_rectangle(x-1,y+1,w+2,h+2,true,backcolor);				
		}
		screen->set_font(backup.width,backup.height,backup.nrch,backup.fonttable);
		screen->GotoXY(oldx,oldy);
	}

	Mouse::GetInstance()->ShowMouse();
}

/*void TEXTAREA::RepaintPitArray()
{
	int i;
	FONT backup;
	dword oldx,oldy;
	dword tempX = x, tempY = y;
	VIDEO* screen = VIDEO::GetInstance();

	if (active)
	{
		backup=screen->GetFont();
		oldx=screen->GetX();
		oldy=screen->GetY();
		screen->set_font(cfont.width,cfont.height,cfont.nrch,cfont.fonttable);
		if (first_time==0)
		{
			screen_under=(byte*)malloc(w*h*(screen->GetBPP()>>3));
			//screen->copyScreenArea(screen_under,x,y,w,h);
			screen->plot_rectangle(x-2,y-cfont.height-4,w+4,cfont.height+4,true,backtitlecolor);
			screen->plot_rectangle(x-2,y,w+4,h+4,true,backcolor);
			screen->plot_rectangle(x-2,y-cfont.height-4,w+4,cfont.height+4,false,gridcolor);
			screen->plot_rectangle(x-2,y,w+4,h+4,false,gridcolor);
			screen->GotoXY(x-2+w/2-(strlen(caption)/2)*cfont.width,y-cfont.height-2);
			screen->SetAttr(foretitlecolor,backtitlecolor,true);
			screen->gprintf(caption);
			first_time++;
		}	


		for (i = 0; i < pitArrNoOfElems; ++i)
		{
			screen->GotoXY(tempX, tempY);
			screen->gprintf("%c", pitArray[i]);
			//screen->gprintf("%c", 65 + i);
			if (((i + 1) % lineNoOfChars) == 0)
			{
				tempX = x;
				tempY += cfont.height;
				if ((tempY + cfont.height)  >= (y + h))
					break;
			}
			else
			{
				tempX += cfont.width;
			}
		}
		
		screen->set_font(backup.width,backup.height,backup.nrch,backup.fonttable);
		screen->GotoXY(oldx,oldy);
	}
}*/

void TEXTAREA::ScrollPitArray(int lines)
{

}

void TEXTAREA::ScrollUp()
{
	if (cline>h/cfont.height)
	{
		if (end_line<cline)
		{
			start_line++;
			end_line++;
		}
	}
}

void TEXTAREA::ScrollDown()
{
	if (cline>h/cfont.height)
	{
		if (start_line>0)
		{
			start_line--;
			end_line--;
		}
	}
}

void TEXTAREA::OnKeyDown(char ascii,byte scancode,byte modifiers)
{
	Mouse* mouse = Mouse::GetInstance();
	char new_line[50];

	if (ascii == 0x48)
	{
		if (GetStartLine() > 0)
		{
			mouse->HideMouse();
			ScrollDown();
			Repaint();
			mouse->ShowMouse();
		}
	}
	else
		if (ascii == 0x50)
		{
			if (GetEndLine() < GetCurrentLine())
			{
				mouse->HideMouse();
				ScrollUp();
				Repaint();
				mouse->ShowMouse();
			}
		}
		else
		{
			mouse->HideMouse();	
			new_line[0] = ascii;
			new_line[1] = 0;
			AddLine(new_line);
			Repaint();
			mouse->ShowMouse();
		}
}

void TEXTAREA::OnKeyUp(char ascii,byte scancode,byte modifiers)
{

}

void TEXTAREA::OnMouseMoveClick(byte buttons,dword x,dword y)
{

}
