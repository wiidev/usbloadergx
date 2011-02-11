#include "syscalls.h"
#include <stdarg.h>

/**
 @brief  simple printf like function that write its output to os_puts
 @param format the format string, followed by format arguments.
 supported formatters are '%X' '%0*X' '% *X' '%d' '%u'
 this may not be perfectly compliant with ANSI-C printf, but its suffisant for common debugging..
*/

#ifdef DEBUG
void debug_printf(const char *format, ...)
{
	static const char HEX[] = "0123456789ABCDEF";
	static const char hex[] = "0123456789abcdef";
        const char *hexp = HEX;
	int val,i;
	unsigned int u_val, u_val_inv, base;
	unsigned char c;
	va_list list;
	int zeros = 0,chars;
	int spaces = 0;
        char buffer[1024],*ptr;
	va_start (list, format);
        
        ptr = buffer;
	for (;;) {
                c = *format++;
		while(c != '%' && c != '\0') // Until '%' or '\0'
		{
                        *ptr++ = c; 
                        c = *format++;
                }
		if(c == '\0')
		{
                        *ptr++ = c;
			va_end (list);
                        os_puts(buffer);
			return ;
		}
                hexp = HEX;
        CONTINUE_FORMAT:
		switch (c = *format++) {
		case '0': c = *format++;
			if(c >= '1' && c <= '9')
			{
				zeros = c - '0';
				goto CONTINUE_FORMAT;
			}
			else
				format--;
			break;
		case ' ': c = *format++;
			if(c >= '1' && c <= '9')
			{
				spaces = c - '0';
				goto CONTINUE_FORMAT;
			}
			else
				format--;
			break;
		case 'c': c = va_arg(list,int);
		case '%': 
                        *ptr++ = c;
			continue;
		case 's': 
                        if(ptr!=buffer){
                                *ptr=0;
                                os_puts(buffer);
                        }
                        os_puts(va_arg(list,char*));
                        ptr=buffer;
                        break;
		case '\0':
		default:   format--; continue; // will write it at next loop..
		case 'u':
		case 'd': base = 10; goto CONVERT_THIS;
		case 'p': zeros = 8;case 'x' : hexp = hex;
                case 'X':base = 16;

		CONVERT_THIS:
			val = va_arg(list,int);
			if (c == 'd') {
				if (val < 0) {
					val = - val;
					c = '-';
                                        *ptr++ = c;
				}
			}
			u_val = val;
			u_val_inv = 0;
			chars = 0;
			while(u_val){u_val_inv*= base;u_val_inv += u_val %base;u_val/=base; chars++;}
			if(chars == 0)chars++;

			if(zeros){
                                for(i=zeros - chars;i>0;i--)
                                        *ptr++ = '0';
                        }
			if(spaces)
                        {
                                for(i=spaces - chars;i>0;i--)
                                        *ptr++ = ' ';
                        }
			do {
				c = u_val_inv % base;
                                *ptr++ = hexp[c];
				u_val_inv /= base;
				chars --;
			} while (chars>0);
			zeros = 0;
			spaces = 0;
		}
	}
}
char ascii(char s) {
  if(s < 0x20) return '.';
  if(s > 0x7E) return '.';
  return s;
}

void hexdump(void *d, int len) {
  u8 *data;
  int i, off;
  data = (u8*)d;
  for (off=0; off<len; off += 16) {
    debug_printf("%08x  ",off);
    for(i=0; i<16; i++)
      if((i+off)>=len) debug_printf("   ");
      else debug_printf("%02x ",data[off+i]);

    debug_printf(" ");
    for(i=0; i<16; i++)
      if((i+off)>=len) debug_printf(" ");
      else debug_printf("%c",ascii(data[off+i]));
    debug_printf("\n");
  }
}

#endif
