#include "./Hall.h"
#include "./ActionCode.h"
#include "./GameHall.pb.h"
#include "../Utils/ThirdParty/MD5.h"
#include "../Utils/XMLHttpRequest.h"

GameServer* gserver = nullptr;

Hall::Hall()
{
}

Hall::~Hall()
{
}


bool Hall::init(GameServer* server)
{
	g_listener = ListenerMgr::getInstance();
	g_server = server;
	gserver = server;
	g_gameMgr = GameMgr::getInstance();
	Log::Info("大厅控制脚本启动...");
	return true;
}

Hall* Hall::create(GameServer* server)
{
	Hall* pRet = new Hall;
	if (!pRet || !pRet->init(server))
	{
		SAFE_DELETE(pRet);
	}

	return pRet;
}

void Hall::regAtHall()
{
	register_class_listener(ActionCode::HEARTBEAT, &Hall::heartBeat, this);
	register_class_listener(ActionCode::USER_LOGIN_CL, &Hall::userLogin, this);
	register_class_listener(ActionCode::USER_ENTER_CL, &Hall::userEnterHall, this);
	register_class_listener(ActionCode::ReturnHall_CL, &Hall::returnGame, this);

}

void Hall::sendMailToUser(NUSocket * nb, cJSON * data)
{
	if (!data) return;

	pb::Mail_LC mail;
	cJSON* item = cJSON_GetObjectItem(data, "fromName");
	if (item) {
		mail.set_fromname(item->valuestring);
		Log::Warn(item->valuestring);
	}
	item = nullptr;
	item = cJSON_GetObjectItem(data, "detail");
	if (item) {
		mail.set_detail(item->valuestring);
		Log::Warn(item->valuestring);
	}
	item = nullptr;
	item = cJSON_GetObjectItem(data, "time");
	if (item) {
		mail.set_time(item->valuestring);
		Log::Warn(item->valuestring);
	}
	
	int GWInfoSize = mail.ByteSize();
	char* GWInfoData = new char[GWInfoSize];
	mail.SerializeToArray(GWInfoData, GWInfoSize);
	g_server->sendMsg(ActionCode::Mail_LC, nb, GWInfoData, GWInfoSize);	
}

//心跳10006
void Hall::heartBeat(_msgData * data, NUSocket * nb)
{
	if (data->_type == 0)
		return;
	pb::HeatBeat msg;
	msg.ParseFromArray(data->_data,data->_msgLength);

	//Log::Warn("收到心跳----->>>>>>>%d", msg.second());
	//更新心跳接收时间
	nb->setLastHeartTime(ModuleTime::getLocalTimeNumber());
	//回复心跳
	
	int GSize = msg.ByteSize();
	char* GData = new char[GSize];
	msg.SerializeToArray(GData, GSize);
	g_server->sendMsg(ActionCode::HEARTBEAT,nb,GData,GSize);
}

size_t getUserInfo(void* data, size_t i1, size_t i2, void* pNode)
{
	if (!data) return 0;

	NUSocket* user = static_cast<NUSocket*>(pNode);
	char* msgData = (char*)data;

	cJSON* root = nullptr, *item = nullptr;
	root = cJSON_Parse(msgData);
	Log::Info("登陆玩家信息---%s", cJSON_Print(root));

	//pb::UserLogin_LC temp;
	pb::UserInfo * temp = new pb::UserInfo;
	if(root)
		item = cJSON_GetObjectItem(root, "errcode");

	if (item && item->valueint == 1)
	{
		item = cJSON_GetObjectItem(root, "uuid");
		user->setUUID(item->valueint);
		temp->set_id(user->getUUID());

		item = cJSON_GetObjectItem(root, "name");
		user->setName(item->valuestring);
		temp->set_name(user->getName());

		item = cJSON_GetObjectItem(root, "sex");
		user->setSex(item->valueint);
		temp->set_sex(user->getSex());

		item = cJSON_GetObjectItem(root, "diamond");
		user->setDiamond(item->valueint);
		temp->set_diamond(user->getDiamond());

		item = cJSON_GetObjectItem(root, "coins");
		user->setCoin(item->valueint);
		temp->set_coin(user->getCoin());

		item = cJSON_GetObjectItem(root, "tempAccount");
		user->setIsTempAccount((item->valueint != 0) ? true : false);
		temp->set_istemp(user->getIsTempAccount());

		temp->set_headurl("http://img.duoziwang.com/2016/11/26/06553333315.jpg");
	}
	else {
		temp->set_id(0);  //获取信息出错了
		char* code = user->getAuthCode();
		SAFE_DELETE_ARRAY(code);
		user->setAuthCode(nullptr);
	}

	pb::UserLogin_LC userLogin_LC;
	userLogin_LC.set_allocated_userinfo(temp);

	int GWInfoSize = userLogin_LC.ByteSize();
	char* GWInfoData = new char[GWInfoSize];
	userLogin_LC.SerializeToArray(GWInfoData, GWInfoSize);
	gserver->sendMsg(ActionCode::USER_LOGIN_LC,user, GWInfoData, GWInfoSize);

	cJSON_Delete(root);
	return i1 * i2;
}

