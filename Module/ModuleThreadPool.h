#ifndef __MODULE_THREAD_POOL_H__
#define __MODULE_THREAD_POOL_H__
/**************************************************************************************
 * FILE:modulethreadpool.h
 * DATE:2017/12/19
 * AUTH:YangW
 * INTR:线程池基础类 注意删除实例化的线程池时必须确保线程已经退出
**************************************************************************************/

#include "./ModuleThread.h"

NS_MODULE_BEGINE

class ModuleThreadPool :public Module
{
protected:
	char* m_data;				//数据
	Task m_treadProc;			//线程处理函数
	void* m_target;				//调用对象
	ModuleThread** m_workThread;    //线程对象

	ModuleThreadPool(int threadNum, const char* data,
		Task proc, void* target);
	SYNTHESIZE(int, threadnum, ThreadNum);	//线程数
public:
	~ModuleThreadPool();

	virtual bool init();
	virtual bool run(bool tag = true);
	virtual bool stop();

	static ModuleThreadPool* create(int threadNum,const char* data, 
		Task proc, void* target);

};

NS_MODULE_END
 
#endif // !__MODULE_THREAD_POOL_H__
