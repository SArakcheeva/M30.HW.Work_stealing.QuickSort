#pragma once
#include <thread>
#include <vector>
#include "BlockedQueue.h"
#include <queue>
#include <future>
#include <condition_variable>


using namespace std;



class ThreadPool
{
public:
    ThreadPool();

    // запуск:
    void start();

    // остановка:
    void stop();

    // проброс задач
    res_type push_task(FuncType func, int* arr, long arg1, long arg2);

    // функция входа для потока
    void threadFunc(int qindex);


private:
    // структура, объединяющая задачу для выполнения и промис
    struct TaskWithPromise {
        task_type task;
        promise<void> prom;
    };
    mutex m_locker;

    // количество потоков
    int m_thread_count;

    // потоки
    vector<thread> m_threads;
    
    // ОЧЕРЕДЬ ЗАДАЧ ДЛЯ ПОТОКОВ теперь будет хранить такие структуры
    vector<BlockedQueue<TaskWithPromise>> m_thread_queues;

    // для равномерного распределения задач
    unsigned m_index;
};