//玩家登陆20001
void Hall::userLogin(_msgData * data, NUSocket * nb)
{
	if (data->_type == 0) return;
	pb::UserLogin_CL msg;
	msg.ParseFromArray(data->_data, data->_msgLength);

	if (!g_gameMgr->checkAuthCode(msg.token().c_str()))
	{
		//玩家已经登录了
		Log::Error("User AuthCode Error,A User is Login!");
		pb::UserInfo* temp = new pb::UserInfo;
		temp->set_id(0);
		pb::UserLogin_LC userLogin_LC;
		userLogin_LC.set_allocated_userinfo(temp);

		int GWInfoSize = userLogin_LC.ByteSize();
		char* GWInfoData = new char[GWInfoSize];
		userLogin_LC.SerializeToArray(GWInfoData, GWInfoSize);
		g_server->sendMsg(ActionCode::USER_LOGIN_LC, nb, GWInfoData, GWInfoSize);
		return;
	}
	char* code = new char[33];
	memcpy(code, msg.token().c_str(), 32);
	code[32] = 0;
	nb->setAuthCode(code);

	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;

	std::string url(WebServerUrl);
	url += "/userinfo?token=";
	url += msg.token();
	Log::Info("real userLogin URL is :%s ", url.c_str());
	
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0.5);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getUserInfo);
		//设置回调函数的参数，见getUserInfo函数的最后一个参数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, nb);

		//执行单条请求  
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			Log::Error("send httpRequest %s failed [%s]", url.c_str(), curl_easy_strerror(res));
		}
		curl_easy_cleanup(curl);
	}
	curl = nullptr;
	res = CURLE_OK;
}

//玩家进入大厅标识20003
void Hall::userEnterHall(_msgData * data, NUSocket * nb)
{
	Log::Info("玩家 %d 进入大厅----", nb->getUUID());
	nb->setState(1);
	getRoomInfo(nb);

	if (nb->getIsTempAccount()) {
		//临时账户，发送邮件提醒玩家
		cJSON *root = cJSON_CreateObject();
		tm t = ModuleTime::getLocalTime();
		
		cJSON_AddItemToObject(root, "time", cJSON_CreateString(ModuleTime::convert2ASCTime(t)));
		cJSON_AddItemToObject(root, "fromName", cJSON_CreateString("系统"));
		char str[300] = {};
		sprintf(str, "尊敬的玩家%s,您还是临时账户,请尽快绑定账户,防止账号遗失。", nb->getName().c_str());
		cJSON_AddItemToObject(root, "detail", cJSON_CreateString(str));
		
		sendMailToUser(nb, root);
		cJSON_Delete(root);
	}
}

