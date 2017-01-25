OUTPUT_FORMAT("elf64-x86-64")

KERNEL_VIRT   = 0xFFFF800000000000;
KERNEL_PHYS   = 0x100000;

ENTRY(main)
SECTIONS
{

    . = KERNEL_VIRT + KERNEL_PHYS;
    
    _kernel_start = .;
    .text ALIGN(0x1000) : AT(ADDR(.text) - KERNEL_VIRT)
    {
        *(.text)

    }

   .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VIRT)
   {
        *(.data)
        *(.rodata)
   }

   .bss ALIGN(0x1000): AT(ADDR(.bss) -  KERNEL_VIRT)
   {
       *(.bss)
        *(COMMON)
   }

   _kernel_end = .;


   /DISCARD/ :
   {
        *(.comment)
    }
}

