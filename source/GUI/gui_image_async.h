#ifndef _GUIIMAGEASYNC_H_
#define _GUIIMAGEASYNC_H_

#include <vector>
#include "gui.h"

typedef GuiImageData * (*ImageLoaderCallback)(void *arg);

class GuiImageAsync: public GuiImage
{
	public:
		GuiImageAsync(const char *Filename, GuiImageData * PreloadImg);
		GuiImageAsync(ImageLoaderCallback Callback, const void *Arg, int ArgLen, GuiImageData * PreloadImg);
		virtual ~GuiImageAsync();

		static void ClearQueue();
	private:
		GuiImageData *imgData;
		ImageLoaderCallback callback;
		void *arg;

		static void * GuiImageAsyncThread(void *arg);
		static void ThreadAddImage(GuiImageAsync* Image);
		static void ThreadRemoveImage(GuiImageAsync* Image);
		static u32 ThreadInit();
		static u32 ThreadExit();

		static std::vector<GuiImageAsync *> List;
		static lwp_t Thread;
		static mutex_t ListLock;
		static GuiImageAsync * InUse;
		static u32 ThreadCount;
		static bool ThreadSleep;
		static bool CloseThread;
};

#endif /*_GUIIMAGEASYNC_H_*/
