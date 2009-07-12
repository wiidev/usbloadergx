/****************************************************************************
 * USB Loader GX Team
 * buffer.cpp
 *
 * Loading covers in a background thread
 ***************************************************************************/

#include <gccore.h>
#include <unistd.h>

#include "libwiigui/gui.h"
#include "usbloader/disc.h"
#include "settings/cfg.h"
#include "buffer.h"
#include "main.h"

#define BUFFERSIZE         9

extern struct discHdr * gameList;
extern u32 gameCnt;

static lwp_t bufferthread = LWP_THREAD_NULL;
static bool BufferHalt = true;
static int loading = 0;
static int offset = 0;
static int direction = 1;
static bool changed = false;
static bool firstime = true;

static GuiImageData NoCoverData(nocover_png);
static GuiImageData * cover[BUFFERSIZE];
static GuiImage * coverImg[BUFFERSIZE];
static GuiImage * NoCover[BUFFERSIZE];

GuiImage * ImageBuffer(int imagenumber)
{
    if((BUFFERSIZE-1 > imagenumber) && direction >= 0) {
        return coverImg[imagenumber];
    }

    if((0 < imagenumber) && direction < 0) {
        return coverImg[imagenumber];
    }

    if(loading == BUFFERSIZE) {
        return coverImg[imagenumber];
    }

    return NoCover[imagenumber];
}

void LoadImages()
{
    if(!changed || BufferHalt)
        return;

	char ID[4];
	char IDfull[7];
    char imgPath[200];

    for(u32 i = offset; (int) i < offset+BUFFERSIZE; i++) {

    struct discHdr *header;
    if(i > gameCnt-1)
        header = &gameList[i-gameCnt];
    else
        header = &gameList[i];

    snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
    snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);


    snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, IDfull); //Load full id image

    //firsttime loading images into memory
    if(firstime) {

        if(coverImg[loading]) {
            delete coverImg[loading];
            coverImg[loading] = NULL;
        }
        if(cover[loading]) {
            delete cover[loading];
            cover[loading] = NULL;
        }

        cover[loading] = new GuiImageData(imgPath,0);
            if (!cover[loading]->GetImage()) {
                delete cover[loading];
                cover[loading] = NULL;
                snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID); //Load short id image
                cover[loading] = new GuiImageData(imgPath, 0);
                if (!cover[loading]->GetImage()) {
                    delete cover[loading];
                    cover[loading] = NULL;
                    snprintf(imgPath, sizeof(imgPath), "%snoimage.png", Settings.covers_path); //Load no image
                    cover[loading] = new GuiImageData(imgPath, nocover_png);
                }
            }
        coverImg[loading] = new GuiImage(cover[loading]);

    } else {

        if(direction >= 0) {

            if(loading < BUFFERSIZE-1) {

                if(coverImg[loading]) {
                    delete coverImg[loading];
                    coverImg[loading] = NULL;
                }

                if(loading == 0) {
                    if(cover[loading]) {
                        delete cover[loading];
                        cover[loading] = NULL;
                    }
                    cover[loading] = new GuiImageData(NULL, 0);
                    cover[loading] = cover[loading+1];
                } else {
                    cover[loading] = cover[loading+1];
                }

                coverImg[loading] = new GuiImage(cover[loading]);

            } else {

            cover[loading] = new GuiImageData(imgPath,0);
                if (!cover[loading]->GetImage()) {
                    delete cover[loading];
                    cover[loading] = NULL;
                    snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID); //Load short id image
                    cover[loading] = new GuiImageData(imgPath, 0);
                    if (!cover[loading]->GetImage()) {
                        delete cover[loading];
                        cover[loading] = NULL;
                        snprintf(imgPath, sizeof(imgPath), "%snoimage.png", Settings.covers_path); //Load no image
                        cover[loading] = new GuiImageData(imgPath, nocover_png);
                    }
                }
                if(coverImg[loading]) {
                    delete coverImg[loading];
                    coverImg[loading] = NULL;
                }
                coverImg[loading] = new GuiImage(cover[loading]);
            }
        } else if(direction < 0) {

            if(BUFFERSIZE-loading-1 > 0) {

                if(coverImg[BUFFERSIZE-loading-1]) {
                    delete coverImg[BUFFERSIZE-loading-1];
                    coverImg[BUFFERSIZE-loading-1] = NULL;
                }

                if(BUFFERSIZE-loading-1 == BUFFERSIZE-1) {
                    if(cover[BUFFERSIZE-loading-1]) {
                        delete cover[BUFFERSIZE-loading-1];
                        cover[BUFFERSIZE-loading-1] = NULL;
                    }
                    cover[BUFFERSIZE-loading-1] = new GuiImageData(NULL, 0);
                    cover[BUFFERSIZE-loading-1] = cover[BUFFERSIZE-loading-1-1];
                } else {
                    cover[BUFFERSIZE-loading-1] = cover[BUFFERSIZE-loading-1-1];
                    coverImg[BUFFERSIZE-loading-1] = new GuiImage(cover[BUFFERSIZE-loading-1]);
                }

            } else {

                header = &gameList[offset];

                snprintf (ID,sizeof(ID),"%c%c%c", header->id[0], header->id[1], header->id[2]);
                snprintf (IDfull,sizeof(IDfull),"%c%c%c%c%c%c", header->id[0], header->id[1], header->id[2],header->id[3], header->id[4], header->id[5]);

                snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, IDfull); //Load full id image

                cover[0] = new GuiImageData(imgPath,0);
                if (!cover[0]->GetImage()) {
                    delete cover[0];
                    cover[0] = NULL;
                    snprintf(imgPath, sizeof(imgPath), "%s%s.png", Settings.covers_path, ID); //Load short id image
                    cover[0] = new GuiImageData(imgPath, 0);
                    if (!cover[0]->GetImage()) {
                        delete cover[0];
                        cover[0] = NULL;
                        snprintf(imgPath, sizeof(imgPath), "%snoimage.png", Settings.covers_path); //Load no image
                        cover[0] = new GuiImageData(imgPath, nocover_png);
                    }
                }
                if(coverImg[0]) {
                    delete coverImg[0];
                    coverImg[0] = NULL;
                }
                coverImg[0] = new GuiImage(cover[0]);
            }
        }
    }
    loading++;
    }

    loading = BUFFERSIZE;
    changed = false;
    firstime = false;
}

