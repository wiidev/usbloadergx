#ifndef _HTTP_H_
#define _HTTP_H_

#include <errno.h>
#include <ogcsys.h>
#include <stdarg.h>
#include <string.h>


#define TCP_CONNECT_TIMEOUT 5000
#define TCP_BLOCK_SIZE (16 * 1024)
#define TCP_BLOCK_RECV_TIMEOUT 4000
#define TCP_BLOCK_SEND_TIMEOUT 4000
#define HTTP_TIMEOUT 300000

//The maximum amount of bytes to send per net_write() call
#define NET_BUFFER_SIZE 3600


typedef enum {
	HTTPR_OK,
	HTTPR_ERR_CONNECT,
	HTTPR_ERR_REQUEST,
	HTTPR_ERR_STATUS,
	HTTPR_ERR_TOOBIG,
	HTTPR_ERR_RECEIVE
} http_res;


#ifdef __cplusplus
extern "C"
{
#endif

#include "dns.h"

	/**
	 * A simple structure to keep track of the size of a malloc()ated block of memory
	 */
	struct block
	{
		u32 size;
		unsigned char *data;
	};

	extern const struct block emptyblock;

	struct block downloadfile(const char *url);
	s32 GetConnection(char * domain);
	void displayDownloadProgress(bool display);

#ifdef __cplusplus
}
#endif

#endif /* _HTTP_H_ */
