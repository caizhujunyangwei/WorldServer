#include "./Server/GameServer.h"
#include "./Utils/ThirdParty/MD5.h"
#include <curl/curl.h>
/**********************************************************************************************
 * FILE:main.cpp
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:[1]Linux环境中地址由8个字节来存储的
**********************************************************************************************/
US_NS_MODULE;
GameServer* g_server = nullptr;

int main(int argc, char **argv)
{
	struct timeval start, end, diff;
	ModuleTime::getDiffTimeStart(start);

	struct tm t = ModuleTime::getLocalTime();
	ModuleLock::getInstance();
	Log::Info("开启服务器程序:%s",ModuleTime::convert2ASCTime(t));

	curl_global_init(CURL_GLOBAL_DEFAULT);
	Log::Info("http Info -----------: %s", curl_version());

	
	g_server = GameServer::create(LISTEN_PORT, MAX_RECV_NUM);
	if (g_server)
	{
		g_server->run();

		GameMgr* gameMgr = GameMgr::getInstance();

		char timestr[16] = {};
		int trigger = 0;
		while (true)
		{
			//清理一下
			gameMgr->clearDeleteFDs();
			
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			trigger += 500;
			if (++trigger >= 25000) {
				trigger = 0;
				int i = ModuleTime::getDiffTimeEnd(start, end, diff);
				Log::Info("\
/**************************************\n\
* 服务器运行时间总共: %s\n\
* **已发送%d**\\**已接收%d**\n\
* 总人数:%d\n\
*****************************************************************/",
ModuleTime::convert2TimeStr(diff, timestr),
g_server->getSendNum(), g_server->getRecvNum(),
gameMgr->getAllConnect()
);
				memset(timestr, 0, 16);
			}
		}
	}

	ModuleTime::getDiffTimeEnd(start, end, diff);
	Log::Error("服务器运行时间总共: %d (s),%ld (ms)", diff.tv_sec, diff.tv_usec);
	curl_global_cleanup();
	SAFE_DELETE(g_server);

	return 0;
}

