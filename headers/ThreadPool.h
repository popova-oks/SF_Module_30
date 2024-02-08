#pragma once
#include <condition_variable>
#include <future>
#include <queue>
#include <vector>

class Quicksort;

typedef std::function<void(int*, long, long, std::shared_ptr<std::promise<void>>)> task_type; // псевдоним task_type функции void  name()

template <class T> class BlockedQueue {
  public:

    // обычный потокобезопасный push
    void push(T &item) {
        std::lock_guard<std::mutex> l(m_locker); // захватываем мьютекс
        m_task_queue.push_back(item); // добавляем элемент в очередь
        // делаем оповещение, чтобы поток, вызвавший
        // pop проснулся и забрал элемент из очереди
        m_event_holder.notify_one();
    }

    // блокирующий метод получения элемента из очереди
    void pop(T& item) {
        std::unique_lock<std::mutex> l(m_locker); // захватываем мьютекс
        if(m_task_queue.empty()) {                // если очередь пуста
            m_event_holder.wait(l, [this] {
                return !m_task_queue.empty();
            }); // ждем, когда появится элемент в очереди
        }
        item = m_task_queue.front(); // забираем первый элемент из очереди
        m_task_queue.pop_front();          // удаляем его из очереди
    }

    // неблокирующий метод получения элемента из очереди
    // возвращает false, если очередь пуста
    bool fast_pop(T& item) {
        std::lock_guard<std::mutex> l(m_locker); // захватываем мьютекс
        if(m_task_queue.empty())                 // если очередь пуста
            return false;                        // выходим
        item = m_task_queue.front();             // иначе забираем элемент
        m_task_queue.pop_front();
        return true;
    }

  private:
    std::mutex m_locker;
    std::deque<T> m_task_queue; // очередь задач
    std::condition_variable m_event_holder;         // уведомитель
};

// пул потоков для задач
class ThreadPool {
  public:
    using FuncType = std::function<void(int*, long, long, std::shared_ptr<std::promise<void>> )>; // Определение FuncType
    ThreadPool();
    void start(int* array, long left, long right, std::shared_ptr<std::promise<void>> promise); // запуск
    void stop();  // остановка
    void push_task (FuncType f,  std::shared_ptr<std::promise<void>> &promise, int* array, long left, long right);
    void threadFunc(int qindex, int* array, long left, long right, std::shared_ptr<std::promise<void>> promise); // функция входа для потока

  private:
    int m_thread_count;                 // количество потоков в пуле
    std::vector<std::thread> m_threads; // потоки
    std::vector<BlockedQueue<task_type>>
        m_thread_queues; // Это массив (или контейнер) очередей, в которых хранятся задачи для
                         // каждого потока в пуле.
    int m_index; // для равномерного распределения задач
};
