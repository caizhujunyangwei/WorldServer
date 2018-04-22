#include "./ActionCode.h"
#include "./SubGameServer.h"
#include "../Server/GameServer.h"
#include "../Utils/Log.h"
#include "../Module/ModuleTime.h"
US_NS_MODULE;

/*
* 小游戏消息格式  消息内容中插入msgid
* {msgid:id,...}
*/
SubGameServer::SubGameServer()
	:
	g_server(nullptr)
{
}

SubGameServer::~SubGameServer()
{
}

SubGameServer * SubGameServer::create(GameServer * server)
{
	SubGameServer* pRet = new SubGameServer();
	if (!pRet || !pRet->init(server))
	{
		SAFE_DELETE(pRet);
		pRet = nullptr;
	}
	return pRet;
}

bool SubGameServer::init(GameServer * server)
{
	if(server == nullptr)
		return false;

	g_server = server;
	g_listenMgr = ListenerMgr::getInstance();

	Log::Info("初始化子游戏服务器控制中心!!");
	return true;
}

bool SubGameServer::regAtSubGameServer()
{
	register_class_listener2(ActionCode::HEARTBEAT_GL, &SubGameServer::heartBeat, this);

	return true;
}

bool SubGameServer::dispathMsg(NUSocket * user, char * recvData, _msgData * msg)
{
	cJSON* root ,*item= nullptr;
	root = cJSON_Parse(recvData);
	item = cJSON_GetObjectItem(root, "msgid");
	if (item)
	{
		Log::Debug("recv Server [%s] msg msgid ：%d msgLen : %d ", user->getName().c_str(), item->valueint, strlen(recvData));

		ListenerMgr::getInstance()->callListener2(item->valueint, root, user);
	}
	cJSON_Delete(root);
	return false;
}

//收到心跳
void SubGameServer::heartBeat(cJSON * msg, NUSocket* user)
{
	cJSON* item = cJSON_GetObjectItem(msg, "time");

	user->setLastHeartTime(ModuleTime::getLocalTimeNumber());

	cJSON* root;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "time", cJSON_CreateNumber(item->valueint));
	cJSON_AddItemToObject(root, "msgid", cJSON_CreateNumber(ActionCode::HEARTBEAT_GL));
	char* data = cJSON_Print(root);
	g_server->sendMsgWithOutProto(user, data, strlen(data), false);

	cJSON_Delete(root);
}
