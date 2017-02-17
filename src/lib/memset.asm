[bits 64]
global memset
global memsetw
global memsetd

;;; memset(rdi = destination, rsi = fill, rdx = count)
memset:
	mov rax, rsi
	mov rcx, rdx
	rep stosb
	ret

;;; memset(rdi = destination, rsi = fill, rdx = count)
memsetw:
	mov rax, rsi
	mov rcx, rdx
	shr rcx, 1
	rep stosw
	ret

;;; memset(rdi = destination, rsi = fill, rdx = count)
memsetd:
	mov rax, rsi
	mov rcx, rdx
	shr rcx, 2
	rep stosd
	ret