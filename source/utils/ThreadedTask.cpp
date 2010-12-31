#include "ThreadedTask.hpp"

ThreadedTask * ThreadedTask::instance = NULL;

ThreadedTask::ThreadedTask()
    : ExitRequested(false)
{
	LWP_CreateThread (&Thread, ThreadCallback, this, NULL, 16384, 80);
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
            myInstance->CallbackList[0]->Execute(myInstance->ArgList[0]);

            myInstance->CallbackList.erase(myInstance->CallbackList.begin());
            myInstance->ArgList.erase(myInstance->ArgList.begin());
        }
    }

    return NULL;
}
