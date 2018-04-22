#include "NUPool.h"
#include "./GameServer.h"
#include <sstream>
#include "../Utils/ThirdParty/SHA1.h"
#include "../Utils/ThirdParty/Base64.h"
#include "../Utils/ThirdParty/MD5.h"
#include "../ServerControl/GameHall.pb.h"
#include "../Utils/Listener/Listener.h"
#include "../ServerControl/SubGameServer.h"


NUPool::NUPool(int threadNum, const char * data, void * target)
	:ModuleThreadPool(threadNum,data,TreadProc,target)
{
	g_gameMgr = GameMgr::getInstance();
	g_server = (GameServer*)target;
	g_subServer = g_server->g_subServer;

	init();
}

NUPool::~NUPool()
{
	SAFE_DELETE(g_threadLock);
}

bool NUPool::init()
{
	RETURN_FALSE_IF(!ModuleThreadPool::init());
	//初始化线程池相关

	g_threadLock = ModuleLock::createAALock(Lock_NUPool, MUTEX_LOCK);


	Log::Info("初始化服务器线程池完成");
	return true;
}

bool NUPool::run(bool tag)
{
	Log::Info("开始服务器线程池");

	RETURN_FALSE_IF(!ModuleThreadPool::run(true));

	return true;
}

bool NUPool::stop()
{
	RETURN_FALSE_IF(!ModuleThreadPool::stop());


	return true;
}

#pragma region _WEBSOCKET_  websocket的方法处理
const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
const char * NUPool::wsHandshake(const char * request, char * res)
{
	bool ret = false;
	std::istringstream stream(request);
	std::string reqType;
	std::getline(stream, reqType);
	if (reqType.substr(0, 4) != "GET ")
	{
		if (reqType.substr(0, 6) == "Server") {
			sprintf(res, "Success");
			return "Server";
		}
		else
			return nullptr;
	}

	std::string header;
	std::string::size_type pos = 0;
	std::string websocketKey;
	while (std::getline(stream, header) && header != "\r")
	{
		header.erase(header.end() - 1);
		pos = header.find(": ", 0);
		if (pos != std::string::npos)
		{
			std::string key = header.substr(0, pos);
			std::string value = header.substr(pos + 2);
			if (key == "Sec-WebSocket-Key")
			{
				ret = true;
				websocketKey = value;
				break;
			}
		}
	}

	if (ret != true)
	{
		return nullptr;
	}

	std::string serverKey = websocketKey + magicKey;

	unsigned int iDigSet[5];
	SHA1* sha = new SHA1();
	//sha->Reset();
	(*sha) << serverKey.c_str();
	sha->Result(iDigSet);
	for (int i = 0; i < 5; i++)
		iDigSet[i] = htonl(iDigSet[i]);           //将字节转换成网络字节顺序  
	serverKey = base64_encode(reinterpret_cast<const unsigned char*>(iDigSet), 20);

	sprintf(res, "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\nUpgrade: websocket\r\n\r\n", 
		serverKey.c_str());
	delete sha;

	return res;
}

int NUPool::enCodeOutPackage(void * inMessage,int size ,unsigned char * outMessage,
	WebSocketOpCode frameType)
{
	const unsigned int  messageLength = size;
	if (messageLength > 32767)
	{
		// 暂不支持这么长的数据  
		return -1;
	}
	unsigned char payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
	// header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节 

	unsigned char frameHeaderSize = 2 + payloadFieldExtraBytes;
	unsigned char* frameHeader = new unsigned char[frameHeaderSize];
	memset(frameHeader, 0, frameHeaderSize);

	// fin位为1, 扩展位为0, 操作位为frameType  
	frameHeader[0] = static_cast<unsigned char>(0x80 | frameType);

	// 填充数据长度  
	if (messageLength <= 0x7d)
	{
		frameHeader[1] = static_cast<unsigned char>(messageLength);
	}
	else
	{
		frameHeader[1] = 0x7e;
		uint16_t len = htons(messageLength);
		memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
	}

	// 填充数据  
	uint32_t frameSize = frameHeaderSize + messageLength;
	memcpy(outMessage, frameHeader, frameHeaderSize);
	memcpy(outMessage + frameHeaderSize, inMessage, messageLength);
	//outMessage[frameSize] = '\0';

	SAFE_DELETE_ARRAY(frameHeader);

	return frameSize;
}

