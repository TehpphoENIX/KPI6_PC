#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

#define NUMBER_OF_THREADS 4

class ThreadPool
{
private:
	std::vector<std::thread> threads;
	bool running = true;
	bool stop = false;
	const bool exitImmediatlyOnTerminate;

	std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int> > priorityQueue;
	std::mutex rwLock;
	std::condition_variable condition;
public:
	void runner();

	ThreadPool(bool exitImmediatlyOnTerminate = false);
	ThreadPool(const ThreadPool&) = delete;
	~ThreadPool();
	void terminate();
	void terminateIm();

	void pause();
	void unpause();
	void pauseToggle();

	unsigned int addTask(unsigned int task);
	unsigned int removeTask();

	unsigned int currentQueueSize();

	unsigned int numberOfThreds() { return NUMBER_OF_THREADS; }
	double avgWaitTime();
	double avgQueueSize();
	double avgTaskCompletionTime();
};

