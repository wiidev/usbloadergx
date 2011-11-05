/* lib/config.h.  Generated from config.h.in by configure.  */
/* lib/config.h.in.  Generated from configure.in by autoheader.  */
#ifndef __CONFIG_H_
#define __CONFIG_H_

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 if debugging the blkid library */
/* #undef CONFIG_BLKID_DEBUG */

/* Define to 1 to compile findfs */
/* #undef CONFIG_BUILD_FINDFS 1 */

/* Define to 1 if debugging ext3/4 journal code */
/* #undef CONFIG_JBD_DEBUG */

/* Define to 1 if the testio I/O manager should be enabled */
/* #undef CONFIG_TESTIO_DEBUG 1 */

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to 1 if ext2 compression enabled */
/* #undef ENABLE_COMPRESSION */

/* Define to 1 if ext3/4 htree support enabled */
#define ENABLE_HTREE 1

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#define HAVE_ALLOCA_H 1

/* Define to 1 if you have the <argz.h> header file. */
#define HAVE_ARGZ_H 1

/* Define to 1 if you have the `asprintf' function. */
#define HAVE_ASPRINTF 1

/* Define to 1 if you have the `backtrace' function. */
/* #undef HAVE_BACKTRACE 1 */

/* Define to 1 if you have the `blkid_probe_get_topology' function. */
/* #undef HAVE_BLKID_PROBE_GET_TOPOLOGY */

/* Define to 1 if you have the `chflags' function. */
/* #undef HAVE_CHFLAGS */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
/* #undef HAVE_DCGETTEXT 1 */

/* Define to 1 if you have the declaration of `feof_unlocked', and to 0 if you
   don't. */
/* #undef HAVE_DECL_FEOF_UNLOCKED 1 */

/* Define to 1 if you have the declaration of `fgets_unlocked', and to 0 if
   you don't. */
/* #undef HAVE_DECL_FGETS_UNLOCKED 0 */

/* Define to 1 if you have the declaration of `getc_unlocked', and to 0 if you
   don't. */
/* #undef HAVE_DECL_GETC_UNLOCKED 1 */

/* Define to 1 if you have the declaration of `_snprintf', and to 0 if you
   don't. */
/* #undef HAVE_DECL__SNPRINTF 0 */

/* Define to 1 if you have the declaration of `_snwprintf', and to 0 if you
   don't. */
/* #undef HAVE_DECL__SNWPRINTF 0 */

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if dlopen/libdl exists */
/* #undef HAVE_DLOPEN 1 */

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <execinfo.h> header file. */
/* #undef HAVE_EXECINFO_H 1 */

/* Define to 1 if Ext2 ioctls present */
/* #undef HAVE_EXT2_IOCTLS 1 */

/* Define to 1 if you have the `fallocate' function. */
/* #undef HAVE_FALLOCATE 1 */

/* Define to 1 if you have the `fallocate64' function. */
/* #undef HAVE_FALLOCATE64 1 */

/* Define to 1 if you have the `fchown' function. */
/* #undef HAVE_FCHOWN 1 */

/* Define to 1 if you have the `fdatasync' function. */
/* #undef HAVE_FDATASYNC 1 */

/* Define to 1 if you have the `fstat64' function. */
/* #undef HAVE_FSTAT64 1 */

/* Define to 1 if you have the `ftruncate64' function. */
/* #undef HAVE_FTRUNCATE64 1 */

/* Define to 1 if you have the `fwprintf' function. */
/* #undef HAVE_FWPRINTF 1 */

/* Define to 1 if you have the `getcwd' function. */
/* #undef HAVE_GETCWD 1 */

/* Define to 1 if you have the `getdtablesize' function. */
/* #undef HAVE_GETDTABLESIZE 1 */

/* Define to 1 if you have the `getegid' function. */
/* #undef HAVE_GETEGID 1 */

/* Define to 1 if you have the `geteuid' function. */
/* #undef HAVE_GETEUID 1 */

/* Define to 1 if you have the `getgid' function. */
/* #undef HAVE_GETGID 1 */

/* Define to 1 if you have the `getmntinfo' function. */
/* #undef HAVE_GETMNTINFO */

/* Define to 1 if you have the <getopt.h> header file. */
/* #undef HAVE_GETOPT_H 1 */

/* Define to 1 if you have the `getpagesize' function. */
/* #undef HAVE_GETPAGESIZE 1 */

/* Define to 1 if you have the `getrlimit' function. */
/* #undef HAVE_GETRLIMIT 1 */

/* Define to 1 if you have the `getrusage' function. */
/* #undef HAVE_GETRUSAGE 1 */

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT 1 */

/* Define to 1 if you have the `getuid' function. */
/* #undef HAVE_GETUID 1 */

/* Define if you have the iconv() function. */
/* #undef HAVE_ICONV 1 */

/* Define if you have the 'intmax_t' type in <stdint.h> or <inttypes.h>. */
#define HAVE_INTMAX_T 1

