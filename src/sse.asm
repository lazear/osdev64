;;; Learning how to use Streaming SIMD Extensions 4.2


[bits 64]
section .text 


;===============================
; IMM8 Control Byte Operation
;
;***** Source Data Format
; Both 128 bit sources treated as...
; [1:0] = 00b, packed unsigned bytes 
; [1:0] = 01b, packed unsigned words 
; [1:0] = 10b, packed signed bytes 
; [1:0] = 11b, packed signed words 
;***** Aggregation Operation
; Mode of per-element comparison
; [3:2] = 00b, "equal any"
; [3:2] = 01b, "ranges"
; [3:2] = 10b, "equal each"
; [3:2] = 11b, "equal ordered"
; ** Equal any: find characters from a set
; ** First operand is character set, second is string
; Range:
; First operand is character set, second is string 
; IntRes1 |= (second[i] > first[j]) & (second[i] < first[j+1])
; ** Equal each: compare two strings, byte by byte
; ** Equal ordered: substring search
;***** Polarity
; Specifies intermediate processing to be performed
; [5:4] = 00b, Positive polarity, IntRes2 = IntRes1
; [5:4] = 01b, Negative polarity, IntRes2 = -1 XOR IntRes1
; [5:4] = 10b, Mask (+), IntRes2 = IntRes1
; [5:4] = 11b, Mask (-), IntRes2[i] = IntRes1[i] if src[i] invalid,
;		else IntRes2[i] = ~IntRes1[i]
;***** Output selection
; Specifies final operation to produce the output
; [6] = 0/1 Least or Most significant index for STRI 
; [6] = 0/1 Bit mask or byte/word mask for STRM 
;***** EFLAGS:
; for IMPLICIT length;
; Carry: Reset if IntRes2 = 0, set otherwise
; Zero: Set if any byte/word in xmm2 is NULL, reset otherwise
; Sign: Set if any byte/word in xmm1 is NULL, reset otherwise
; Over: IntRes2[0]



global sse42_enabled
sse42_enabled:
	mov rax, 1
	cpuid 
	; and ecx, (1<<20)
	xor rax, rax
	test ecx, (1<<20)
	jz .fail 

	mov rax, cr0
	and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
	or ax, 0x2			;set coprocessor monitoring  CR0.MP
	mov cr0, rax
	mov rax, cr4
	or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	mov cr4, rax

	mov rax, 1
	ret
.fail:
	mov rax, 0
	ret


;;; sse_strchr(const char* s, char c)
global sse_strchr
sse_strchr:
	push rbp  		; get the stack aligned
	mov rbp, rsp 

	pxor xmm0, xmm0
	mov [rsp-16], rsi
	movdqa xmm1, [rsp-16] 		
	pshufb xmm1, xmm0		; xmm1 = cccccccccccccccc
	xor rax, rax
	mov rcx, rax 
	mov rax, -16
	.l1:
		add rax, 16
		;Unsigned bytes, equal any, positive polarity, lsb
		pcmpistri xmm1, [rdi+rax], 0000000b
		jc .done 	; CF = 1 if IntRes2 != 0
		;int 3
		jnz .l1 	; ZF = 1 if NULL byte in [rdi+rax]
		
	.fail: 			; Fall through to failure
		xor rax, rax
		cmovc rax, rcx   ; IntRes2 != 0, ZF = 1
		ret
	.done:
	int 3
		lea rax, [rdi+rcx] 	; RAX = pointer to string at position

		mov rbp, rsp
		pop rbp
		ret


; rdi = dest, rsi = source, rdx = count
global sse_memcpy_aligned:
sse_memcpy_aligned:
	mov rcx, rdx 
	and rcx, 0xF			; get unaligned count
	shr rdx, 4 				; divide RDX by 16
	test rdx, rdx  			; if RDX is 0, move however many bytes
	je .done
