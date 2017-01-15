#ifndef __CTYPE__
#define __CTYPE__

#define isascii(c)		(c >= 0 && c <= 127)
#define isdigit(c)		(c >= '0' && c <= '9')
#define islower(c)		(c >= 'a' && c <= 'z')
#define isupper(c)		(c >= 'A' && c <= 'Z')
#define tolower(c)		(isdigit(c) ? c : (islower(c) ? c : ((c - 'A') + 'a')))
#define toupper(c)		(isdigit(c) ? c : (isupper(c) ? c : ((c - 'a') + 'A')))

#endif