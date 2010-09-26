/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_imagea_sync.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
//#include <string.h>
#include <unistd.h>
#include "gui_image_async.h"
static mutex_t debugLock = LWP_MUTEX_NULL;

void debug(int Line, const char* Format, ...)
{
    if (debugLock == 0) LWP_MutexInit(&debugLock, false);

    LWP_MutexLock(debugLock);

    FILE *fp = fopen("SD:/debug.txt", "a");
    if (fp)
    {
        char theTime[10];
        time_t rawtime = time(0); //this fixes code dump caused by the clock
        struct tm * timeinfo = localtime(&rawtime);
        strftime(theTime, sizeof(theTime), "%H:%M:%S", timeinfo);
        char format[10 + strlen(Format) + strlen(theTime)];
        sprintf(format, "%s %i - %s\n", theTime, Line, Format);
        va_list va;
        va_start( va, Format );
        vfprintf(fp, format, va);
        va_end( va );
        fclose(fp);
    }
    LWP_MutexUnlock(debugLock);
}
//#define DEBUG(format, ...) debug(__LINE__, format, ##__VA_ARGS__)
#define DEBUG(format, ...)

static void *memdup(const void* src, size_t len)
{
    void *dst = malloc(len);
    if (dst) memcpy(dst, src, len);
    return dst;
}
static std::vector<GuiImageAsync *> List;
static u32 ThreadCount = 0;
static lwp_t Thread = LWP_THREAD_NULL;
static mutex_t ListLock = LWP_MUTEX_NULL;
static mutex_t InUseLock = LWP_MUTEX_NULL;
static GuiImageAsync *InUse = NULL;
static bool Quit = false;
static bool CanSleep = true;
void *GuiImageAsyncThread(void *arg)
{
    while (!Quit)
    {
        LWP_MutexLock(ListLock);
        if (List.size())
        {
            LWP_MutexLock(InUseLock);

            InUse = List.front();
            List.erase(List.begin());

            LWP_MutexUnlock(ListLock);

            if (InUse)
            {
                GuiImageData *data = InUse->callback(InUse->arg);
                InUse->loadet_imgdata = data;
                if (InUse->loadet_imgdata && InUse->loadet_imgdata->GetImage())
                {
                    //  InUse->SetImage(InUse->loadet_imgdata); can’t use here. There can occur a deadlock
                    // Sets the image directly without lock. This is not fine, but it prevents a deadlock
                    InUse->image = InUse->loadet_imgdata->GetImage();
                    InUse->width = InUse->loadet_imgdata->GetWidth();
                    InUse->height = InUse->loadet_imgdata->GetHeight();
                }
            }
            InUse = NULL;
            LWP_MutexUnlock(InUseLock);
        }
        else
        {
            LWP_MutexUnlock(ListLock);
            if (!Quit && CanSleep) LWP_SuspendThread(Thread);
        }
        CanSleep = true;
    }
    Quit = false;
    return NULL;
}

static u32 GuiImageAsyncThreadInit()
{
    if (0 == ThreadCount++)
    {
        CanSleep = false;
        LWP_MutexInit(&ListLock, false);
        LWP_MutexInit(&InUseLock, false);
        LWP_CreateThread(&Thread, GuiImageAsyncThread, NULL, NULL, 16384, 75);
        //      while(!CanSleep)
        //          usleep(20);
    }
    return ThreadCount;
}
static u32 GuiImageAsyncThreadExit()
{
    if (--ThreadCount == 0)
    {
        Quit = true;
        LWP_ResumeThread(Thread);
        //      while(Quit)
        //          usleep(20);
        LWP_JoinThread(Thread, NULL);
        LWP_MutexDestroy(ListLock);
        LWP_MutexDestroy(InUseLock);
        Thread = LWP_THREAD_NULL;
        ListLock = LWP_MUTEX_NULL;
        InUseLock = LWP_MUTEX_NULL;
    }
    return ThreadCount;
}

static void GuiImageAsyncThread_AddImage(GuiImageAsync* Image)
{
    LWP_MutexLock(ListLock);
    List.push_back(Image);
    LWP_MutexUnlock(ListLock);
    CanSleep = false;
    //  if(LWP_ThreadIsSuspended(Thread))
    LWP_ResumeThread(Thread);
}
static void GuiImageAsyncThread_RemoveImage(GuiImageAsync* Image)
{
    LWP_MutexLock(ListLock);
    for (std::vector<GuiImageAsync *>::iterator iter = List.begin(); iter != List.end(); iter++)
    {
        if (*iter == Image)
        {
            List.erase(iter);
            LWP_MutexUnlock(ListLock);
            return;
        }
    }
    if (InUse == Image)
    {
        LWP_MutexLock(InUseLock);
        LWP_MutexUnlock(InUseLock);
    }
    LWP_MutexUnlock(ListLock);
}

/**
 * Constructor for the GuiImageAsync class.
 */
GuiImageData *StdImageLoaderCallback(void *arg)
{
    return new GuiImageData((char*) arg);
}

GuiImageAsync::GuiImageAsync(const char *Filename, GuiImageData * PreloadImg) :
    GuiImage(PreloadImg), loadet_imgdata(NULL), callback(StdImageLoaderCallback), arg(strdup(Filename))
{
    GuiImageAsyncThreadInit();
    GuiImageAsyncThread_AddImage(this);
}
GuiImageAsync::GuiImageAsync(ImageLoaderCallback Callback, void *Arg, int ArgLen, GuiImageData * PreloadImg) :
    GuiImage(PreloadImg), loadet_imgdata(NULL), callback(Callback), arg(memdup(Arg, ArgLen))
{
    DEBUG( "Constructor %p", this );
    GuiImageAsyncThreadInit();
    GuiImageAsyncThread_AddImage(this);
}
GuiImageAsync::~GuiImageAsync()
{
    GuiImageAsyncThread_RemoveImage(this);
    GuiImageAsyncThreadExit();
    DEBUG( "Deconstructor %p (loadet_imgdata=%p)", this, loadet_imgdata );
    if (loadet_imgdata) delete loadet_imgdata;
    if (arg) free(arg);
}

