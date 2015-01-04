// A simple wrapper for libsicksaxis, to make it resemble WPAD/PAD more closely.
// Written by daxtsu/thedax. I'm releasing this code into the public domain, so do whatever you want with it.

#ifndef _DS3WRAPPER_H_
#define _DS3WRAPPER_H_

#include <gctypes.h>
#include <wiiuse/wpad.h>

struct ss_device;

enum
{
//	DS3_BUTTON_PS = 1,
//	DS3_BUTTON_START = 2,
//	DS3_BUTTON_SELECT = 4,
//	DS3_BUTTON_TRIANGLE = 8,
//	DS3_BUTTON_CIRCLE = 16,
//	DS3_BUTTON_CROSS = 32,
//	DS3_BUTTON_SQUARE = 64,
//	DS3_BUTTON_UP = 128,
//	DS3_BUTTON_RIGHT = 256,
//	DS3_BUTTON_DOWN = 512,
//	DS3_BUTTON_LEFT = 1024,
//	DS3_BUTTON_L1 = 2048,
//	DS3_BUTTON_L2 = 4096,
//	DS3_BUTTON_L3 = 8192,
//	DS3_BUTTON_R1 = 16384,
//	DS3_BUTTON_R2 = 32768,
//	DS3_BUTTON_R3 = 65536,

	// Classic Controller mapping
	DS3_BUTTON_PS = 		WPAD_CLASSIC_BUTTON_HOME,
	DS3_BUTTON_START = 		WPAD_CLASSIC_BUTTON_PLUS,
	DS3_BUTTON_SELECT = 	WPAD_CLASSIC_BUTTON_MINUS,
	DS3_BUTTON_TRIANGLE = 	WPAD_CLASSIC_BUTTON_X,
	DS3_BUTTON_CIRCLE = 	WPAD_CLASSIC_BUTTON_A,
	DS3_BUTTON_CROSS = 		WPAD_CLASSIC_BUTTON_B,
	DS3_BUTTON_SQUARE = 	WPAD_CLASSIC_BUTTON_Y,
	DS3_BUTTON_UP = 		WPAD_CLASSIC_BUTTON_UP,
	DS3_BUTTON_RIGHT = 		WPAD_CLASSIC_BUTTON_RIGHT,
	DS3_BUTTON_DOWN = 		WPAD_CLASSIC_BUTTON_DOWN,
	DS3_BUTTON_LEFT = 		WPAD_CLASSIC_BUTTON_LEFT,
	DS3_BUTTON_L1 = 		WPAD_CLASSIC_BUTTON_FULL_L,
	DS3_BUTTON_L2 = 		WPAD_CLASSIC_BUTTON_ZL,
//	DS3_BUTTON_L3 =
	DS3_BUTTON_R1 = 		WPAD_CLASSIC_BUTTON_FULL_R,
	DS3_BUTTON_R2 = 		WPAD_CLASSIC_BUTTON_ZR,
//	DS3_BUTTON_R3 =
};
	
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ss_device DS3;
bool DS3_Init();
void DS3_Rumble();
void DS3_Cleanup();
u32 DS3_ButtonsDown();
u32 DS3_ButtonsHeld();
u32 DS3_ButtonsUp();
bool DS3_Connected();
void DS3_ScanPads();
int DS3_StickX();
int DS3_SubStickX();
int DS3_StickY();
int DS3_SubStickY();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
