#include "m_mouse.h"
#include "System.h"
#include "scancodes.h"
#include "Video.h"
#include "MemoryManager.h"

Mouse* Mouse::instance = 0;

dword default_pointer[290]={0x0C,0x18,
							0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
							1,1,0,0,0,0,0,0,0,0,0,0,1,0xFFFFFF,1,0,0,0,0,0,0,0,0,0,
							1,0xFFFFFF,0xFFFFFF,1,0,0,0,0,0,0,0,0,1,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,0,0,0,0,0,0,
							1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,0,0,0,0,0,1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,0,0,0,0,
							1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,0,0,0,1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,0,0,
							1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,0,1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0,
							1,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,1,1,1,1,1,0xFFFFFF,0xFFFFFF,0xFFFFFF,1,0xFFFFFF,0xFFFFFF,1,0,0,0,0,
							1,0xFFFFFF,0xFFFFFF,1,1,0xFFFFFF,0xFFFFFF,1,0,0,0,0,1,0xFFFFFF,1,0,0,1,0xFFFFFF,0xFFFFFF,1,0,0,0,
							1,1,0,0,0,1,0xFFFFFF,0xFFFFFF,1,0,0,0,1,0,0,0,0,0,1,0xFFFFFF,0xFFFFFF,1,0,0,
							0,0,0,0,0,0,1,0xFFFFFF,0xFFFFFF,1,0,0,0,0,0,0,0,0,0,1,0xFFFFFF,0xFFFFFF,1,0,
							0,0,0,0,0,0,0,1,0xFFFFFF,0xFFFFFF,1,0,0,0,0,0,0,0,0,0,1,1,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

bool first_time;

Mouse* Mouse::GetInstance()
{
	if (!instance)
		instance = new Mouse;
	return instance;
}

void Mouse::FreeInstance()
{

}

Mouse::Mouse()
{
	Init((byte*)default_pointer,false);
	mouse_install();
	new_data=false;
}

void Mouse::Init(byte *mouseptr,bool is_bmp)
{
	VIDEO* screen = VIDEO::GetInstance();
	dword raw_bitmap_size;
    dword bpp=screen->GetBPP() / 8;

	use_bmp=is_bmp;	
	resx=screen->GetWidth();
	resy=screen->GetHeight();
	
    first_time=true;
	smovex=resx/2;
	smovey=resy/2;
	mouse_cycle=0;
	mouse_x=0;
	mouse_y=0;
	buttons=0;
	devtype=0;
    mbl=0;
	mbr=0;
	mbm=0;
	mwh=0;
	currentx=smovex;
	currenty=smovey;
	
	if (use_bmp)
	{
		arrow_width=*((dword*)(mouseptr+0x12));
		arrow_height=*((dword*)(mouseptr+0x16));
		raw_bitmap_size=*((dword*)(mouseptr+0x22));
		arrow_screen_buf=(byte*)malloc(raw_bitmap_size+36);
	}
	else
	{
		arrow_width=*((dword*)(mouseptr));
		arrow_height=*((dword*)(mouseptr+4));
		raw_bitmap_size=(arrow_width*arrow_height+2)*4;
		arrow_screen_buf=(byte*)mouseptr+8;
	}
	arrow_screen_backup=(byte*)malloc((arrow_width+2)*(arrow_height+2)*bpp);
	
	if (use_bmp)
		memcpy(arrow_screen_buf,mouseptr,raw_bitmap_size+0x36);	

	screen->copyScreenArea(arrow_screen_backup, smovex, smovey, arrow_width, arrow_height);
    if (hide_mouse==0)
	{
		if (use_bmp)
			screen->PutBMP(smovex,smovey,arrow_screen_buf);
		else
			screen->restoreScreenArea(mouseptr,smovex,smovey,arrow_width,arrow_height,true);
	}
}

void Mouse::mouse_handler(REGS *a_r)
{
	dword *loc;	
    dword i;
    byte x_dir,y_dir;
	VIDEO* screen = VIDEO::GetInstance();
	KEYBOARD* keyboard = KEYBOARD::GetInstance();

	keyboard->keyboard_cmd(CMD_DISABLEKBD);
  
	old_buttons=buttons;
	switch (devtype)
	{
        case 0:
            switch(mouse_cycle)
            {
                case 0:
                    mouse_byte[0]=inb(DATA_PORT);
                    mouse_cycle++;
                    break;
                case 1:
                    mouse_byte[1]=inb(DATA_PORT);
                    mouse_cycle++;
                    break;
                case 2:
					mouse_byte[2]=inb(DATA_PORT);
					buttons=mouse_byte[1];
					mouse_x=mouse_byte[2];
					mouse_y=mouse_byte[0];
					mwh=0;
					mouse_cycle=0;
					break;
			}
			break;          
		case 3:
			switch(mouse_cycle)
			{
                case 0:
                    mouse_byte[0]=inb(DATA_PORT);
                    mouse_cycle++;
                    break;
                case 1:
                    mouse_byte[1]=inb(DATA_PORT);
                    mouse_cycle++;
                    break;
                case 2:
                    mouse_byte[2]=inb(DATA_PORT);
                    mouse_cycle++;
                    break;
                case 3:
                    mouse_byte[3]=inb(DATA_PORT);
                    buttons=mouse_byte[1];
                    mouse_x=mouse_byte[2];
                    mouse_y=mouse_byte[3];
                    mouse_z=mouse_byte[0];
                    mwh=(mouse_z==1)?DOWN:(mouse_z==0xFF)?UP:FREE;
                    mouse_cycle=0;
                    break;
            }		
            break;
	}
    
    if (mouse_cycle)
    {
        keyboard->keyboard_cmd(CMD_ENABLE_KBD);
        return;
    }
    
	mbl=buttons&0x01;
	mbr=(buttons&0x02)>>1;
	mbm=(buttons&0x04)>>2;	
    x_dir=((buttons&0x10)^0x10)>>4;
    y_dir=((buttons&0x20)^0x20)>>5;
       
    if (x_dir && currentx<resx-1)
        currentx+=(int)mouse_x;
    if (x_dir && (int)currentx>=resx)
        currentx=resx-1;
    if (!x_dir && currentx>0)
        currentx+=(int)mouse_x;
    if (!x_dir && (int)currentx<=0)
        currentx=0;
        
    if (y_dir && currenty>0)
        currenty-=mouse_y;
    if (y_dir && (int)currenty<=0)
        currenty=0;
    if (!y_dir && currenty<resy-1)
        currenty-=(int)mouse_y;
    if (!y_dir && (int)currenty>=resy)
        currenty=resy-1;

    if (hide_mouse==0)
    {
        screen->restoreScreenArea(arrow_screen_backup,smovex,smovey,arrow_width,arrow_height,false);	
        screen->copyScreenArea(arrow_screen_backup, currentx, currenty, arrow_width, arrow_height);
		if (use_bmp)
			screen->PutBMP(currentx,currenty,arrow_screen_buf);
		else
			screen->restoreScreenArea(arrow_screen_buf,currentx,currenty,arrow_width,arrow_height,true);
    }
    else
    {
        screen->restoreScreenArea(arrow_screen_backup,smovex,smovey,arrow_width,arrow_height,false);	
        screen->copyScreenArea(arrow_screen_backup, currentx, currenty, arrow_width, arrow_height);
    }
    
	smovex=currentx;
	smovey=currenty;
	new_data=true;
    keyboard->keyboard_cmd(CMD_ENABLE_KBD);
}

bool Mouse::PollEvent(MOUSE_event *mev)
{
	if (new_data)
	{
		mev->x=smovex;
		mev->y=smovey;
		if (old_buttons==buttons)
			mev->state=(old_buttons==0)?_UP:_DOWN;
		else
		{
			if (old_buttons&0x4!=buttons&4)
			{
				if ((buttons>>2)==0)
					mev->state=_UP;
				else
					mev->state=_DOWN;
			}
			if (old_buttons&2!=buttons&2)
			{
				if ((buttons&2)==0)
					mev->state=_UP;
				else
					mev->state=_DOWN;
			}
			if (old_buttons&1!=buttons&1)
			{
				if ((buttons&1)==0)
					mev->state=_UP;
				else
					mev->state=_DOWN;
			}
		}
		mev->button=buttons;
		new_data=false;
		return true;
	}
	return false;
}

void Mouse::mouse_wait(byte a_type) 
{
	dword _time_out=0xFFFF; 
	if(a_type==0)
	{
		while(_time_out--) 
		{
			if((inb(STATUS_PORT) & STATUS_OUTBUF)==STATUS_OUTBUF)
				return;		  
		}
		return;
	}
	else
	{
		while(_time_out--) 
		{
			if((inb(STATUS_PORT) & STATUS_INBUF)==0)
				return;
		}
		return;
	}
}

void Mouse::mouse_write(byte a_write) 
{
	mouse_wait(1);
	outb(COMMAND_PORT, CMD_WRITEMICEDATA);
	mouse_wait(1);
	outb(CMD_PORT60, a_write);
}

byte Mouse::mouse_read()
{
	mouse_wait(0);
	return inb(DATA_PORT);
}

void Mouse::set_samplerate(byte rate)
{
	mouse_write(CMD_TYPEMATIC_RATE);
	mouse_read();
	mouse_write(rate);
	mouse_read();
}

byte Mouse::get_devicetype(void)
{
	byte tip;
    
	set_samplerate(200);
	set_samplerate(100);
	set_samplerate(80);
	mouse_write(CMD_GET_DEVICE_ID);
	mouse_read();
	tip=mouse_read();
	
	return 	tip;
}

void Mouse::set_scaling21(void)
{
	mouse_write(CMD_SET_SCALE21);
	mouse_read();	
}

void Mouse::set_scaling11(void)
{
	mouse_write(CMD_SET_SCALE11);
	mouse_read();	
}

void Mouse::set_resolution(byte res)
{
	mouse_write(CMD_SET_RESOLUTION);
	mouse_read();
	mouse_write(res);
	mouse_read();
}

void Mouse::stub_handler(void* mclass, REGS *r)
{
	Mouse* cclass = (Mouse*)mclass;
	cclass->mouse_handler(r);
}

void Mouse::mouse_install()
{
	byte _status;
    word data;
    dword i;

	mouse_write(CMD_RESETTOPOST);
	mouse_read();
	mouse_read();
	mouse_read();

	mouse_write(CMD_RESETTOPOST);
	mouse_read();
	mouse_read();
	mouse_read();

	devtype=get_devicetype();
    set_samplerate(10);
    mouse_write(CMD_GET_DEVICE_ID);
    mouse_read();
    devtype=mouse_read();  

	mouse_wait(1);
	outb(COMMAND_PORT, CMD_ENABLEMICE);    
	mouse_wait(1);
	outb(COMMAND_PORT, CMD_CMDASSCAN);
	mouse_wait(0);
	_status=(inb(DATA_PORT) | 2);
	mouse_wait(1);
	outb(COMMAND_PORT, CMD_STORECMD);
	mouse_wait(1);
	outb(CMD_PORT60, _status);
    
	set_samplerate(10);
	mouse_write(CMD_GET_DEVICE_ID);
	mouse_read();
	mouse_read();
	set_resolution(1);
	set_scaling11();
	set_samplerate(40);

	mouse_write(CMD_ENABLE_KEYBOARD);

	IDT::GetInstance()->irq_install_handler(12, stub_handler, this);
}

void Mouse::HideMouse()
{
    hide_mouse=1;
    VIDEO::GetInstance()->restoreScreenArea(arrow_screen_backup, smovex, smovey, arrow_width, arrow_height, false);
}

void Mouse::ShowMouse()
{    
    hide_mouse=0;
    if (first_time==false)
    {
        VIDEO::GetInstance()->copyScreenArea(arrow_screen_backup, currentx, currenty, arrow_width, arrow_height);
		if (use_bmp)
			VIDEO::GetInstance()->PutBMP(currentx,currenty,arrow_screen_buf);
		else
			VIDEO::GetInstance()->restoreScreenArea(arrow_screen_buf,currentx,currenty,arrow_width,arrow_height,true);
    }
    else
        first_time=false;
}

MouseData Mouse::GetMouseData()
{
    MouseData m;
    
    m.x=currentx;
    m.y=currenty;
    m.LButton=(mbl==1)?DOWN:UP;
    m.RButton=(mbr==1)?DOWN:UP;
    m.MButton=(mbm==1)?DOWN:UP;
    m.Wheel=mwh;
    
    return m;
}

bool Mouse::MouseInArea(dword x,dword y,dword w,dword h)
{
    if (currentx+arrow_width>=x && currentx<=x+w && currenty<=y+h && currenty+arrow_height>=y)
        return true;
    return false;
}
