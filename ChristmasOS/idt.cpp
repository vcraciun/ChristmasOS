#include "idt.h"
#include "System.h"

IDT* IDT::instance = 0;

const char *exception_messages[] = {
"Division By Zero",  "Debug",                      "Non Maskable Interrupt","Breakpoint",         "Into Detected Overflow","Out of Bounds",           "Invalid Opcode","No Coprocessor",
"Double Fault",      "Coprocessor Segment Overrun","Bad TSS",               "Segment Not Present","Stack Fault",           "General Protection Fault","Page Fault",    "Unknown Interrupt",
"Coprocessor Fault", "Alignment Check",            "Machine Check",         "Reserved",           "Reserved",              "Reserved",                "Reserved",      "Reserved",
"Reserved",          "Reserved",                   "Reserved",              "Reserved",           "Reserved",              "Reserved",                "Reserved",      "Reserved"};

//meotde de servire a IRQ-urilor
void *irq_routines[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//pointeri la clasele de baza din care fac parte metodele de tratare a IRQ-urilor
void *irq_classes[16]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

IDT* IDT::GetInstance()
{
    if (!instance)
        instance = new IDT;
    return instance;
}

void IDT::FreeInstance()
{
    delete instance;
}

IDT::IDT()
{
	idt_install(); 
	isrs_install();
	irq_install(); 
}

IDT::~IDT()
{

}

void IDT::idt_set_gate(byte num, dword base, word sel, byte flags)
{
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void IDT::idt_install()
{
	*(word*)IDT_PTR_ADDR=IDT_TABLE_ADDR+IDT_DESCRIPTORS*8-1;
	*(dword*)(IDT_PTR_ADDR+2)=IDT_TABLE_ADDR;

	idt=(IDT_ENTRY*)IDT_TABLE_ADDR;

	memsetSSEB((byte*)idt,0,sizeof(IDT_ENTRY) * IDT_DESCRIPTORS);

    idt_load();	
}

void IDT::isrs_install()
{
	int i;
	
    idt_set_gate(0,  (unsigned)isr0,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(1,  (unsigned)isr1,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(2,  (unsigned)isr2,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(3,  (unsigned)isr3,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(4,  (unsigned)isr4,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(5,  (unsigned)isr5,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(6,  (unsigned)isr6,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(7,  (unsigned)isr7,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(8,  (unsigned)isr8,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(9,  (unsigned)isr9,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(10, (unsigned)isr10, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(11, (unsigned)isr11, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(12, (unsigned)isr12, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(13, (unsigned)isr13, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(14, (unsigned)isr14, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(15, (unsigned)isr15, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(16, (unsigned)isr16, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
	for (i=17;i<32;i++)
		idt_set_gate(i, (unsigned)isr15, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
		
	for (i=32;i<256;i++)
		idt_set_gate(i, (unsigned)isr_generic, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
}

void IDT::fault_handler(REGS *r)
{
    VIDEO* screen = VIDEO::GetInstance();

	__asm{
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	}
	screen->GotoXY(30 * screen->GetFont().width, 0);
    screen->SetAttr(0xFFFFFF,0,true);

    if (r->int_no < 32)
    {
		screen->gprintf("<<EXCEPTION -> %z[FFFF00]%d%z:%z[FF0000]%s!%z System HALTED!>>",r->int_no,exception_messages[r->int_no]);
        //Display::SwapVideoBuffers();
	}
    else
	{
		screen->gprintf("Unhandled INT fired! -> %z[A00000]%d%z",r->int_no);
        //Display::SwapVideoBuffers();
		return;
	}
		
    for (;;)
		;		
}

void IDT::irq_install_handler(int irq, void (*handler)(void*,REGS*), void *base_class)
{
    irq_routines[irq] = (void*)(unsigned)handler;
	irq_classes[irq] = base_class;
}

void IDT::irq_uninstall_handler(int irq)
{
    irq_routines[irq] = 0;
}

void IDT::irq_remap(void)
{
    outb(PIC_MASTER_CMD,  ICW1_INIT | ICW1_ICW4);
    outb(PIC_SLAVE_CMD,   ICW1_INIT | ICW1_ICW4);
    outb(PIC_MASTER_DATA, ISRS_SET0);
    outb(PIC_SLAVE_DATA,  ISRS_SET1);
    outb(PIC_MASTER_DATA, ICW1_INTERVAL4);
    outb(PIC_SLAVE_DATA,  ICW4_AUTO);
    outb(PIC_MASTER_DATA, ICW4_8086);
    outb(PIC_SLAVE_DATA,  ICW4_8086);
    outb(PIC_MASTER_DATA, 0);
    outb(PIC_SLAVE_DATA,  0);
}

void IDT::enable_irq(byte irq)
{
    byte pic_irq;
    word port;
    
    if (irq<PIC_MAX_PINS)
    {
        pic_irq=irq;
        port=PIC_MASTER_DATA;
    }
    else
    {
        pic_irq=irq-PIC_MAX_PINS;
        port=PIC_SLAVE_DATA;
        outb(PIC_MASTER_DATA,(inb(port))&((1<<SLAVE_PIC_PIN)^0xFF));
    }   
    outb(port,(inb(port))&((1<<pic_irq)^0xFF));
}

void IDT::irq_install()
{
    irq_remap();

    idt_set_gate(32, (unsigned)irq0,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(33, (unsigned)irq1,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(34, (unsigned)irq2,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(35, (unsigned)irq3,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(36, (unsigned)irq4,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(37, (unsigned)irq5,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(38, (unsigned)irq6,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(39, (unsigned)irq7,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(40, (unsigned)irq8,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(41, (unsigned)irq9,  IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(42, (unsigned)irq10, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(43, (unsigned)irq11, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(44, (unsigned)irq12, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(45, (unsigned)irq13, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(46, (unsigned)irq14, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
    idt_set_gate(47, (unsigned)irq15, IDT_CODE_SELECTOR, (IDT_FLAGS_PRESENT<<7)|(IDT_FLAGS_DPLRING0<<5)|(IDT_FLAGS_STORAGEINT<<4)|IDT_FLAGS_INTGATE32);
}

void IDT::irq_handler(REGS *r)
{
	__asm{
		nop
		nop
		nop
		nop
		nop
	}
    void (*handler)(void *clasa,REGS *r);

    handler = (void(*)(void*,REGS*))irq_routines[r->int_no - 32];
    if (handler)
		handler(irq_classes[r->int_no - 32],r);

    if (r->int_no >= 40)
    {
        outb(PIC_SLAVE_CMD, PIC_EOI);
    }

    outb(PIC_MASTER_CMD, PIC_EOI);
}

void IDT::disable_irqs()
{
	outb(PIC_MASTER_DATA,0xFF);
	outb(PIC_SLAVE_DATA,0xFF);
}

void IDT::enable_irqs()
{
	outb(PIC_MASTER_DATA,0x00);
	outb(PIC_SLAVE_DATA,0x00);	
}
