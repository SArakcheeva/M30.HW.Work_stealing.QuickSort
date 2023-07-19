#include "RequestHandler.h"


// � ������������ ��������� ���
RequestHandler::RequestHandler()
{
	m_tpool.start();
}


// � ����������� - ������������� ���
RequestHandler::~RequestHandler()
{
	m_tpool.stop();
}


//� ������ pushRequest ������� ������� - ������ � ���.
//� �������� ������ ����� ���� �������� ����� �������, ���������� ��� ������ FuncType.
res_type RequestHandler::pushRequest(FuncType f, int* arr, long arg1, long arg2)
{
	return m_tpool.push_task(f, arr, arg1, arg2);
}