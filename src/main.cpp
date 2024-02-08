#include "../headers/Quicksort .h"
#include "../headers/ThreadPool.h"
#include <chrono>
#include <future>
#include <iostream>

using namespace std;

int main() {
    constexpr int arr_size = 1000000;
    int* array = new int[arr_size];

    for(int i = 0; i < arr_size; ++i) {
        array[i] = rand() % 1000000;
    }

    ThreadPool threadPool;
    Quicksort qsort(&threadPool, 100000);
    auto promise = std::make_shared<std::promise<void>>();
    auto start = chrono::high_resolution_clock::now();
    threadPool.start(array, 0, arr_size - 1, promise);
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end - start;
    cout << "Time: " << elapsed.count() << " seconds" << endl;

    delete[] array;
    return 0;
}
