format COFF

use32

extrn '_main' as main
extrn '?fault_handler@IDT@@AAEXPAU_REGS@@@Z' as fault_handler
extrn '?irq_handler@IDT@@AAEXPAU_REGS@@@Z' as irq_handler  

RM_ESP EQU 0x7E00
RM_EBP EQU 0x7E04
PM_ESP EQU 0x7E08
PM_EBP EQU 0x7E0C

public start as '_mainCRTStartup'
start:
	mov dword [RM_ESP],esp
	mov dword [RM_EBP],ebp
	;stiva incepe la adrese mari
	mov esp,0x300000
	mov ebp,0x300000
	call main+5+22
	jmp $
ret

reserved: times 100-($-$$) db 0
signature: db 'BOOT',0
		   dd 0x000101
		   db 'Christmas Boot Manager'        
		   
;DoRmInt(int,ax,bx,cx,dx)
public DoRmInt as '_DoRmInt'
DoRmInt:
	push ebp
	mov ebp,esp
	sub esp,0x40
	mov eax,dword[ebp+8]
	mov byte[0x800],al
	mov eax,dword[ebp+12]
	mov word[0x802],ax
	mov eax,dword[ebp+16]
	mov word[0x804],ax
	mov eax,dword[ebp+20]
	mov word[0x806],ax
	mov eax,dword[ebp+24]
	mov word[0x808],ax
	call rmode_int		
	mov esp,ebp
	pop ebp
ret

rmode_int:
	;backup ebp/esp
	mov dword[PM_ESP],esp
	mov dword[PM_EBP],ebp
	;use RM stack
	mov esp,dword[RM_ESP]
	mov ebp,dword[RM_EBP]

	;disable IRQS
	mov	  al, 0xFF
	out	  0x21, al
	out	  0xA1, al

	;remap PICs
	mov   al,11h
	out   0x20,al
	out   0xA0,al
	mov   al,0x08
	out   0x21,al
	mov   al,0x70
	out   0xA1,al
	mov   al,4
	out   0x21,al
	mov   al,2
	out   0xA1,al
	mov   al,1
	out   0x21,al
	out   0xA1,al

	;continuam in MBR, pentru ca acolo codul este pe 16 biti
	jmp   pword 0x18:0x6E4
	
gobackto_pm:
	mov	  ax, 0x10			     
	mov	  ds, ax
	mov	  es, ax
	mov	  fs, ax
	mov	  gs, ax
	mov	  ss, ax

	;restore PM stack
	mov esp,dword[PM_ESP]
	mov ebp,dword[PM_EBP]

	lidt  [0x7B80]
	
	;remap PICs
	mov  al,0x11					  ; put both 8259s in init mode
	out  0x20,al
	out  0xA0,al
	mov  al,0x20					  ; remap pic irq0-irq7 -> int 0x20-27
	out  0x21,al
	mov  al,0x28
	out  0xA1,al					  ; remap pic irq8-irq15 -> int 0x28-30
	mov  al,4						  ; icw3 pic1(master)
	out  0x21,al					  ; bit 2=1: irq2 is the slave
	mov  al,2						  ; icw3 pic2
	out  0xA1,al					  ; bit 1=1: slave id is 2
	mov  al,1						  ; icw4 to both controllers
	out  0x21,al					  ; bit 0=1: 8086 mode
	out  0xA1,al

	;enable IRQs
	mov	  al, 0
	out	  0x21, al
	out	  0xA1, al
	;sti
	clc					          ; clear carry.
	cmp	  byte[0x801], 1		  ; if error, then
	jne	  .end
	stc					      ; set carry.
	.end:
	ret

public gdt_flush as '_gdt_flush'
gdt_flush:
    lgdt [0x820]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:$+7
flush2:
    ret

public idt_load as '_idt_load'
idt_load:
	lidt [0x7B80]
	ret

public isr_common_stub as '_isr_common_stub'
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
	mov eax,fault_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
	
public irq_common_stub as '_irq_common_stub'
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax 

	mov eax,irq_handler
    call eax
	sub esp,4   ;corectie facuta pentru parametrul de clasa de la irq_handler

	;call task_scheduler

    pop eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

