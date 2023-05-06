#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<int> value(0);

void function(int n)
{
	for (int i = 0; i < n; i++)
	{
		value += 10;
	}
}

int Tmain()
{
	const int num_threads = 8;
	const int iterations_per_thread = 50000;

	std::thread threads[num_threads];

	for (int i = 0; i < num_threads; i++)
	{
		threads[i] = std::thread(function, iterations_per_thread);
	}
	for (int i = 0; i < num_threads; i++)
	{
		threads[i].join();
	}

	std::cout << "Value: " << value << std::endl;
}