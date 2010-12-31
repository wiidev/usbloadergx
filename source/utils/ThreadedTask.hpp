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
		void AddCallback(cCallback * c, void * arg = 0) { CallbackList.push_back(c); ArgList.push_back(arg); };
		void Execute() { LWP_ResumeThread(Thread); };
    private:
        ThreadedTask();
        ~ThreadedTask();
		void ThreadLoop(void *arg);
		static void * ThreadCallback(void *arg);

		static ThreadedTask *instance;
		lwp_t Thread;
		bool ExitRequested;
        std::vector<cCallback *> CallbackList;
        std::vector<void *> ArgList;
};

#endif
