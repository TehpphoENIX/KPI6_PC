#pragma once
#include <array>
#include <thread>
#include <queue>
#include <functional>
#define NUMBER_OF_THREADS 4
class ThreadPool
{
private:
	std::array<std::thread, NUMBER_OF_THREADS> threads;
	bool running = true;
	bool stop = false;

	std::priority_queue <unsigned int> priorityQueue;
public:
	void runner();

	ThreadPool();
	~ThreadPool();
	void terminate();
	void terminateIm();

	bool pause() { running = false; }
	bool unpause() { running = true; }
	bool pauseToggle() { running = !running; }

	unsigned int addTask();
	unsigned int removeTask();

	unsigned int numberOfThreds() { return NUMBER_OF_THREADS; }
	double avgWaitTime();
	double avgQueueSize();
	double avgTaskCompletionTime();
};

