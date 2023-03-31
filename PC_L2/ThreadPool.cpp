#include "ThreadPool.h"
#include <chrono>

#define LOCK_MUTEX std::unique_lock<std::mutex> lock(rwLock)

void ThreadPool::runner() {
	while (!stop)
	{
		
		unsigned int task;
		{
			LOCK_MUTEX;
			condition.wait(lock, [this] {return stop || (running && !priorityQueue.empty()); });
			if (stop) 
			{
				break;
			}
			task = priorityQueue.top();
			priorityQueue.pop();
		}
		std::this_thread::sleep_for(std::chrono::seconds(task));
	}
}

ThreadPool::ThreadPool(bool ExitImmediatlyOnTerminate) :
	exitImmediatlyOnTerminate(ExitImmediatlyOnTerminate)
{
	for (unsigned int i = 0; i < NUMBER_OF_THREADS; i++)
	{
		threads.emplace_back(std::thread(&ThreadPool::runner, this));
	}
}

ThreadPool::~ThreadPool()
{
	if (exitImmediatlyOnTerminate)
		terminateIm();
	else
		terminate();
}

void ThreadPool::terminate()
{
	{
		LOCK_MUTEX;
		if (stop) return;
		else stop = true;
		running = true;
	}

	condition.notify_all();

	for (auto it = threads.begin(); it != threads.end(); it++)
	{
		(*it).join();
	}
}

void ThreadPool::terminateIm()
{
	{
		LOCK_MUTEX;
		if (stop) return;
		else stop = true;
	}

	condition.notify_all();

	for (auto it = threads.begin(); it != threads.end(); it++)
	{
		(*it).detach();
	}
}

void ThreadPool::pause()
{
	LOCK_MUTEX;
	running = false;
}

void ThreadPool::unpause()
{
	LOCK_MUTEX;
	running = true;

	condition.notify_all();
}

void ThreadPool::pauseToggle()
{
	LOCK_MUTEX;
	running = !running;
	if (running) condition.notify_all();
}

unsigned int ThreadPool::addTask(unsigned int task)
{
	LOCK_MUTEX;
	priorityQueue.push(task);
	size_t out = priorityQueue.size();
	if (out = 1) condition.notify_one();
	return out;
}

unsigned int ThreadPool::removeTask()
{
	LOCK_MUTEX;
	unsigned int out = priorityQueue.top();
	priorityQueue.pop();
	return out;
}

unsigned int ThreadPool::currentQueueSize()
{
	LOCK_MUTEX;
	return priorityQueue.size();
}
