/****************************************************************************
 * USB Loader GX
 *
 * gui_imagea_sync.cpp
 ***************************************************************************/
#include <unistd.h>
#include "gui_image_async.h"

std::vector<GuiImageAsync *> GuiImageAsync::List;
lwp_t GuiImageAsync::Thread = LWP_THREAD_NULL;
mutex_t GuiImageAsync::ListLock = LWP_THREAD_NULL;
GuiImageAsync * GuiImageAsync::InUse = NULL;
u32 GuiImageAsync::ThreadCount = 0;
bool GuiImageAsync::ThreadSleep = true;
bool GuiImageAsync::CloseThread = false;

static inline void * memdup(const void* src, size_t len)
{
	if(!src) return NULL;

	void *dst = malloc(len);
	if (dst) memcpy(dst, src, len);
	return dst;
}

static GuiImageData * StdImageLoaderCallback(void *arg)
{
	return new GuiImageData((char *) arg);
}

GuiImageAsync::GuiImageAsync(const char *Filename, GuiImageData * PreloadImg) :
	GuiImage(PreloadImg), imgData(NULL), callback(StdImageLoaderCallback), arg(strdup(Filename))
{
	ThreadInit();
	ThreadAddImage(this);
}

GuiImageAsync::GuiImageAsync(ImageLoaderCallback Callback, const void * Arg, int ArgLen, GuiImageData * PreloadImg) :
	GuiImage(PreloadImg), imgData(NULL), callback(Callback), arg(memdup(Arg, ArgLen))
{
	ThreadInit();
	ThreadAddImage(this);
}

GuiImageAsync::~GuiImageAsync()
{
	ThreadRemoveImage(this);
	ThreadExit();
	while(InUse == this) usleep(100);
	if (imgData) delete imgData;
	if (arg) free(arg);
}

void GuiImageAsync::ThreadAddImage(GuiImageAsync *Image)
{
	LWP_MutexLock(ListLock);
	List.push_back(Image);
	LWP_MutexUnlock(ListLock);
	ThreadSleep = false;
	LWP_ResumeThread(Thread);
}

void GuiImageAsync::ThreadRemoveImage(GuiImageAsync *Image)
{
	for(u32 i = 0; i < List.size(); ++i)
	{
		if(List[i] == Image)
		{
			LWP_MutexLock(ListLock);
			List.erase(List.begin()+i);
			LWP_MutexUnlock(ListLock);
			break;
		}
	}
}

void GuiImageAsync::ClearQueue()
{
	LWP_MutexLock(ListLock);
	List.clear();
	LWP_MutexUnlock(ListLock);
}

void * GuiImageAsync::GuiImageAsyncThread(void *arg)
{
	while(!CloseThread)
	{
		if(ThreadSleep)
			LWP_SuspendThread(Thread);

		while(!List.empty() && !CloseThread)
		{
			LWP_MutexLock(ListLock);
			InUse = List.front();
			List.erase(List.begin());
			LWP_MutexUnlock(ListLock);

			if (!InUse)
				continue;

			InUse->imgData = InUse->callback(InUse->arg);

			if (InUse->imgData && InUse->imgData->GetImage())
			{
				InUse->width = InUse->imgData->GetWidth();
				InUse->height = InUse->imgData->GetHeight();
				InUse->image = InUse->imgData->GetImage();
			}

			InUse = NULL;
		}

		ThreadSleep = true;
	}

	return NULL;
}

u32 GuiImageAsync::ThreadInit()
{
	if (Thread == LWP_THREAD_NULL)
	{
		LWP_MutexInit(&ListLock, false);
		LWP_CreateThread(&Thread, GuiImageAsyncThread, NULL, NULL, 32768, 70);
	}
	return ++ThreadCount;
}

u32 GuiImageAsync::ThreadExit()
{
	//! We don't need to always shutdown and startup the thread, especially
	//! since this is a nested startup/shutdown from the gui thread.
	//! It's fine with being put to suspended only.
	/*
	if (--ThreadCount == 0)
	{
		CloseThread = true;
		LWP_ResumeThread(Thread);
		LWP_JoinThread(Thread, NULL);
		LWP_MutexUnlock(ListLock);
		LWP_MutexDestroy(ListLock);
		Thread = LWP_THREAD_NULL;
		ListLock = LWP_MUTEX_NULL;
		ListLock = LWP_MUTEX_NULL;
	}
	*/
	return --ThreadCount;
}

