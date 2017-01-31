#osdev64 makefile
CC	= gcc
LD	= ld
AS	= nasm

CCFLAGS	= -ffreestanding -nostdlib -Wall -mno-red-zone -mcmodel=large -I $(INCLUDE) -m64 -c
LDFLAGS	= -Map map.txt -T linker.ld -z max-page-size=0x1000
ASFLAGS = -f elf64

ASOURCE += $(wildcard *.asm)
CSOURCE += $(wildcard *.c)
CSOURCE += $(wildcard drivers/*.c)
CSOURCE += $(wildcard lib/*.c)

HEADERS += $(wildcard $(INCLUDE)/*.h)
HEADERS += $(wildcard $(INCLUDE)/*/*.h)
OBJECTS += $(ASOURCE:.asm=.o)
OBJECTS += $(CSOURCE:.c=.o)

AP_ENTRY = ap_entry 
AP_ENTRY_SRC = ap_entry.s


.PHONY: default all clean
default: all

%.o: %.c $(HEADERS)
	@echo "\033[1;32mcc $<\033[0m"
	@$(CC) $(CCFLAGS) $< -o $@

%.o: %.asm
	@echo "\033[1;32mas $<\033[0m"
	@$(AS) $(ASFLAGS) $< -o $@

$(AP_ENTRY): $(AP_ENTRY_SRC) 
	@echo "\033[1;32mas $<\033[0m"
	@$(AS) -f bin $< -o $@

$(FINAL): $(OBJECTS) $(AP_ENTRY)
	$(LD) $(LDFLAGS) $(OBJECTS) -b binary $(EXE) $(AP_ENTRY) -o $(FINAL)

all: $(FINAL)
	@echo $(CSOURCE) $(ASOURCE) $(HEADERS) | xargs wc -l | grep total
	@make -s -C ./boot	
	@dd if=$(FINAL) of=$(DISK) bs=1k seek=1 conv=notrunc

clean:
	@echo "cleaning"
	@rm -f $(OBJECTS)				# Delete all of the object files
	@rm -f $(FINAL)
	@rm -f ./boot/bootstrap
	@rm -f *.txt

