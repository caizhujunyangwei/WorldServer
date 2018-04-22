#ifndef __HALL_H__
#define __HALL_H__
/**************************************************************************************
 * FILE:hall.h
 * DATE:2017/12/29
 * AUTH:YangW
 * INTR:玩家在大厅的所有相关处理
**************************************************************************************/

#include "./GameHall.pb.h"
#include "../Server/GameServer.h"
#include "../Utils/Listener/Listener.h"
#include "../Utils/ThirdParty/cJSON.h"

size_t getUserInfo(void* data, size_t i1, size_t i2, void* pNode); //网页抓取函数回调
size_t getUserInfo2(void* data, size_t i1, size_t i2, void* pNode); //网页抓取函数回调

class Hall
{
	Hall();
private:
	GameMgr* g_gameMgr;

	ListenerMgr* g_listener;
	GameServer* g_server;

	bool init(GameServer*);

public:
	~Hall();
	static Hall* create(GameServer*);

	//大厅所有监听消息注册
	void regAtHall();

	//发送邮件给谁
	void sendMailToUser(NUSocket*nb, cJSON* data);

private:
	void getRoomInfo(NUSocket* nb); //玩家进入大厅检测有没有遗留的房间信息
protected:
#pragma region 大厅消息处理
	friend size_t getUserInfo(void* data, size_t i1, size_t i2, void* pNode);
	friend size_t getUserInfo2(void* data, size_t i1, size_t i2, void* pNode);

	void heartBeat(_msgData* data, NUSocket* nb);  //心跳消息 10006
	void userLogin(_msgData* data, NUSocket* nb);  //玩家登陆 20001
	void userEnterHall(_msgData* data, NUSocket* nb);//玩家进入大厅 20010
	void returnGame(_msgData* data, NUSocket* nb); //断线刷新(切后台回来请求刷新数据)30001
	
#pragma endregion 大厅消息处理

};

#endif // !__HALL_H__

