;;; Kernel common library in x86_64 assembly

[bits 64]

global print
global strlen
global memset
global memsetw
global memcpy
global strcpy
global strncpy
global gdt_flush
global writemsr 
global readmsr 
global outb 
global inb 
global halt_catch_fire

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
	shr rax, 32 
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

; print(destination, source)
; returns current pointer to desination in rax
print:
	.loop:
		; move byte at address DS:ESI into AL
		lodsb
		; test for NULL terminated string
		or al, al 
		jz .done
		mov byte [rdi], al
		add rdi, 2
		jmp .loop
	.done:
		mov rax, rdi
		ret




