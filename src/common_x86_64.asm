; MIT License

; Copyright (c) Michael Lazear, 2016 - 2017 

; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:

; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.

; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.



; Common library in x86_64 assembly. Functions are defined in include/arch/x86_64/kernel.h
; SysV x86_64 ABI:
; First 6 parameters are passed in rdi, rsi, rdx, rcx, r8, and r9.
; Additional parameters are pushed onto the stack
; Callee must preserve rsp, rbp, rbx, r12-r15
; Return value is in RDX:RAX


[bits 64]

global idt_flush
global gdt_flush
global writemsr 
global readmsr 
global outb 
global inb 
global halt_catch_fire
global x2apic_enabled

x2apic_enabled:
	mov rax, 1
	cpuid
	test ecx, (1<<21)
	mov rax, rcx
	;xor rax, rax
	;setne al
	ret

;;; Disable interrupts, halt, and repeat.
halt_catch_fire:
	cli
	hlt 
	jmp halt_catch_fire

;;; void outb(rdi = port, rsi = data)
outb:
	mov rdx, rdi 
	mov rax, rsi 
	out dx, al 
	ret 

;;; uint16_t inb(rdi = port)
inb:
	mov rdx, rdi 
	xor rax, rax 
	in al, dx 
	ret


;;; writemsr(rdi = MSR register, rsi = contents)
writemsr:
	mov rax, rsi 
	mov rdx, rsi 
	; keep low bits 
	and eax, -1 
	; keep high bits
	shr rdx, 32
	mov rcx, rdi 
	wrmsr 
	ret

;;; readmsr(rdi = MSR register)
readmsr:
	push rbp
	xor rax, rax 
	mov rdx, rax
	mov rcx, rdi
	rdmsr 
	shl rdx, 32 
	or rax, rdx
	pop rbp
	ret


idt_flush:
	push rbp
	mov rbp, rsp
	sub rsp, 10 
	mov word  [rsp+0], 0xFFF 
	mov qword [rsp+2], rdi
	lidt [rsp]
	add rsp, 10
	mov rsp, rbp
	pop rbp
	ret

;;; gdt_flush(rdi = GDT descriptor)
;;; load and flush a new GDT, and task register
;;; KERNEL_CODE = 0x08
;;; KERNEL_DATA = 0x10 
;;; KERNEL_TSS = 0x30 
gdt_flush:
	;push rbp
	;mov rbp, rsp
	;hlt
	lgdt [rdi]

	push 0x10 			; push ss 
	push rsp 		; push rsp/rbp, same thing here 
	pushf 			; push rflags 
	push 0x08 		; push cs
	mov rax, .flush
	push rax 		; push rip 
	iretq			; call iret

.flush:
	; 0x10 is still on the stack, since we push rsp after ss
	pop rax
	mov es, ax
	mov ds, ax 
	mov gs, ax
	mov fs, ax
	mov ax, 0x30
	ltr ax


	ret