/* Define to 1 if the system has the type `intptr_t'. */
#define HAVE_INTPTR_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if <inttypes.h> exists, doesn't clash with <sys/types.h>, and
   declares uintmax_t. */
#define HAVE_INTTYPES_H_WITH_UINTMAX 1

/* Define to 1 if you have the `jrand48' function. */
/* #undef HAVE_JRAND48 1 */

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
/* #undef HAVE_LANGINFO_CODESET 1 */

/* Define if your <locale.h> file defines LC_MESSAGES. */
#define HAVE_LC_MESSAGES 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <linux/falloc.h> header file. */
/* #undef HAVE_LINUX_FALLOC_H 1 */

/* Define to 1 if you have the <linux/fd.h> header file. */
/* #undef HAVE_LINUX_FD_H 1 */

/* Define to 1 if you have the <linux/major.h> header file. */
/* #undef HAVE_LINUX_MAJOR_H 1 */

/* Define to 1 if you have the `llseek' function. */
/* #undef HAVE_LLSEEK 1 */

/* Define to 1 if llseek declared in unistd.h */
/* #undef HAVE_LLSEEK_PROTOTYPE */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define if you have the 'long double' type. */
#define HAVE_LONG_DOUBLE 1

/* Define if you have the 'long long' type. */
#define HAVE_LONG_LONG 1

/* Define to 1 if you have the `lseek64' function. */
/* #undef HAVE_LSEEK64 1 */

/* Define to 1 if lseek64 declared in unistd.h */
#define HAVE_LSEEK64_PROTOTYPE 1

/* Define to 1 if you have the `mallinfo' function. */
/* #undef HAVE_MALLINFO 1 */

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the `mbstowcs' function. */
#define HAVE_MBSTOWCS 1

/* Define to 1 if you have the `memalign' function. */
#define HAVE_MEMALIGN 1

/* Define to 1 if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H 1 */

/* Define to 1 if you have the `mempcpy' function. */
/* #undef HAVE_MEMPCPY 1 */

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP 1 */

/* Define to 1 if you have the <mntent.h> header file. */
/* #undef HAVE_MNTENT_H 1 */

/* Define to 1 if you have the `munmap' function. */
/* #undef HAVE_MUNMAP 1 */

/* Define to 1 if you have the `nanosleep' function. */
#define HAVE_NANOSLEEP 1

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H 1 */

/* Define to 1 if you have the <net/if_dl.h> header file. */
/* #undef HAVE_NET_IF_DL_H */

/* Define to 1 if you have the <net/if.h> header file. */
/* #undef HAVE_NET_IF_H 1 */

/* Define to 1 if you have the <nl_types.h> header file. */
/* #undef HAVE_NL_TYPES_H 1 */

/* Define to 1 if you have the `open64' function. */
/* #undef HAVE_OPEN64 1 */

/* Define to 1 if optreset for getopt is present */
/* #undef HAVE_OPTRESET */

/* Define to 1 if you have the `pathconf' function. */
/* #undef HAVE_PATHCONF 1 */

/* Define to 1 if you have the <paths.h> header file. */
/* #undef HAVE_PATHS_H 1 */

/* Define to 1 if you have the `posix_fadvise' function. */
/* #undef HAVE_POSIX_FADVISE 1 */

/* Define to 1 if you have the `posix_memalign' function. */
/* #undef HAVE_POSIX_MEMALIGN 1 */

/* Define if your printf() function supports format strings with positions. */
#define HAVE_POSIX_PRINTF 1

/* Define to 1 if you have the `prctl' function. */
/* #undef HAVE_PRCTL 1 */

/* Define to 1 if you have the `putenv' function. */
/* #undef HAVE_PUTENV 1 */

/* Define to 1 if you have the `quotactl' function. */
/* #undef HAVE_QUOTACTL 1 */

/* Define to 1 if dirent has d_reclen */
/* #undef HAVE_RECLEN_DIRENT 1 */

/* Define to 1 if if struct sockaddr contains sa_len */
/* #undef HAVE_SA_LEN */

/* Define to 1 if you have the <semaphore.h> header file. */
/* #undef HAVE_SEMAPHORE_H 1 */

/* Define to 1 if sem_init() exists */
/* #undef HAVE_SEM_INIT 1 */

/* Define to 1 if you have the `setenv' function. */
/* #undef HAVE_SETENV 1 */

/* Define to 1 if you have the <setjmp.h> header file. */
/* #undef HAVE_SETJMP_H 1 */

/* Define to 1 if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define to 1 if you have the `setresgid' function. */
/* #undef HAVE_SETRESGID 1 */

/* Define to 1 if you have the `setresuid' function. */
/* #undef HAVE_SETRESUID 1*/

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `srandom' function. */
/* #undef HAVE_SRANDOM 1 */

/* Define to 1 if struct stat has st_flags */
/* #undef HAVE_STAT_FLAGS */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define if <stdint.h> exists, doesn't clash with <sys/types.h>, and declares
   uintmax_t. */
#define HAVE_STDINT_H_WITH_UINTMAX 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpcpy' function. */
#define HAVE_STPCPY 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H 1 */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the `strptime' function. */
#define HAVE_STRPTIME 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the `strtoull' function. */
#define HAVE_STRTOULL 1

