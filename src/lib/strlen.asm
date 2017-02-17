[bits 64]
global strlen
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
