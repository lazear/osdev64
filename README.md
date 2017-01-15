# osdev64
Operating System for x86_64 architecture

Note: No cross compiler is required to build on 64 bit machines. However it is important to not redefine some of the usual header files that GCC uses to prevent weird effects (i.e. stdarg, stdint, stdefs).