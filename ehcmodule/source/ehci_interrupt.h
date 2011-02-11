#ifndef _EHCI_INTERRUPT_H_
#define _EHCI_INTERRUPT_H_

#include "syscalls.h"
#include "ehci_types.h"
#include "ehci.h"

#define INTR_MASK (STS_IAA | STS_FATAL | STS_PCD | STS_ERR | STS_INT)

extern int heaphandle;

void enable_EHCI(void);

void disable_EHCI(void);

void enable_OHCI1(void);

void disable_OHCI1(void);

void init_thread_ehci(void);

void ehci_int_passive_callback( void (*callback)(u32 flags));

void ehci_int_working_callback_part1(int (*callback)(u32 flags), u32 timeout);

int ehci_int_working_callback_part2(void);

#endif
