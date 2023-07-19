#include "ThreadPool.h"

using namespace std;


// КОНСТРУКТОР
// В конструкторе переменная m_thread_count инициализируется
// результатом вызова функции thread::hardware_concurrency().
// Эта функция возвращает количество потоков, которые физически могут выполняться одновременно.
// Фактически эта функция возвращает количество ядер процессора.
// Если функция вдруг вернет ноль, то выставляется количество потоков, равное 4. 
// Вектор очередей задач инициализируется с количеством, равным m_thread_count.
ThreadPool::ThreadPool() :
	m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4),
	m_thread_queues(m_thread_count)
{

}


// МЕТОД СТАРТА ПОТОКОВ
// Метод старта потоков остался тем же, за одним единственным исключением - 
// Теперь в метод threadFunc передаётся индекс локальной очереди для запускаемого потока.
void ThreadPool::start() 
{
	for (int i = 0; i < m_thread_count; i++) {
		m_threads.emplace_back(&ThreadPool::threadFunc, this, i); // (функция; объект, для которой вызывается функция; индекс)
	}
}


// ФУНКЦИЯ ВЫПОЛНЕНИЯ ДЛЯ ПОТОКА
void ThreadPool::threadFunc(int qindex) 
{
    // В цикле выполняется попытка быстро заполучить задачу сначала из своей очереди,
    // а затем уже из чужих очередей.
    while (true) {

        // очередная задача
        TaskWithPromise task_to_do;
        {

            //удалось ли получить элемент из очереди
            bool res;

            int i = 0;

            // ПОПЫТКА БЫСТРО ЗАБРАТЬ ЗАДАЧУ ИЗ ЛЮБОЙ ОЧЕРЕДИ, НАЧИНАЯ СО СВОЕЙ
            // Напоминаем, что метод fast_pop() не является блокирующим
            // В этих строчках заключается реализация концепции work stealing.
            for (; i < m_thread_count; i++) {
                if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do)) {
                    break;
                }
            }

            // Далее, ЕСЛИ ПОТОКУ НЕ УДАЛОСЬ БЫСТРО ЗАПОЛУЧИТЬ ЗАДАЧУ, 
            // он вызывает уже блокирующий метод pop() у своей очереди.
            if (!res) {
                // вызываем блокирующее получение очереди
                m_thread_queues[qindex].pop(task_to_do);
            }
            // Если поток получил валидный std::function, в который была помещена функция
            // (проверяется с помощью неявного вызова оператора bool: if (!task_to_do)), то он её вызывает, 
            else if (!task_to_do.task) {
                // чтобы не допустить зависания потока, кладем обратно задачу-пустышку
                m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
            }
            // в противном случае поток завершается через return из функции.
            if (!task_to_do.task) {
                return;
            }

            // выполняем задачу
            task_to_do.task;
            task_to_do.prom.set_value();
        }
    }
}


// МЕТОД ЗАВЕРШЕНИЯ ПОТОКА
// Незаметно мы подошли к тому, как можно более лаконично реализовать завершение потока
// без различных флагов(таких как work в предыдущем юните).
void ThreadPool::stop()
{
    // Весь смысл в том, чтобы в каждую очередь передать пустой std::function.
    // Тогда рано или поздно каждая функция потока вычитает невалидный функтор и завершится.
    for (int i = 0; i < m_thread_count; i++) {
        // кладем ЗАДАЧУ-ПУСТЫШКУ в каждую очередь для завершения потока
        TaskWithPromise empty_task;
        m_thread_queues[i].push(empty_task);
    }

    // ждем завершения потоков
    for (auto& t : m_threads) {
        t.join();
    }
}


// МЕТОД ПРОБРОСА ЗАДАЧИ
// с помощью переменной m_index вычисляется очередь, в которую нужно положить очередную задачу, 
// после чего задача кладется в очередь.
res_type ThreadPool::push_task(FuncType f, int* vec, long arg1, long arg2)
{
    lock_guard<mutex> l(m_locker);

    // вычисляем индекс очереди, куда положим задачу
    int queue_to_push = m_index++ % m_thread_count; // индекс будет в пределах от 0 до m_thread_count

    // формируем функтор
    TaskWithPromise twp;
    twp.task = [&vec, arg1, arg2, f] {
        f(vec, arg1, arg2);
    };

    res_type res = twp.prom.get_future();

    // кладем в очередь
    m_thread_queues[queue_to_push].push(twp);

    return res;
}