#include "GameServer.h"
#include "../ServerControl/Hall.h"
#include "../ServerControl/GameHall.pb.h"


GameServer::GameServer(unsigned short _port, unsigned int max)
	:
	fd(0),
	g_gameMgr(nullptr),
	g_subServer(nullptr),
	g_hall(nullptr), 
	serveraddr(nullptr)
{
	m_nuPool = nullptr;
	setPort(_port);
	setMaxRecvNum(max);
	setSendNum(0);
	setRecvNum(0);
	setNeedSendNum(0);
}

GameServer::~GameServer()
{
	//关闭套接字
	close(fd);
	SAFE_DELETE(this->serveraddr);
	SAFE_DELETE(g_hall);
	SAFE_DELETE(g_subServer);
}

GameServer * GameServer::create(unsigned short _port, unsigned int max)
{
	GameServer* pRet = new GameServer(_port, max);
	if (!pRet || !pRet->init())
		SAFE_DELETE(pRet);

	return pRet;
}

bool GameServer::init()
{
	RETURN_FALSE_IF((this->port < 1000) || (this->max < 1));

	epfd = epoll_create1(0);

	this->serveraddr = new sockaddr_in;
	this->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int opts;
	opts = fcntl(this->fd, F_GETFL);
	if (opts < 0) {
		Log::Error("fcntl(sock,GETFL)");
		return false;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(this->fd, F_SETFL, opts) < 0) {
		Log::Error("fcntl(sock,SETFL,opts)");
		return false;
	}

	/*int optionVal = 0;
	setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &optionVal, sizeof(optionVal));*/

	memset(this->serveraddr, 0, sizeof(sockaddr_in));
	this->serveraddr->sin_addr.s_addr = htons(INADDR_ANY);
	this->serveraddr->sin_family = AF_INET;
	this->serveraddr->sin_port = htons(this->port);

	//usleep(1000);
	int err = bind(this->fd, (sockaddr*)this->serveraddr, sizeof(sockaddr));
	if (0 != err)
	{
		Log::Error("bind server socket failed!,error[%d],%s\n",err,strerror(err));
		return false;
	}

	//初始化线程池
	m_nuPool = new NUPool(SERVER_POOL_THREAD_NUM, "服务器线程池", this);
	if (!m_nuPool)
		return false;

	//初始化大厅
	g_hall = Hall::create(this);
	if (!g_hall)
		return false;
	g_hall->regAtHall();

	//初始化子游戏大厅相关
	g_subServer = SubGameServer::create(this);
	if (!g_subServer)
		return false;
	g_subServer->regAtSubGameServer();
	
	return true;
}

bool GameServer::run(bool tag)
{
	//初始化游戏管理器
	g_gameMgr = GameMgr::getInstance();

	int r = listen(this->getListenFD(), 2);

	struct epoll_event ev;
	//设置与要处理的事件相关的文件描述符
	ev.data.fd = this->getListenFD();
	//设置要处理的事件类型
	ev.events = EPOLLIN | EPOLLET;
	//ev.events=EPOLLIN;
	//注册epoll事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, this->getListenFD(), &ev);

	m_nuPool->run();

	return true;
}

bool GameServer::stop()
{
	//Nothing to do
	return true;
}

//发送消息必须进行优化，现在有太多的new导致效率低下
void GameServer::sendMsg(int msgid,NUSocket * user, void * data, int len, bool clean)
{
	if (user && data)
	{
		//打包消息
		pb::Package pack;
		pack.set_msgid(msgid);
		pack.set_msgdata(data,len);
		int GSize = pack.ByteSize();
		unsigned char* GData = new unsigned char[GSize];
		pack.SerializeToArray(GData, GSize);

		//格式化消息
		unsigned char* msg = new unsigned char[GSize + 5];
		len = this->m_nuPool->enCodeOutPackage(GData, GSize, msg, WebSocketOpCode::BinaryFrame);

		int Index = 0;
		int hasWrite = 0;
		while (true)
		{
			Index = write(user->getFD(), msg + hasWrite, len - hasWrite);
			if (Index == -1 || Index == 0 || (hasWrite += Index) >= len)
				break;
		}
		this->sendnum += 1; //已发送+1
		SAFE_DELETE_ARRAY(msg);
		SAFE_DELETE_ARRAY(GData);

		Log::Info("send msg id :%d, user id :%d, msgLen :%d, sendLen :%d", msgid, user->getUUID(), len, hasWrite);
	}

	if (clean) {
		char* temp = (char*)data;
		SAFE_DELETE_ARRAY(temp);
	}
}

void GameServer::sendMsg(int msgid, SOCKET fd, void * data, int len, bool clean)
{
	NUSocket* user = g_gameMgr->findSocketByFD(fd);
	this->sendMsg(msgid, user, data, len, clean);
}

void GameServer::sendMsgWithOutProto(SOCKET fd, char * data, int len, bool clean)
{
	if (fd > 0 && data)
	{
		int Index = 0;
		int hasWrite = 0;
		while (true)
		{
			Index = write(fd, data + hasWrite, len - hasWrite);
			if (Index == -1 || Index == 0 || (hasWrite += Index) >= len)
				break;
		}
		this->sendnum += 1; //已发送
		Log::Info("send msg without protobuf encode,msgLen :%d, sendLen : %d",len, hasWrite);
	}

	if (clean) {
		SAFE_DELETE_ARRAY(data);
	}
}

void GameServer::sendMsgWithOutProto(NUSocket * user, char * data, int len, bool clean)
{
	this->sendMsgWithOutProto(user->getFD(), data, len, clean);
}
