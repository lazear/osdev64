[bits 64]

section .text

extern printf
extern _kernel_start
extern _kernel_end

global stack_trace

; stack_trace(rbp)
; if RBP is null, start with current call stack
stack_trace:
	push rbp
	mov rbp, rsp
	sub rsp, 16

	; rcx = depth 
	mov rcx, -1

	; start with current stack frame, or RDI if non-NULL
	mov rax, rbp
	test rdi, rdi
	cmovne rax, rdi

	.l1:
		add rcx, 1
		; store current stack frame address
		mov [rsp], rax		
		; store current depth
		mov [rsp+8], rcx

		mov rdi, stack_trace_fmt 	; format string
		mov rsi, rcx				; depth
		mov rdx, [rax + 8]			; return address
		; test to make sure that RIP is within kernel space
		mov r8, _kernel_start
		cmp rdx, r8
		jb .exit

		mov r8, _kernel_end
		cmp rdx, r8
		ja .exit

		call printf

		mov rcx, [rsp+8] 	; restore depth
		mov rax, [rsp]		; restore rbp 
		mov rax, [rax]		; rbp = rbp[0]

		jmp .l1
	.exit:
		xor rax, rax
		mov rsp, rbp
		pop rbp
		ret

section .data
stack_trace_fmt:
	db "%2d: RIP %#x", 10, 0

