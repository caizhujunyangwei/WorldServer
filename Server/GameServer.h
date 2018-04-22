#ifndef __GAMESERVER_H__
#define __GAMESERVER_H__
/**************************************************************************************
 * FILE:gameserver.h
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:服务器基本类 可重载
**************************************************************************************/
#include "./NUSocket.h"
#include "./NUPool.h"
#include "../Module/ModuleTime.h"
#include "../Module/ModuleThread.h"
#include "../ServerControl/SubGameServer.h"

US_NS_MODULE;

class Hall;
class GameMgr;

#define MAX_EPOLL_NUM			2

class GameServer
{
private:
	GameServer(unsigned short _port, unsigned int max);
public:
	~GameServer();
	//监听端口号和最大连接人数
	static GameServer* create(unsigned short _port, unsigned int max);
	virtual bool init();
	SYNTHESIZE(unsigned short, port, Port);			//监听端口号
	SYNTHESIZE(unsigned int, max, MaxRecvNum);		//最大接收数
	SYNTHESIZE(SOCKET, fd, ListenFD);				//监听套接字
	SYNTHESIZE(sockaddr_in*, serveraddr, ServerAddr);	//服务器地址信息
	SYNTHESIZE(unsigned int, sendnum, SendNum);					//发送的消息数量
	SYNTHESIZE(unsigned int, needsendnum, NeedSendNum);			//待发送的消息数量
	SYNTHESIZE(unsigned int, recvnum, RecvNum);					//接收的消息数量
	SYNTHESIZE(int, epfd, Epfd);

public:
	NUPool* m_nuPool;			//服务器线程池

	SubGameServer* g_subServer;	//子游戏大厅
protected:
	Hall* g_hall;		//大厅控制
	GameMgr* g_gameMgr;  //游戏管理器
public:
	virtual bool run(bool tag = true);				//运行服务器
	virtual bool stop();							//停止服务器

	//发送消息
	void sendMsg(int msgid, NUSocket* user, void* data, int len,bool clean = true);
	void sendMsg(int msgid, SOCKET fd, void* data, int len, bool clean = true);

	//不经过proto封装处理的消息，正常发送的
	void sendMsgWithOutProto(SOCKET fd, char * data, int len, bool clean = true);
	void sendMsgWithOutProto(NUSocket* user, char * data, int len, bool clean = true);
};

#endif // !__GAMESERVER_H__
