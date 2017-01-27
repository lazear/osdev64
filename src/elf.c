/*
elf.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

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

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <common.h>
#include <assert.h>
#include <elf.h>
#include <frame.h>
#include <mmu.h>
#include <desc.h>
#include <interrupts.h>

void elf_objdump(void* data) {
	Elf64_Ehdr *ehdr = (Elf64_Ehdr*) data;

	/* Make sure the file ain't fucked */
	// assert(ehdr->e_ident[0] == ELF_MAGIC);
	// assert(ehdr->e_machine 	== EM_386);

	char *types[] = { "NONE", "RELOCATABLE", "EXECUTABLE", "SHARED", "CORE"};

	printf("OBJDUMP\n");
	printf("ELF ident\t%x\t",ehdr->e_ident[0]);     
	printf("Type %s\t", types[ehdr->e_type]);                
	printf("Machine %s\n", "i386");              
	printf("Version \t%x\t",ehdr->e_version);              
	printf("Entry\t%x\t",ehdr->e_entry);                
         
	printf("Flags\t%x\n",ehdr->e_flags);           
	

	/* Parse the program headers */
	Elf64_Phdr* phdr 		= (size_t) data + ehdr->e_phoff;
	Elf64_Phdr* last_phdr 	= (size_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum);
	while(phdr < last_phdr) {
		printf("LOAD:\toff 0x%x\tvaddr\t0x%x\tpaddr\t0x%x\n\t\tfilesz\t%d\tmemsz\t%d\talign\t%d\t\n",
		 	phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, phdr->p_align);
		phdr++;
	} 

	//uint32_t* buf = ext2_file_seek(ext2_inode(1,14), BLOCK_SIZE, ehdr->e_shoff);

	/* Parse the section headers */
	// Elf64_Shdr* shdr 		= ((uint32_t) data) + ehdr->e_shoff;
	// Elf64_Shdr* sh_str		= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shstrndx);
	// Elf64_Shdr* last_shdr 	= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shnum);

	// Elf64_Shdr* strtab 		= NULL;
	// Elf64_Shdr* symtab		= NULL;
	// Elf64_Shdr* reltab		= NULL;

	// char* string_table 		= (uint32_t) data + sh_str->sh_offset;

	// shdr++;					// Skip null entry
	// int q = 1;

	// printf("Idx %19s Size\t Address    Offset    Align Type\n", "Name");
	// while (shdr < last_shdr) {	
	// 	printf("%2d:%20s %#8x %#8x %5d %#5x %d\n", 
	// 		q++, (string_table + shdr->sh_name), shdr->sh_size,
	// 		shdr->sh_addr, shdr->sh_offset, shdr->sh_addralign, shdr->sh_type);
	// 	if (strcmp(".symtab", string_table + shdr->sh_name) == 0 && shdr->sh_type == SHT_SYMTAB)
	// 		symtab = shdr;
	// 	if (strcmp(".strtab", string_table + shdr->sh_name) == 0 && shdr->sh_type == SHT_STRTAB)
	// 		strtab = shdr;
	// 	if (strcmp(".rel.text", string_table + shdr->sh_name) == 0 && shdr->sh_type == SHT_REL)
	// 		reltab = shdr;
	// 	shdr++;
	// }

	// if (!strtab || !symtab) {
	// 	vga_pretty("ERROR: Could not load symbol table", 0x4);
	// 	return;
	// }

	// elf32_sym* sym 		= ((uint32_t) data) + symtab->sh_offset;
	// elf32_sym* last_sym = (uint32_t) sym + symtab->sh_size;
	// void* strtab_d 		= ((uint32_t) data) + strtab->sh_offset;

	/* Output symbol information*/
	

/* BEGIN RELOCATION CODE */	
	// while(sym < last_sym) {
	// 	if (sym->st_name) 
	// 		printf("%x %s\t0x%x\n", sym->st_info, (char*) (sym->st_name + (uint32_t)strtab_d), sym->st_value);
	// 	sym++;
	// }

	// if (reltab) {
	// 	printf("Found relocation table\n");
	// 	printf("Link: %x\tInfo %x\n", reltab->sh_link, reltab->sh_info);

	// 	elf32_rel* r 	= ((uint32_t) data) + reltab->sh_offset;
	// 	elf32_rel* last = ((uint32_t) r) + reltab->sh_size;
		
	// 	Elf64_Shdr* target = ((uint32_t) data + ehdr->e_shoff) + (reltab->sh_info * ehdr->e_shentsize);


	// 	while(r < last) {

	// 		uint8_t t 	= (unsigned char) (r->r_info);
	// 		uint8_t s 	= (r->r_info) >> 8;
	// 		sym 		= (((uint32_t) data) + symtab->sh_offset);
	// 		sym += s;
			
	// 		char* sym_name = (char*) (sym->st_name + (uint32_t)strtab_d);
	

	// 		uint32_t addend = *(uint32_t*) ((uint32_t) data + target->sh_offset + r->r_offset);
	// 		addend -= r->r_offset;
	// 		addend += 0;
	// 		printf("addr %x addend %x type %X %s\n", r->r_offset, addend, t, sym_name);
	// 		//printf("value @ offset: %x %x\n", addend, addend - r->r_offset);
	// 		r++;
	// 	}
	// }	
	
}



// struct elf_executable {
// 	uint32_t* pd;	/* page directory */
// 	struct _proc_mmap* m;
// 	elf32_ehdr* ehdr;
// 	uint32_t esp;
// 	uint32_t eip;
// 	void (*execute)(struct elf_executable*);
// };

// extern void enter_usermode(uint32_t eip, uint32_t esp);


void memcpy(void *s1,  void *s2, size_t n) {
	uint8_t* src = (uint8_t*) s2;
	uint8_t* dest = (uint8_t*) s1;
	int i;
	for (i = 0; i < n; i++)
		dest[i] = src[i];
}

void elf_load(void* data) {
	Elf64_Ehdr * ehdr = (Elf64_Ehdr*) data; 

	Elf64_Phdr* phdr 		= (void*) ((size_t) data + ehdr->e_phoff);
	Elf64_Phdr* last_phdr 	= (void*) ((size_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum));
	size_t off = (phdr->p_vaddr - phdr->p_paddr);


	while(phdr < last_phdr) {
				if (phdr->p_type != 1) {
			phdr++;
			continue;
		}
		
		/*printf("LOAD:\toff 0x%x\tvaddr\t0x%x\tpaddr\t0x%x\n\t\tfilesz\t%d\tmemsz\t%d\talign\t%x\t\n",
		 	phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, phdr->p_align);
		*/
		for (int i = 0; i <= phdr->p_memsz; i += 0x1000) {
			struct page* p = mmu_req_page(phdr->p_vaddr + i, 0x7);
		}
		memset(phdr->p_vaddr, 0, phdr->p_memsz);
		memcpy(phdr->p_vaddr, (size_t)data + phdr->p_offset, phdr->p_filesz);
		phdr++;
	}
	last_phdr--;
	extern void execat(size_t argc, size_t argv, size_t rip);
	extern void exec_user(size_t argc, size_t argv, size_t rip, size_t rsp);

	struct page* rsp = mmu_req_page(0xC0000000, 0x7);

	int i = setjmp(&sys_exit_buf);
	if (!i)
		exec_user(1, NULL, 0xC0000000, 0xC000F000);
		//execat(1, NULL, ehdr->e_entry);
}
