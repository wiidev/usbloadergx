#include <stdio.h>
#include <ogcsys.h>

#include "sys.h"
#include "video2.h"

/* Video variables */
static void *framebuffer = NULL;
static GXRModeObj *vmode = NULL;


void Con_Init(u32 x, u32 y, u32 w, u32 h)
{
	/* Create console in the framebuffer */
	CON_InitEx(vmode, x, y, w, h);
}

void Con_Clear(void)
{
	/* Clear console */
	printf("\x1b[2J");
	fflush(stdout);
}

void Con_ClearLine(void)
{
	s32 cols, rows;
	u32 cnt;

	printf("\r");
	fflush(stdout);

	/* Get console metrics */
	CON_GetMetrics(&cols, &rows);

	/* Erase line */
	for (cnt = 1; cnt < cols; cnt++) {
		printf(" ");
		fflush(stdout);
	}

	printf("\r");
	fflush(stdout);
}

void Con_FgColor(u32 color, u8 bold)
{
	/* Set foreground color */
	printf("\x1b[%u;%um", color + 30, bold);
	fflush(stdout);
}

void Con_BgColor(u32 color, u8 bold)
{
	/* Set background color */
	printf("\x1b[%u;%um", color + 40, bold);
	fflush(stdout);
}

void Con_FillRow(u32 row, u32 color, u8 bold)
{
	s32 cols, rows;
	u32 cnt;

	/* Set color */
	printf("\x1b[%u;%um", color + 40, bold);
	fflush(stdout);

	/* Get console metrics */
	CON_GetMetrics(&cols, &rows);

	/* Save current row and col */
	printf("\x1b[s");
	fflush(stdout);

	/* Move to specified row */
	printf("\x1b[%u;0H", row);
	fflush(stdout);

	/* Fill row */
	for (cnt = 0; cnt < cols; cnt++) {
		printf(" ");
		fflush(stdout);
	}

	/* Load saved row and col */
	printf("\x1b[u");
	fflush(stdout);

	/* Set default color */
	Con_BgColor(0, 0);
	Con_FgColor(7, 1);
}

void Video_Configure(GXRModeObj *rmode)
{
	/* Configure the video subsystem */
	VIDEO_Configure(rmode);

	/* Setup video */
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

void Video_SetMode(void)
{
	/* Select preferred video mode */
	vmode = VIDEO_GetPreferredMode(NULL);

	/* Allocate memory for the framebuffer */
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

	/* Configure the video subsystem */
	VIDEO_Configure(vmode);

	/* Setup video */
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	/* Clear the screen */
	Video_Clear(COLOR_BLACK);
}

void Video_Clear(s32 color)
{
	VIDEO_ClearFrameBuffer(vmode, framebuffer, color);
}
