[bits 64]
global memcpy

;;; memcpy(rdi = destination, rsi = source, rdx = count)
;;; unoptimized memcpy
memcpy:
	mov rcx, rdx 
	rep movsb
	ret