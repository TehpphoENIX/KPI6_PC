#include "ThreadPool.h"
#include <chrono>

#define LOCK_MUTEX std::unique_lock<std::mutex> lock(*rwLock)

void ThreadPool::runner(unsigned short id) {
	std::shared_ptr<std::mutex> mutex(rwLock);
	std::shared_ptr<bool> localStop(stop);
	std::shared_ptr<bool> deadHost(unsafeShutdown);
	threadStatusMap.insert({ id, started});
	{
		std::unique_lock<std::mutex> lock(*mutex);
		poolStartCountdown--;
		if (poolStartCountdown == 0) condition.notify_one();
	}
	while (true)
	{
		unsigned int task;
		{
			std::unique_lock<std::mutex> lock(*mutex);
			auto startTime = std::chrono::high_resolution_clock::now();
			if (!*localStop && !running) threadStatusMap[id] = paused;
			if (!*localStop && priorityQueue.empty()) threadStatusMap[id] = waiting;
			condition.wait(lock, [=] {return *localStop || (running && !priorityQueue.empty()); });
			auto endTime = std::chrono::high_resolution_clock::now();
			if (*localStop) 
			{
				if (!*deadHost)
				{
					threadStatusMap[id] = dead;
				}
				break;
			}
			else
			{
				avgWaitTimeVar += std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
				avgWaitTimeDivider++;
			}
			updateAvgQueueSize();
			task = priorityQueue.top();
			priorityQueue.pop();
			threadStatusMap[id] = working;
		}
		auto startTime = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::seconds(task));
		auto endTime = std::chrono::high_resolution_clock::now();
		{
			std::unique_lock<std::mutex> lock(*mutex);
			if (!*localStop || !*deadHost)
			{
				avgTaskCompletionTimeVar += std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
				avgTaskCompletionTimeDivider++;
			}
		}
	}
}

void ThreadPool::updateAvgQueueSize()
{
	//get vars
	auto now = std::chrono::high_resolution_clock::now();
	double queueSize = priorityQueue.size();
	double time = std::chrono::duration_cast<std::chrono::milliseconds>(now - timeFromLastUpdate).count();
	
	//update statistics
	timeFromLastUpdate = now;
	avgQueueSizeVar += queueSize * time;
	avgQueueSizeDivider += time;
}

ThreadPool::ThreadPool(bool ExitImmediatlyOnTerminate) :
	exitImmediatlyOnTerminate(ExitImmediatlyOnTerminate)
{
	for (unsigned int i = 0; i < NUMBER_OF_THREADS; i++)
	{
		threads.emplace_back(std::thread(&ThreadPool::runner, this, i));
	}
	LOCK_MUTEX;
	condition.wait(lock, [this] {return poolStartCountdown == 0; });
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
		if (*stop) return;
		else *stop = true;
		running = true;
	}

	condition.notify_all();

	for (auto it = threads.begin(); it != threads.end(); it++)
	{
		(*it).join();
	}
	updateAvgQueueSize();
}

void ThreadPool::terminateIm()
{
	{
		LOCK_MUTEX;
		*unsafeShutdown = true;
		if (*stop) return;
		else *stop = true;
	}

	condition.notify_all();

	for (auto it = threads.begin(); it != threads.end(); it++)
	{
		(*it).detach();
	}
	updateAvgQueueSize();
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
	updateAvgQueueSize();
	priorityQueue.push(task);
	size_t out = priorityQueue.size();
	if (out = 1) condition.notify_one();
	return out;
}

unsigned int ThreadPool::removeTask()
{
	LOCK_MUTEX;
	updateAvgQueueSize();
	unsigned int out = priorityQueue.top();
	priorityQueue.pop();
	return out;
}

//monitoring
unsigned int ThreadPool::currentQueueSize()
{
	LOCK_MUTEX;
	return priorityQueue.size();
}

std::unordered_map<unsigned short, ThreadPool::threadStatusEnum> ThreadPool::currentThreadStatus()
{
	LOCK_MUTEX;
	return threadStatusMap;
}

//statistics:
double ThreadPool::avgWaitTime()
{
	LOCK_MUTEX;
	return avgWaitTimeVar / avgWaitTimeDivider;
}

void ThreadPool::avgWaitTimeReset()
{
	LOCK_MUTEX;
	avgWaitTimeVar = 0.0;
	avgWaitTimeDivider = 0;
}

double ThreadPool::avgQueueSize()
{
	LOCK_MUTEX;
	return avgQueueSizeVar/avgQueueSizeDivider;
}

double ThreadPool::avgTaskCompletionTime()
{
	LOCK_MUTEX;
	return avgTaskCompletionTimeVar / avgTaskCompletionTimeDivider;
}

void ThreadPool::avgTaskCompletionTimeReset()
{
	LOCK_MUTEX;
	avgTaskCompletionTimeVar = 0.0;
	avgTaskCompletionTimeDivider = 0;
}
