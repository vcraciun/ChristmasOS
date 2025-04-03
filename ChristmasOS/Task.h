#ifndef __TASKS__
#define __TASKS__

#include "types.h"
#include "timer.h"
#include "keyboard.h"
#include "m_Mouse.h"

//iotype
#define MSG_INPUT 0
#define MSG_OUTPUT 1

//data_type
#define TIP_STRING 0
#define TIP_BYTE 1
#define TIP_WORD 2
#define TIP_DWORD 3
#define TIP_STRUCTURA 4

//state
#define READY 0
#define RUNNING 1
#define PAUSEED 2

typedef struct _THREAD_CONTEXT{
	dword R_GS;
	dword R_FS;
	dword R_ES;
	dword R_DS;
	dword R_EDI;
	dword R_ESI;
	dword R_EBP;
	dword R_ESP;
	dword R_EBX;
	dword R_EDX;
	dword R_ECX;
	dword R_EAX;
	dword int_id;
	dword reserved;
	dword R_EIP;
	dword R_CS;
	dword R_FLAGS;
}ThreadContext;

typedef struct TASK_MESSAGE{
	void *data;
	dword data_type;
	int io_type;
	dword addr_task_to_from;	//adresa taskului la care trimite/ de la care primim
}TaskMessage;

typedef struct TASK_STRUCT{
	dword TID;
	void (*handler)(void*);
	int priority;
	int state;
	ThreadContext *context;
	void *params;
	char *name;
	TaskMessage *mesaj;
	void **timer_handle_list;			//pot fi adaugate mai multe evenimente ontimer
}TaskHandle;

class Thread{
	private:
		TaskHandle **task_list;
		int running_tasks;
		int total_tasks;
		int current_Task;
		TIMER *timer;
		int timer_callbacks;

	public:
		Thread(TIMER *_tmrh,KEYBOARD *_keybh,Mouse *_mouseh);
		~Thread();

		static void stub_handler(void *class_data,ThreadContext *context);
		void task_handler(ThreadContext *context);	

		TaskHandle *CreateTask(void (*task_function)(void*),void *paramaetru,int sleep_time,char *name,int priority);
		bool KillTask(TaskHandle* task_handle);
		bool PauseTask(TaskHandle* task_handle);
		bool ResumeTask(TaskHandle* task_handle);
		bool SetTaskPriority(TaskHandle* task_handle,int priority);
		bool CreateTaskStack(TaskHandle* task_handle,int size);
		
		bool SendMessage(TaskHandle* task_handle,void *data,int data_type);
		bool ReceiveMessage(TaskHandle* task_handle,void **data,int *data_type);

		void AddTimerCallback(void *callback_func);
};


#endif