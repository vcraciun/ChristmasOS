#pragma once

#include "types.h"
#include "VIDEO.h"
#include "keyboard.h"
#include "m_mouse.h"
#include "timer.h"
#include "GuiComponent.h"

#define MAX_COMPONENTS 50
#define MAX_WINDOWS 50

class GUI{
	private:
		int window_depths[MAX_COMPONENTS];
		int numcomponents[MAX_WINDOWS];
		int last_z;
		int CurrentWindow;

    public:
		GUI(VIDEO *_screen,KEYBOARD *keyb,Mouse *_mouse,TIMER *tmr);
        void AddComponent(GUIComponent *comp,int parent);
        void Display();
        void ChangeWindow(int window);
        void Run();
        int GetWindow();
        void DealocateWindow(int window);
        GUIComponent *GetComponentByID(int id,int parentID);
        void RepaintAllWin(void);
		VIDEO *GetScreen();
        
    protected:
        VIDEO *screen;
		KEYBOARD *keyboard;
		Mouse *mouse;
		KEY_event kev;
		MOUSE_event mev;
		TIMER *timer;
        GUIComponent ***components;
        void HandleKeyEvent(KEY_event *event);        
		void HandleMouseEvent(MOUSE_event *event);        
        bool can_change_window;
        bool refresh_background;
};

typedef struct _GUI_PACKET{
    GUI *gui;
    int ID;
    int window;
}GUI_PACKET;


