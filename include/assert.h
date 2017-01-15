#ifndef __ASSERT__
#define __ASSERT__

extern int printf(const char* fmt, ...);

#define assert(e) ((e) ? (void) 0 : printf("KERNEL ASSERT FAILED: %s %s:%d:%s\n", #e, __FILE__, __LINE__, __func__))

#endif
