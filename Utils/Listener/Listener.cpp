#include "Listener.h"
#include "../Log.h"

ListenerMgr* ListenerMgr::instance = nullptr;

ListenerMgr::ListenerMgr()
	:max_listener_count(MAX_LISTEN_NUM),cur_listener_count(0)
{
}

ListenerMgr::~ListenerMgr()
{
	for (auto lis : m_vlisteners)
	{
		if (lis.first > 0)
			delete lis.second;
	}
	m_vlisteners.clear();
}

ListenerMgr * ListenerMgr::getInstance()
{
	if (instance == nullptr)
	{
		instance = new ListenerMgr();
	}
	return instance;
}

bool ListenerMgr::addListener(int id, Listener * l)
{
	if (cur_listener_count >= max_listener_count)
		return false;

	m_vlisteners.insert(std::pair<int, Listener*>(id, l));
	cur_listener_count += 1;

	return true;
}

bool ListenerMgr::removeListener(int id)
{
	if (cur_listener_count == 0)
		return false;

	std::map<int, Listener*>::iterator it;
	for (it = m_vlisteners.begin(); it != m_vlisteners.end(); ++it)
	{
		if (it->first == id)
		{
			delete it->second;
			m_vlisteners.erase(it);
			return true;
		}
	}

	return false;
}

void ListenerMgr::callListener(int id, _msgData* msg, NUSocket* p)
{
	std::map<int, Listener*>::iterator it = m_vlisteners.find(id);

	if (it != m_vlisteners.end())
	{
		((Listener_0*)(it->second))->func(msg,p);
	}
	else
	{
		Log::Warn("No listener for msgid: %d\n", id);
	}
}

void ListenerMgr::callListener2(int id , cJSON * root, NUSocket * user)
{
	std::map<int, Listener*>::iterator it = m_vlisteners.find(id);

	if (it != m_vlisteners.end())
	{
		((Listener_1*)(it->second))->func(root, user);
	}
	else
	{
		Log::Warn("No listener for msgid: %d\n", id);
	}
}
