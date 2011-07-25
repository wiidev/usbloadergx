#ifndef _DNS_H_
#define _DNS_H_

#include <network.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h> //usleep
#ifdef __cplusplus
extern "C"
{
#endif

	u32 getipbyname(char *domain);
	u32 getipbynamecached(char *domain);

#ifdef __cplusplus
}
#endif

#endif /* _DNS_H_ */
