#include "pch.h"
#include "CppUnitTest.h"

#include "../PC_L2/ThreadPool.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PC_L2_UnitTest
{
	TEST_CLASS(ThreadPoolTesting)
	{
	public:
		//start ThreadPool and end it immediately
		TEST_METHOD(StartStop)
		{
			try {
				{
					ThreadPool tp(false);
				}
				{
					ThreadPool tp(true);
				}
			}
			catch (std::exception) {
				Assert::Fail();
			}
		}
		//start ThreadPool, insert several tasks and end it immediately
		TEST_METHOD(StartTaskStopIm)
		{
			std::mt19937 generator;
			std::uniform_int_distribution<unsigned int> distribution(10, 15);
			std::vector<unsigned int> tasks;
			for (size_t i = 0; i < 10; i++)
			{
				tasks.push_back(distribution(generator));
			}

			ThreadPool tp;
			for (auto item : tasks)
			{
				tp.addTask(item);
			}
			tp.terminateIm();
		}
		//start ThreadPool, insert several tasks, pause for n seconds and end it waiting for completion
		TEST_METHOD(StartTaskStop)
		{
			std::mt19937 generator;
			std::uniform_int_distribution<unsigned int> distribution(10, 15);
			std::vector<unsigned int> tasks;
			for (size_t i = 0; i < 10; i++)
			{
				tasks.push_back(distribution(generator));
			}

			ThreadPool tp;
			for (auto item : tasks)
			{
				tp.addTask(item);
			}

			std::this_thread::sleep_for(std::chrono::seconds(5));

			tp.terminate();
		}

		TEST_METHOD(FullCompletion)
		{
			std::mt19937 generator;
			std::uniform_int_distribution<unsigned int> distribution(10, 15);
			std::vector<unsigned int> tasks;
			for (size_t i = 0; i < 10; i++)
			{
				tasks.push_back(distribution(generator));
			}

			ThreadPool tp;
			std::this_thread::sleep_for(std::chrono::seconds(5));
			for (auto item : tasks)
			{
				tp.addTask(item);
			}

			while (tp.currentQueueSize() != 0) std::this_thread::sleep_for(std::chrono::seconds(1));
			Assert::AreEqual(0u, tp.currentQueueSize());
		}

		TEST_METHOD(PauseTest) {
			std::mt19937 generator;
			std::uniform_int_distribution<unsigned int> distribution(10, 15);
			std::vector<unsigned int> tasks;
			for (size_t i = 0; i < 10; i++)
			{
				tasks.push_back(distribution(generator));
			}

			ThreadPool tp;
			for (auto item : tasks)
			{
				tp.addTask(item);
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
			tp.pause();
			//unsigned int expected = tp.currentQueueSize();
			std::this_thread::sleep_for(std::chrono::seconds(5));
			//Assert::AreEqual(expected, tp.currentQueueSize());
		}

		TEST_METHOD(StartWaitStop) {
			ThreadPool tp;
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	};
}
