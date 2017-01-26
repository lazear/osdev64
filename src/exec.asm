[bits 64]

global execat

;execat (rdi = argc, rsi = argv, rdx = rip)
execat:
	jmp rdx 
	ret
