[bits 64]
section .text 
global setjmp 
global longjmp 

; Save a jmp_buf, containing registers that must be preserved across calls
; rbx, rsp, rbp, r12, r13, r15, r15, rip


;;; setjmp(rdi= jmp_buf)
setjmp:
	pop rsi 				; rip is at top of stack 
	xor rax, rax 			; return value
	mov [rdi+0x00], rbx 
	mov [rdi+0x08], rsp		; Post-return rsp
	push rsi 				; Realign stack 
	mov [rdi+0x10], rbp 
	mov [rdi+0x18], r12 
	mov [rdi+0x20], r13 
	mov [rdi+0x28], r14 
	mov [rdi+0x30], r15
	mov [rdi+0x38], rsi 	; rip

	ret 

;;; longjmp(rdi= jmp_buf, rsi= value)
longjmp:
	mov rax, rsi 
	mov rbx, [rdi+0x00]
	mov rsp, [rdi+0x08]
	mov rbp, [rdi+0x10]
	mov r12, [rdi+0x18]
	mov r13, [rdi+0x20]
	mov r14, [rdi+0x28]
	mov r15, [rdi+0x30]
	jmp [rdi+0x38]