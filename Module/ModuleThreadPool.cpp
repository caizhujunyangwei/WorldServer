#include "ModuleThreadPool.h"
#include "../Utils/Log.h"

NS_MODULE_BEGINE

ModuleThreadPool::ModuleThreadPool(int threadNum, const char* data,Task proc, void* target)
{
	setThreadNum(threadNum);
	int len = strlen(data) + 1;
	this->m_data = new char[len];
	strcpy(m_data, data);
	this->m_treadProc = proc;
	this->m_target = target;
	this->m_workThread = new ModuleThread*[threadNum];
}

ModuleThreadPool::~ModuleThreadPool()
{
	setRunFlag(false);
	SAFE_DELETE_ARRAY(this->m_data);
	for (int i = 0;i < this->getThreadNum();++i)
		SAFE_DELETE(this->m_workThread[i]);
	SAFE_DELETE_ARRAY(this->m_workThread);
}

ModuleThreadPool * ModuleThreadPool::create(int threadNum, const char * data, Task proc, void * target)
{
	ModuleThreadPool* pRet = new ModuleThreadPool(threadNum, data, proc, target);
	if (!pRet || !pRet->init())
	{
		SAFE_DELETE(pRet);
	}
	return pRet;
}

bool ModuleThreadPool::init()
{
	RETURN_FALSE_IF(!Module::init());


	Log::Info("初始化内存池完成:%s", this->m_data);

	return true;
}

bool ModuleThreadPool::run(bool tag)
{
	Log::Info("run threadpool start!");
	for (int i = 0;i < this->getThreadNum();++i)
	{
		Log::Debug("threadpool thread %d start", i);
		this->m_workThread[i] = ModuleThread::create(this->m_data, this->m_treadProc, this->m_target);
		this->m_workThread[i]->run(true);
	}

	return true;
}

bool ModuleThreadPool::stop()
{
	RETURN_FALSE_IF(!Module::stop());

	for (int i = 0;i < this->getThreadNum();++i)
	{
		this->m_workThread[i]->stop();
		sleep(1);
	}

	delete this;
	return true;
}

NS_MODULE_END