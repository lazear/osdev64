#ifndef __SSE__
#define __SSE__

extern int sse42_enabled(void);
extern char* sse_strchr(const char*, char);
extern void sse_memcpy_aligned(void* restrict dest, void* restrict src, int count);
extern int sse_strlen(const char*);
extern int sse_strcmp(const char*, const char*);
extern int sse_strcmp_mask(const char*, const char*);
extern int sse_strstr_mask(const char* haystack, const char* needle);

#endif