use16
org 0x7C00

NULL_SELECTOR = 0
DATA_SELECTOR = 2 shl 3 		; flat data selector (ring 0)
CODE_SELECTOR = 1 shl 3 		; 32-bit code selector (ring 0)

SEGMENT_LFB EQU 0x1000
OFFSET_LFB  EQU 0xF10

VESA_GINFO EQU 0x2000
VESA_MINFO EQU 0x2800

DELAY_1SECOND EQU 1000000

virtual at 0x3000
modes_list:
	 rw 1 ;resx
	 rw 1 ;resy
	 rb 1 ;bpp
	 rd 1 ;lfb
	 rw 1 ;video-mode
end virtual

virtual at 0x800
int_number      rb 1
realmode_error  rb 1
realmode_ax     rw 1
realmode_bx     rw 1
realmode_cx     rw 1
realmode_dx     rw 1
end virtual

virtual at 0x810
packet:
	.size    rw 1
	.bloks   rw 1
	.offset  rw 1
	.seg     rw 1
	.lower32 rd 1
	.upper32 rd 1
end virtual

;------------------------------------------;
;  Standard BIOS Parameter Block, "BPB".   ;
;------------------------------------------;
boot:	  
	xor ax, ax				  ; ax=0
	mov ds, ax				  ; segment de date = 0
	mov es, ax				  ; segment extra = 0
	mov ss, ax				  ; segment stiva = 0
	mov sp,0x7B00
	mov bp,0x7B00

	mov di,0x600
	mov si,0x7C00
	mov cx,0x100
	rep movsw

	jmp 0:0x600+continue_loading-boot

continue_loading:
	call read_42h

	call get_lfb	

	cli				
	call Enable_A20	

	lgdt [cs:GDTR]

	mov	eax,cr0 		
	or	al,1
	mov	cr0,eax

	jmp	CODE_SELECTOR:0x600+pm_start-boot

Enable_A20:
    pusha				 
    cld
    mov   al,255			   
    out   0xa1,al
    out   0x21,al
l.5:
    in	  al,0x64			   
    test  al,2				   
    jnz   l.5				   
    mov   al,0xD1			   
    out   0x64,al			   
l.6:
    in	  al,0x64			   
    test  al,2
    jnz   l.6				   
    mov   al,0xDF			   
    out   0x60,al			   
    mov   cx,0x14
l.7:					       
    out   0xed,ax			   
    loop  l.7				   
    popa
    ret

get_lfb:
	mov di,VESA_GINFO
	mov ax,0x4F00
	mov dword[VESA_GINFO],'VBE2'
	int 0x10
	mov si,word[VESA_GINFO+0x0e]
	push word[VESA_GINFO+0x10]
	pop ds
	mov bx,modes_list
	xor dx,dx
	cld
@@:
	lodsw
	push ds
	push 0
	pop ds
	mov word [bx+9],ax
	cmp ax,0xFFFF
	je @f
	mov cx,0x4F01
	xchg ax,cx
	mov di,VESA_MINFO
	int 0x10
	mov ax,word[VESA_MINFO+0x12]
	mov word[bx],ax
	mov ax,word[VESA_MINFO+0x14]
	mov word[bx+2],ax
	mov al,byte[VESA_MINFO+0x19]
	mov byte[bx+4],al	
	mov eax,dword[VESA_MINFO+0x28]
	mov dword[bx+5],eax	
	add bx,11
	inc dx
	pop ds
	jmp @b
@@:
	pop ds
	mov ax,0
	mov ds,ax
	movzx edx,dx
	mov dword[es:modes_list-4],edx
	ret

;----------------------------------------------------;
; read_42h - read sector(s) using INT13 extensions   ;
; Entry:                                             ;
;     dx = sector buffer segment                     ;
;     cx = sector count                              ;
;     eax = sector number (32-bit LBA)               ;
; Exit:                                              ;
;     cx = 1                                         ;
;     [lba_read] = number of bytes read              ;
;----------------------------------------------------;
read_42h:
	mov dl,0x80
	mov si,0x7CE
	mov ah,42h
	int 13h
	mov dl,0x80
	mov byte[0x7CE+7],0x20
	mov byte[0x7CE+8],0x81
	mov si,0x7CE
	mov ah,0x42
	int 13h
	ret

do_16pmode:
	  mov	  ax, 0x20
	  mov	  ds, ax
	  mov	  es, ax
	  mov	  fs, ax
	  mov	  gs, ax
	  mov	  ss, ax

	  cli
	  mov	  eax, cr0
	  and	  al, 0xFE
	  mov	  cr0, eax

	  jmp	  0:0x600+do_realm-boot

do_realm:
	  xor	  ax, ax
	  mov	  ds, ax
	  mov	  es, ax
	  mov	  ss, ax

	  lidt	  [0x600-boot+ridtr]		      ; realmode/variables.inc
	  sti

	  mov	  ax, word [realmode_ax]
	  mov	  bx, word [realmode_bx]
	  mov	  cx, word [realmode_cx]
	  mov	  dx, word [realmode_dx]

	  push	  ax				      ; this is some cool shit.. ;)
	  mov	  al, [int_number]	      ; interrupt to preform
	  mov	  [0x600-boot+$+5], al		          ; move it to right pos.
	  pop	  ax
	  int     0

	  jnc	  .no_error
	  mov	  byte [realmode_error], 1

     .no_error:
	  mov	  word [realmode_ax], ax
	  mov	  word [realmode_bx], bx
	  mov	  word [realmode_cx], cx
	  mov	  word [realmode_dx], dx

	  cli

	  lgdt    [0x820]
	  mov	  eax, cr0
	  or	  al, 1
	  mov	  cr0, eax

	  ;jmp	  pword 0x08:0x103E7
	  jmp	  pword 0x08:0x103FF

USE32

pm_start:
	mov	eax,DATA_SELECTOR	; load 4 GB data descriptor
	mov	ds,ax			; to all data segment registers
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax

	mov	eax,cr0
	or  eax,3
	and eax,0xFFFFFFFB	; enable FPU x87 
	mov	cr0,eax 		

	mov edx,CR4
	or  edx,0x00000600	; enable SSE   
	mov CR4,edx
	
	jmp far CODE_SELECTOR:0x10300

GDTR:					; Global Descriptors Table Register
  dw 5*8-1				; limit of GDT (size minus one)
  dq GDT				; linear address of GDT

GDT dw 000000,0,00000,000h		
    dw 0FFFFh,0,9A00h,0CFh		
    dw 0FFFFh,0,9200h,0CFh		
    dw 0FFFFh,0,9A00h,000h		
	dw 0FFFFh,0,9200h,000h

ridtr:
	dw   0x3FF
	dd   0

	rb boot+512-66-$

ptable:
	db 0x80,0x01,0x08,0x00,0x06,0x07,0x60,0xDB,0x00,0x01,0x00,0x00,0x00,0xDB,0x01,0x00
	db 0x10,0x00,0x80,0x00,0x00,0x00,0x00,0x10,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	

	dw 0xAA55				  ; semnatura sectorului de BOOT

