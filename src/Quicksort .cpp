#include "../headers/Quicksort .h"

void Quicksort::quicksort(int* array, long left, long right, std::shared_ptr<std::promise<void>> promise) {
    if(left >= right) {
        if (promise) {
            promise->set_value();
        }
        return;
    }

    long left_bound = left;
    long right_bound = right;
    long middle = array[(left_bound + right_bound) / 2];

    do {
        while(array[left_bound] < middle) {
            left_bound++;
        }
        while(array[right_bound] > middle) {
            right_bound--;
        }

        if(left_bound <= right_bound) {
            std::swap(array[left_bound], array[right_bound]);
            left_bound++;
            right_bound--;
        }
    } while(left_bound <= right_bound);

    if(left_bound < right) {
        auto promise_left = std::make_shared<std::promise<void>>();

        // Если элементов больше порогового значения, запускаем сортировку в потоке
        if(right - left_bound > 10000) {
            // Запускаем рекурсивную сортировку в потоке для левой части
            pool_->push_task(std::bind(&Quicksort::quicksort, this, array, left_bound, right, promise_left),
                 promise_left, array, left_bound, right);

            // Сортируем правую часть синхронно
            quicksort(array, left, right_bound, nullptr);
        } else {
            // Иначе сортируем обе части синхронно
            quicksort(array, left_bound, right, nullptr);
            quicksort(array, left, right_bound, nullptr);
        }

        // Ожидаем завершения сортировки левой части
        if (promise) {
            promise_left->get_future().wait();
        }
    }
}

