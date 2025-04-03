#include "timer.h"
#include "system.h"
#include "cmos.h"
#include "Video.h"
#include "MemoryManager.h"
#include "m_mouse.h"

TIMER* TIMER::instance = 0;

char days[7][20]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char months[12][20]={"January","February","March","April","May","June","July","August","September","Octomber","November","December"};

TIMER* TIMER::GetInstance(int max_handlers)
{
	if (!instance)
		instance = new TIMER(max_handlers);
	return instance;
}

void TIMER::FreeInstance()
{
	delete instance;
}

void TIMER::EnableTimer()
{
	timer_activated=true;
}

void TIMER::DisableTimer()
{
	timer_activated=false;
}

TIMER::TIMER(int _max_timer_handlers)
{
	VIDEO* screen = VIDEO::GetInstance();
	CMOS::GetInstance()->GetDateTime(&time);
	freq = 0;
	clock_backup = (byte*)malloc(screen->GetFont().height * CLOCK_LEN * screen->GetFont().width * screen->GetBPP() / 8);
	screen->copyScreenArea(clock_backup, screen->GetWidth() - CLOCK_LEN * screen->GetFont().width, 0, CLOCK_LEN * screen->GetFont().width, screen->GetFont().height);
	cnt_down = 0;
	SetPITFreq(1000);
	timer_install();
}

TIMER::~TIMER(){}

void TIMER::timer_handler(REGS *r)
{   
	bool in_area = false;
	int i;
	VIDEO* screen = VIDEO::GetInstance();
	CMOS* cmos = CMOS::GetInstance();
	CMOS_DATE_TIME second = { 0 };	

	__asm {cli}
	if (timer_activated)
	{
		cmos->GetDateTime(&second);
		if (second.second != time.second)
		{
			memcpy(&time, &second, sizeof(CMOS_DATE_TIME));
			Mouse::GetInstance()->HideMouse();
			screen->restoreScreenArea(clock_backup, screen->GetWidth() - CLOCK_LEN * screen->GetFont().width, 0, CLOCK_LEN * screen->GetFont().width, screen->GetFont().height, false);
			screen->GotoXY(screen->GetWidth() - CLOCK_LEN * screen->GetFont().width, 0);
			screen->SetAttr(0xFFFFFF, 0x0000FF, true);
			screen->gprintf("%s %d %s %02d%02d   %02d:%02d:%02d",
				days[second.week_day],
				second.day,
				months[second.month - 1],
				second.century,
				second.year,
				second.hour,
				second.minute,
				second.second);
			Mouse::GetInstance()->ShowMouse();
			if (cnt_down && cnt_down > 0)
				cnt_down--;
		}
		timer_ticks++;
	}
	__asm {sti}
}

TimerHandler *TIMER::AddTimerHandler(void (*chand)(void*),int period,void *paramaetru,int TID_owner)
{
	int i;
	bool found=false;

	for (i=0;i<max_timer_handlers;i++)
	{
		if (!handler_array[i])
		{
			handler_array[i]=new TimerHandler;
			handler_array[i]->handler=chand;
			handler_array[i]->initial=0;
			handler_array[i]->period_milis=period;
			handler_array[i]->params=paramaetru;
			handler_array[i]->TID_owner=TID_owner;
			found=true;
			handler_used++;
			break;
		}
	}

	return handler_array[i];
}

bool TIMER::RemoveTimerHandler(TimerHandler *chand)
{
	int i;
	bool found=false;

	for (i=0;i<max_timer_handlers;i++)
	{
		if (handler_array[i] && handler_array[i]==chand)
		{
			free(handler_array[i]);
			handler_array[i]=0;
			found=true;
			handler_used--;
			break;
		}
	}

	return found;
}

bool TIMER::AdjustHandlerPeriod(TimerHandler *chand,int period)
{
	int i;
	bool found=false;

	for (i=0;i<max_timer_handlers;i++)
	{
		if (handler_array[i] && handler_array[i]==chand)
		{
			handler_array[i]->period_milis=period;
			found=true;
			break;
		}
	}

	return found;
}

int TIMER::GetHandlerPeriod(TimerHandler *chand,int period)
{
	int i;

	for (i=0;i<max_timer_handlers;i++)
	{
		if (handler_array[i] && handler_array[i]==chand)
		{
			return handler_array[i]->period_milis;
		}
	}

	return 0;
}

int TIMER::GetTotalHandlers()
{
	int i;

	return handler_used;
}


void TIMER::SetCountDown(dword seconds)
{
	cnt_down=seconds;
}

dword TIMER::GetCountDown(void)
{
	return cnt_down;
}

void TIMER::timer_wait(qword ticks)
{
    dword eticks;

    eticks = timer_ticks + ticks;
    
	while(timer_ticks < eticks)
		;
}

void TIMER::stub_handler(void *class_data,REGS *r)
{
	TIMER *cclass=(TIMER*)class_data;
	cclass->timer_handler(r);
}

void TIMER::timer_install()
{	
	IDT::GetInstance()->irq_install_handler(0, TIMER::stub_handler, this);
}

void TIMER::SetPITFreq(word f)
{
	dword data=1193181/f;
	freq=f;
	timer_ticks=0;
	cnt_down=0;
	
	outb(0x43,0x34);
	outb(0x40,(byte)data);
	outb(0x40,(byte)(data>>8));
}