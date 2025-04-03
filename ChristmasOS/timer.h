#ifndef __TIMER__
#define __TIMER__

#include "types.h"
#include "idt.h"
#include "cmos.h"

#define CLOCK_LEN 40

typedef struct STATIC_HANDLER{
	void (*handler)(void*);
	int period_milis;
	int initial;
	void *params;
	int TID_owner;
}TimerHandler;

class TIMER{
	public:
		static void stub_handler(void *class_data,REGS *r);
		void timer_handler(REGS *r);
		void timer_wait(qword ticks);
		void timer_install();
		void SetPITFreq(word f);	
		void SetCountDown(dword seconds);
		dword GetCountDown(void);

		//Evenimentele de timerhandler permit executarea unor taskuri care ies rapid, la intervale de timp prestabilite
		//taskurile pe de alta parte, nu ies niciodata
		TimerHandler *AddTimerHandler(void (*chand)(void*),int period,void *paramaetru,int TID_owner);
		bool RemoveTimerHandler(TimerHandler *chand);
		bool AdjustHandlerPeriod(TimerHandler *chand,int period);
		int GetHandlerPeriod(TimerHandler *chand,int period);
		int GetTotalHandlers();
		void EnableTimer();
		void DisableTimer();

		static TIMER* GetInstance(int max_handlers = 0);
		static void FreeInstance();

	private:	
		TIMER(int _max_timer_handlers);
		~TIMER();

		static TIMER* instance;

		dword timer_ticks;
		word freq;
		byte* clock_backup;
		dword cnt_down;
		bool timer_activated;
		CMOS_DATE_TIME time;
		int max_timer_handlers;
		TimerHandler **handler_array;
		int handler_used;
};

#endif