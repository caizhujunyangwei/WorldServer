#ifndef __PUBLIC_COMMON_H__
#define __PUBLIC_COMMON_H__
/**************************************************************************************
 * FILE:publiccommon.h
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:程序中常用的一些定义，声明
**************************************************************************************/

const char WebServerUrl[] = "172.18.87.141:6011";

#define LISTEN_PORT							6012		//监听端口
#define MAX_RECV_NUM						10000		//最大支持连接人数
#define SERVER_POOL_THREAD_NUM				2			//服务器线程池线程数量

//锁定义
//锁的值最好不要为正数，免得和玩家的锁重合
#define Lock_Log			0
#define Lock_SOCKETS		-1	
#define Lock_NUPool			-2
#define Lock_Delete_SOCKETS -3

//delete删除地址
#define SAFE_DELETE(_TYPE_)															\
do{																					\
	if (_TYPE_){																	\
		delete _TYPE_;																\
		_TYPE_ = nullptr;															\
	}																				\
}while(_TYPE_);																			

//delet删除组
#define SAFE_DELETE_ARRAY(_TYPE_)													\
do{																					\
	if (_TYPE_){																	\
		delete[] _TYPE_;															\
		_TYPE_ = nullptr;															\
	}																				\
}while(_TYPE_);		

//工厂宏
#define CREATE_FUNC(_TYPE_)															\
static _TYPE_* create(){															\
	_TYPE_* pRet = new (std::nothrow) _TYPE_();										\
	if(pRet == nullptr || !pRet->init()){											\
		delete pRet;																\
		pRet = nullptr;																\
	}																				\
	return pRet;																	\
}																		

//生成变量的get，set方法
#define SYNTHESIZE(varType, varName, funName)										\
protected: varType varName;															\
public: virtual varType get##funName(void) const { return varName; }				\
public: virtual void set##funName(varType var) { varName = var; }	

//条件真返回false
#define RETURN_FALSE_IF(_Condition)													\
{																					\
	if(_Condition)																	\
		return false;																\
}

#endif // !__PUBLIC_COMMON_H__

