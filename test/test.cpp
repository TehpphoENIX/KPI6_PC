#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool isReady = false;

void producer()
{
	std::cout << "Producer is producing data..." << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(5));
	{
		std::lock_guard<std::mutex> guard(mtx);
		isReady = true;
		std::cout << "Producer has produced data" << std::endl;
	}
	cv.notify_one();
}

void consumer()
{
	std::cout << "Consumer is waiting for data..." << std::endl;
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] { return isReady; });
		std::cout << "Consumer has recieved data" << std::endl;
	}
}

int main()
{
	std::thread t1(producer);
	std::thread t2(consumer);

	t1.join();
	t2.join();

	return 0;
}