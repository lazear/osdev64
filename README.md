# osdev64
Operating System for x86_64 architecture

Requirements:
* 64 bit x86_64 processor, newer than ~2011 (Nehalem or newer) for SSE4.2 support
* 128 MB of RAM



Note: No cross compiler is required to build on 64 bit machines. However it is important to not redefine some of the usual header files that GCC uses to prevent weird effects (i.e. stdarg, stdint, stdefs).