;------------------------------------------------------ISRs-----------------------------------------
public isr0 as '_isr0'
isr0:
    cli
    push 0
    push 0
    jmp isr_common_stub

public isr1 as '_isr1'
isr1:
    cli
    push 0
    push 1
    jmp isr_common_stub

public isr2 as '_isr2'
isr2:
    cli
    push 0
    push 2
    jmp isr_common_stub

public isr3 as '_isr3'
isr3:
    cli
    push 0
    push 3
    jmp isr_common_stub

public isr4 as '_isr4'
isr4:
    cli
    push 0
    push 4
    jmp isr_common_stub

public isr5 as '_isr5'	
isr5:
    cli
    push 0
    push 5
    jmp isr_common_stub

public isr6 as '_isr6'
isr6:
    cli
    push 0
    push 6
    jmp isr_common_stub

public isr7 as '_isr7'	
isr7:
    cli
    push 0
    push 7
    jmp isr_common_stub

public isr8 as '_isr8'	
isr8:
    cli
	push 0
    push 8
    jmp isr_common_stub

public isr9 as '_isr9'	
isr9:
    cli
    push 0
    push 9
    jmp isr_common_stub

public isr10 as '_isr10'
isr10:
    cli
	push 0
    push 10
    jmp isr_common_stub

public isr11 as '_isr11'
isr11:
    cli
	push 0
    push 11
    jmp isr_common_stub

public isr12 as '_isr12'
isr12:
    cli
	push 0
    push 12
    jmp isr_common_stub

public isr13 as '_isr13'
isr13:
    cli
	push 0
    push 13
    jmp isr_common_stub

public isr14 as '_isr14'
isr14:
    cli
	push 0
    push 14
    jmp isr_common_stub

public isr15 as '_isr15'
isr15:
    cli
    push 0
    push 15
    jmp isr_common_stub

public isr16 as '_isr16'	
isr16:
    cli
    push 0
    push 16
    jmp isr_common_stub

public isr_generic as '_isr_generic'	
isr_generic:
    cli
    push 0
    push 50
    jmp isr_common_stub

;---------------------------------------------------IRQs----------------------------------------	
public irq0 as '_irq0'
irq0:
    cli
    push 0
    push 32
    jmp irq_common_stub

public irq1 as '_irq1'
irq1:
    cli
    push 0
    push 33
    jmp irq_common_stub

public irq2 as '_irq2'
irq2:
    cli
    push 0
    push 34
    jmp irq_common_stub

public irq3 as '_irq3'
irq3:
    cli
    push 0
    push 35
    jmp irq_common_stub

public irq4 as '_irq4'
irq4:
    cli
    push 0
    push 36
    jmp irq_common_stub

public irq5 as '_irq5'
irq5:
    cli
    push 0
    push 37
    jmp irq_common_stub

public irq6 as '_irq6'
irq6:
    cli
    push 0
    push 38
    jmp irq_common_stub

public irq7 as '_irq7'
irq7:
    cli
    push 0
    push 39
    jmp irq_common_stub

public irq8 as '_irq8'
irq8:
    cli
    push 0
    push 40
    jmp irq_common_stub

public irq9 as '_irq9'
irq9:
    cli
    push 0
    push 41
    jmp irq_common_stub

public irq10 as '_irq10'
irq10:
    cli
    push 0
    push 42
    jmp irq_common_stub

public irq11 as '_irq11'
irq11:
    cli
    push 0
    push 43
    jmp irq_common_stub

public irq12 as '_irq12'
irq12:
    cli
    push 0
    push 44
    jmp irq_common_stub

public irq13 as '_irq13'
irq13:
    cli
    push 0
    push 45
    jmp irq_common_stub

public irq14 as '_irq14'
irq14:
    cli
    push 0
    push 46
    jmp irq_common_stub

public irq15 as '_irq15'
irq15:
    cli
    push 0
    push 47
    jmp irq_common_stub

public type_info as '??_7type_info@@6B@'
type_info:
	dd 0
	dd 0