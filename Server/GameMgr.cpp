#include "./GameMgr.h"

GameMgr* GameMgr::instance = nullptr;
GameMgr::GameMgr()
{
	setListenSize(0);
	mapFDs.clear();
	listDeleteFDs.clear();
	lastCheckTime = nowCheckTime = 0;

	g_AllLocks = ModuleLock::createAALock(Lock_SOCKETS,SPIN_LOCK);
	g_DeleteLocks = ModuleLock::createAALock(Lock_Delete_SOCKETS, SPIN_LOCK);
}

GameMgr::~GameMgr()
{
	clearAll();
	mapFDs.clear();
	listDeleteFDs.clear();

	ModuleLock::clearLock(g_AllLocks);
	ModuleLock::clearLock(g_DeleteLocks);
}

GameMgr * GameMgr::getInstance()
{
	if (!instance)
		instance = new GameMgr;

	return instance;
}

void GameMgr::clearAll()
{
	g_AllLocks->m_spinMutex->lock();
	while (mapFDs.size() > 0) {
		auto fd = mapFDs.begin();
		if (fd->second->getFD() > 0)
		{
			close(fd->second->getFD());
			delete (fd->second);
			mapFDs.erase(fd);
		}
	}
	g_AllLocks->m_spinMutex->unlock();

	g_DeleteLocks->m_spinMutex->lock();
	listDeleteFDs.clear();
	g_DeleteLocks->m_spinMutex->unlock();
}

bool GameMgr::pushNewConnect(NUSocket *s)
{
	g_AllLocks->m_spinMutex->lock();
	mapFDs.insert(std::pair<SOCKET, NUSocket*>(s->getFD(), s));
	g_AllLocks->m_spinMutex->unlock();

	return true;
}

bool GameMgr::pushDeleteFDS(NUSocket *s)
{
	auto ss = listDeleteFDs.begin();
	for(;ss!=listDeleteFDs.end();++ss)
	{
		if ((*ss)->getFD() == s->getFD())
		{
			return false;
		}
	}

	close(s->getFD());
	g_DeleteLocks->m_spinMutex->lock();
	listDeleteFDs.push_back(s);
	g_DeleteLocks->m_spinMutex->unlock();
	return true;
}

bool GameMgr::pushDeleteFDS(SOCKET fd)
{
	auto s = mapFDs.find(fd);
	if (s != mapFDs.end())
	{
		auto ss = listDeleteFDs.begin();
		for (; ss != listDeleteFDs.end(); ++ss)
		{
			if ((*ss)->getFD() == s->second->getFD())
			{
				return false;
			}
		}

		close(s->first);
		g_DeleteLocks->m_spinMutex->lock();
		listDeleteFDs.push_back(s->second);
		g_DeleteLocks->m_spinMutex->unlock();
		return true;
	}
	
	return false;
}

void GameMgr::clearDeleteFDs()
{
	if (listDeleteFDs.size() > 0) {
		g_AllLocks->m_spinMutex->lock();

		for (auto it = listDeleteFDs.begin(); it != listDeleteFDs.end();)
		{
			if ((*it) && (*it)->m_userLock->m_mutex->try_lock() && (*it)->m_userLock->m_spinMutex->trylock())
			{
				auto s = mapFDs.find((*it)->getFD());
				if (s != mapFDs.end()) {
					delete (s->second);
					mapFDs.erase(s);
				}
				g_DeleteLocks->m_spinMutex->lock();
				it = listDeleteFDs.erase(it);
				g_DeleteLocks->m_spinMutex->unlock();
			}
			else
				++it;
		}

		g_AllLocks->m_spinMutex->unlock();
	}

	//检测心跳
	nowCheckTime = ModuleTime::getLocalTimeNumber();
	if (nowCheckTime - lastCheckTime > 2)
	{
		lastCheckTime = nowCheckTime;
		for (auto user = mapFDs.begin(); user != mapFDs.end();++user)
		{
			if (nowCheckTime - (user->second->getLastHeartTime()) > 18) //正常超过8s连接不上认为是断线了
			{
				const std::string str = user->second->getName();
				Log::Error("heart is long time no recv,userName :%s", str.c_str());
				close(user->first);
				pushDeleteFDS(user->second);
			}
		}
	}

}

bool GameMgr::checkAuthCode(const char * code)
{
	//Log::Warn("code---: %s", code);
	const char* authcode = nullptr;
	for (auto& var : mapFDs)
	{
		authcode = var.second->getAuthCode();
		if (authcode && strcmp(authcode, code) == 0){
				return false;
		}
		authcode = nullptr;
	}
	
	return true;
}

