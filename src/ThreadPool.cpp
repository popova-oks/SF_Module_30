
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>

#include "../headers/ThreadPool.h"
#include "../headers/Quicksort .h"
using namespace std;

ThreadPool::ThreadPool()
    : m_thread_count(thread::hardware_concurrency() ? thread::hardware_concurrency() : 4),
      m_thread_queues(m_thread_count) {}

void ThreadPool::start(int* array, long left, long right, std::shared_ptr<std::promise<void>> promise) {
    for(int i = 0; i < m_thread_count; i++) {
        m_threads.emplace_back(&ThreadPool::threadFunc, this, i, array, left, right, promise);
    }
    
}

void ThreadPool::stop() {
    for(int i = 0; i < m_thread_count; i++) {
        // кладем задачу-пустышку в каждую очередь
        // для завершения потока
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for(auto& t : m_threads) {
        t.join();
    }
}

void ThreadPool::push_task (FuncType f,  std::shared_ptr<std::promise<void>> &promise, int* array, long left, long right) {
   // вычисляем индекс очереди, куда положим задачу
   int queue_to_push = m_index++ % m_thread_count;
   
   // формируем функтор с помощью std::bind
    task_type task ([=](int*, long, long, std::shared_ptr<std::promise<void>>) {
        f(array, left, right, promise);
        promise->set_value();
    });
   // кладем в очередь
   m_thread_queues[queue_to_push].push(task);
}


void ThreadPool::threadFunc(int qindex, int* array, long left, long right, std::shared_ptr<std::promise<void>> promise) {
    while(true) {
        // обработка очередной задачи
        task_type task_to_do; // Создается переменная для хранения следующей задачи.
        bool res; // Инициализируется переменная res для отслеживания успешности операции получения
                  // задачи из очереди.
        int i = 0;
        // Используется цикл for для попытки быстрого извлечения задачи из любой из очередей.
        // Цикл проходится по всем очередям, начиная с той, которая соответствует текущему индексу
        // потока (qindex). Если задача быстро извлекается (res == true), цикл прерывается.
        for(; i < m_thread_count; i++) {
            // Это арифметическое выражение для выбора следующей очереди задач.
            // qindex - индекс текущего потока, i - переменная цикла, m_thread_count - общее
            // количество потоков .fast_pop(task_to_do): Это вызов функции fast_pop у объекта
            // очереди.
            if(res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break; // если задача быстро извлечена
        }
        // Если задача не была извлечена быстро, выполняется блокирующая операция получения задачи
        // из собственной очереди потока
        if(!res) {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        } else if(!task_to_do) {
            // чтобы не допустить зависания потока
            // кладем обратно задачу-пустышку другого потока обратно
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        // Если в потоке задача пустышка,
        // поток выходит из цикла и завершает свою работу.
        if(!task_to_do) {
            return;
        }
        // иначе выполняем задачу
        task_to_do(array, left, right, promise);
    }
}