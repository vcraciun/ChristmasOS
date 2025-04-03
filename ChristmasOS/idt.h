#ifndef __IDT__
#define __IDT__

#include "types.h"
#include "video.h"

#define IDT_DESCRIPTORS      512
#define IDT_CODE_SELECTOR    0x08
#define PMINTGATEDPL0        0x8E
#define PIC_MASTER_CMD       0x20
#define PIC_MASTER_DATA		 0x21
#define PIC_SLAVE_CMD		 0xA0
#define PIC_SLAVE_DATA		 0xA1
#define PIC_EOI              0x20
#define ICW1_ICW4			 0x01		
#define ICW1_SINGLE			 0x02		
#define ICW1_INTERVAL4		 0x04		
#define ICW1_LEVEL			 0x08		
#define ICW1_INIT			 0x10		
#define ICW4_8086			 0x01		
#define ICW4_AUTO			 0x02		
#define ICW4_BUF_SLAVE		 0x08		
#define ICW4_BUF_MASTER		 0x0C	
#define ICW4_SFNM			 0x10		
#define SLAVE_PIC_PIN        0x02
#define PIC_MAX_PINS         0x08

#define ISRS_SET0			 0x20
#define ISRS_SET1			 0x28

#define IDT_FLAGS_PRESENT    1
#define IDT_FLAGS_ABSENT     0
#define IDT_FLAGS_DPLRING0   0
#define IDT_FLAGS_DPLRING1   1
#define IDT_FLAGS_DPLRING2   2
#define IDT_FLAGS_DPLRING3   3
#define IDT_FLAGS_STORAGEINT 0
#define IDT_FLAGS_TASKGATE32 5
#define IDT_FLAGS_INTGATE16  6
#define IDT_FLAGS_TRAPGATE16 7
#define IDT_FLAGS_INTGATE32  0xE
#define IDT_FLAGS_TRAPGATE32 0xF

#define IDT_PTR_ADDR    0x7B80
#define IDT_TABLE_ADDR  0x1000
#define IDT_DESCRIPTORS 0x200

typedef struct _REGS{
    dword gs, fs, es, ds;
    dword edi, esi, ebp, esp, ebx, edx, ecx, eax;
    dword int_no, err_code;
    dword eip, cs, eflags, useresp, ss;    
}REGS;

typedef struct _IDT_ENTRY{
    word base_lo;
    word sel;
    byte  always0;
    byte  flags;
    word base_hi;
}IDT_ENTRY;

extern "C" void  idt_load();
extern "C" void  isr0();
extern "C" void  isr1();
extern "C" void  isr2();
extern "C" void  isr3();
extern "C" void  isr4();
extern "C" void  isr5();
extern "C" void  isr6();
extern "C" void  isr7();
extern "C" void  isr8();
extern "C" void  isr9();
extern "C" void  isr10();
extern "C" void  isr11();
extern "C" void  isr12();
extern "C" void  isr13();
extern "C" void  isr14();
extern "C" void  isr15();
extern "C" void  isr16();
extern "C" void  isr_generic();

extern "C" void  irq0();
extern "C" void  irq1();
extern "C" void  irq2();
extern "C" void  irq3();
extern "C" void  irq4();
extern "C" void  irq5();
extern "C" void  irq6();
extern "C" void  irq7();
extern "C" void  irq8();
extern "C" void  irq9();
extern "C" void  irq10();
extern "C" void  irq11();
extern "C" void  irq12();
extern "C" void  irq13();
extern "C" void  irq14();
extern "C" void  irq15();

class IDT{
public:
	void idt_install();
	void isrs_install();
	void irq_install();
	void disable_irqs();
	void enable_irqs();
    void enable_irq(byte irq);
	void irq_install_handler(int irq, void (*handler)(void*,REGS*),void *base_class);
	void irq_uninstall_handler(int irq);

	static IDT* GetInstance();
	static void FreeInstance();

private:
	IDT();
	~IDT();

	static IDT* instance;

	IDT_ENTRY *idt;
	void idt_set_gate(byte num, dword base, word sel, byte flags);
	void fault_handler(REGS *r);
	void irq_remap(void);
	void irq_handler(REGS *r);
};

#endif