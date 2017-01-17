/*
cpuid.c
===============================================================================
MIT License
Copyright (c) Michael Lazear 2016-2017 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
*/


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define cpuid(in, a, b, c, d) \
	asm volatile("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

/* Feature flags for EAX=1 cpuid leaf */
static struct feature_flags {
	char value;
	char desc[32];
} feat_ecx[] = {
	{0, "SSE3"},
	{19, "SSE4.1"},
	{20, "SSE4.2"},
	{21, "x2APIC"},
}, feat_edx[] = {
	{0, "Floating Point Unit"},
	{1, "Virtual 8086"},
	{3, "Page Size Extension"},
	{5, "Model Specific Registers"},
	{6, "Physical Address Extension"},
	{9, "Local APIC"},
	{11, "Fast system call"},
	{18, "Processor Serial Number"},
	{22, "ACPI"},
	{23, "MMX Technology"},
	{25, "SSE"},
	{26, "SSE2"},
	{28, "Hyperthreading"},				// Check EBX
	{30, "64-bit processor"}
};

void cpuid_vendor() {
	uint32_t eax, ebx, ecx, edx;
	cpuid(0, eax, ebx, ecx, edx);
	char vendor[13];
	for (int i = 0; i < 4; i++)	{
		vendor[i]		= ebx >> (8 * i);
		vendor[i + 4]	= edx >> (8 * i);
		vendor[i + 8]	= ecx >> (8 * i);
	}
	vendor[12] = '\0';
	vga_puts(vendor);
}

void cpuid_features(void) {
	uint32_t unused, ecx, edx;
	cpuid(1, unused, unused, ecx, edx);
	printf("CPU Features:\n");

	for (int i = 0; i < sizeof(feat_edx)/sizeof(struct feature_flags); i++) 
		if (edx & (1<<feat_edx[i].value))	printf("\t%s\n", feat_edx[i].desc);
	for (int i = 0; i < sizeof(feat_ecx)/sizeof(struct feature_flags); i++) 
		if (ecx & (1<<feat_ecx[i].value))	printf("\t%s\n", feat_ecx[i].desc);

	cpuid(0x80000001, unused, unused, unused, edx);
	printf("Extended Features (%b):\n", edx);
	if (edx & (1<<29))		printf("\tAMD64 Compliant\n");

}


void cpuid_name(void) {
	uint32_t eax, ebx, ecx, edx, max_eax, unused;

	cpuid(1, eax, ebx, ecx, edx);
	// int model		= (eax >> 4) & 0xF;
	// int ext_model	= (eax >> 16) & 0xF;
	// int family		= (eax >> 8) & 0xF;
	// int ext_family 	= (eax >> 20) & 0xFF;
	// int type		= (eax >> 12) & 0x3;

	// int brand		= (ebx & 0xFF);
	// int stepping	= (eax & 0xF);
	// int reserved	= (eax >> 14);
	// int signature	= (eax);

	//printf("Family %d Model %d\n", (family + ext_family), (model | (ext_model<<4)));

	char vendor[16];

	for (int q = 0x80000002; q < 0x80000005; q++) {
		cpuid(q, eax, ebx, ecx, edx);
		*((uint32_t*)vendor)		= eax;
		*((uint32_t*)vendor + 1)	= ebx;
		*((uint32_t*)vendor + 2)	= ecx;
		*((uint32_t*)vendor + 3)	= edx;
		*((uint32_t*)vendor + 4)	= 0;
		vga_puts(vendor);
	}
	vga_putc('\n');
}

void cpuid_detect(void) {
	uint32_t x, eax, ebx;
	cpuid(0, eax, ebx, x, x);
	printf("EAX: %#x\n", eax);
	switch (ebx) {
		case 0x756E6547: 	// Intel
		case 0x68747541: {	// AMD
			cpuid_vendor();
			vga_putc('\t');
			cpuid_name();
			cpuid_features();
			break;
		} default:
			printf("Unknown x86 CPU (0x%X) Detected\n", ebx);
			break;
	}

}