.L1:
	;sub rdx, r10 			; subtract number of bytes moved

	cmp rdx, 128
	jae .M7
	jmp .done
	; mov r9, rdx 			; R9 = number of remaining bytes
	; shr r9, 4 				; R9 = Multiples of 16
	; ;int 3
	; lea rax, [(r9*8)+.JT]
	; jmp [rax]
	;int 3
	.JT:
		dq .done, .M0, .M1, .M2, .M3, .M4, .M5, .M6, .M7

	; Move 0x80 bytes	
	.M7:
		sub rdx, 128
		;int 3
		movdqa xmm7, [rsi+0x70]
		movdqa [rdi+0x70], xmm7
	; Move 0x70 bytes
	.M6:
		movdqa xmm6, [rsi+0x60]
		movdqa [rdi+0x60], xmm6
	; Move 0x60 bytes
	.M5:
		movdqa xmm5, [rsi+0x50]
		movdqa [rdi+0x50], xmm5
	; Move 0x50 bytes
	.M4:
		movdqa xmm4, [rsi+0x40]
		movdqa [rdi+0x40], xmm4
		; Move 0x40 bytes	
	.M3:
		movdqa xmm3, [rsi+0x30]
		movdqa [rdi+0x30], xmm3
	; Move 0x30 bytes
	.M2:
		movdqa xmm2, [rsi+0x20]
		movdqa [rdi+0x20], xmm2
	; Move 0x20 bytes
	.M1:
		movdqa xmm1, [rsi+0x10]
		movdqa [rdi+0x10], xmm1
	; Move 0x10 bytes
	.M0:
		movdqa xmm0, [rsi+0]
		movdqa [rdi+0], xmm0

		add rsi, 128 
		add rdi, 128
		jmp .L1

	.done:
		; add rsi, r8
		; add rdi, r8
	;rep movsb
	ret



global sse_strlen
sse_strlen:
	xor rax, rax 
	pxor xmm0, xmm0
	mov rax, -16
	.loop:
		; imm[1:0] = 0, source data is unsigned bytes
		; imm[3:2] = 2, equal each aggregation
		; imm[5:4] = 0, positive polarity, res2 = res1
		; imm[6] = 0, ecx is LS bit
		add rax, 16
		; packed compare implicit length strings, return index (in ECX)
		; Zero Flag set if any byte in source operand is zero
		pcmpistri xmm0, [rdi + rax], 001000b
		jnz .loop 
		; rcx contains offset from [rdi+rax] where NULL terminator 
		; is found
	add rax, rcx

	ret

global sse_strcmp
sse_strcmp:
	pxor xmm0, xmm0
	pxor xmm1, xmm1

	xor rax, rax 
	mov rax, -16 
	.loop:
		add rax, 16
		movdqu xmm1, [rdi+rax]
		; Packed Compare Implicit Length Strings, return index (in ECX)
		; Negative polarity, differing characters have value of 1
		; Least significant bit flag set, so first differing index is in ECX
		pcmpistri xmm1, [rsi+rax], 0011000b
		; zero flag set if any byte == NULL
		jnz .loop 

	; carry flag set if IntRes2 == 0
	jc .diff 
	xor rax, rax 
	ret
	.diff:
	movzx rax, byte [rdi+rcx]
	movzx rdx, byte [rsi+rcx]
	sub rax, rdx 
	ret

;;; Return byte mask for characters
global sse_strcmp_mask
sse_strcmp_mask:
	pxor xmm0, xmm0
	pxor xmm1, xmm1

	xor rax, rax 
	mov rax, -16 
	.loop:
		add rax, 16
		movdqu xmm1, [rdi+rax]
		; Packed Compare Implicit Length Strings, return Mask (in XMM0)
		; Negative polarity, byte mask, equal each
		pcmpistrm xmm1, [rsi+rax], 1011000b
		; zero flag set if any byte == NULL
		jnz .loop 
	; Zero flag has been set, we've reached implicit end of string.
	pmovmskb eax, xmm0 
	ret

;;; Return byte mask for characters
global sse_strstr_mask
sse_strstr_mask:
	pxor xmm0, xmm0
	pxor xmm1, xmm1

	xor rax, rax 			; clear RAX 
	mov rcx, rax 			; clear RCX 

	mov rax, -16 
	.loop:
		add rax, 16
		movdqu xmm1, [rsi]
		; Packed Compare Implicit Length Strings, return Mask (in XMM0)
		; Positive polarity, byte mask, equal ordered
		pcmpistrm xmm1, [rdi+rax], 1001100b
		; Zero flag set if any byte == NULL
		; Carry flag set is ALL bytes == NULL
		jnc .loop 
	; Transfer a byte mask from XMM to EAX
	; If MSB of each byte in XMM0 is 1, then set corresponding index bit in EAX
	pmovmskb eax, xmm0	
	popcnt eax, eax
	; popcnt eax, eax would give us how many times the substring occurred
	ret