void NewOffset(int off, int d)
{
    if(offset == off || loading < BUFFERSIZE)
        return;

    direction = d;

    offset = off;

    loading = 0;
    changed = true;
}

/****************************************************************************
 * HaltBuffer
 ***************************************************************************/
void HaltBufferThread()
{
    BufferHalt = true;
    firstime = true;
    changed = true;
    loading = 0;
    offset = 0;
    direction = 0;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(bufferthread))
		usleep(100);

    for(int i = 0; i < BUFFERSIZE; i++) {
        delete cover[i];
        cover[i] = NULL;
        delete coverImg[i];
        coverImg[i] = NULL;
        delete NoCover[i];
        NoCover[i] = NULL;
    }
}

/****************************************************************************
 * ResumeBufferThread
 ***************************************************************************/
void ResumeBufferThread(int offset)
{
	BufferHalt = false;
    firstime = true;
    changed = true;
    loading = 0;
    offset = offset;
    direction = 0;

    for(u8 i = 0; i < BUFFERSIZE; i++) {
        if(NoCover[i] != NULL) {
            delete NoCover[i];
            NoCover[i] = NULL;
        }
        NoCover[i] = new GuiImage(&NoCoverData);
    }

	LWP_ResumeThread(bufferthread);
}

/*********************************************************************************
 * Bufferthread
 *********************************************************************************/
static void * bufferinitcallback(void *arg)
{
    while(1)
    {
        if(BufferHalt)
            LWP_SuspendThread(bufferthread);

        LoadImages();
    }
	return NULL;
}

/****************************************************************************
 * InitBufferThread with priority 50
 ***************************************************************************/
void InitBufferThread()
{
	LWP_CreateThread(&bufferthread, bufferinitcallback, NULL, NULL, 0, 50);
}

/****************************************************************************
 * ShutdownThread
 ***************************************************************************/
void ShutdownBufferThread()
{
	LWP_JoinThread (bufferthread, NULL);
	bufferthread = LWP_THREAD_NULL;
}
