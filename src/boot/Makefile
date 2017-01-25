FINAL = bootstrap
DISK = ../disk.img
AS	= nasm
ASFLAGS = -f bin
SOURCE = boot.asm elf64.asm

.PHONY: all

%.o: %.asm

$(FINAL): $(SOURCE)
	@echo "building bootloader"
	@$(AS) $(ASFLAGS) $< -o $(FINAL)
	@dd if=/dev/zero of=$(DISK) bs=1k count=2880
	@dd if=$(FINAL) of=$(DISK) bs=1k count=1 conv=notrunc 

all: $(FINAL)