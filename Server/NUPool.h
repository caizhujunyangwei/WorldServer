#ifndef __NUPOOL_H__
#define __NUPOOL_H__
/**************************************************************************************
 * FILE:nupool.h
 * DATE:2017/12/20
 * AUTH:YangW
 * INTR:网络线程池实例，服务器线程池处理
**************************************************************************************/

#include "../Module/ModuleThreadPool.h"
#include "../Utils/Log.h"
#include <sys/epoll.h>
#include "./GameMgr.h"

US_NS_MODULE;


class GameServer;
class SubGameServer;

// 数据包操作类型  
enum WebSocketOpCode {
	ErrorMSG = 0xFA,							//错误
	ContinuationFrame = 0x0,                //连续帧  
	TextFrame = 0x1,						 //文本帧  
	BinaryFrame = 0x2,						 //二进制帧  
	ConnectionClose = 0x8,                  //连接关闭  
	Ping = 0x9,
	Pong = 0xA
};

//Websocket数据包数据头信息  
struct WebSocketStreamHeader {
	unsigned int header_size;               //数据包头大小  
	int mask_offset;                    //掩码偏移  
	unsigned int payload_size;              //数据大小  
	bool fin;                                               //帧标记  
	bool masked;                            //掩码  
	unsigned char opcode;                   //操作码  
	unsigned char res[3];
};

void TreadProc(const char* data, void* pthread, void* target);

class NUPool :public ModuleThreadPool
{
public:
	~NUPool();
	NUPool(const NUPool&) = delete;
	NUPool(int threadNum, const char* data, void* target);

	virtual bool init();
	virtual bool run(bool tag = true);
	virtual bool stop();

	//友元可以访问该类的私有成员
	friend void TreadProc(const char* data, void* pthread, void* target);
private:
	//struct epoll_event ev;
	GameServer* g_server;
	GameMgr* g_gameMgr;
	SubGameServer* g_subServer;  //子游戏服务器

	AALock* g_threadLock;
public:
	//升级到websocket的协议握手
	const char* wsHandshake(const char* request, char* res);
	//对发送的数据加密到websocket方式
	/*
	* 返回0 出错
	* 返回-1 数据过长
	*/
	int enCodeOutPackage(void* inMessage, int size, unsigned char *outMessage, 
		WebSocketOpCode frameType);

	//接收的数据的处理解析
	//表头处理
	//返回数据包操作类型  
	WebSocketOpCode readHeader(const char* cData, WebSocketStreamHeader* header);

	bool deCodeInPackage(const WebSocketStreamHeader& header, char* cbSrcData,
		int wSrcLen, char* cbTagData);

	//设置非阻塞
	void setnonblocking(SOCKET sock);
	
	//建立新的连接
	bool setnewconnect(SOCKET listenFD,int, _msgData* msg, char* data);

	//读取数据
	bool readSocketMsg(SOCKET fd,_msgData* msg,char* data);

	//发送数据
	bool sendSocketMsg(SOCKET fd);

	//收到服务器的消息
	bool treadServerMsg(NUSocket* server, char* recvData, _msgData* msg);
};

#endif // !__NUPOOL_H__

