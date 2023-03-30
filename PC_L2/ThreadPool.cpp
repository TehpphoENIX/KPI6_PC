#include "ThreadPool.h"

void ThreadPool::runner() {
	while (!stop)
	{

	}
}

ThreadPool::ThreadPool()
{
	threads.fill(std::thread(&ThreadPool::runner, this));
}
