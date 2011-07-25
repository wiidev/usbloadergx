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
#ifndef THREADED_TASK_HPP_
#define THREADED_TASK_HPP_

#include <gccore.h>
#include <vector>
#include "Callback.hpp"

class ThreadedTask
{
	public:
		static ThreadedTask * Instance() { if(!instance) instance = new ThreadedTask(); return instance; };
		static void DestroyInstance() { delete instance; instance = NULL; };

		//! This callback is used for C-Like callbacks
		typedef void (*Callback)(void * arg);
		void AddCallback(ThreadedTask::Callback C_Standard, void * arg = 0)
		{
			CallbackList.push_back(std::pair<cCallback *, Callback>(0, C_Standard));
			ArgList.push_back(arg);
		}
		//! This callback is used for C++-Like class callbacks
		void AddCallback(cCallback * classCallback, void * arg = 0)
		{
			CallbackList.push_back(std::pair<cCallback *, Callback>(classCallback, 0));
			ArgList.push_back(arg);
		}
		//! Start the threaded task thread and execute one callback after another - FIFO style
		void Execute() { LWP_ResumeThread(Thread); };
	private:
		ThreadedTask();
		~ThreadedTask();
		void ThreadLoop(void *arg);
		static void * ThreadCallback(void *arg);

		static ThreadedTask *instance;
		lwp_t Thread;
		bool ExitRequested;
		std::vector<std::pair<cCallback *, Callback> > CallbackList;
		std::vector<void *> ArgList;
};

#endif
