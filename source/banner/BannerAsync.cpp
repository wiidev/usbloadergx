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
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <gccore.h>
#include "settings/CSettings.h"
#include "OpeningBNR.hpp"
#include "BannerAsync.h"

vector<BannerAsync *> BannerAsync::List;
queue<BannerAsync *> BannerAsync::DeleteList;
lwp_t BannerAsync::Thread = LWP_THREAD_NULL;
mutex_t BannerAsync::ListLock = LWP_THREAD_NULL;
BannerAsync * BannerAsync::InUse = NULL;
bool BannerAsync::SleepThread = false;
bool BannerAsync::CloseThread = false;


BannerAsync::BannerAsync(const discHdr *hdr)
	: header(hdr)
{
	ThreadInit();
	ThreadAdd(this);
}

BannerAsync::~BannerAsync()
{
	ThreadRemove(this);
	while(InUse == this)
		usleep(100);
}

void BannerAsync::ThreadAdd(BannerAsync *banner)
{
	LWP_MutexLock(ListLock);
	List.push_back(banner);
	LWP_MutexUnlock(ListLock);
	LWP_ResumeThread(Thread);
}

void BannerAsync::ThreadRemove(BannerAsync *banner)
{
	LWP_MutexLock(ListLock);
	for(u32 i = 0; i < List.size(); ++i)
	{
		if(List[i] == banner)
		{
			List.erase(List.begin()+i);
			break;
		}
	}
	LWP_MutexUnlock(ListLock);
}

void BannerAsync::RemoveBanner(BannerAsync *banner)
{
	LWP_MutexLock(ListLock);
	DeleteList.push(banner);
	LWP_MutexUnlock(ListLock);
	LWP_ResumeThread(Thread);
}

void BannerAsync::ClearQueue()
{
	LWP_MutexLock(ListLock);
	List.clear();
	LWP_MutexUnlock(ListLock);
}

void BannerAsync::PushFront(BannerAsync *banner)
{
	if(banner == InUse)
		return;

	LWP_MutexLock(ListLock);
	for(u32 i = 0; i < List.size(); ++i)
	{
		if(List[i] == banner)
		{
			List.erase(List.begin()+i);
			break;
		}
	}
	List.insert(List.begin(), banner);
	LWP_MutexUnlock(ListLock);
	LWP_ResumeThread(Thread);
}

void * BannerAsync::BannerAsyncThread(void *arg)
{
	while(!CloseThread)
	{
		while(!List.empty() && !CloseThread && !SleepThread)
		{
			LWP_MutexLock(ListLock);
			//! Delete the delete queue
			while(!DeleteList.empty())
			{
				delete DeleteList.front();
				DeleteList.pop();
			}
			//! Check if there is still something to do
			if(List.empty())
			{
				LWP_MutexUnlock(ListLock);
				continue;
			}

			//! Get first entry and pop
			InUse = List.front();
			List.erase(List.begin());
			LWP_MutexUnlock(ListLock);

			if (!InUse || !InUse->header)
				continue;

			const u8 * banner = NULL;
			u32 bannerSize = 0;

			if((InUse->header->type == TYPE_GAME_GC_IMG) || (InUse->header->type == TYPE_GAME_GC_DISC) || (InUse->header->type == TYPE_GAME_GC_EXTRACTED))
			{
				//! first see if a cache file is present and load that if needed
				if(BNRInstance::Instance()->Load(InUse->header))
				{
					banner = BNRInstance::Instance()->Get();
					bannerSize = BNRInstance::Instance()->GetSize();
				}
				else
				{
					//! load our default one
					CustomBanner *gcBanner = BNRInstance::Instance()->CreateGCIcon(InUse->header);
					if(gcBanner) {
						InUse->swap(*gcBanner);
						delete gcBanner;
					}
				}
			}
			else
			{
				BNRInstance::Instance()->Load(InUse->header);
				banner = BNRInstance::Instance()->Get();
				bannerSize = BNRInstance::Instance()->GetSize();
			}

			if(banner != NULL && bannerSize > 0)
				InUse->LoadIcon(banner, bannerSize);

			InUse = NULL;
		}

		//! Delete the delete queue here as well in case list was empty
		if(!DeleteList.empty())
		{
			LWP_MutexLock(ListLock);
			while(!DeleteList.empty())
			{
				delete DeleteList.front();
				DeleteList.pop();
			}
			LWP_MutexUnlock(ListLock);
		}

		if(!CloseThread)
			LWP_SuspendThread(Thread);
	}

	return NULL;
}

void BannerAsync::ResumeThread(void)
{
	SleepThread = false;

	if(Thread != LWP_THREAD_NULL)
		LWP_ResumeThread(Thread);
}

void BannerAsync::HaltThread(void)
{
	SleepThread = true;

	if(Thread == LWP_THREAD_NULL)
		return;

	// wait for thread to finish
	while (!LWP_ThreadIsSuspended(Thread))
		usleep(100);
}

void BannerAsync::ThreadInit(void)
{
	if (Thread == LWP_THREAD_NULL)
	{
		LWP_MutexInit(&ListLock, false);
		LWP_CreateThread(&Thread, BannerAsyncThread, NULL, NULL, 65536, 70);
	}
}

void BannerAsync::ThreadExit(void)
{
	if(Thread != LWP_THREAD_NULL)
	{
		ClearQueue();
		CloseThread = true;
		LWP_ResumeThread(Thread);
		LWP_JoinThread(Thread, NULL);
		LWP_MutexUnlock(ListLock);
		LWP_MutexDestroy(ListLock);
		Thread = LWP_THREAD_NULL;
		ListLock = LWP_MUTEX_NULL;
	}
}

