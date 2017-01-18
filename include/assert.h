#ifndef __ASSERT__
#define __ASSERT__

extern int pprintf(int, const char* fmt, ...);

#define assert(e) ((e) ? (void) 0 : pprintf(0xC, "KERNEL ASSERT FAILED: %s %s:%d:%s\n", #e, __FILE__, __LINE__, __func__))

#endif
