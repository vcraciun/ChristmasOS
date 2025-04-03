#include "GUI.h"
#include "GUIComponent.h"

GUIComponent::GUIComponent(VIDEO *_screen,OnEvent proc, int x, int y, int w, int h,int ID)
{
    Init(x, y, w, h);
    screen=_screen;
    repaint=true;
    event_proc=proc;
    component_ID=ID;
}

int GUIComponent::GetID(void)
{
    return component_ID;
}

int GUIComponent::MustRepaint()
{
    return repaint;
}

void GUIComponent::SetRepaint(bool state)
{
    repaint=state;
}

void GUIComponent::Init(int x, int y, int w, int h)
{
    SetRect(x, y, w, h);
    pressed=0;
}

void GUIComponent::SetRect(int x, int y, int w, int h)
{
    xx = x;
    yy = y;
    if ( w >= 0 )
        ww = w;
    if ( h >= 0 )
        hh = h;
}

int GUIComponent::HitRect(int x, int y)
{
    int hit;
    
    hit = 1;
    if ( (x < xx) || (x >= (xx+ww)) || (y < yy) || (y >= (yy+hh)) ) 
        hit = 0;
    return(hit);   
}

void GUIComponent::Redraw(void)
{
}

int GUIComponent::KeyDown(int x, int y,void *data)
{
    return(GUI_PASS);
}

int GUIComponent::KeyUp(int x, int y,void *data)
{
    return(GUI_PASS);
}

int GUIComponent::MouseDown(int x, int y,void *data)
{
	return(GUI_PASS);
}

int GUIComponent::MouseUp(int x, int y,void *data)
{
	return(GUI_PASS);
}

/*int GUIComponent::MouseMove(int x, int y,void *data)
{
	return(GUI_PASS);
}*/

int GUIComponent::HandleKeyEvent(KEY_event *event,void *event_data)
{
    switch (event->state) 
    {
        case _DOWN: 
			return(KeyDown(event->data,event->key_scan,event_data));
        break;
        case _UP: 
                return(KeyUp(event->data,event->key_scan,event_data));
        break;
        default: 
        break;
    }
    return(GUI_PASS);
}

int GUIComponent::HandleMouseEvent(MOUSE_event *event,void *event_data)
{
    switch (event->state) 
    {
        case _DOWN: 
            {
                if ( HitRect(event->x, event->y) )                   
                {
                    return(MouseDown(event->x, event->y,event_data));
                }
            }
        break;
        case _UP: 
            {
                if ( HitRect(event->x, event->y) )
                {
                    return(MouseUp(event->x, event->y,event_data));
                }
            }
        break;
        default: 
        break;
    }
    return(GUI_PASS);
}

