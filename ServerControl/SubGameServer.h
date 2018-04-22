#ifndef __SUBGAME_SERVER_H__
#define __SUBGAME_SERVER_H__
/**************************************************************************************
 * FILE:subgameserver.h
 * DATE:2018/3/11
 * AUTH:YangW
 * INTR:对小游戏的大厅做的处理
**************************************************************************************/
#include "../Common/PublicCommon.h"
#include "../Server/NUSocket.h"
#include "../Utils/Listener/Listener.h"

class GameServer;

class SubGameServer
{
	SubGameServer();

protected:
	virtual bool init(GameServer*);

	GameServer* g_server;
	ListenerMgr* g_listenMgr;
public:
	virtual ~SubGameServer();
	static SubGameServer* create(GameServer*);

	//注册消息
	bool regAtSubGameServer();
	//分发消息
	bool dispathMsg(NUSocket * user, char * recvData, _msgData * msg);

public:
	void heartBeat(cJSON* msg, NUSocket*);
};


#endif // !__SUBGAME_SERVER_H__

