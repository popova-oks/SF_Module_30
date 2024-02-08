#include <iostream>
#include <vector>
#include <algorithm>
#include <future>
#include <memory>
#include <chrono>
#include "ThreadPool.h"

using namespace std;

class Quicksort {
private:    
    ThreadPool* pool_;
    int max_elements_;
public:
    Quicksort(ThreadPool* pool, int max_elements)
        : pool_(pool), max_elements_(max_elements) {}
    void quicksort (int* array, long left, long right, std::shared_ptr<std::promise<void>> promise);    
};
