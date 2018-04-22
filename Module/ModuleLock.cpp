#include "ModuleLock.h"
#include "../Utils/Log.h"

NS_MODULE_BEGINE
ModuleLock* ModuleLock::instance = nullptr;
ModuleLock::ModuleLock()
{
	mapLocks.clear();
}

ModuleLock::~ModuleLock()
{
	clearAllLocks();
}

AALock * ModuleLock::createAALock(int id,char type)
{
	if (instance->mapLocks.size() > 0)
	{
		if (instance->mapLocks.end() != instance->mapLocks.find(id))
		{
			//这个id已经有了啊
			return nullptr;
		}
	}

	AALock* pRet = new AALock();
	if (type & MUTEX_LOCK) //互斥锁
	{
		pRet->m_mutex = new std::mutex();
		pRet->m_type |= MUTEX_LOCK;
	}
	if (type & SPIN_LOCK) //自旋锁
	{
		pRet->m_spinMutex = new spin_mutex();
		pRet->m_type |= SPIN_LOCK;
	}

	pRet->m_id = id;
	instance->mapLocks.insert(std::pair<int, AALock*>(id, pRet));
	Log::Debug("new lock[%d] init success!",id);

	return pRet;
}

ModuleLock * ModuleLock::getInstance()
{
	if (!instance)
	{
		instance = new ModuleLock();
		instance->init();
	}

	return instance;
}

bool ModuleLock::stop()
{
	RETURN_FALSE_IF(!Module::stop());
	delete this;
	return true;
}

bool ModuleLock::setMutexLock(AALock * lock, bool status)
{
	if (lock && lock->m_mutex)
	{
		if (status)
			lock->m_mutex->lock();
		else
			lock->m_mutex->unlock();
		return true;
	}
	return false;
}

bool ModuleLock::setMutexTryLock(AALock * lock)
{
	if (lock && lock->m_mutex)
	{
		return (lock->m_mutex->try_lock());
	}
	return false;
}

bool ModuleLock::setSpinLock(AALock * lock, bool status)
{
	if (lock && lock->m_spinMutex)
	{
		if (status)
			lock->m_spinMutex->lock();
		else
			lock->m_spinMutex->unlock();
		return true;
	}
	return false;
}

bool ModuleLock::setSpinTryLock(AALock * lock)
{
	if (lock && lock->m_spinMutex)
	{
		return (lock->m_spinMutex->trylock());
	}
	return false;
}

AALock * ModuleLock::getAALockByID(int id)
{
	std::map<int, AALock*>::iterator it = mapLocks.find(id);
	if (it != mapLocks.end())
		return (it->second);
	else
		return nullptr;
}

void ModuleLock::clearAllLocks()
{
	std::map<int, AALock*>::iterator it = mapLocks.begin();
	for (; it != mapLocks.end();)
	{
		delete ((it)->second);
		it = mapLocks.erase(it);
	}
	mapLocks.clear();
}

void ModuleLock::clearLock(AALock * lock)
{
	if (lock)
	{
		std::map<int, AALock*>::iterator it = instance->mapLocks.find(lock->m_id);
		if(it != instance->mapLocks.end())
			instance->mapLocks.erase(it);
		SAFE_DELETE(lock);
	}
}


NS_MODULE_END