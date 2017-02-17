[bits 64]
global strcpy
;;; (rdi = dest, rsi = src)
;;; increases rdi and rsi, trashes rax 
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