#ifndef __MODULE_LOCK_H__
#define __MODULE_LOCK_H__
/**************************************************************************************
 * FILE:modulelock.h
 * DATE:2017/12/17
 * AUTH:YangW
 * INTR:线程互斥量，自旋锁等的封装 (单例类)
**************************************************************************************/
#include "./Module.h"
#include <mutex>
#include <atomic>
#include <map>

NS_MODULE_BEGINE

#define NON_LOCK		0x00	//空
#define SPIN_LOCK		0x01	//自旋锁
#define MUTEX_LOCK		0x02	//互斥锁
#define RW_LOCK			0x04	//读写锁 //暂未实现

//自旋锁实现
class spin_mutex {
	std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
public:
	spin_mutex() = default;
	spin_mutex(const spin_mutex&) = delete;
	spin_mutex& operator= (const spin_mutex&) = delete;
	void lock() {
		while (flag.exchange(true, std::memory_order_acquire));
	}
	void unlock() {
		flag.store(false, std::memory_order_release);
	}
	bool trylock() {
		if (flag == false)
		{
			lock();
			return true;
		}
		return false;
	}
};

//封装的锁
struct AALock
{
	AALock(const AALock&) = delete;
	AALock& operator= (const AALock&) = delete;
	AALock():m_id(-1),m_type(0x00),m_mutex(nullptr),m_spinMutex(nullptr){}
	~AALock() {
		SAFE_DELETE(m_mutex);
		SAFE_DELETE(m_spinMutex);
	}

	int m_id;				//唯一id
	char m_type;			//类型
	std::mutex* m_mutex;		//互斥锁 m_type == MUTEX_LOCK
	spin_mutex* m_spinMutex;	//自旋锁 m_type == SPIN_LOCK
};

class ModuleLock :public Module
{
	ModuleLock();
	~ModuleLock();
	static ModuleLock* instance;

	std::map<int, AALock*> mapLocks;	//所有锁的存储
public:
	//@type:锁类型 SPIN_LOCK MUTEX_LOCK RW_LOCK
	//可以用 | 实现多个锁
	static AALock* createAALock(int id,char type = NON_LOCK);
	//获取单例类
	static ModuleLock* getInstance();

	virtual bool stop();
#pragma region 通用方法
	//设置互斥锁状态
	static bool setMutexLock(AALock* lock, bool status);
	//尝试设定互斥锁状态 如果锁已经锁定那么将返回false 
	//否则返回true并且锁定锁
	static bool setMutexTryLock(AALock* lock);

	//设置自旋锁状态
	static bool setSpinLock(AALock* lock, bool status);
	//尝试设定自旋锁状态 如果锁已经锁定那么将返回false 
	//否则返回true并且锁定锁
	static bool setSpinTryLock(AALock* lock);

	//根据id获取Lock
	AALock* getAALockByID(int id);
	//清楚所有Lock
	void clearAllLocks();

	//清除指定的Lock
	static void clearLock(AALock* lock);

#pragma endregion 通用方法
};

NS_MODULE_END
#endif
