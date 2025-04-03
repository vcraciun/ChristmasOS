#pragma once

#include "types.h"
#include "keyboard.h"

typedef struct _Mouse_Data{
    dword x;
    dword y;
    byte LButton;
    byte MButton;
    byte RButton;
    byte Wheel;
}MouseData;

class Mouse{
	private:
		static void stub_handler(void* mclass, REGS *r);
		void mouse_handler(REGS *a_r);
		void mouse_wait(byte a_type);
		void mouse_write(byte a_write);
		byte mouse_read();
		void set_scaling21(void);
		void set_scaling11(void);	

		byte mouse_cycle;    
		byte mouse_byte[4];
		byte buttons;
		byte old_buttons;
		byte devtype;
		dword resx,resy;
		dword currentx,currenty;
		dword smovex,smovey;
		char mouse_x;        
		char mouse_y;    
		byte mouse_z;  
		byte mbl,mbm,mbr,mwh;	
		dword arrow_width,arrow_height;
		byte *arrow_screen_buf;
		byte *arrow_screen_backup;
		byte hide_mouse;
		bool use_bmp;
		bool new_data;

		static Mouse* instance;

		Mouse();
		~Mouse() {}
	
	public:	
		static Mouse* GetInstance();
		static void FreeInstance();

		void mouse_install();
		void set_resolution(byte res);
		void set_samplerate(byte rate);
		byte get_devicetype(void);
		void Init(byte *mouseptr,bool is_bmp);
		void HideMouse();
		void ShowMouse();
		MouseData GetMouseData();
		bool MouseInArea(dword x,dword y,dword w,dword h);  

		bool PollEvent(MOUSE_event *mev);
};

