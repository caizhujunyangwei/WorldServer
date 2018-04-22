#include "ModuleThread.h"
#include <string.h>
#include "../Utils/Log.h"

NS_MODULE_BEGINE

ModuleThread::ModuleThread(const char * data, Task proc, void * target)
	:Module(),m_data(nullptr),m_treadProc(nullptr),m_target(nullptr)
{
	if (data)
	{
		int len = strlen(data) + 1;
		m_data = new char[len];
		strcpy(m_data, data);
	}
	if (proc)
		m_treadProc = proc;
	if (target)
		m_target = target;
}

ModuleThread::~ModuleThread()
{
	this->setRunFlag(false);
	this->setTag(0);
	SAFE_DELETE_ARRAY(m_data);
	SAFE_DELETE(this->m_workThread);
}

ModuleThread * ModuleThread::create(const char * data, Task proc, void * target)
{
	ModuleThread* pRet = new ModuleThread(data, proc, target);
	if (!pRet || !pRet->init())
		SAFE_DELETE(pRet);
	return pRet;
}

bool ModuleThread::init()
{
	RETURN_FALSE_IF(!Module::init());

	RETURN_FALSE_IF(this->m_treadProc == nullptr);

	return true;
}

bool ModuleThread::run(bool detach)
{
	RETURN_FALSE_IF(!Module::run());
	//setRunFlag(true);

	this->m_workThread = new std::thread(this->m_treadProc, m_data, this, m_target);
	Log::Debug("thread [%d] is run.", this->m_workThread->get_id());

	if (detach)
		this->m_workThread->detach();
	else
		this->m_workThread->join();

	return true;
}

bool ModuleThread::stop()
{
	RETURN_FALSE_IF(!Module::stop());
	//setRunFlag(false);

	delete this;
	return true;
}


NS_MODULE_END
