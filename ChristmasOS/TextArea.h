#pragma once

#include "types.h"
#include "Video.h"
#include "m_mouse.h"

extern "C" int _purecall();

#define MAX_LINE_SIZE 256

class TEXTAREA: public EventHandler {
	private:
		dword x,y;
		dword w,h;
		char *caption;
		char **text;
		char *pitArray;
		dword lines;
		dword start_line;
		dword end_line;
		dword ID;
		bool active;
		dword backcolor;
		dword forecolor;
		dword gridcolor;
		dword backtitlecolor;
		dword foretitlecolor;
		FONT  cfont;
		byte *screen_under;
		int first_time;
		int cline;
		int colon_index;
		int pitArrNoOfElems;
		int lineNoOfChars;
		int noOfLines;

	public:
		TEXTAREA(dword left,dword top,dword width,dword height,dword totallines);
		~TEXTAREA();
		void SetColors(dword bc,dword fc,dword gc,dword btc,dword ftc);
		void SetCaption(char *title);
		void AddLine(char *line);
		void AddChar(char data);
		void SetActive(bool isact);
		void Repaint(); 
		dword GetID();  
		void set_font(dword width,dword height,dword chars,byte *tbl);
		void Destroy(); 
		void ClearAll();
		void ScrollUp();
		void ScrollDown();
		dword GetStartLine();
		dword GetEndLine();
		dword GetCurrentLine();
		void AllocPitArray(dword chars);
		//void RepaintPitArray();
		void ScrollPitArray(int lines);

		bool IsActive();
		void OnKeyDown(char ascii,byte scancode,byte modifiers);
		void OnKeyUp(char ascii,byte scancode,byte modifiers);
		void OnMouseMoveClick(byte buttons,dword x,dword y);		
};