/* Define to 1 if you have the `sync_file_range' function. */
/* #undef HAVE_SYNC_FILE_RANGE 1 */

/* Define to 1 if you have the `sysconf' function. */
/* #undef HAVE_SYSCONF 1 */

/* Define to 1 if you have the <sys/disklabel.h> header file. */
/* #undef HAVE_SYS_DISKLABEL_H */

/* Define to 1 if you have the <sys/disk.h> header file. */
/* #undef HAVE_SYS_DISK_H */

/* Define to 1 if you have the <sys/file.h> header file. */
#define HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
/* #undef HAVE_SYS_IOCTL_H 1 */

/* Define to 1 if you have the <sys/mkdev.h> header file. */
/* #undef HAVE_SYS_MKDEV_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
/* #undef HAVE_SYS_MMAN_H 1 */

/* Define to 1 if you have the <sys/mount.h> header file. */
/* #undef HAVE_SYS_MOUNT_H 1 */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/prctl.h> header file. */
/* #undef HAVE_SYS_PRCTL_H 1 */

/* Define to 1 if you have the <sys/queue.h> header file. */
#define HAVE_SYS_QUEUE_H 1

/* Define to 1 if you have the <sys/quota.h> header file. */
/* #undef HAVE_SYS_QUOTA_H 1 */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef HAVE_SYS_SELECT_H 1 */

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H 1 */

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/syscall.h> header file. */
/* #undef HAVE_SYS_SYSCALL_H 1 */

/* Define to 1 if you have the <sys/sysmacros.h> header file. */
/* #undef HAVE_SYS_SYSMACROS_H 1 */

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
/* #undef HAVE_SYS_UN_H 1 */

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the <termio.h> header file. */
/* #undef HAVE_TERMIO_H 1 */

/* Define to 1 if you have the `tsearch' function. */
/* #undef HAVE_TSEARCH 1 */

/* Define to 1 if ssize_t declared */
#define HAVE_TYPE_SSIZE_T 1

/* Define if you have the 'uintmax_t' type in <stdint.h> or <inttypes.h>. */
#define HAVE_UINTMAX_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define if you have the 'unsigned long long' type. */
#define HAVE_UNSIGNED_LONG_LONG 1

/* Define to 1 if you have the `usleep' function. */
#define HAVE_USLEEP 1

/* Define to 1 if you have the `utime' function. */
/* #undef HAVE_UTIME 1 */

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1

/* Define to 1 if you have the `valloc' function. */
/* #undef HAVE_VALLOC 1 */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define if you have the 'wchar_t' type. */
#define HAVE_WCHAR_T 1

/* Define to 1 if you have the `wcslen' function. */
#define HAVE_WCSLEN 1

/* Define if you have the 'wint_t' type. */
#define HAVE_WINT_T 1

/* Define to 1 if you have the `__argz_count' function. */
/* #undef HAVE___ARGZ_COUNT 1 */

/* Define to 1 if you have the `__argz_next' function. */
/* #undef HAVE___ARGZ_NEXT 1 */

/* Define to 1 if you have the `__argz_stringify' function. */
/* #undef HAVE___ARGZ_STRINGIFY 1 */

/* Define to 1 if you have the `__fsetlocking' function. */
/* #undef HAVE___FSETLOCKING 1 */

/* Define to 1 if you have the `__secure_getenv' function. */
/* #undef HAVE___SECURE_GETENV 1 */

/* Define as const if the declaration of iconv() needs const. */
/* #undef ICONV_CONST */

/* Define if integer division by zero raises signal SIGFPE. */
/* #undef INTDIV0_RAISES_SIGFPE 1 */

/* package name for gettext */
#define PACKAGE "e2fsprogs"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define if <inttypes.h> exists and defines unusable PRI* macros. */
/* #undef PRI_MACROS_BROKEN */

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* Define as the maximum value of type 'size_t', if the system doesn't define
   it. */
/* #undef SIZE_MAX */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* If the compiler supports a TLS storage class define it to that here */
/* #undef TLS __thread */

/* Define to 1 to build uuidd */
/* #undef USE_UUIDD 1 */

/* version for gettext */
#define VERSION "0.14.1"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#define WORDS_BIGENDIAN 1

/* Define to 1 if Apple Darwin libintl workaround is needed */
/* #undef _INTL_REDIRECT_MACROS */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define as the type of the result of subtracting two pointers, if the system
   doesn't define it. */
/* #undef ptrdiff_t */

/* Define to empty if the C compiler doesn't support this keyword. */
/* #undef signed */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to unsigned long or unsigned long long if <stdint.h> and
   <inttypes.h> don't define. */
/* #undef uintmax_t */

#endif
