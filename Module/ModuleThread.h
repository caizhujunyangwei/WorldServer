#ifndef __NODULE_THREAD_H__
#define __NODULE_THREAD_H__
/**************************************************************************************
 * FILE:modulethread.h
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:线程创建类 注意删除实例化的线程时必须确保线程已经退出
**************************************************************************************/
#include "./Module.h"
#include <thread>
#include <unistd.h>  //sleep

NS_MODULE_BEGINE

//线程函数格式
typedef void (*Task)(const char* data,void* pthread,void* target);

class ModuleThread: public Module
{
public:
	ModuleThread(const char* data, Task proc, void* target);
	virtual ~ModuleThread();
	static ModuleThread* create(const char* data,Task proc, void* target);

	char* m_data;				//数据
	Task m_treadProc;			//线程处理函数
	void* m_target;				//调用对象
	std::thread* m_workThread;    //线程对象

protected:
	//重载父类函数
	virtual bool init() ;
public:
	virtual bool run(bool detach = true);
	virtual bool stop();
};

NS_MODULE_END
#endif
