#define PREFIX			1
#define ALWAYS_SIGN		2
#define PAD 			4
#define LONG			8
#define NOTNUM			0x10

int strrev(char* s)
{
	int length = strlen(s) - 1;
	int i;
	for (i = 0; i <= length/2; i++) {
		char temp = s[i];
		s[i] = s[length-i];
		s[length-i] = temp;
	}
	return length+1;
}

int ulltostr(size_t num, char* buffer, int base, int len) 
{
	int i = 0;
	if (num == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	while (num != 0 && len--) {
		size_t remainder = num % base;
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}
	buffer[i] = '\0';
	
	return strrev(buffer);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) 
{
	size_t n = 0;
	size_t pad = 0;
	int flags = 0;
	char buf[24];
	
	while(*format && n < size) {
		switch(*format) {
			case '%': {
				format++;

next_format:
				switch(*format) {
					case '%': {
						buf[0] = '%';
						buf[1] = '\0';
						break;
					}
					case '#': {
						flags |= PREFIX | PAD;
						pad = 16;
						format++;
						goto next_format;
					}
					case '+': {
						flags |= ALWAYS_SIGN;
						format++;
						goto next_format;
					}
					case 'p': {
						flags |= PREFX | PAD;
						pad = 16;
						/* fall through to x */
					}
					case 'x': {
						ulltostr(va_arg(ap, size_t), buf, 16, 16);
						if ((flags & PREFIX) && (n+2 < size)) {
							str[n++] = '0';
							str[n++] = 'x';
						}
						break;
					}
					case 'd':{
						size_t d = va_arg(ap, size_t);
						if ((flags & ALWAYS_SIGN) && (n+1 < size)) 
							str[n++] = (d > 0) ? '+' : '-';
						ulltostr(d, buf, 10, 20);				
						break;
					}
				 	case 'i': {
						size_t d = va_arg(ap, size_t);
						if ((n+1 < size)) 
							if (d < 0) str[n++] = '-';
						ulltostr(d, buf, 10, 20);
				
						break;
					}
					case 'u': {
						ulltostr(va_arg(ap, size_t), buf, 10, 20);
						break;
					}
					case 'l': {
							flags |= LONG;
						ulltostr(va_arg(ap, size_t), buf, 16, 16);
						break;
					}
					case 'o': {
						ulltostr(va_arg(ap, size_t), buf, 8, 16);
						if ((flags & PREFIX) && (n+2 < size)) {
							str[n++] = '0';
						}
						break;
					}
					case 's': {
						char* tmp = va_arg(ap, char*);
						strncpy(buf, tmp, size - n);
						flags |= NOTNUM;
						break;
					}
					case 'c': {
						buf[0] = (char) va_arg(ap, int);
						buf[1] = '\0';
						flags |= NOTNUM;
						break;
					}
					case 'w': {
						flags |= PAD;
						buf[0] = ' ';
						break;
					}
					default: {
						/* Parse out padding information */
						pad = 0;
						while(isdigit(*format)) {
							flags |= PAD;
							pad *= 10;
							pad += *format - '0';
							format++;
						}
						goto next_format;
					}

				}
				if ((flags & PAD) && (pad > strlen(buf))) {
					for (int i = 0; (i < (pad - strlen(buf))) && ((i+n) < size); i++)
						str[n++] = (flags & NOTNUM) ? ' ' : '0';
				}
				for (int i = 0; (i < strlen(buf)) && ((i+n) < size); i++) 	
					str[n++] = buf[i];		
				break;
			}
			default: {
				str[n++] = *format;
				flags = 0;
				break;
			}
		}
		format++;
	}

	while(n < size)
		str[n++] = '\0';
	return n;
}