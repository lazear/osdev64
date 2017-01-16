#ifndef __MSR__
#define __MSR__

#define IA32_APIC_BASE 			0x01B
#define IA32_FEATURE_CONTROl 	0x03A
#define IA32_SYSENTER_CS 		0x174 
#define IA32_SYSENTER_ESP 		0x175
#define IA32_SYSENTER_EIP 		0x176
#define IA32_EFER 		0xC0000080

/* IA-32e mode system call. 
CPUID.80000001:EDX[29] must = 1*/
#define IA32_STAR 			0xC0000081 /* System call target address */
#define IA32_LSTAR 			0xC0000082 /* IA32e system call target address */
#define IA32_FMASK 			0xC0000084 /* System call flag mask */
#define IA32_FS_BASE 		0xC0000100
#define IA32_GS_BASE 		0xC0000101
#define IA32_KERNEL_GS_BASE 0xC0000102 /* SWAP target */
#endif