#ifndef __UART__
#define __UART__

#define COM1	0x3F8
#define COM2	0x2F8
#define COM3	0x3E8
#define COM4	0x2E8

extern void uart_init();
extern void uart_putc(char c);
extern void uart_write(const char* c);

#endif