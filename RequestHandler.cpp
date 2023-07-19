#include "RequestHandler.h"


// в конструкторе запускает пул
RequestHandler::RequestHandler()
{
	m_tpool.start();
}


// в деструкторе - останавливает пул
RequestHandler::~RequestHandler()
{
	m_tpool.stop();
}


//в методе pushRequest бросает функцию - задачу в пул.
//в качестве задачи может быть передана любая функция, подходящая под шаблон FuncType.
res_type RequestHandler::pushRequest(FuncType f, int* arr, long arg1, long arg2)
{
	return m_tpool.push_task(f, arr, arg1, arg2);
}