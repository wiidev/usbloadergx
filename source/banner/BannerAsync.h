/*
Copyright (c) 2012 - Dimok

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#ifndef _BANNERASYNC_H_
#define _BANNERASYNC_H_

#include <vector>
#include <queue>
#include <string>
#include "usbloader/GameList.h"
#include "Banner.h"

using namespace std;

class BannerAsync : public Banner
{
public:

	BannerAsync( const discHdr *header );
	virtual ~BannerAsync();

	static void ThreadInit(void);
	static void ThreadExit(void);
	static void HaltThread(void);
	static void ResumeThread(void);
	static void RemoveBanner(BannerAsync *banner);
	static void ClearQueue();
	static void PushFront(BannerAsync *banner);
private:
	const discHdr *header;

	static void * BannerAsyncThread(void *arg);
	static void ThreadAdd(BannerAsync* banner);
	static void ThreadRemove(BannerAsync* banner);

	static vector<BannerAsync *> List;
	static queue<BannerAsync *> DeleteList;
	static lwp_t Thread;
	static mutex_t ListLock;
	static BannerAsync * InUse;
	static bool SleepThread;
	static bool CloseThread;
};

#endif /*_GUIIMAGEASYNC_H_*/
