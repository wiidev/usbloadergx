/* sys/uio.h

   netport
*/

#ifndef _UIO_H_
#define _UIO_H_

/* For size_t */
#include <stddef.h>
/* For ssize_t */
#include <sys/types.h>

/*
 * Define the uio buffers used for writev, readv.
 */

struct iovec
{
  void *iov_base;
  size_t iov_len;
};

extern ssize_t readv (int filedes, const struct iovec *vector, int count);
extern ssize_t writev(int filedes, const struct iovec *vector, int count);

#endif /* _UIO_H_ */
