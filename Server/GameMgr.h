#ifndef __GAMENGR_H__
#define __GAMENGR_H__
/**************************************************************************************
 * FILE:gamemgr.h
 * DATE:2017/12/24
 * AUTH:YangW
 * INTR:游戏控制，主要对连接对象的统一管理
**************************************************************************************/
#include "./NUSocket.h"
#include "../Module/ModuleLock.h"

US_NS_MODULE;

class GameMgr
{
private:
	GameMgr();
	static GameMgr* instance;

	time_t lastCheckTime; //上一次心跳检测时间
	time_t nowCheckTime;  //这一次心跳检测时间
protected:
	std::map<SOCKET, NUSocket*> mapFDs;//所有连接
	std::list<NUSocket*> listDeleteFDs;//可以删除的连接

	SYNTHESIZE(int, lsitensize, ListenSize); //全部连接数
public:
	AALock* g_AllLocks; //所有连接的控制锁
	AALock* g_DeleteLocks;//清除表锁
	~GameMgr();
	static GameMgr* getInstance();

	//关闭所有连接清理
	void clearAll();

	//新加连接
	bool pushNewConnect(NUSocket*);
	//把连接加入待删除表 并不一定会立刻删除 当玩家的锁全部释放完后才会移除
	bool pushDeleteFDS(NUSocket*);
	bool pushDeleteFDS(SOCKET fd);
	//检测待删除表可以移除的就移除了
	void clearDeleteFDs();
	//找到user
	NUSocket* findSocketByFD(SOCKET);
	//获取所有user表
	std::map<SOCKET, NUSocket*>& getAllUsers();

	//获取所有连接人数
	int getAllConnect();

	//检测code是否已经登录合法等
	bool checkAuthCode(const char* code);
};

inline NUSocket* GameMgr::findSocketByFD(SOCKET s) {
	std::map<int,NUSocket*>::iterator it = mapFDs.find(s);
	if (it != mapFDs.end())
		return (it)->second;
	else
		return nullptr;
}

inline std::map<SOCKET, NUSocket*>& GameMgr::getAllUsers()
{
	return (this->mapFDs);
}

inline int GameMgr::getAllConnect()
{
	return this->mapFDs.size();
}

#endif //! __GAMENGR_H__