size_t getUserInfo2(void* data, size_t i1, size_t i2, void* pNode)
{
	if (!data) return 0;

	NUSocket* user = static_cast<NUSocket*>(pNode);
	char* msgData = (char*)data;

	cJSON* root = nullptr, *item = nullptr;
	root = cJSON_Parse(msgData);
	Log::Info("getUserInfo2 登陆玩家信息---%s", cJSON_Print(root));

	//pb::UserLogin_LC temp;
	pb::UserInfo * temp = new pb::UserInfo;
	if (root)
		item = cJSON_GetObjectItem(root, "errcode");

	if (item && item->valueint == 1)
	{
		item = cJSON_GetObjectItem(root, "uuid");
		user->setUUID(item->valueint);
		temp->set_id(user->getUUID());

		item = cJSON_GetObjectItem(root, "name");
		user->setName(item->valuestring);
		temp->set_name(user->getName());

		item = cJSON_GetObjectItem(root, "sex");
		user->setSex(item->valueint);
		temp->set_sex(user->getSex());

		item = cJSON_GetObjectItem(root, "diamond");
		user->setDiamond(item->valueint);
		temp->set_diamond(user->getDiamond());

		item = cJSON_GetObjectItem(root, "coins");
		user->setCoin(item->valueint);
		temp->set_coin(user->getCoin());

		item = cJSON_GetObjectItem(root, "tempAccount");
		user->setIsTempAccount((item->valueint != 0) ? true : false);
		temp->set_istemp(user->getIsTempAccount());

		temp->set_headurl("http://img.duoziwang.com/2016/11/26/06553333315.jpg");
	}
	else {
		temp->set_id(0);  //获取信息出错了
		char* code = user->getAuthCode();
		if (code)
		{
			SAFE_DELETE_ARRAY(code);
		}
		user->setAuthCode(nullptr);
	}

	pb::FreshUserInfo_LC userLogin_LC;
	userLogin_LC.set_allocated_userinfo(temp);

	int GWInfoSize = userLogin_LC.ByteSize();
	char* GWInfoData = new char[GWInfoSize];
	userLogin_LC.SerializeToArray(GWInfoData, GWInfoSize);
	gserver->sendMsg(ActionCode::FreshUserInfo_LC, user, GWInfoData, GWInfoSize);

	cJSON_Delete(root);
	return i1 * i2;
}

//断线刷新(切后台回来请求刷新数据)20004
void Hall::returnGame(_msgData* data, NUSocket* nb)
{
	nb->setState(1);
	if (data->_type == 0) return;
	pb::ReturnHall_CL msg;
	msg.ParseFromArray(data->_data, data->_msgLength);

	if (g_gameMgr->checkAuthCode(msg.token().c_str())) //玩家已经不在了
	{
		CURL* curl = nullptr;
		CURLcode res = CURLE_OK;

		if (nb->getAuthCode())
		{
			char* code1 = nb->getAuthCode();
			SAFE_DELETE_ARRAY(code1);
		}

		char* code = new char[33];
		memcpy(code, msg.token().c_str(), 32);
		code[32] = 0;
		nb->setAuthCode(code);

		std::string url(WebServerUrl);
		url += "/userinfo?token=";
		url += msg.token();
		Log::Info("real returnGame URL is :%s ", url.c_str());

		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0.5);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getUserInfo2);
			//设置回调函数的参数，见getUserInfo函数的最后一个参数
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, nb);

			//执行单条请求  
			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				Log::Error("returnGame send httpRequest %s failed [%s]", url.c_str(), curl_easy_strerror(res));
			}
			curl_easy_cleanup(curl);
		}
		curl = nullptr;
		res = CURLE_OK;
	}
	else
	{
		pb::UserInfo * temp = new pb::UserInfo;
		temp->set_id(nb->getUUID());
		temp->set_name(nb->getName());
		temp->set_sex(nb->getSex());
		temp->set_diamond(nb->getDiamond());
		temp->set_coin(nb->getCoin());
		temp->set_istemp(nb->getIsTempAccount());
		temp->set_headurl("http://img.duoziwang.com/2016/11/26/06553333315.jpg");

		pb::FreshUserInfo_LC userLogin_LC;
		userLogin_LC.set_allocated_userinfo(temp);

		int GWInfoSize = userLogin_LC.ByteSize();
		char* GWInfoData = new char[GWInfoSize];
		userLogin_LC.SerializeToArray(GWInfoData, GWInfoSize);
		gserver->sendMsg(ActionCode::FreshUserInfo_LC, nb, GWInfoData, GWInfoSize);
	}
}

void Hall::getRoomInfo(NUSocket * nb)
{
	if (!nb)
		return;

	std::string url(WebServerUrl);
	url += "/userinfo?token=";
	url += nb->getAuthCode();
	
	HttpRequest* hr = new HttpRequest();
	hr->setTargetData(nb);
	hr->open("GET", url);

	hr->onreadystatechange = [this](void* pRet,size_t size) {
		HttpRequest* http = static_cast<HttpRequest*>(pRet);
		if (http->m_readystate == ReadyState::GET && http->m_responseDataSize > 0) {
			Log::Info("get user subgame roominfo: %s", http->m_responseData);
		}
	};
	
	hr->send();
	SAFE_DELETE(hr);
}
