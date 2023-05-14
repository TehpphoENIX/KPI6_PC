#include <iostream>
#include <vector>
#include <cassert>
#include <future>

std::vector<std::vector<int>> matrix = {
    {1,2},
    {3,4}
};

int main()
{
    std::promise<std::vector<std::vector<int>>> promise;
    promise.set_value(matrix);
    promise.get_future().get();
    promise.get_future().get();
}