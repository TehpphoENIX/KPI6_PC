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

const unsigned int sizeOfArray = 10000;
unsigned int numberOfThreads = 1;
int minRandValue = 0, maxRandValue = 10;

//target 
int Array[sizeOfArray][sizeOfArray] = {};

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

	switch (params.size()) 
	{
	case 3:
		maxRandValue = std::stoi(params[2]);
	case 2:
		minRandValue = std::stoi(params[1]);
	case 1:
		numberOfThreads = std::stoi(params[0]);
	}

	const float randOpPerThread = (float)sizeOfArray * sizeOfArray / numberOfThreads,
		calcOpPerThread = (float)sizeOfArray / numberOfThreads;

	cout << "Starting parameters:" <<
		"\n Matrix height/width: " << sizeOfArray <<
		"\n Number of threads: " << numberOfThreads <<
		"\n min random value: " << minRandValue <<
		"\n max random value: " << maxRandValue <<
		"\n Random operations per thread: " << randOpPerThread <<
		"\n Calculations per thread: " << calcOpPerThread << std::endl;

	std::ofstream matrixSave("PCL1.log");


	std::vector<std::thread> threads;
	
	auto startTime = high_resolution_clock::now();
	for (size_t i = 0; i < numberOfThreads; i++)
	{
		unsigned int start = i * randOpPerThread;
		unsigned int end = (i + 1) * randOpPerThread-1;
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
	/*print(matrixSave);
	cout << "Logged" << std::endl;*/

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

	/*print(matrixSave);
	cout << "Logged" << std::endl;*/
	matrixSave.close();
}