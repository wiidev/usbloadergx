/* sys/socket.h

   netport
*/

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H


#include <inttypes.h>
#include <sys/types.h>
#include <gctypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef socklen_t
typedef uint8_t socklen_t;
#define socklen_t socklen_t
#endif

typedef uint8_t  sa_family_t;

struct sockaddr {
  u8 sa_len;
  sa_family_t sa_family; 	    /* address family, AF_xxx	*/
  char sa_data[14];	            /* 14 bytes of protocol address	*/
};

/* Definition of sockaddr_storage according to SUSv3. */
#define _SS_MAXSIZE 128			/* Maximum size. */
#define _SS_ALIGNSIZE (sizeof (int64_t))/* Desired alignment. */
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof (sa_family_t))
#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof (sa_family_t) \
		      + _SS_PAD1SIZE + _SS_ALIGNSIZE))

struct sockaddr_storage {
  sa_family_t		ss_family;
  char			_ss_pad1[_SS_PAD1SIZE];
  int64_t		__ss_align;
  char			_ss_pad2[_SS_PAD2SIZE];
};

#include <sys/uio.h>

struct msghdr
{
  void *		msg_name;	/* Socket name			*/
  socklen_t		msg_namelen;	/* Length of name		*/
  struct iovec *	msg_iov;	/* Data blocks			*/
  int			msg_iovlen;	/* Number of blocks		*/
  void *		msg_control;	/* Ancillary data		*/
  socklen_t		msg_controllen;	/* Ancillary data buffer length	*/
  int			msg_flags;	/* Received flags on recvmsg	*/
};

struct cmsghdr
{
  socklen_t		cmsg_len;	/* Length of cmsghdr + data	*/
  int			cmsg_level;	/* Protocol			*/
  int			cmsg_type;	/* Protocol type		*/
};

#define CMSG_ALIGN(len) \
	(((len) + sizeof (size_t) - 1) & ~(sizeof (size_t) - 1))
#define CMSG_LEN(len) \
	(CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))
#define CMSG_SPACE(len) \
	(CMSG_ALIGN (sizeof (struct cmsghdr)) + CMSG_ALIGN(len))
#define CMSG_FIRSTHDR(mhdr)	\
	({ \
	  struct msghdr *_m = (struct msghdr *) mhdr; \
	  (unsigned) (_m)->msg_controllen >= sizeof (struct cmsghdr) \
	  ? (struct cmsghdr *) (_m)->msg_control \
	  : (struct cmsghdr *) NULL; \
	})
#define CMSG_NXTHDR(mhdr,cmsg)	\
	({ \
	  struct msghdr *_m = (struct msghdr *) mhdr; \
	  struct cmsghdr *_c = (struct cmsghdr *) cmsg; \
	  ((char *) _c + CMSG_SPACE (_c->cmsg_len) \
	   > (char *) _m->msg_control + _m->msg_controllen) \
	  ? (struct cmsghdr *) NULL \
	  : (struct cmsghdr *) ((char *) _c + CMSG_ALIGN (_c->cmsg_len)); \
	})
#define CMSG_DATA(cmsg)		\
	((unsigned char *) ((struct cmsghdr *)(cmsg) + 1))

/* "Socket"-level control message types: */
#define	SCM_RIGHTS	0x01		/* access rights (array of int) */

/*
 * Structure used for manipulating linger option.
 */
struct linger {
       int l_onoff;                /* option on/off */
       int l_linger;               /* linger time */
};


#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
#if(0)
#define SOCK_SEQPACKET  5
#endif

#define  SOL_SOCKET			0xffff    /* options for socket level */

/*
 * Option flags per-socket.
 */
#define  SO_DEBUG			0x0001    /* turn on debugging info recording */
#define  SO_ACCEPTCONN		0x0002    /* socket has had listen() */
#define  SO_REUSEADDR		0x0004    /* allow local address reuse */
#define  SO_KEEPALIVE		0x0008    /* keep connections alive */
#define  SO_DONTROUTE		0x0010    /* just use interface addresses */
#define  SO_BROADCAST		0x0020    /* permit sending of broadcast msgs */
#define  SO_USELOOPBACK		0x0040    /* bypass hardware when possible */
#define  SO_LINGER			0x0080    /* linger on close if data present */
#define  SO_OOBINLINE		0x0100    /* leave received OOB data in line */
#define	 SO_REUSEPORT		0x0200		/* allow local address & port reuse */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF			0x1001    /* send buffer size */
#define SO_RCVBUF			0x1002    /* receive buffer size */
#define SO_SNDLOWAT			0x1003    /* send low-water mark */
#define SO_RCVLOWAT			0x1004    /* receive low-water mark */
#define SO_SNDTIMEO			0x1005    /* send timeout */
#define SO_RCVTIMEO			0x1006    /* receive timeout */
#define  SO_ERROR			0x1007    /* get error status and clear */
#define  SO_TYPE			0x1008    /* get socket type */

#if(0)
#define SOMAXCONN       0x7fffffff
#endif


/* Flags we can use with send/ and recv. */
// Flags that are made up
#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */
#define MSG_WINMASK     0x7             /* flags understood by WinSock calls */
#define MSG_NOSIGNAL    0x20            /* Don't raise SIGPIPE */
#define MSG_TRUNC       0x0100          /* Normal data truncated */
#define MSG_CTRUNC      0x0200          /* Control data truncated */

// Flag not made up
#define MSG_DONTWAIT		0x40            /* Nonblocking i/o for this operation only */


/* Supported address families. */
/*
 * Address families.
 */
#define AF_UNSPEC			0
#define AF_INET				2
#if(0)
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET6        23              /* IP version 6 */
#endif


#if(0)
/* SUS symbolic values for the second parm to shutdown(2) */
#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2
#endif

int     accept(int, struct sockaddr *__restrict__, socklen_t *__restrict__);
int     bind(int, const struct sockaddr *, socklen_t);
int     connect(int, const struct sockaddr *, socklen_t);
int     getpeername(int, struct sockaddr *__restrict__, socklen_t *__restrict__);
int     getsockname(int, struct sockaddr *__restrict__, socklen_t *__restrict__);
int     getsockopt(int, int, int, void *__restrict__, socklen_t *__restrict__);
int     listen(int, int);
ssize_t recv(int, void *, size_t, int);
ssize_t recvfrom(int, void *__restrict__, size_t, int,
        struct sockaddr *__restrict__, socklen_t *__restrict__);
ssize_t recvmsg(int, struct msghdr *, int);
ssize_t send(int, const void *, size_t, int);
ssize_t sendmsg(int, const struct msghdr *, int);
ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *,
        socklen_t);
int     setsockopt(int, int, int, const void *, socklen_t);
int     close(int);
int     shutdown(int, int);
int     socket(int, int, int);
int     sockatmark(int);
int     socketpair(int, int, int, int[2]);


#ifdef __cplusplus
};
#endif

#endif /* _SYS_SOCKET_H */
