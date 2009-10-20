

#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C" {
#endif
//giantpune's functions for USB gecko

//use this just like printf();
void gprintf(const char *str, ...);
bool InitGecko();

#ifdef __cplusplus
}
#endif

#endif

