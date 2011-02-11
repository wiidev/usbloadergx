#ifndef IOS_SYSCALLS_H
#define IOS_SYSCALLS_H

#include "types.h"

typedef struct _ioctlv
{
	void *data;
	unsigned long len;
} ioctlv;
typedef struct ipcmessage
{
	unsigned int command;			// 0
	unsigned int result;			// 4
        unsigned int fd;			// 8
	union 
	{
		struct
		{
			char *device;			// 12
			unsigned int mode;		// 16
			unsigned int resultfd;	// 20
		} open;
	
		struct 
		{
			void *data;
			unsigned int length;
		} read, write;
		
		struct 
		{
			int offset;
			int origin;
		} seek;
		
		struct 
		{
			unsigned int command;

			unsigned int *buffer_in;
			unsigned int length_in;
			unsigned int *buffer_io;
			unsigned int length_io;
		} ioctl;
		struct 
		{
			unsigned int command;

			unsigned int num_in;
			unsigned int num_io;
			ioctlv *vector;
		} ioctlv;
	};
} __attribute__((packed)) ipcmessage;

// NOTE: I think "autostart" is a flag to indicate an internal (child thread) or external thread
int os_thread_create( unsigned int (*entry)(void* arg), void* arg, void* stack, unsigned int stacksize, unsigned int priority, int autostart);
void os_thread_set_priority(int id, unsigned int priority);
int os_thread_get_priority(void);
int os_get_thread_id(void);
int os_get_parent_thread_id(void);

int os_thread_continue(int id);
int os_thread_stop(int id);

int os_message_queue_create(void* ptr, unsigned int max_entries);
int os_message_queue_receive(int queue, unsigned int* message, unsigned int flags);
int os_message_queue_send(int queue, unsigned int message, int flags);
int os_message_queue_send_now(int queue, unsigned int message, int flags);
void os_message_queue_ack(void* message, int result);

int os_heap_create(void* ptr, int size);
int os_heap_destroy(int heap);
void* os_heap_alloc(int heap, unsigned int size);
void* os_heap_alloc_aligned(int heap, int size, int align);
void os_heap_free(int heap, void* ptr);
int os_device_register(const char* devicename, int queuehandle);

void os_sync_before_read(void* ptr, int size);
void os_sync_after_write(void* ptr, int size);
void os_syscall_50(unsigned int unknown);

int os_open(char* device, int mode);
int os_close(int fd);
int os_read(int fd, void *d, int len);
int os_write(int fd, void *s, int len);
int os_seek(int fd, int offset, int mode);
int os_ioctlv(int fd, int request, int in, int out, ioctlv *vector);
int os_ioctl(int fd, int request, void *in,  int bytes_in, void *out, int bytes_out);

// timer control
int os_create_timer(int time_us, int repeat_time_us, int message_queue, int message); // return the timer_id
int os_destroy_timer(int time_id);
int os_stop_timer(int timer_id);
int os_restart_timer(int timer_id, int time_us); // restart one stopped timer
int os_timer_now(int time_id); 

#define DEV_EHCI 4
int	os_register_event_handler(int device, int queue, int message);

int	os_unregister_event_handler(int device);

int os_software_IRQ(int dev);

void os_puts(char *str); // to create log in dev/mload


#ifdef DEBUG
void debug_printf(const char *fmt, ...);
void hexdump(void *d, int len);
#else
#define debug_printf(a...) do{}while(0)
#endif
#endif // IOS_SYSCALLS_H
