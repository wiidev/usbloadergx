#ifndef LIBWBFS_OS_H
#define LIBWBFS_OS_H
// libwbfs_os.h for ehc module env

#include "syscalls.h"
#include "ehci_types.h"
#include "ehci.h"

void *WBFS_Alloc(int size);
void WBFS_Free(void *ptr);
void my_sprint(char *cad, char *s);

#define wbfs_fatal(x) do{os_puts("wbfs panic ");os_puts(x);os_puts("\n"); while(1) ehci_msleep(100);}while(0)
#define wbfs_error(x) do{os_puts("wbfs error ");os_puts(x);;os_puts("\n");}while(0)
#define wbfs_malloc(x) WBFS_Alloc(x)
#define wbfs_free(x) WBFS_Free(x)
#define wbfs_ioalloc(x) WBFS_Alloc(x)
#define wbfs_iofree(x) WBFS_Free(x)
#define wbfs_ntohl(x) (x)
#define wbfs_htonl(x) (x)
#define wbfs_ntohs(x) (x)
#define wbfs_htons(x) (x)

#include <string.h>
#define wbfs_memcmp(x,y,z) memcmp(x,y,z)
#define wbfs_memcpy(x,y,z) memcpy(x,y,z)
#define wbfs_memset(x,y,z) memset(x,y,z)


#endif
