#include "keyboard.h"
#include "system.h"
#include "m_mouse.h"
#include "pit.h"
#include "Disk.h"
#include "TextArea.h"
#include "Timer.h"

KEYBOARD* KEYBOARD::instance = 0;

extern bool reset_request;

typedef void (*handler)(REGS *r);

byte kbdus_normal[256] =
{
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,   9,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,  0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39,  '`', 0,   '\\',  'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   3,   3,	 3,   3,   3,
    3,   3,   3,   3,   3,	 0,   0,   0,0x48,   0,   '-', 0,   0,	 0,   '+', 0,
 0x50,   0,   0,   127, 0,	 0,   92,  3,	3,   0,   0,   0,   0,	 0,   0,   0,
    13,  0,   '/', 0,   0,	 0,   0,   0,	0,   0,   0,   0,   0,	 0,   0,   127,
    0,   0,   0,   0,   0,	 0,   0,   0,	0,   0,   '/', 0,   0,	 0,   0,   0,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255
};

byte kbdus_shift[256] =
{
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 126, 126,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 126, 0,   'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',  '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,  ' ',   0,	1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,	 0,   0,   0,	0,   0,   '-', 0,   0,	 0,   '+', 0,
    0,   0,   1,   127, 0,	 0, '|',   1,	1,   0,   0,   0,   0,	 0,   0,   0,
    13,  0,  '/',  0,   0,	 0,   0,   0,	0,   0,   0,   0,   0,	 0,   0,   127,
    0,   0,   0,   0,   0,	 0,   0,   0,	0,   0,   '/', 0,   0,	 0,   0,   0,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255
};

byte kbdus_alt[256] =
{
    0,   0,   2,   2,   2,	 2,   2,   2,	2,   2,   2,   2,   0,	 0,   127, 127,
    17,  23,  5,   18,  20,  25,  21,  9,	15,  16,  2,   2,   10,  0,   1,   19,
    4,   6,   7,   8,   10,  11,  12,  0,	0,   0,   0,   0,   26,  24,  3,   22,
    2,   14,  13,  0,   0,	 0,   0,   0,	0,   0,   0,   2,   2,	 2,   2,   2,
    2,   2,   2,   2,   2,	 0,   0,   0,	0,   0,   0,   0,   0,	 0,   0,   0,
    0,   0,   2,   0,   0,	 0,   0,   2,	2,   0,   0,   0,   0,	 0,   0,   0,
    0,   0,   0,   0,   0,	 0,   0,   0,	0,   0,   0,   0,   0,	 0,   0,   0,
    0,   0,   0,   0,   0,	 0,   0,   0,	0,   0,   0,   0,   0,	 0,   0,   0,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255
};

byte kbdus_capslock[256] =
{
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,   9,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 13,  0,   'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', 39,  '`', 0,  '#',  'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   3,   3,	 3,   3,   3,
    3,   3,   3,   3,   3,	 0,   0,0x48,	0,   0,   '-', 0,   0,	 0,   '+', 0,
0x50,   0,   0,   127, 0,	 0,   92,  3,	3,   0,   0,   0,   0,	 0,   0,   0,
    13,  0,   '/', 0,   0,	 0,   0,   0,	0,   0,   0,   0,   0,	 0,   0,   127,
    0,   0,   0,   0,   0,	 0,   0,   0,	0,   0,   '/', 0,   0,	 0,   0,   0,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255,
    255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255 ,255
};

//------------------------------------------------------------------------------------

KEYBOARD* KEYBOARD::GetInstance()
{
    if (!instance)
        instance = new KEYBOARD;
    return instance;
}

void KEYBOARD::FreeInstance()
{
    delete instance;
}

KEYBOARD::KEYBOARD()
{
	keyboard_install();
	new_key=false;
    obj_count = 0;
}

KEYBOARD::~KEYBOARD()
{

}

void KEYBOARD::AddEventWatcherKey(EventHandler* obj)
{
	objects_key[obj_count++]=obj;
}

