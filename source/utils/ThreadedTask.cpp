/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "ThreadedTask.hpp"

ThreadedTask * ThreadedTask::instance = NULL;

ThreadedTask::ThreadedTask()
	: ExitRequested(false)
{
	LWP_CreateThread (&Thread, ThreadCallback, this, NULL, 16384, 70);
}

ThreadedTask::~ThreadedTask()
{
	ExitRequested = true;
	Execute();
	LWP_JoinThread(Thread, NULL);
}

void * ThreadedTask::ThreadCallback(void *arg)
{
	ThreadedTask * myInstance = (ThreadedTask *) arg;

	while(!myInstance->ExitRequested)
	{
		LWP_SuspendThread(myInstance->Thread);

		while(!myInstance->CallbackList.empty())
		{
			if(myInstance->CallbackList[0].first)
				myInstance->CallbackList[0].first->Execute(myInstance->ArgList[0]);

			else if(myInstance->CallbackList[0].second)
				myInstance->CallbackList[0].second(myInstance->ArgList[0]);

			myInstance->CallbackList.erase(myInstance->CallbackList.begin());
			myInstance->ArgList.erase(myInstance->ArgList.begin());
		}
	}

	return NULL;
}
