[bits 64]

global execat
global exit

;execat (rdi = argc, rsi = argv, rdx = rip)
execat:
	jmp rdx 
	ret

; exec_user(rdi = argc, rsi = argv, rdx = rip, rcx = rsp)
global exec_user
exec_user:	
	xor rax, rax
	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rbp, rcx
	push rax		; push SS
	push rcx		; push RSP

	pushf 			; push flags onto stack
	push 0x18|3		; push CS, requested priv. level = 3

	push rdx		; push RIP

	iretq

exit:
	add rbp, 16
	mov rsp, rbp
	ret