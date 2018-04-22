#ifndef __NUSOCKET_H__
#define __NUSOCKET_H__
/**************************************************************************************
 * FILE:nusocket.h
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:远端套接字封装类
**************************************************************************************/
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <map>
#include <list>
#include "../Common/PublicCommon.h"
#include "../Utils/Log.h"
#include "../Module/ModuleLock.h"

#ifndef SOCKET
#define SOCKET	int			  
#endif // !SOCKET

#define MAX_RECV_SIZE  1024 * 10  //收到消息最大大小

//套接字类型
enum SOCKET_TYPE
{
	SOCKET_TYPE_NULL = 0,	//无
	SOCKET_TYPE_CLIENT = 1,	//客户端
	SOCKET_TYPE_SERVER = 2, //服务器
};

//连接套接字客户端状态
enum SOCKET_STATE
{
	SOCKET_STATE_NULL = 0,		//无
	SOCKET_STATE_FOR_SHAKE = 1, //连接发送握手，等待登陆
	SOCKET_STATE_CONNECTED = 2,	//连接成功正常状态
	SOCKET_STATE_DISCONNECTED = 4, //离线状态(当没有引用等时会被删除)
	SOCKET_STATE_WAIT_FINISH = 8,  //等待结束状态，没有什么可做的等待释放
};

//消息结构体
class _msgData
{
public:
	//消息状态 
	//0无用
	//1可用
	char _type;	
	//消息长度
	unsigned int _msgLength;
	//消息
	char* _data;

	//构造消息
	_msgData() {
		this->_type = 0;
		this->_msgLength = 0;
		this->_data = new char [MAX_RECV_SIZE];
	}

	//清理发送了的消息
	~_msgData() {
		this->_type = 0;
		SAFE_DELETE_ARRAY(_data);
	}
};

class NUSocket
{
	NUSocket(SOCKET sock, sockaddr_in& addr);
public:
	friend class GameServer;

	virtual ~NUSocket();
	static NUSocket* create(SOCKET, sockaddr_in&);
protected:
	virtual bool init();

	SYNTHESIZE(SOCKET_TYPE, sockettype, SocketType);		//套接字类型
	SYNTHESIZE(SOCKET_STATE, socketstate, SocketState);		//套接字状态
	SYNTHESIZE(SOCKET, fd, FD);								//套接字
	SYNTHESIZE(char*, ip, IP);								//Ip地址
	SYNTHESIZE(sockaddr_in*, sockaddr, SockAddr);			//套接字地址详情
	SYNTHESIZE(time_t, lastHeart, LastHeartTime);				//记录上一次的心跳时间
	
	SYNTHESIZE(int, sex, Sex);
	SYNTHESIZE(unsigned int, uuid, UUID);					//玩家唯一验证ID
	SYNTHESIZE(std::string, name, Name);					//玩家姓名
	SYNTHESIZE(std::string, password, PassWord);			//玩家密码
	SYNTHESIZE(char*, authCode, AuthCode);			//玩家生成的md5校验码
	SYNTHESIZE(int, coin, Coin);							//金币
	SYNTHESIZE(int, diamond, Diamond);						//钻石
	SYNTHESIZE(bool, isTempAccount, IsTempAccount);			//是不是临时账户，游客账号
	SYNTHESIZE(int, state, State);							//玩家状态 0未登录 1登陆在大厅 2游戏中
	SYNTHESIZE(int, gameID, GameID);						//GameID
	SYNTHESIZE(int, roomID, RoomID);						//RoomID
public:
	moduleTask::AALock* m_userLock;	//玩家操作锁



};


#endif //!__NUSOCKET_H__
