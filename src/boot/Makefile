FINAL = bootstrap
DISK = ../disk.img
AS	= nasm
ASFLAGS = -f bin
OBJECTS = boot.o 

.PHONY: all
all: $(FINAL)

%.o: %.asm
	@echo "Assembling bootloader"
	@$(AS) $(ASFLAGS) $< -o $@

$(FINAL): $(OBJECTS)
	@mv $(OBJECTS) $(FINAL)
	@dd if=/dev/zero of=$(DISK) bs=1k count=2880
	@dd if=$(FINAL) of=$(DISK) bs=1k count=1 conv=notrunc 
