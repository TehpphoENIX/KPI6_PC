#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <iostream>


int main()
{
    std::shared_ptr<std::mutex> A;

    {
        std::shared_ptr<std::mutex> B = std::make_shared<std::mutex>();
        A = B;
    }
}