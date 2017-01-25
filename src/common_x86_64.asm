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

; Common library in x86_64 assembly. Functions are defined in include/common.h
; SysV x86_64 ABI:
; First 6 parameters are passed in rdi, rsi, rdx, rcx, r8, and r9.
; Additional parameters are pushed onto the stack
; Callee must preserve rsp, rbp, rbx, r12-r15
; Return value is in RDX:RAX

; Memcpy/memset etc are unoptimized.

[bits 64]

global strlen
global strcpy
global strncpy
global memcpy
global memset
global memsetw

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
	push rbp
	mov rax, rsi 
	mov rdx, rsi 
	; keep low bits 
	and rax, 0x00000000FFFFFFFF 
	; keep high bits
	shr rdx, 32
	mov rcx, rdi 
	wrmsr 
	pop rbp
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

;;; gdt_flush(rdi = GDT descriptor)
;;; load and flush a new GDT, and task register
;;; KERNEL_CODE = 0x08
;;; KERNEL_DATA = 0x10 
;;; KERNEL_TSS = 0x30 
gdt_flush:
	lgdt [rdi]

	; It's important to push rbp and not rsp
	; otherwise we'll lose the return address 
	push 0 			; push ss 
	push rbp 		; push rsp 
	pushf 			; push rflags 
	push 0x08 		; push cs
	mov rax, .flush
	push rax 		; push rip 
	iretq			; call iret

.flush:
	mov rax, 0x10
	mov es, ax
	mov ds, ax 
	mov gs, ax
	mov fs, ax

	mov ax, 0x30
	ltr ax
	pop rbp
	ret


;;; memset(rdi = destination, rsi = fill, rdx = count)
memset:
	mov rax, rsi
	mov rcx, rdx
	rep stosb
	ret

;;; memsetw(rdi = destination, rsi = fill, rdx = count)
memsetw:
	mov rax, rsi
	mov rcx, rdx 
	rep stosw 
	ret

;;; memcpy(rdi = destination, rsi = source, rdx = count)
;;; unoptimized memcpy
memcpy:
	mov rcx, rdx 
	rep movsb

;;; strncpy(rdi = destination, rsi = source, rdx = length)
;;; copy until rdx, or source byte is zero 
strncpy:
	push rbp 
	mov rbp, rsp
	sub rsp, 8
	; save desination buffer
	mov [rsp - 8], rdi 
	xor rax, rax 

	.l1:
		lodsb 		; al = [rsi++]
		test al, al  
		jz .done 	; al = 0, done 
		stosb  		; [rdi++] = al
		dec rdx 
		test rdx, rdx 	
		jnz .l1 		; length = rdx, done
	.done:
	; C standard library says that we should fill up dest
	; buffer with zeroes if length remains
	mov rcx, rdx 
	mov al, 0
	rep stosb

	; restore original destination buffer
	mov rax, [rsp - 8]
	add rsp, 8 
	mov rsp, rbp
	pop rbp 
	ret

; (rdi = dest, rsi = src)
; increases rdi and rsi, trashes rax 
strcpy:
	xor rax, rax 
	mov rcx, rax
	.l1:
		lodsb  		; al = [esi]
		test al, al 
		jz .done  	; al = 0, EOF 
		stosb 		; [edi] = al
		jmp .l1
	.done:
		ret

; rax = strlen(rdi)
; trashes rax, rsi, rcx
strlen:
	xor rax, rax 
	mov rcx, rax
	mov rsi, rdi
	.loop:
		lodsb
		test al, al 
		jz .done
		inc rcx 
		jmp .loop 
	.done:
		mov rax, rcx
		ret
