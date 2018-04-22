#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <functional>
#include <map>
#include "../../Server/NUSocket.h"
#include "../ThirdParty/cJSON.h"

#define MAX_LISTEN_NUM	1000
class Listener
{
public:
	virtual ~Listener() {};
};


class Listener_0:public Listener
{
public:
	std::function<void(_msgData*, NUSocket*)> func;
};

class Listener_1:public Listener
{
public:
	std::function<void(cJSON*, NUSocket*)> func;
};

class ListenerMgr
{
private:
	ListenerMgr();
	static ListenerMgr* instance;
public:
	static ListenerMgr* getInstance();
	~ListenerMgr();
private:
	int max_listener_count;
	int cur_listener_count;

	std::map<int, Listener*> m_vlisteners;

public:
	bool addListener(int, Listener*);

	bool removeListener(int);

	void callListener(int, _msgData*,NUSocket*);

	void callListener2(int, cJSON*, NUSocket*);
};

//注册类中的函数回调
#define register_class_listener(id,__selector__,__target__)												\
do																										\
{																										\
	Listener_0* ll = new Listener_0();																		\
	ll->func = std::bind(__selector__, __target__, std::placeholders::_1,std::placeholders::_2);		\
	ListenerMgr::getInstance()->addListener(id, ll);													\
	ll = nullptr;																						\
} while (0);

//注册全局的函数回调
#define register_func_listener(id,__selector__)									\
do																				\
{																				\
	Listener_0* ll = new (std::nothrow) Listener_0();								\
	ll->func = __selector__;													\
	ListenerMgr::getInstance()->addListener(id, ll);							\
	ll = nullptr;																\
}while (0);																			

//注册类中的函数回调
#define register_class_listener2(id,__selector__,__target__)												\
do																										\
{																										\
	Listener_1* ll = new Listener_1();																		\
	ll->func = std::bind(__selector__, __target__, std::placeholders::_1,std::placeholders::_2);		\
	ListenerMgr::getInstance()->addListener(id, ll);													\
	ll = nullptr;																						\
} while (0);

#endif //!__LISTENER_H__
