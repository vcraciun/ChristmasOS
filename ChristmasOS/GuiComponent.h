#pragma once

#include "types.h"
#include "video.h"
#include "m_mouse.h"
#include "keyboard.h"

#define MAX_STRING_SIZE 40

typedef void (*OnEvent)(void *data);

class GUIComponent
{
    public:
        GUIComponent(VIDEO *_screen,OnEvent proc, int x, int y, int w, int h,int ID);
    
        virtual void SetRect(int x, int y, int w, int h);
        virtual void GetRect(int *x, int *y, int *w, int *h) {*x=xx;*y=yy;*w=ww;*h=hh;}
        virtual int X() { return xx; }
        virtual int Y() { return yy; }
        virtual int W() { return ww; }
        virtual int H() { return hh; }
        virtual int HitRect(int x, int y);
        virtual void Redraw(void);
        virtual int KeyDown(int x, int y,void *data);
        virtual int KeyUp(int x, int y,void *data);
		virtual int MouseDown(int x, int y,void *data);
		virtual int MouseUp(int x, int y,void *data);
		//virtual int MouseMove(int x, int y,void *data);
        virtual int HandleKeyEvent(KEY_event *event,void *event_data);
		virtual int HandleMouseEvent(MOUSE_event *event,void *event_data);
        virtual int MustRepaint();
        virtual void SetRepaint(bool state);
        virtual int GetID(void);
    
    protected:
        VIDEO *screen;
        void Init(int x, int y, int w, int h);
        int xx,yy,ww,hh;
        int status;
        int pressed;
        bool repaint;
        OnEvent event_proc;
        int component_ID;
};

