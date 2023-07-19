// RequestHandler - класс, в котором будем хранить пул потоков


#pragma once
#include "ThreadPool.h"

class RequestHandler {
public:
    RequestHandler();
    ~RequestHandler();

    // отправка запроса на выполнение
    res_type pushRequest(FuncType f, int* arr, long arg1, long arg2);

private:
    // пул потоков
    ThreadPool m_tpool;
};