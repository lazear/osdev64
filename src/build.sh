#!/bin/bash

dd if=/dev/zero of=disk.img bs=1k count=2800

nasm -f bin boot.asm
nasm -f elf64 common_x86_64.asm
nasm -f elf64 vectors.asm

#nasm -f elf64 idt.asm
#~/opt/cross/bin/x86_64-elf-ld -o kernel -T 64.ld -z max-page-size=0x1000 kernel.o 



gcc -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -m64 -c kernel.c -o kernel.o
gcc -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -m64 -c vsnprintf.c -o vsnprintf.o
gcc -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -m64 -c desc.c -o desc.o
gcc -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -m64 -c term.c -o term.o
ld -T 64.ld -z max-page-size=0x1000 vectors.o kernel.o vsnprintf.o desc.o common_x86_64.o term.o -o ckernel

dd if=boot of=disk.img bs=1k count=1 conv=notrunc
dd if=ckernel of=disk.img seek=1 bs=1k conv=notrunc
qemu-system-x86_64 disk.img -m 32 -curses