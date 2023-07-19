/* В качестве решения по оптимизации M30.TreadsPoll.Start можно создать локальную очередь для каждого потока.
   Таким образом каждый поток будет отрабатывать задачи из своей очереди,
   не блокируя другие потоки во время служебных проверок очереди.

   Причем если локальная очередь у потока опустошается,
   то пусть он пытается заполучить задачу из чужих очередей, если они есть.

   Такой подход называется WORK STEALING («кража работы»).
   Его можно сравнить с очередью в магазине, когда покупатели переходят на свободные кассы, если есть такие.

   Приступим к реализации описанной теории. Так как у нас теперь будет несколько очередей,
   введём вспомогательный класс BlockedQueue<>, который является потокобезопасной очередью.
 */

#include <iostream>
#include <chrono>
#include <random>
#include "RequestHandler.h"


// флаг - для сравнения скорости работы многопоточной и обычной сортировок
// (для указания функции запускать вычисления в потоке или нет)
bool make_thread = true;

RequestHandler rh;

// вектор фьючерсов, в который складываются все фьючерсы, полученные в результате передачи задачи на отработку
std::vector<res_type> results;


// ФУНКЦИЯ, ВЫПОЛНЯЮЩАЯ ЗАДАЧУ - функция сортировки.
// принцип работы быстрой сортировки состоит в выборе опорного элемента, 
// после чего все бОльшие элементы переносятся вправо, а меньшие влево. 
// После этого запускается рекурсивно та же функция для левых и правых частей разбитого массива.
void quicksort(int* array, long left, long right)
{
    if (left >= right) return;
    long left_bound = left;
    long right_bound = right;

    long middle = array[(left_bound + right_bound) / 2];

    do {
        while (array[left_bound] < middle) {
            left_bound++;
        }
        while (array[right_bound] > middle) {
            right_bound--;
        }

        //Меняем элементы местами
        if (left_bound <= right_bound) {
            std::swap(array[left_bound], array[right_bound]);
            left_bound++;
            right_bound--;
        }
    } while (left_bound <= right_bound);


    // Идея распараллеливания в том, что сортировку для правых и левых частей 
    // можно запустить в различных потоках, так как эти части независимы друг от друга
    //
    // Тут мы видим проверку того самого флага make_thread и еще дополнительное условие, 
    // что размер части для сортировки не превышает 100000. 
    // Это сделано для того, чтобы ограничить количество запускаемых потоков. 
    // Если условия выполняются, то с помощью std::async мы запускаем поток,
    // передав в него функцию, сортирующую левую часть. 
    // После чего мы синхронно в том же потоке вызываем рекурсию для правой части.
    //
    // если элементов в левой части больше чем 100000, бросаем задачу в пул потоков    
    if (make_thread && (right_bound - left > 100000))
    {
        // будем возвращать результат в качестве выходного значения  
        res_type result = rh.pushRequest(quicksort, array, left, right_bound);
        result.wait();

        quicksort(array, left_bound, right);       
    }
    else {
        // запускаем обе части синхронно
        quicksort(array, left, right_bound);
        quicksort(array, left_bound, right);
    }
}


int main()
{
    // сгенерируем массив целых чисел
    srand(0);
    long arr_size = 10000000;
    int* array = new int[arr_size];
    if (array == nullptr) {
        return 0;
    }
    for (long i = 0; i < arr_size; i++) {
        array[i] = rand() % 50000;
    }

    // запускаем быструю сортировку в многопоточном варианте
    time_t start, end;
    time(&start);
    quicksort(array, 0, arr_size - 1);
    time(&end);
    // и выводим время, за которое она была выполнена
    double seconds = difftime(end, start);
    printf("The time: %f seconds\n", seconds);


    for (long i = 0; i < arr_size - 1; i++) {
        if (array[i] > array[i + 1]) {
            cout << "Unsorted" << endl;
            break;
        }
    }

    // сгенерируем массив целых чисел для однопоточного варианта
    for (long i = 0; i < arr_size; i++) {
        array[i] = rand() % 500000;
    }
    // однопоточный запуск быстрой сортировки
    make_thread = false;// маркер, что выполнять сортировку в одном потоке
    time(&start);
    quicksort(array, 0, arr_size - 1);
    time(&end);
    seconds = difftime(end, start);
    printf("The time: %f seconds\n", seconds);

    delete[] array;

    return 0;
}