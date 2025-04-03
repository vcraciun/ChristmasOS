#ifndef __TYPES__
#define __TYPES__

#include "defines.h"

typedef unsigned char byte,BYTE;
typedef unsigned short word,WORD;
typedef unsigned long dword,DWORD;
typedef __int64 qword,QWORD;

enum wheel_modes {UP,DOWN,FREE};
typedef enum { FALSE = 0, TRUE } BOOL;

typedef struct _FONT{
	int width;
	int height;
	int nrch;
	byte *fonttable;
	dword color;
}FONT;

typedef enum {
    GUI_REDRAW,         
    GUI_YUM,          
    GUI_PASS,           
} GUI_status;

typedef enum {
    COMPONENT_VISIBLE,
    COMPONENT_HIDDEN,
} COMPONENT_status;

typedef enum {
    _DOWN,
    _UP,    
} POSITION_type;

typedef struct _KEYB_event {
    POSITION_type state;
    char data;
	int key_scan;
} KEY_event;

typedef struct _MOUSE_event {
	POSITION_type state;
	byte button;
	int x;
	int y;
} MOUSE_event;

#define NULL 0

#endif