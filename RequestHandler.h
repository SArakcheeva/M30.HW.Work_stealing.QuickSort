// RequestHandler - �����, � ������� ����� ������� ��� �������


#pragma once
#include "ThreadPool.h"

class RequestHandler {
public:
    RequestHandler();
    ~RequestHandler();

    // �������� ������� �� ����������
    res_type pushRequest(FuncType f, int* arr, long arg1, long arg2);

private:
    // ��� �������
    ThreadPool m_tpool;
};