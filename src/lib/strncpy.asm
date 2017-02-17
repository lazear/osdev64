[bits 64]
global strncpy
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
