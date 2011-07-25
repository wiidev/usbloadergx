
#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C"
{
#endif

	char ascii(char s);

#ifndef NO_DEBUG
	//use this just like printf();
	void gprintf(const char *str, ...);
	bool InitGecko();
	void hexdump(void *d, int len);
	void USBGeckoOutput();
#else
#define gprintf(...)
#define InitGecko()	  false
#define hexdump( x, y )
#endif /* NO_DEBUG */

#ifdef __cplusplus
}
#endif

#endif

