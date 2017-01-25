#osdev64 makefile

CC	= gcc
LD	= ld
AS	= nasm
BOOT = ./boot/boot.asm
INCLUDE = ../include
CCFLAGS	= -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -I $(INCLUDE) -m64 -c
LDFLAGS	= -Map map.txt -T linker.ld -z max-page-size=0x1000
ASFLAGS = -f elf64

DISK = disk.img
ASOURCE += $(wildcard *.asm)
CSOURCE += $(wildcard *.c)
CSOURCE += $(wildcard drivers/*.c)
HEADERS += $(wildcard $(INCLUDE)/*.h)
HEADERS += $(wildcard $(INCLUDE)/*/*.h)
OBJECTS += $(ASOURCE:.asm=.o)
OBJECTS += $(CSOURCE:.c=.o)
FINAL	= ../bin/kernel 		# Output binary

.PHONY: default all clean run
default: all run	
	
%.o: %.c $(HEADERS)
	@echo "cc $<"
	@$(CC) $(CCFLAGS) $< -o $@

%.o: %.asm
	@echo "as $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(FINAL): $(OBJECTS) 
	@echo $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(FINAL)



all: $(BOOT) $(FINAL)
	@make -C ./boot
	dd if=$(FINAL) of=$(DISK) bs=1k seek=1 conv=notrunc
	
clean:
	@echo "clean"
	@rm -f *.o				# Delete all of the object files
	@rm -f $(FINAL)

run:
	qemu-system-x86_64 $(DISK) -m 128 -cpu Nehalem,+x2apic -curses -smp cpus=4 -serial file:serial.txt