WebSocketOpCode NUPool::readHeader(const char * cData, WebSocketStreamHeader * header)
{
	if (cData == nullptr)
		return WebSocketOpCode::ErrorMSG;

	const char *buf = cData;

	header->fin = buf[0] & 0x80;
	header->masked = buf[1] & 0x80;
	unsigned char stream_size = buf[1] & 0x7F;

	header->opcode = buf[0] & 0x0F;
	if (header->opcode == WebSocketOpCode::ContinuationFrame) {
		//连续帧  
		return WebSocketOpCode::ContinuationFrame;
	}
	else if (header->opcode == WebSocketOpCode::TextFrame) {
		//文本帧  
		//return WebSocketOpCode::ContinuationFrame;
	}
	else if (header->opcode == WebSocketOpCode::BinaryFrame) {
		//二进制帧  
		//return WebSocketOpCode::BinaryFrame;
	}
	else if (header->opcode == WebSocketOpCode::ConnectionClose) {
		//连接关闭消息  
		return WebSocketOpCode::ConnectionClose;
	}
	else if (header->opcode == WebSocketOpCode::Ping) {
		//  ping  
		return WebSocketOpCode::Ping;
	}
	else if (header->opcode == WebSocketOpCode::Pong) {
		// pong  
		return WebSocketOpCode::Pong;
	}
	else {
		//非法帧  
		return WebSocketOpCode::ErrorMSG;
	}

	if (stream_size <= 125) {
		//  small stream  
		header->header_size = 6;
		header->payload_size = stream_size;
		header->mask_offset = 2;
	}
	else if (stream_size == 126) {
		//  medium stream   
		header->header_size = 8;
		unsigned short s = 0;
		memcpy(&s, (const char*)&buf[2], 2);
		header->payload_size = ntohs(s);
		header->mask_offset = 4;
	}
	else if (stream_size == 127) {

		unsigned long long l = 0;
		memcpy(&l, (const char*)&buf[2], 8);

		header->payload_size = l;
		header->mask_offset = 10;
	}
	else {
		//Couldnt decode stream size 非法大小数据包  
		return WebSocketOpCode::ErrorMSG;
	}

	/*if (header->payload_size >= max_recv_size) {
		return WebSocketOpCode::ErrorMSG;
	}*/

	return WebSocketOpCode(header->opcode);
}

bool NUPool::deCodeInPackage(const WebSocketStreamHeader & header, char * cbSrcData,
	int wSrcLen, char * cbTagData)
{
	const char *final_buf = cbSrcData;
	if (wSrcLen < header.header_size + 1)
	{
		return false;
	}

	char masks[4];
	memcpy(masks, final_buf + header.mask_offset, 4);
	memcpy(cbTagData, final_buf + header.mask_offset + 4, header.payload_size);

	for (int i = 0; i < header.payload_size; ++i)
	{
		cbTagData[i] = (cbTagData[i] ^ masks[i % 4]);
	}
	//如果是文本包，在数据最后加一个结束字符“\0”  
	if (header.opcode == WebSocketOpCode::TextFrame)
		cbTagData[header.payload_size] = 0;

	return true;
}
#pragma endregion _WEBSOCKET_

void NUPool::setnonblocking(SOCKET sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL);
	if (opts < 0)
	{
		Log::Error("fcntl(sock,GETFL)");
		return;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
	{
		Log::Error("fcntl(sock,SETFL,opts)");
		return;
	}
}

