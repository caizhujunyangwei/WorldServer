#ifndef __ACTION_CODE_H__
#define __ACTION_CODE_H__

enum ActionCode
{
	//服务器和小游戏服务器的消息
	HEARTBEAT_GL			= 10010,

	//end

	HEARTBEAT						= 10006,			 //封装的心跳消息10006
	Mail_LC							= 10010,			 //邮件
	USER_LOGIN_CL					= 20001,			 //玩家登陆大厅消息 20001
	USER_LOGIN_LC					= 20002,			 //大厅回复玩家信息 20002
	USER_ENTER_CL					= 20003,			 //进入大厅 20003
	ReturnHall_CL					= 20004,             //重新回到大厅20004
	FreshUserInfo_LC				= 20005,		     //刷新玩家信息20005只有在大厅时
	FreshUserGoods					= 20006,			 //刷新玩家物品信息钻石和金币等20006
};

#endif // !__ACTION_CODE_H__

