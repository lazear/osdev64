.PHONY: all run

export FINAL = $(shell pwd)/bin/kernel
export INCLUDE = $(shell pwd)/include/
export DISK = $(shell pwd)/disk
export EXE = $(shell pwd)/a.out

RAM = 128
CPUS = 4

all:
	@make -s -C src

clean:
	@make -s -C src clean

run:
	@qemu-system-x86_64 $(DISK) -m $(RAM) -cpu Nehalem,+x2apic -curses -smp cpus=$(CPUS) -serial file:serial.txt