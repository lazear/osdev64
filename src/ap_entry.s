;;; Bootstrap an AP up to 64bit mode 
;;; 0x8000 execution point
;;; 0x7FF8 page directory 
;;; 0x7FF0 stack (32 bit location)
;;; 0x7FE8 entry function
;;; 0x7FE0 stack (64 bit)

[BITS 16]
org 0x8000

global start
start:
	cli
	xor ax, ax 			; Clear AX register
	mov ds, ax			; Set DS-register to 0 
	mov es, ax
	mov fs, ax
	mov gs, ax

	lgdt [gdt_desc] 	; Load the Global Descriptor Table

	mov eax, cr0
	or eax, 1               ; Set bit 0 
	mov cr0, eax
	;jmp $

	jmp GDT_CODE32:start32

[BITS 32]

start32:

	mov ax, GDT_DATA32
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	mov eax, [start-0x08]			; We need to put _init_pd on the stack
	mov cr3, eax				; set up page directory

	mov eax, cr0
	or eax, 0x80000000			; Enable paging
	mov cr0, eax

	mov esp, [start-0x10]			; Pass stack

	; Set LME bit in EFER MSR, which is bit 8
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr 

	; Set PAE bit in CR4
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; Set PG bit in CR0
	mov eax, cr0 
	or eax, 1 << 31
	mov cr0, eax
	
	; Reload segment registers
	mov ax, GDT_DATA64			
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	; Far jump to 64 bit compliant code segment
	jmp GDT_CODE64:start64

[BITS 64]
start64:
	mov rsp, [start-0x20]
	mov rax, [start-0x18]
	jmp rax

	hlt

;;; GLOBAL DESCRIPTOR TABLE
;;; Use a very simply GDT with both 32 and 64 bit segments just to bootstrap
align 32
gdt_null:
	dd 0
	dd 0

GDT_CODE32 equ $ - gdt_null
	dw 0xFFFF 	; Limit 0xFFFF
	dw 0		; Base 0:15
	db 0		; Base 16:23
	db 0x9A 	; Present, Ring 0, Code, Non-conforming, Readable
	db 0xCF		; Page-granular
	db 0 		; Base 24:31

GDT_DATA32 equ $ - gdt_null               
	dw 0xFFFF 	; Limit 0xFFFF
	dw 0		; Base 0:15
	db 0		; Base 16:23
	db 0x92 	; Present, Ring 0, Code, Non-conforming, Readable
	db 0xCF		; Page-granular
	db 0 		; Base 24:31

GDT_CODE64 equ $ - gdt_null
    dw 0
    dw 0
    db 0
    db 0x9A
    db 0x20
    db 0

GDT_DATA64 equ $ - gdt_null
    dw 0
    dw 0
    db 0
    db 0x92
    db 0x20
    db 0

gdt_desc:					; The GDT descriptor
	dw $ - gdt_null - 1		; Limit (size)
	dd gdt_null 			; Address of the GDT

times 0x100-($-$$) db 0 		; Fill up the file with zeros
