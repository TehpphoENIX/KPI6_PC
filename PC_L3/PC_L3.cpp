#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <assert.h>
#include "rapidscv.h"
#include <numeric>

#define NUMBER_OF_EXPERIMENTS 5

using std::chrono::duration_cast,
std::chrono::milliseconds,
std::chrono::nanoseconds ,
std::chrono::high_resolution_clock, 
std::cout;

int intRand(const int& min, const int& max) {
	static thread_local std::mt19937 generator(duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}

double average(std::vector<double> const& v) {
	if (v.empty()) {
		return 0;
	}

	auto const count = static_cast<double>(v.size());
	return std::reduce(v.begin(), v.end()) / count;
}

enum Parameter {
	Threads = 1,
	ThreadsMin = 2,
	ThreadsMinMax = 3
};

unsigned int ammountOfThreads = 4;
int minRandValue = 0, maxRandValue = 100;
double opPerThread;
unsigned int minSizeOfArray = pow(10,8),
			maxSizeOfArray = 5*pow(10,8),
			ammountOfTests = 5;

std::vector<int> Array;

bool firstCalculation;
int expectedMin, expectedSum;

nanoseconds singleThread(){
	int minResult = INFINITY, sumResult = 0;
	auto startTime = high_resolution_clock::now();
	for (auto& item : Array)
		if (item % 10 == 0)
		{
			if (minResult > item) minResult = item;
			sumResult += item;
		}
	auto endTime = high_resolution_clock::now();
	auto elapsed = duration_cast<nanoseconds>(endTime - startTime);
	cout << "min: " << minResult <<
		"\nsum: " << sumResult << '\n';
	cout << "Randomized in " << (elapsed.count() * 1e-9) << " s\n" << std::endl;

	if (firstCalculation)
	{
		expectedMin = minResult;
		expectedSum = sumResult;
		firstCalculation = false;
	}
	else
	{
		assert(expectedMin == minResult);
		assert(expectedSum == sumResult);
	}

	return elapsed;
}

nanoseconds multiThreadMutex() {
	std::vector<std::thread> threads;
	int minResult = INFINITY, sumResult = 0;
	std::mutex rwLock;
	auto startTime = high_resolution_clock::now();
	for (size_t i = 0; i < ammountOfThreads; i++)
	{
		unsigned int start = i * opPerThread;
		unsigned int end = (i + 1) * opPerThread - 1;
		threads.push_back(std::thread([&, start, end]
			{
				for (unsigned int i = start; i < end; i++)
					if (Array[i] % 10 == 0)
					{
						std::unique_lock<std::mutex> lock(rwLock);
						if (minResult > Array[i]) {
							minResult = Array[i];
						}
						sumResult += Array[i];
					}
			}
		));
	}
	for (auto& item : threads)
	{
		item.join();
	}
	auto endTime = high_resolution_clock::now();
	auto elapsed = duration_cast<nanoseconds>(endTime - startTime);
	cout << "min: " << minResult <<
		"\nsum: " << sumResult << '\n';
	cout << "Randomized in " << (elapsed.count() * 1e-9) << " s\n" << std::endl;
	threads.clear();

	assert(expectedMin == minResult);
	assert(expectedSum == sumResult);

	return elapsed;
}

nanoseconds multiThreadAtomic()
{
	std::vector<std::thread> threads;
	std::atomic<int> minResult(INFINITY), sumResult(0);
	auto startTime = high_resolution_clock::now();
	for (size_t i = 0; i < ammountOfThreads; i++)
	{
		unsigned int start = i * opPerThread;
		unsigned int end = (i + 1) * opPerThread - 1;
		threads.push_back(std::thread([&, start, end]
			{
				for (unsigned int i = start; i < end; i++)
					if (Array[i] % 10 == 0)
					{
						int expected = minResult.load(), desired;
						bool failed = true;
						while (failed && expected > Array[i]) {
							desired = Array[i];
							failed = !minResult.compare_exchange_strong(expected, desired);
							expected = desired;
						}
						sumResult.fetch_add(Array[i]);
					}
			}
		));
	}
	for (auto& item : threads)
	{
		item.join();
	}
	auto endTime = high_resolution_clock::now();
	auto elapsed = duration_cast<nanoseconds>(endTime - startTime);
	cout << "min: " << minResult.load() <<
		"\nsum: " << sumResult.load() << '\n';
	cout << "Randomized in " << (elapsed.count() * 1e-9) << " s\n" << std::endl;
	threads.clear();

	assert(expectedMin == minResult);
	assert(expectedSum == sumResult);

	return elapsed;
}

int main(int argc, char* argv[])
{
	std::vector<std::string> params;

	for (int i = 1; i < argc; i++)
	{
		params.push_back(std::string(argv[i]));
	}

	auto it = params.rbegin();
	switch (params.size())
	{
	case ThreadsMinMax:
		maxRandValue = std::stoi(*it);
		it++;
	case ThreadsMin:
		minRandValue = std::stoi(*it);
		it++;
	case Threads:
		ammountOfThreads = std::stoi(*it);
	}

	cout << "Starting parameters:" <<
		"\n Number of threads: " << ammountOfThreads <<
		"\n min random value: " << minRandValue <<
		"\n max random value: " << maxRandValue << std::endl;

	const unsigned int delta = ((double)maxSizeOfArray - minSizeOfArray) / (ammountOfTests-1);
	std::ofstream file("output.csv");
	rapidcsv::Document output("output.csv", rapidcsv::LabelParams());
	output.InsertColumn(0, std::vector<double>(), "N");
	output.InsertColumn(1, std::vector<double>(), "ST");
	output.InsertColumn(2, std::vector<double>(), "MM");
	output.InsertColumn(3, std::vector<double>(), "MA");
	for (size_t i = 0; i < ammountOfTests; i++)
	{
		firstCalculation = true;
		std::vector<double> row;
		unsigned int sizeOfArray = minSizeOfArray + delta * i;
		opPerThread = (double)sizeOfArray / ammountOfThreads;
		Array = std::vector<int>(sizeOfArray, 0);
		row.push_back(sizeOfArray);

		cout << "size of array: " << sizeOfArray <<
			"\noperations per thread: " << opPerThread <<
			"\n======================" << std::endl;
		//Randomization operation
		{
			std::vector<std::thread> threads;
			cout << "Randomizing..." << std::endl;
			for (size_t i = 0; i < ammountOfThreads; i++)
			{
				unsigned int start = i * opPerThread;
				unsigned int end = (i + 1) * opPerThread - 1;
				threads.push_back(std::thread([=]
					{
						for (unsigned int i = start; i < end; i++)
						{
							Array[i] = intRand(minRandValue, maxRandValue);
						}
					}
				));
			}
			for (auto& item : threads)
			{
				item.join();
			}
			threads.clear();
			cout << "done!" << std::endl;
		}

		//Calculate with 1 thread
		{
			cout << "------single thread------" << std::endl;
			std::vector<double> results;
			for (size_t i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
			{
				results.push_back(singleThread().count());
			}
			row.push_back(average(results));
		}
		//Calculate with multiple threads using single mutex
		{
			cout << "------1 mutex------" << std::endl;
			std::vector<double> results;
			for (size_t i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
			{
				results.push_back(multiThreadMutex().count());
			}
			row.push_back(average(results));
		}
		//Calculate with multiple threads using atomics
		{
			cout << "------atomics------" << std::endl;
			std::vector<double> results;
			for (size_t i = 0; i < NUMBER_OF_EXPERIMENTS; i++)
			{
				results.push_back(multiThreadAtomic().count());
			}
			row.push_back(average(results));
		}
		cout << std::endl;
		output.InsertRow(i, row);
	}
	output.Save(file);
	file.close();
}