# osdev64
Operating System for x86_64 architecture

### Requirements
* nasm
* gcc 4.9.0+ (C11 required)
* x86_64 CPU emulator software, QEMU or BOCHS recommended - or a 64 bit x86_64 processor with 128+ MB of RAM.


### Installation and building
If you do not already have nasm, qemu, or gcc, install them via your favorite package manager, then run the following:

```
git clone https://github.com/lazear/osdev64.git
cd osdev64
make
```

* Note: Cross compiler is not required to build on 64 bit machines. 