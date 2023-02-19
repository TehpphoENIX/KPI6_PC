#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <string>
#include <fstream>
using std::thread;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::cout;

unsigned int sizeOfArray = 10000;
unsigned int numberOfThreads = 1;
int minRandValue = 0, maxRandValue = 10;
bool logMatrix = false;

enum Parameter {
	size = 1,
	sizeThreads = 2,
	sizeThreadsMin = 3,
	sizeThreadsMinMax = 4,
	sizeThreadsMinMaxLog = 5
};

//target 
std::vector<std::vector<int>> Array;

int intRand(const int& min, const int& max) {
	static thread_local std::mt19937 generator;
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}

void print(std::ofstream& file) {
	for (size_t i = 0; i < sizeOfArray; i++)
	{
		for (size_t j = 0; j < sizeOfArray; j++)
		{
			file.width(5);
			file << Array[i][j] << ' ';
		}
		file << '\n' << std::endl;
	}
}

void randomizeCellsEx(const unsigned int startPos, const unsigned int EndPos) {
	for (size_t i = startPos; i <= EndPos; i++)
	{
		if (i / sizeOfArray != i % sizeOfArray) {
			Array[i / sizeOfArray][i % sizeOfArray] = intRand(minRandValue, maxRandValue);
		}
	}
}

void calculateCellsEx(const unsigned int startPos, const unsigned int EndPos) {
	for (size_t i = startPos; i <= EndPos; i++)
	{
		for (size_t j = 0; j < sizeOfArray; j++)
		{
			if (j != i) {
				Array[i][i] += Array[i][j];
			}
		}
	}
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
	case sizeThreadsMinMaxLog:
		logMatrix = std::stoi(*it);
		it++;
	case sizeThreadsMinMax:
		maxRandValue = std::stoi(*it);
		it++;
	case sizeThreadsMin:
		minRandValue = std::stoi(*it);
		it++;
	case sizeThreads:
		numberOfThreads = std::stoi(*it);
		it++;
	case size:
		sizeOfArray = std::stoi(*it);
	}

	const double randOpPerThread = (double)sizeOfArray * sizeOfArray / numberOfThreads,
		calcOpPerThread = (double)sizeOfArray / numberOfThreads;

	cout << "Starting parameters:" <<
		"\n Matrix height/width: " << sizeOfArray <<
		"\n Number of threads: " << numberOfThreads <<
		"\n min random value: " << minRandValue <<
		"\n max random value: " << maxRandValue <<
		"\n Random operations per thread: " << randOpPerThread <<
		"\n Calculations per thread: " << calcOpPerThread << std::endl;

	Array = std::vector<std::vector<int>>(sizeOfArray, std::vector<int>(sizeOfArray, 0));

	std::ofstream matrixSave("PCL1(1).log");
	std::ofstream matrixSave2("PCL1(2).log");

	std::vector<std::thread> threads;
	
	auto startTime = high_resolution_clock::now();
	for (size_t i = 0; i < numberOfThreads; i++)
	{
		unsigned int start = i * randOpPerThread;
		unsigned int end = (i + 1) * randOpPerThread - 1;
		threads.push_back(thread(randomizeCellsEx, start, end));
	}
	for (thread& th : threads)
	{
		th.join();
	}
	auto endTime = high_resolution_clock::now();
	auto elapsed = duration_cast<nanoseconds>(endTime - startTime);

	threads.clear();
	cout << "Randomized in " << (elapsed.count() * 1e-9) << " s\n" << std::endl;

	if (logMatrix)
	{
		print(matrixSave);
		cout << "Logged" << std::endl;
		matrixSave.close();
	}

	startTime = high_resolution_clock::now();
	for (size_t i = 0; i < numberOfThreads; i++)
	{
		unsigned int start = i * calcOpPerThread;
		unsigned int end = (i + 1) * calcOpPerThread - 1;
		threads.push_back(thread(calculateCellsEx, start, end));
	}
	for (thread& th : threads)
	{
		th.join();
	}
	endTime = high_resolution_clock::now();
	auto elapsed2 = duration_cast<nanoseconds>(endTime - startTime);

	threads.clear();
	cout << "Calculated in " << elapsed2.count() * 1e-9 << " s\n";
	cout << "total: " << (elapsed.count() + elapsed2.count()) * 1e-9 << " s\n" << std::endl;

	if (logMatrix)
	{
		print(matrixSave2);
		cout << "logged" << std::endl;
		matrixSave2.close();
	}
}