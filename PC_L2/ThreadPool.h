#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <string>

#define NUMBER_OF_THREADS 4

class ThreadPool
{
private:
	enum threadStatusEnum
	{
		started,
		dead,
		waiting,
		working,
		paused
	};
private:
	std::vector<std::thread> threads;
	bool running = true;
	std::shared_ptr<bool> stop = std::make_shared<bool>(false);
	const bool exitImmediatlyOnTerminate;

	unsigned int poolStartCountdown = NUMBER_OF_THREADS;
	std::shared_ptr<bool> unsafeShutdown = std::make_shared<bool>(false);

	std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int> > priorityQueue;
	std::shared_ptr<std::mutex> rwLock = std::make_shared<std::mutex>();
	std::condition_variable condition;

	double avgWaitTimeVar = 0.0;
	unsigned int avgWaitTimeDivider = 0;

	double avgTaskCompletionTimeVar = 0.0;
	unsigned int avgTaskCompletionTimeDivider = 0;

	double avgQueueSizeVar = 0.0;
	double avgQueueSizeDivider = 0.0;
	std::chrono::steady_clock::time_point timeFromLastUpdate = std::chrono::high_resolution_clock::now();
	std::unordered_map<unsigned short, threadStatusEnum> threadStatusMap;
public:
	ThreadPool(bool exitImmediatlyOnTerminate = false);
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&& other) = delete;
	ThreadPool& operator=(ThreadPool& rhs) = delete;
	ThreadPool& operator=(ThreadPool&& rhs) = delete;
	~ThreadPool();
	void terminate();
	void terminateIm();

	void pause();
	void unpause();
	void pauseToggle();

	unsigned int addTask(unsigned int task);
	unsigned int removeTask();

	//monitoring
	unsigned int currentQueueSize();
	std::unordered_map<unsigned short, ThreadPool::threadStatusEnum> currentThreadStatus();
	static const char* toString(ThreadPool::threadStatusEnum v)
	{
		switch (v)
		{
		case started:   return "started";
		case dead:   return "dead";
		case waiting: return "waiting";
		case working: return "working";
		case paused: return "paused";
		default:      return "???";
		}
	}

	//statistics
	unsigned int numberOfThreds() { return NUMBER_OF_THREADS; }
	double avgWaitTime();
	void avgWaitTimeReset();
	double avgQueueSize();
	double avgTaskCompletionTime();
	void avgTaskCompletionTimeReset();

private:
	void runner(unsigned short id);

	//must be called before queue operations
	void updateAvgQueueSize();
};