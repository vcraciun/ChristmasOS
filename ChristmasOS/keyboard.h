#ifndef __KEYBOARD__
#define __KEYBOARD__

#include "types.h"
#include "idt.h"
#include "scancodes.h"
#include "EventHandler.h"

class KEYBOARD{
private:
	byte WaitKPressed;	
	byte keyBoardStatus;
	byte rshift_key;
	byte lshift_key;	
	byte ctrl_key;	
	byte alt_key; 			     
	byte cap_key; 	
    byte num_key;
    byte scl_key;
	byte ScanCode;	
	byte ScanCode2;
	dword textx;
	dword texty;
	bool new_key;
	
	static void stub_handler(void *class_data,REGS *r);
	void keyboard_handler(REGS *r);
	void SetKeyBoardLeds();

	dword obj_count;
	static KEYBOARD* instance;

	EventHandler* objects_key[20];

	KEYBOARD();
	~KEYBOARD();
public:
	static KEYBOARD* GetInstance();
	static void FreeInstance();

	void keyboard_install();
	word keyboard_read();
	bool keyboard_cmd(byte _cmd);
	bool keyboard_write(byte _cmd);

	bool PollEvent(KEY_event *kev);

	void AddEventWatcherKey(EventHandler* obj);
	
	byte keybuffer;
};

#endif