void KEYBOARD::keyboard_handler(REGS *r)
{
    byte scancode;
	int i;

    ScanCode2=0;
   
    scancode = inb(DATA_PORT);
    if (scancode == EXTSCAN1)
        scancode = inb(DATA_PORT);
    if (scancode == EXTSCAN2)
    {
        scancode = inb(DATA_PORT);
        ScanCode2 = inb(DATA_PORT);
    }
    
	ScanCode=scancode;     
	new_key=true;

    if (scancode & 0x80 == 0)
    {
		WaitKPressed=1;
    }
    else
    {
        WaitKPressed=0;
    }
    
    switch (scancode)
    {
        case CTRL_DOWN:
            ctrl_key=1;
			return;
        break;
        case CTRL_UP:
            ctrl_key=0;
			return;
        break;
        case ALT_DOWN:
            alt_key=1;
			return;
        break;
        case ALT_UP:
            alt_key=0;
			return;
        break;
        case LSHIFT_DOWN:
            lshift_key=1;
			return;
        break;
        case LSHIFT_UP:
            lshift_key=0;
			return;
        break;
        case RSHIFT_DOWN:
            rshift_key=1;
			return;
        break;
        case RSHIFT_UP:
            rshift_key=0;
			return;
        break;
        case CAPLOCK_DOWN:
            if (cap_key==1)
            {
                cap_key=0;
                keyBoardStatus&=0xFB;			
            }
            else
            {
                cap_key=1;
                keyBoardStatus|=4;			
            }		
            SetKeyBoardLeds();
			return;
        break;
        case NUMLOCK_DOWN:
            if (num_key == 1)
            {
                num_key=0;
                keyBoardStatus&=0xFD;
            }
            else
            {
                num_key=1;
                keyBoardStatus|=2;
            }
            SetKeyBoardLeds();
			return;
        break;
        case SCRLOCK_DOWN:
            if (scl_key == 1)
            {
                scl_key=0;
                keyBoardStatus&=0xFE;
            }
            else
            {
                scl_key=1;
                keyBoardStatus|=1;
            }
            SetKeyBoardLeds();
			return;
        break;
    }
    
    if (scancode < 128 && scancode > 1 && scancode != 0x0E && scancode != 0x0F)
    {
		if (rshift_key==1 || lshift_key==1)
			keybuffer=kbdus_shift[scancode];
		else
		if (ctrl_key==1)
			keybuffer=kbdus_normal[scancode]-0x60;
		else
		if (alt_key==1)
			keybuffer=kbdus_alt[scancode];
		else
		if (cap_key==1)
			keybuffer=kbdus_capslock[scancode];
		else
			keybuffer=kbdus_normal[scancode];
		for (i=0;i<obj_count;i++)
		{
			if (objects_key[i]->IsActive())
				objects_key[i]->OnKeyDown(keybuffer,scancode,(ctrl_key<<2)|(alt_key<<1)|lshift_key|rshift_key);
		}
    }

	if (scancode>128 && scancode != 0x0E && scancode != 0x0F)
	{
		if (rshift_key==1 || lshift_key==1)
			keybuffer=kbdus_shift[scancode];
		else
		if (ctrl_key==1)
			keybuffer=kbdus_normal[scancode]-0x60;
		else
		if (alt_key==1)
			keybuffer=kbdus_alt[scancode];
		else
		if (cap_key==1)
			keybuffer=kbdus_capslock[scancode];
		else
			keybuffer=kbdus_normal[scancode];
		for (i=0;i<obj_count;i++)
		{
			if (objects_key[i]->IsActive())
				objects_key[i]->OnKeyUp(keybuffer,scancode,(ctrl_key<<2)|(alt_key<<1)|lshift_key|rshift_key);
		}
	}

    if (ScanCode==1)
		reset_request=true;
}

void KEYBOARD::stub_handler(void *class_data,REGS *r)
{
	KEYBOARD *cclass=(KEYBOARD*)class_data;
	cclass->keyboard_handler(r);
}

void KEYBOARD::keyboard_install()
{
	//textx=VIDEO::text_limit_left;
	//texty=VIDEO::GetY()+64;
    cap_key=0;
    scl_key=0;
    num_key=0;
	obj_count=0;
	IDT::GetInstance()->irq_install_handler(1, KEYBOARD::stub_handler, this);
}

word KEYBOARD::keyboard_read()
{
	dword i,j;
	byte data;
	for (i=0;i<=0xFFFF;i++)
	{
		data=inb(STATUS_PORT);
		if ((data & 1)==1)
        {
            for (j=0;j<32;j++)
                ;
            return (0x0000|inb(DATA_PORT));
        }
	}
	return 0xFF00;
}

bool KEYBOARD::keyboard_cmd(byte _cmd)
{
	byte data1;
	dword i;
    
	for (i=0;i<=0xFFFF;i++)
	{
		data1=inb(STATUS_PORT);
		if ((data1 & STATUS_INBUF)==0)
		{
			outb(COMMAND_PORT,_cmd);
			return true;
		}
	}
	return false;
}

bool KEYBOARD::keyboard_write(byte _cmd)
{
	byte data1;
	dword i;
	
	keyboard_cmd(CMD_DISABLEKBD);
	for (i=0;i<=0xFFFF;i++)
	{
		data1=inb(STATUS_PORT);
		if ((data1 & STATUS_INBUF)==0)
		{
			outb(CMD_PORT60,_cmd);
			keyboard_cmd(CMD_ENABLE_KBD);
			return true;
		}
	}
	keyboard_cmd(CMD_ENABLE_KBD);
	return false;	
}

void KEYBOARD::SetKeyBoardLeds()
{
	outb(CMD_PORT60,CMD_UPDATE_LEDS);
	outb(CMD_PORT60,keyBoardStatus&7);
}

bool KEYBOARD::PollEvent(KEY_event *kev)
{
	if (new_key)
	{
		kev->state=(WaitKPressed==1)?_DOWN:_UP;
		kev->key_scan=ScanCode;
		kev->data=keybuffer;
		new_key=false;
		return true;
	}
	return false;
}