bool NUPool::setnewconnect(SOCKET listenFD,int epfd,_msgData* msg, char* data)
{
	struct sockaddr_in clientaddr = { 0 };
	socklen_t clilen = sizeof(clientaddr);
	SOCKET fd = accept(listenFD, (sockaddr*)&clientaddr, &clilen);
	if (fd == -1) {
		Log::Error("accept socket failed!!!");
		return false;
	}
	setnonblocking(fd);

	//int nRecvBuf = 128 * 1024;//设置为32K  
	//setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	////发送缓冲区  
	//int nSendBuf = 128 * 1024;//设置为32K  
	//setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	//int nNetTimeout = 1000;//1秒  
	////发送时限  
	//setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&nNetTimeout, sizeof(int));
	////接收时限  
	//setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&nNetTimeout, sizeof(int));

	//握手 暂时先这样处理，下来再优化
	std::this_thread::sleep_for(std::chrono::seconds(1)); //确保数据到达
	int recvNum = read(fd, data, MAX_RECV_SIZE);
	if (recvNum > 1) {
		const char* hand = wsHandshake(data, msg->_data);
		if (hand != nullptr) {
			g_server->sendMsgWithOutProto(fd, msg->_data, strlen(msg->_data), false);

			NUSocket* pRet = NUSocket::create(fd, clientaddr);
			
			if (strcmp(hand, "Server") == 0) {
				pRet->setSocketState(SOCKET_STATE_CONNECTED);
				pRet->setSocketType(SOCKET_TYPE_SERVER);
				pRet->setUUID(100);
				pRet->setName(data);
			}
			else{
				pRet->setSocketState(SOCKET_STATE_CONNECTED);
				pRet->setSocketType(SOCKET_TYPE_CLIENT);
			}

			g_gameMgr->pushNewConnect(pRet);

			struct epoll_event ev;
			ev.data.fd = fd;
			//设置用于注测的读操作事件
			ev.events = EPOLLIN | EPOLLET;
			//ev.events=EPOLLIN;

			//注册ev
			epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

			return true;
		}
		return false;
	}
	Log::Error("握手失败！！！！");
	return false;
}

bool NUPool::readSocketMsg(SOCKET fd,_msgData* msg, char* data)
{
	NUSocket* user = g_gameMgr->findSocketByFD(fd);
	if (nullptr == user)
		return false;

	msg->_msgLength = 0;
	//msg->_type &= 0;

	int recvNum = read(fd, data, MAX_RECV_SIZE);

	if (recvNum < 0) //出错了啊
	{
		if (errno == EAGAIN)
		{
			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
			// 在这里就当作是该次事件已处理.
			Log::Error("read EAGAIN");
			return false;
		}
		else if (errno == ECONNRESET)
		{
			// 对方发送了RST
			//shutdown(fd, SHUT_RD);
			g_gameMgr->pushDeleteFDS(fd);
			user->setSocketState(SOCKET_STATE_WAIT_FINISH);
			Log::Warn("socket send close");
		}
		else if (errno == EINTR)
		{
			// 被信号中断
			Log::Error("被信号中断");
			return false;
		}
		else
		{
			//其他不可弥补的错误
			//shutdown(fd, SHUT_RD);
			g_gameMgr->pushDeleteFDS(fd);
			user->setSocketState(SOCKET_STATE_DISCONNECTED);
			Log::Error("something happened");
			return false;
		}
	}
	else if (recvNum == 0)
	{
		// 这里表示对端的socket已正常关闭.发送过FIN了。
		g_gameMgr->pushDeleteFDS(fd);
		user->setSocketState(SOCKET_STATE_DISCONNECTED);
		Log::Warn("socket close success");
	}
	else {
		//如果是服务器
		if (user->getSocketType() == SOCKET_TYPE_SERVER) {
			this->treadServerMsg(user, data, msg);
		}
		else {
			//正常取到数据啊
			WebSocketStreamHeader* pheader = new WebSocketStreamHeader();
			if (readHeader(data, pheader) != WebSocketOpCode::ErrorMSG)
			{
				if (true == deCodeInPackage(*pheader, data, recvNum, msg->_data))
				{
					if (pheader->opcode == WebSocketOpCode::TextFrame)
						pheader->payload_size += 1;

					//等待proto解析消息号
					pb::Package pbmsg;
					pbmsg.ParseFromArray(msg->_data, pheader->payload_size);

					msg->_msgLength = pbmsg.msgdata().size();
					memcpy(msg->_data, pbmsg.msgdata().c_str(), msg->_msgLength);
					msg->_data[msg->_msgLength] = '\0';

					Log::Debug("recv msg id :%d, user id :%d, msgLen :%d dataLen %d", pbmsg.msgid(), user->getUUID(), msg->_msgLength, recvNum);

					if (pbmsg.msgid() < 1000)//出错了好像
					{
						g_gameMgr->pushDeleteFDS(fd);
						user->setSocketState(SOCKET_STATE_DISCONNECTED);
						Log::Error("消息出错了可能强制断线了,收到消息号是：%d", pbmsg.msgid());
					}
					else
					{
						//回调消息处理
						msg->_type |= 0x01;
						ListenerMgr::getInstance()->callListener(pbmsg.msgid(), msg, user);
					}
					//重置接收缓冲区
					memset(data, 0, MAX_RECV_SIZE);
				}
			}
		}
	}
	msg->_type &= 0; //设置消息不可再用
	return true;
}

