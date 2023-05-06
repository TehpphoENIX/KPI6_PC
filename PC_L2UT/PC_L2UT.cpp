#include "pch.h"
#include "CppUnitTest.h"

#include "../PC_L2/ThreadPool.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#define DISTRIBUTION std::mt19937 generator(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());\
std::uniform_int_distribution<unsigned int> distribution(5, 10);

namespace PC_L2_UnitTest
{
	void monitor(ThreadPool& tp)
	{
		auto reading = tp.currentThreadStatus();
		auto queueSize = tp.currentQueueSize();
		Logger::WriteMessage(("(queue:" + std::to_string(queueSize) + "); ").c_str());
		for (auto item : reading)
			Logger::WriteMessage((std::to_string(item.first) + "-" + ThreadPool::toString(item.second) + "; ").c_str());
		Logger::WriteMessage("\n");
	}
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
			DISTRIBUTION
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
			DISTRIBUTION
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
			DISTRIBUTION
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

			while (tp.currentQueueSize() != 0)
			{
				monitor(tp);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			Assert::AreEqual(0u, tp.currentQueueSize());
		}

		TEST_METHOD(PauseTest) {
			DISTRIBUTION
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
			Logger::WriteMessage("Unpaused:");
			monitor(tp);
			tp.pause();
			Logger::WriteMessage(" Paused1:"); 
			monitor(tp);
			auto queueSize = tp.currentQueueSize();
			//unsigned int expected = tp.currentQueueSize();
			std::this_thread::sleep_for(std::chrono::seconds(5));
			//Assert::AreEqual(expected, tp.currentQueueSize());
			Logger::WriteMessage(" Paused2:");
			monitor(tp);
			Assert::AreEqual(queueSize, tp.currentQueueSize());
		}

		TEST_METHOD(StartWaitStop) {
			ThreadPool tp;
			std::this_thread::sleep_for(std::chrono::seconds(5));
			monitor(tp);
		}

		TEST_METHOD(AddDeleteTestQueueSize)
		{
			ThreadPool tp;
			tp.pause();
			tp.addTask(1);
			Assert::AreEqual(1u, tp.currentQueueSize());
			std::this_thread::sleep_for(std::chrono::seconds(1));
			tp.addTask(2);
			Assert::AreEqual(2u, tp.currentQueueSize());
			std::this_thread::sleep_for(std::chrono::seconds(1));
			tp.removeTask();
			tp.removeTask();
			Assert::AreEqual(0u, tp.currentQueueSize());
			std::this_thread::sleep_for(std::chrono::seconds(1));
			tp.terminate();
			Logger::WriteMessage(std::to_string(tp.avgQueueSize()).c_str());
			Assert::AreEqual(1.0, std::round(tp.avgQueueSize()));
		}

		TEST_METHOD(TestAvgTaskTime)
		{
			DISTRIBUTION
			std::vector<unsigned int> tasks;

			ThreadPool tp;
			for (int i = 0; i < 5; i++)
			{
				tasks.push_back(distribution(generator));
				tp.addTask(tasks.back());
				Logger::WriteMessage((std::to_string(tasks.back()) + " ").c_str());
			}

			double avg = 0.0;
			for (auto item : tasks)
			{
				avg += item;
			}
			avg /= tasks.size();

			Logger::WriteMessage(("\navg: " + std::to_string(avg)).c_str());

			while (tp.currentQueueSize() != 0)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			tp.terminate();
			Assert::AreEqual(avg, tp.avgTaskCompletionTime());
		}
	};
	TEST_CLASS(ThreadPoolMeasurements) 
	{
	public:
		void MeasureNTasks(const unsigned int N)
		{
			DISTRIBUTION
			std::vector<unsigned int> tasks;
			for (size_t i = 0; i < N; i++)
			{
				tasks.push_back(distribution(generator));
			}

			ThreadPool tp;
			for (auto item : tasks)
			{
				tp.addTask(item);
			}

			while (tp.currentQueueSize() != 0)
			{
				monitor(tp);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			Assert::AreEqual(0u, tp.currentQueueSize());
			Logger::WriteMessage(("numberOfThreads: " + std::to_string(tp.numberOfThreds()) + '\n').c_str());
			Logger::WriteMessage(("AvgWaitTime: " + std::to_string(tp.avgWaitTime()) + " ns\n").c_str());
			Logger::WriteMessage(("AvgTaskCompletionTime: " + std::to_string(tp.avgTaskCompletionTime()) + " seconds\n").c_str());
			Logger::WriteMessage(("AvgQueueSize: " + std::to_string(tp.avgQueueSize()) + '\n').c_str());
		}

		TEST_METHOD(MesureOn5Tasks) { MeasureNTasks(5); }
		TEST_METHOD(MesureOn10Tasks) { MeasureNTasks(10); }
		TEST_METHOD(MesureOn15Tasks) { MeasureNTasks(15); }
		TEST_METHOD(MesureOn20Tasks) { MeasureNTasks(20); }
	};
}
