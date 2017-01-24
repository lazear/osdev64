#osdev64 makefile

FINAL	= ../bin/kernel 		# Output binary
OBJS	= *.o				# Elf object files
SOBJS = *.aso
CC	= gcc
LD	= ld
AS	= nasm
AR	= ar
CP	= cp
CD  = cd
BOOTSECT = bootstrap 
CCFLAGS	= -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -I ../include/ -m64 -c
LDFLAGS	= -Map map.txt -T linker.ld -z max-page-size=0x1000  $(SOBJS) $(OBJS) -o $(FINAL)
ASFLAGS = -f elf64
DISK = disk.img

all: compile clean 
build: compile link 
emu: compile clean run

compile:
	#Compile C source
	$(CC) $(CCFLAGS) *.c	# Compile top level
	$(CC) $(CCFLAGS) drivers/*.c	# Compile top level
	#Assembly
	$(AS) $(ASFLAGS) common_x86_64.asm -o common_x86_64.aso
	$(AS) $(ASFLAGS) vectors.asm -o vectors.aso
	$(AS) $(ASFLAGS) sse.asm -o sse.o
	$(AS) $(ASFLAGS) setjmp.asm -o setjmp.o
	#Bootsector
	$(AS) -f bin boot/boot.asm -o $(BOOTSECT) 
	$(LD) $(LDFLAGS)	# Link using the i686-elf toolchain

	dd if=/dev/zero of=$(DISK) bs=1k count=2880
	dd if=$(BOOTSECT) of=$(DISK) bs=1k count=1 conv=notrunc 
	dd if=$(FINAL) of=$(DISK) bs=1k seek=1 conv=notrunc 

run:
	qemu-system-x86_64 $(DISK) -m 256 -cpu Nehalem -curses -smp cpus=4 -serial file:serial.txt

	
clean:
	rm *.o				# Delete all of the object files
	rm *.aso
	rm $(BOOTSECT)
	
debug:
	gdb
	

	
