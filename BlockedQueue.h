// BlockedQueue<> - вспомогательный класс, который является потокобезопасной очередью


#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>

using namespace std;

// удобное определение для сокращения кода
typedef function<void()> task_type;

// будет символизировать тип результата метода push_task
typedef future<void> res_type;

// тип указатель на функцию, которая является эталоном для функций задач
// Его предназначение — задать эталон для функций, которые могут быть переданы в качестве функций обработки задач.
typedef void (*FuncType) (int* array, long left, long right);


template<class T>
class BlockedQueue 
{
public:
    void push(T& item)
    {
        // блокируется мьютекс 
        lock_guard<mutex> l(m_locker);

        // элемент добавляется в очередь m_task_queue (обычный потокобезопасный push)
        m_task_queue.push(std::move(item));

        // После этого вызывается m_event_holder.notify_one() - т.е. делаем оповещение,
        // чтобы поток, вызвавший pop, проснулся и забрал элемент из очереди
        // (Этот вызов разбудит поток, который вызвал pop(), когда элементов не было в очереди)
        m_event_holder.notify_one();
    }

    // блокирующий метод получения элемента из очереди
    void pop(T& item)
    {
        // блокируется мьютекс
        unique_lock<mutex> l(m_locker);

        // затем очередь проверяется на наличие элементов
        if (m_task_queue.empty()) {
            // если таковых нет, то вызывается m_event_holder.wait().
            // Поток, вызвавший этот метод, блокируется, пока какой - нибудь другой поток не вызовет push().
            m_event_holder.wait(l, [this] {return !m_task_queue.empty(); });
        }

        item = std::move(m_task_queue.front());
        m_task_queue.pop();
    }

    // неблокирующий метод получения элемента из очереди
    // возвращает false, если очередь пуста
    bool fast_pop(T& item)
    {
        lock_guard<mutex> l(m_locker);

        // проверяет, не является ли очередь пустой
        if (m_task_queue.empty()) {
            // просто выходим
            return false; // Возвращает результат того, получилось ли что - то достать из очереди
        }

        // и достает оттуда элемент (забираем элемент)
        item = std::move(m_task_queue.front());
        m_task_queue.pop();

        // Возвращает результат того, получилось ли что - то достать из очереди
        return true;
    }


private:
    // мьютекс
    mutex m_locker;

    // очередь задач
    queue<T> m_task_queue;

    // уведомитель (условная переменная)
    condition_variable m_event_holder;
};