bool NUPool::sendSocketMsg(SOCKET fd)
{
	//暂时没有处理
	return false;
}

bool NUPool::treadServerMsg(NUSocket * user, char * recvData, _msgData * msg)
{
	if (recvData == nullptr) return false;

	msg->_type &= 1;
	return g_subServer->dispathMsg(user, recvData, msg);
}


//服务器处理线程 多线程处理
//@data ：线程输入内容
//@pthread ：线程
//@target ：启动TreadProc的实例
void TreadProc(const char * data, void * _pthread, void * target)
{
	std::this_thread::sleep_for(std::chrono::seconds(1)); //确保线程运行
	Log::Debug("进入线程池，%s", data);

	ModuleThread* pthread = (ModuleThread*)_pthread;
	GameServer* server = (GameServer*)target;
	NUPool* pool = server->m_nuPool;

	struct epoll_event events[MAX_EPOLL_NUM];
	int epfd = server->getEpfd(); //服务器ev事件

	GameMgr* gameMgr = GameMgr::getInstance();

	SOCKET listenFD = server->getListenFD();

	//收到的消息
	_msgData* msgData = new _msgData();
	//临时存储消息的
	char* tempData = new char[MAX_RECV_SIZE];

	int i = 0;
	int size = 0;
	while (pthread->getRunFlag())
	{
		pool->g_threadLock->m_mutex->lock();
		size = 0;
		size = epoll_wait(epfd, events, MAX_EPOLL_NUM,-1);
		Log::Debug("recv epool events ===================:%d",size);
		pool->g_threadLock->m_mutex->unlock();

		//循环取出所有的events处理
		for (i=0;i < size;++i)
		{
			//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
			if (events[i].data.fd == listenFD)
			{
				if (!pool->setnewconnect(listenFD, epfd, msgData,tempData))
				{
					Log::Error("new connect socket failed");
				}
			}
			//如果是已经连接的用户
			else
			{
				if (events[i].events & EPOLLIN) //收到数据，那么进行读入
				{
					if (events[i].data.fd < 0)
					{
						Log::Error("recv data socket error");
						continue;
					}
					if (!pool->readSocketMsg(events[i].data.fd, msgData, tempData))
					{
						Log::Error("read msg failed");
					}
					//接收消息+1 可能会不准，多线程不安全
					server->setRecvNum(server->getRecvNum() + 1);
				}
				//if (events[i].events & EPOLLOUT)	// 如果有数据发送 暂时没有用的到
				//{
				//	if (events[i].data.fd < 0)
				//	{
				//		Log::Error("send data socket error");
				//		continue;
				//	}
				//	if (!pool->sendSocketMsg(events[i].data.fd))
				//	{
				//		Log::Error("send msg failed");
				//	}
				//}
			}
		}

		memset(tempData, 0, MAX_RECV_SIZE);
	}

	Log::Debug("退出线程池，%s", data);
	SAFE_DELETE_ARRAY(tempData);
	SAFE_DELETE(msgData);
}
