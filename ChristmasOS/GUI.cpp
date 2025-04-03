#include "GUI.h"
#include "MemoryManager.h"
#include "System.h"

GUI::GUI(VIDEO *_screen,KEYBOARD *keyb,Mouse *_mouse,TIMER *tmr)
{
    int i;
    screen=_screen;
	keyboard=keyb;
	mouse=_mouse;
	timer=tmr;
    memset(numcomponents,0,MAX_WINDOWS*sizeof(int));
    
    //printf("Alocam [%d] ferestre pentru obiecte\r\n",MAX_WINDOWS);
    components=new GUIComponent**[MAX_WINDOWS];
    memset(components,0,MAX_WINDOWS*sizeof(GUIComponent**));    
    for (i=0;i<MAX_WINDOWS;i++)
    {
        //printf("Alocam [%d] componente pentru fereastra [%d]\r\n",MAX_COMPONENTS,i);
        components[i] = new GUIComponent*[MAX_COMPONENTS];
        memset(components[i],0,MAX_COMPONENTS*sizeof(GUIComponent*));    
    }
    can_change_window=true;
    //printf("Clear deallocation vector\r\n");
    refresh_background=false;
	last_z=0;
	CurrentWindow=0;
}

VIDEO *GUI::GetScreen()
{
	return screen;
}

void GUI::DealocateWindow(int window)
{
    int i;
    
    //printf("dealocam obiectele de pe fereastra [%d]\r\n",window);
    for (i = 0; i < numcomponents[window]; i++)
        free(components[window][i]);
    numcomponents[window]=0;
}

int GUI::GetWindow()
{
	return CurrentWindow;
}

GUIComponent *GUI::GetComponentByID(int id,int win)
{
    int i;
    
    for (i=0;i<numcomponents[win];i++)
        if (id==components[win][i]->GetID())
            return components[win][i];
    return NULL;
}

void GUI::AddComponent(GUIComponent *comp,int window)
{
    int i=numcomponents[window];

    numcomponents[window]++;
    //printf("[AddComponent]: 1 component added on window %d. Total Components = %d\r\n",window, numcomponents[window]);
    components[window][i]=comp;
    //printf("[AddComponent]: New component @ 0x%08X\r\n",components[window][i]);
}

void GUI::ChangeWindow(int window)
{
    int i;
    
    if (can_change_window)
    {
        CurrentWindow=window;
        //printf("[ChangeWindow]: current_window = %d | components = %d | force repaint\r\n",window,numcomponents[CurrentWindow]);
        
        //fortam componentele sa faca repaint        
        for (i=0;i<numcomponents[CurrentWindow];i++)
        {
            //printf("Repaint component @ 0x%08X\r\n",components[CurrentWindow][i]);
            components[CurrentWindow][i]->SetRepaint(true);
        }
        
        //clear window
        //printf("[ChangeWindow]: clear window\r\n");
        //_lcd->FillScreen(COLOR_CYAN);        
           
        //fortam un eveniment inainte de schimbarea ferestrei, ca sa nu avem cazul in care schimbam cu eroare ferestre consecutive, fara evenimente relevante
        can_change_window=false;
    }
    else
    {
        //printf("[ChangeWindow]: Tentativa blocata!!!\r\n");
        //printf("[ChangeWindow]: Nu am voie sa schimb fereastra inainte sa afisez toate componentele!\r\n");
    }
}

void GUI::RepaintAllWin(void)
{
    int i;

    refresh_background=true;
    
    for ( i=0; i<numcomponents[CurrentWindow]; ++i ) 
    {
        components[CurrentWindow][i]->SetRepaint(true);
        can_change_window=true;
    }    
}

void GUI::Display()
{
    int i;
    
    for ( i=0; i<numcomponents[CurrentWindow]; ++i ) 
    {
        if (components[CurrentWindow][i]->MustRepaint())
        {
            //printf("[Display]: Repaint Component %d on Window %d\r\n",i,CurrentWindow);
            components[CurrentWindow][i]->Redraw();
            components[CurrentWindow][i]->SetRepaint(false);
        }
    }
}

void GUI::HandleKeyEvent(KEY_event *event)
{
    int i;
    GUI_PACKET gui_data;
    
	switch (event->state) 
    {
        case _DOWN:        
        case _UP:
             for (i=0; i<numcomponents[CurrentWindow]; i++) 
            {     
                gui_data.gui=this;
                gui_data.ID=components[CurrentWindow][i]->GetID();
                gui_data.window=CurrentWindow;
                components[CurrentWindow][i]->HandleKeyEvent(event,(void*)&gui_data);
            }
            break;
        default:
            break;
    }
}

void GUI::HandleMouseEvent(MOUSE_event *event)
{
    int i;
    GUI_PACKET gui_data;
    
	switch (event->state) 
    {
        case _DOWN:        
        case _UP:
             for (i=0; i<numcomponents[CurrentWindow]; i++) 
            {     
                gui_data.gui=this;
                gui_data.ID=components[CurrentWindow][i]->GetID();
                gui_data.window=CurrentWindow;
                components[CurrentWindow][i]->HandleMouseEvent(event,(void*)&gui_data);
            }
            break;
        default:
            break;
    }
}

void GUI::Run()
{
    do 
    {
        Display();
        
		if (keyboard->PollEvent(&kev))
			HandleKeyEvent(&kev);

		if (mouse->PollEvent(&mev))
			HandleMouseEvent(&mev);

		timer->timer_wait(100);
        //-----------------------------------------------
    } while ( true );
}


