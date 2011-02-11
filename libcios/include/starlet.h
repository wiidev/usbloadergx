#ifndef STARLET_H
#define STARLET_H

// info from wiibrew.org

// IRQS (32 bits registers)

/*
IRQ		Description

0 		Starlet Timer
1 		NAND Interface
2 		AES Engine
3 		SHA-1 Engine
4 		USB Host Controller (EHCI)
5 		USB Host Controller (OHCI0)
6 		USB Host Controller (OHCI1)
7 		SD Host Controller
8 		802.11 Wireless
9 		Unknown
10 		Hollywood GPIOs (Broadway)
11 		Hollywood GPIOs (Starlet)
12-16 	Unknown
17 		Reset button
18-29 	Unknown
30 		IPC (Broadway)
31 		IPC (Starlet) 
*/

#define HW_PPCIRQFLAG 0x0d800030 // Broadway IRQ Flags (write 1 to clear)
#define HW_PPCIRQMASK 0x0d800034 // Broadway IRQ Mask  (write 1 to set)
#define HW_ARMIRQFLAG 0x0d800038 // Starlet IRQ Flags  (write 1 to clear)
#define HW_ARMIRQMASK 0x0d80003c // Starlet IRQ Mask   (write 1 to set)

// GPIOS (32 bits registers)

/*

Bit  Direction 	 Connection 	 Description

0 		IN 		POWER 		Power button input (pulse width limited; will not detect a held-down state).
1 		OUT 	SHUTDOWN 	Output high to turn system off (Power LED = red).
2 		OUT 	FAN 		Fan power, active high.
3 		OUT 	DC_DC 		DC/DC converter power, active high (powers the Broadway?[check]). When off, also triggers the Yellow power LED.
4 		OUT 	DI_SPIN 	DI spinup disable. If clear, the drive attempts to spin up a disc when reset (if there is one in the drive). If set, the drive ignores a present disc when reset.
5 		OUT 	SLOT_LED 	Blue disc slot LED, active high.
6 		IN 		EJECT_BTN 	Eject button (pulse width limited). Button press will also trigger the drive directly.
7 		IN 		SLOT_IN 	Disc slot optical detector. High if disc in drive, disc being inserted, or disc still in slot after eject.
8 		OUT 	SENSOR_BAR 	Sensor bar, active high.
9 		OUT 	DO_EJECT 	Pulse high to trigger a DI eject from software.
10 		OUT 	EEP_CS 		SEEPROM Chip Select.
11 		OUT 	EEP_CLK 	SEEPROM Clock.
12 		OUT 	EEP_MOSI 	Data to SEEPROM.
13 		IN 		EEP_MISO 	Data from SEEPROM.
14 		OUT 	AVE_SCL 	A/V Encoder I²C Clock.
15 		I/O 	AVE_SDA 	A/V Encoder I²C Data (has an external pull-up, so you should only drive it low).
16 		OUT 	DEBUG0 		Debug Testpoint TP221.
17 		OUT 	DEBUG1 		Debug Testpoint TP222.
18 		OUT 	DEBUG2 		Debug Testpoint TP223.
19 		OUT 	DEBUG3 		Debug Testpoint TP224.
20 		OUT 	DEBUG4 		Debug Testpoint TP225.
21 		OUT 	DEBUG5 		Debug Testpoint TP226.
22 		OUT 	DEBUG6 		Debug Testpoint TP219.
23 		OUT 	DEBUG7 		Debug Testpoint TP220. 

*/

#define HW_GPIOB_OUT		0x0d8000c0 // GPIO Outputs (Broadway access) 
#define HW_GPIOB_DIR		0x0d8000c4 // GPIO Direction (Broadway access) 
#define HW_GPIOB_IN			0x0d8000c8 // GPIO Inputs (Broadway access) 
#define HW_GPIOB_INTLVL		0x0d8000cc // GPIO Interrupt Levels (Broadway access)
#define HW_GPIOB_INTFLAG	0x0d8000d0 // GPIO Interrupt Flags (Broadway access) 
#define HW_GPIOB_INTMASK	0x0d8000d4 // GPIO Interrupt Masks (Broadway access) 
#define HW_GPIOB_INMIR		0x0d8000d8 // GPIO Input Mirror (Broadway access) 
#define HW_GPIO_ENABLE		0x0d8000dc // GPIO Enable (Starlet only) 
#define HW_GPIO_OUT			0x0d8000e0 // GPIO Outputs (Starlet only) 
#define HW_GPIO_DIR 		0x0d8000e4 // GPIO Direction (Starlet only) 
#define HW_GPIO_IN			0x0d8000e8 // GPIO Inputs (Starlet only) 
#define HW_GPIO_INTLVL		0x0d8000ec // GPIO Interrupt Levels (Starlet only) 
#define HW_GPIO_INTFLAG		0x0d8000f0 // GPIO Interrupt Flags (Starlet only) 
#define HW_GPIO_INTMASK		0x0d8000f4 // GPIO Interrupt Masks (Starlet only) 
#define HW_GPIO_INMIR		0x0d8000f8 // GPIO Input Mirror (Starlet only) 
#define HW_GPIO_OWNER		0x0d8000fc // GPIO Owner Select (Starlet only) 

// TIMER (32 bits registers)

#define HW_TIMER			0x0d800010 // Timer counter
#define HW_ALARM			0x0d800014 // Alarm value 


#endif

